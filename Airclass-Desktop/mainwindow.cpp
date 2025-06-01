#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QPushButton>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QHBoxLayout>
#include <QTextStream>
#include <QFile>
#include <QScrollArea>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QDateTime>
#include <QSettings>
#include <QColorDialog>
#include <QComboBox>
#include <QSlider>
#include "logindialog.h"
#include "sessiondialog.h"
#include <QScrollBar>
#include <QPdfPageNavigator>
#include "gestureguide.h"
// Include UI header in implementation file, not in header
#include "ui_mainwindow.h"
#include "logindialog.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QAudioOutput>
#include <QSpinBox>
#include <QListWidgetItem>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QAudioOutput>
#include <QSpinBox>
#include <QListWidgetItem>

MainWindow::MainWindow(RestApiClient* restApi, WebSocketClient* wsClient, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_webSocketClient(wsClient),
    m_presentationManager(new PresentationManager(this)),
    m_attendanceManager(new AttendanceManager(this)),
    m_gestureProcessor(new GestureProcessor(this)),
    m_uiController(new UIController(this)),
    m_restApi(restApi),
    m_timer(new QTimer(this)),
    m_isTimerRunning(false),
    m_remainingSeconds(0),
    m_notificationPlayer(new QMediaPlayer(this)),
    m_drawingLayer(nullptr),
    m_breakTimer(new QTimer(this))
    , m_breakTimerLabel(new QLabel(this))
    , m_qrNetworkManager(new QNetworkAccessManager(this)) // EKLENDİ
{
    ui->setupUi(this);


    // Connect close session button
    connect(ui->closeSessionButton, &QPushButton::clicked, this, [this]() {
        if (!m_sessionId.isEmpty()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, 
                "Close Session",
                "Are you sure you want to close the current session?",
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                // Send request to close session
                QJsonObject data;
                data["id"] = m_sessionId;
                data["status"] = false;
                m_restApi->sendRequest("classroom", "PUT", data);
            }
        }
    });

    // Clear existing menu items first
    ui->menuView->clear();
    ui->menuView->addAction(ui->actionDashboard);
    ui->menuView->addAction(ui->actionPresentation);
    ui->menuView->addAction(ui->actionStudents);
    ui->menuView->addAction(ui->actionGenerate_QR_Code);

    ui->menuFile->menuAction()->setVisible(false);
    ui->menuConnection->menuAction()->setVisible(false);
    ui->menuView->menuAction()->setVisible(false);

    if (ui->serverGroupBox) ui->serverGroupBox->setVisible(false);
    if (ui->classInfoGroupBox) ui->classInfoGroupBox->setVisible(false);

    if (ui->stackedWidget && ui->dashboardView)
        ui->stackedWidget->setCurrentWidget(ui->dashboardView);

    // Connect to WebSocket server using broadcast-discovered IP and port
    if (!wsClient->serverUrl().isEmpty()) {
        m_webSocketClient->connectToServer(wsClient->serverUrl());
    } else {
        qDebug() << "[MainWindow] Broadcast IP not available, skipping WebSocket connection";
    }

    connect(m_webSocketClient, &WebSocketClient::gestureReceived,
            this, [this](const QString &gestureType, const QString &clientId, const QString &timestamp) {
                qDebug() << "Gesture received in MainWindow:" << gestureType;
                QJsonObject data;
                data["gesture_type"] = gestureType;
                data["client_id"] = clientId;
                data["timestamp"] = timestamp;
                handleGestureCommand(data);
            });

    connect(m_webSocketClient, &WebSocketClient::connected, this, [this]() {
        qDebug() << "WebSocket connected";
        if (ui->connectionStatusLabel) {
            // Optionally update label
        }
    });

    connect(m_webSocketClient, &WebSocketClient::disconnected, this, [this]() {
        qDebug() << "WebSocket disconnected";
        if (ui && ui->connectionStatusLabel) {
            // Optionally update label
        }
    });

    connect(m_restApi, &RestApiClient::attendanceCodeGenerated, this, &MainWindow::onAttendanceCodeReady);
    connect(m_restApi, &RestApiClient::attendanceCodeGenerationFailed, this, &MainWindow::onAttendanceCodeFailed);

    // QR Network Manager için sinyal bağlantısı
    connect(m_qrNetworkManager, &QNetworkAccessManager::finished, this, &MainWindow::onQrCodeImageDownloaded);
    

    // m_notificationPlayer = new QMediaPlayer(this);
    QAudioOutput *audioOutput = new QAudioOutput(this);
    m_notificationPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.7f);

    if (ui->notificationWidget) ui->notificationWidget->setVisible(false);

    m_currentRoomId = "48";

    QPushButton *logoutButton = new QPushButton("Log out", ui->topBar);
    logoutButton->setObjectName("logoutButton");
    logoutButton->setStyleSheet("QPushButton { color: #e74c3c; font-weight: bold; background: transparent; border: 1px solid #e74c3c; border-radius: 6px; padding: 6px 16px; } QPushButton:hover { background: #fbeee6; }");
    logoutButton->setVisible(false);
    ui->horizontalLayout->addWidget(logoutButton);

    connect(logoutButton, &QPushButton::clicked, this, [this, logoutButton]() {
        ui->menuFile->menuAction()->setVisible(false);
        ui->menuConnection->menuAction()->setVisible(false);
        ui->menuView->menuAction()->setVisible(false);
        logoutButton->setVisible(false);

        m_authToken.clear();
        m_restApi->setAuthToken("");

        showLoginDialog();
    });

    setupRestApiConnections();

    connect(ui->recentFilesList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        QString filePath = item->data(Qt::UserRole).toString();
        if (!filePath.isEmpty()) {
            bool loaded = m_presentationManager->loadPresentation(filePath);
            if (loaded && ui->stackedWidget) {
                ui->stackedWidget->setCurrentIndex(1);
            }
        }
    });

    showLoginDialog();

    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    connect(ui->timerStartButton, &QPushButton::clicked, this, &MainWindow::on_timerStartButton_clicked);
    connect(ui->timerStopButton, &QPushButton::clicked, this, &MainWindow::on_timerStopButton_clicked);
    connect(ui->timerResetButton, &QPushButton::clicked, this, &MainWindow::on_timerResetButton_clicked);
    connect(ui->timerMinutesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_timerMinutesSpinBox_valueChanged);
    connect(ui->timerSecondsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_timerSecondsSpinBox_valueChanged);

    ui->timerStopButton->setEnabled(false);
    ui->timerMinutesSpinBox->setRange(0, 59);
    ui->timerSecondsSpinBox->setRange(0, 59);
    updateTimerDisplay();
}



// Update showLoginDialog method in mainwindow.cpp
void MainWindow::showLoginDialog()
{
    // Clear menu items before showing login dialog
    ui->menuView->clear();

    // Add menu items back
    ui->menuView->addAction(ui->actionDashboard);
    ui->menuView->addAction(ui->actionPresentation);
    ui->menuView->addAction(ui->actionStudents);
    ui->menuView->addAction(ui->actionGenerate_QR_Code);

    // Hide all menus
    ui->menuFile->menuAction()->setVisible(false);
    ui->menuConnection->menuAction()->setVisible(false);
    ui->menuView->menuAction()->setVisible(false);

    // Create and show login dialog
    LoginDialog loginDialog(m_restApi, this);

    // // Sinyal bağlantısı: Sadece username'i almak için
    // connect(&loginDialog, QOverload<const QString &>::of(&LoginDialog::loginSuccessful),
    //         this, [this](const QString &username) {
    //     m_userName = username; // MainWindow'daki kullanıcı adını set et
    //     qDebug() << "LoginDialog::loginSuccessful signal received for user (in MainWindow):" << m_userName;
    //     // Token zaten RestApiClient içinde set edilmiş olmalı.
    //     // Burada showSessionDialog ÇAĞIRMAYIN, exec() dönüşünü bekleyin.
    // });


    // Eskisini silin/yorumlayın: connect(&loginDialog, QOverload<const QString &>::of(&LoginDialog::loginSuccessful), ... )
    connect(&loginDialog, QOverload<const QString &, const QString &>::of(&LoginDialog::loginSuccessful),
            this, [this](const QString &username, const QString &token) {
        m_userName = username;
        qDebug() << "LoginDialog::loginSuccessful signal received in MainWindow. User:" << username << "Token:" << token.left(15) << "...";

        // Token'ı RestApiClient'e set et. Bu, RestApiClient'in kendi içinde zaten
        // yaptığı setAuthToken işlemini tekrar etse de, akışın doğru olduğunu garantiler.
        // En önemlisi, MainWindow artık RestApiClient'in token'ını boş bir string ile ezmiyor.
        m_restApi->setAuthToken(token);
        qDebug() << "Token set in RestApiClient from MainWindow. Current token:" << m_restApi->getAuthToken().left(15) << "...";
    });

    // Hide main window while showing login dialog
    this->hide();

    int loginResult = loginDialog.exec(); // Login penceresini modal olarak çalıştır ve sonucu al
    qDebug() << "LoginDialog.exec() returned:" << loginResult;

    if (loginResult == QDialog::Accepted) {
        // Login başarılı ve LoginDialog kapandı (accept() çağrıldı).
        // Şimdi SessionDialog'u göster. m_userName zaten set edilmiş durumda.
        qDebug() << "Login was accepted. Preparing to show SessionDialog for user:" << m_userName;
        if (m_restApi) { // m_restApi'nin null olmadığından emin ol
            qDebug() << "Current token in RestApiClient before showing SessionDialog: " << m_restApi->getAuthToken();
        }
        showSessionDialog();
    } else {
        // LoginDialog iptal edildi veya başka bir nedenle kapatıldı.
        qDebug() << "LoginDialog was not accepted. Closing application.";
        QTimer::singleShot(0, this, &QWidget::close); // Uygulamayı kapat
    }

    // Set initial timer display
    updateTimerDisplay();

    // Connect WebSocket drawing signals to DrawingLayer
    connect(m_webSocketClient, &WebSocketClient::drawingReceived,
            this, [this](double x, double y, bool isStart, const QString &color, int width) {
                QPointF point(x, y);
                m_drawingLayer->setPenColor(QColor(color));
                m_drawingLayer->setPenWidth(width);
                m_drawingLayer->drawRemotePoint(point, isStart);
            });
}

// Add new method to show session dialog
void MainWindow::showSessionDialog()
{
    qDebug() << "Showing session dialog for user:" << m_userName;
    SessionDialog sessionDialog(m_restApi, this);

    connect(&sessionDialog, &SessionDialog::sessionSelected,
            this, [this](const QString &sessionId, const QString &sessionName, const QString &sessionCode) {
        qDebug() << "Session selected:" << sessionName << "Code:" << sessionCode;

        // Update current room/session info
        m_currentRoomId = sessionId; // Use session code as room ID
        // m_currentRoomId = sessionCode; // Use session code as room ID
        m_sessionId = sessionId;
        m_sessionName = sessionName;
        m_sessionCode = sessionCode;

        // Update window title and UI
        updateSessionInfo(sessionId, sessionName, sessionCode);

        // Show all menus after successful session selection
        ui->menuFile->menuAction()->setVisible(true);
        ui->menuConnection->menuAction()->setVisible(true);
        ui->menuView->menuAction()->setVisible(true);

        // Show logout button
        if (QWidget *logoutButton = findChild<QWidget*>("logoutButton")) {
            logoutButton->setVisible(true);
        }

        // Connect to WebSocket server
        m_webSocketClient->connectToServer("ws://localhost:8082");

        // Show the main window
        this->show();

        // Initialize the UI and setup connections
        initialize();
        setupConnections();

        // Just get requests once instead of continuous polling
        m_restApi->listRequests(m_currentRoomId);
    });

    // Connect session creation signals
    connect(m_restApi, &RestApiClient::sessionCreated,
            this, [this](const QJsonObject &sessionData) {
        QString sessionId = QString::number(sessionData["id"].toInt());
        QString sessionName = sessionData["name"].toString();
        QString sessionCode = sessionData["code"].toString();

        // Update current room/session info
        m_currentRoomId = sessionCode;
        m_sessionId = sessionId;
        m_sessionName = sessionName;
        m_sessionCode = sessionCode;

        // Update window title and UI
        updateSessionInfo(sessionId, sessionName, sessionCode);

        // Show all menus
        ui->menuFile->menuAction()->setVisible(true);
        ui->menuConnection->menuAction()->setVisible(true);
        ui->menuView->menuAction()->setVisible(true);

        // Show logout button
        if (QWidget *logoutButton = findChild<QWidget*>("logoutButton")) {
            logoutButton->setVisible(true);
        }

        // Connect to WebSocket server
        m_webSocketClient->connectToServer("ws://localhost:8082");

        // Show the main window
        this->show();

        // Initialize the UI and setup connections
        initialize();
        setupConnections();

        // Just get requests once instead of continuous polling
        m_restApi->listRequests(m_currentRoomId);
    });

    // Show session dialog
    if (sessionDialog.exec() != QDialog::Accepted) {
        // If session selection was cancelled, go back to login
        showLoginDialog();
    }
}

void MainWindow::setupRestApiConnections()
{
    // Connect REST API signals
    // connect(m_restApi, &RestApiClient::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(m_restApi, &RestApiClient::loginFailed, this, &MainWindow::onLoginFailed);
    connect(m_restApi, &RestApiClient::attendanceListReceived, this, &MainWindow::onAttendanceListReceived);
    // connect(m_restApi, &RestApiClient::attendanceCodeGenerated, this, &MainWindow::onAttendanceCodeGenerated);
    connect(m_restApi, &RestApiClient::requestsReceived, this, &MainWindow::onRequestsReceived);
    connect(m_restApi, &RestApiClient::newSpeakRequest, this, &MainWindow::onNewSpeakRequest);
    connect(m_restApi, &RestApiClient::requestUpdated, this, &MainWindow::onRequestUpdated);
    connect(m_restApi, &RestApiClient::sessionClosed, this, &MainWindow::onSessionClosed);
    connect(m_restApi, &RestApiClient::qrCodeReceived, this, &MainWindow::onQrCodeReceived);
    // Connect close session button
    connect(ui->closeSessionButton, &QPushButton::clicked, this, &MainWindow::closeSession);
    connect(m_restApi, &RestApiClient::attendanceTestSuccess, this, &MainWindow::onAttendanceTestSuccess);
    connect(m_restApi, &RestApiClient::attendanceTestFailed, this, &MainWindow::onAttendanceTestFailed);
    connect(m_restApi, &RestApiClient::attendanceCodeGenerated, this, &MainWindow::onAttendanceCodeReady);
    connect(m_restApi, &RestApiClient::attendanceCodeGenerationFailed, this, &MainWindow::onAttendanceCodeFailed);
}

// void MainWindow::onLoginSuccess(const QJsonObject &data)
// {
//     m_userName = data["name"].toString();
//     m_authToken = data["token"].toString();
//     // m_authToken =
//     qDebug() << "m_authToken in MainWindow2222:" << m_authToken;
//     m_restApi->setAuthToken(m_authToken);
// }


void MainWindow::onLoginSuccess(const QJsonObject &data, const QString &authToken)
{
    m_userName = data["name"].toString();
    // m_authToken = data["token"].toString();
    m_authToken = authToken; // Use the token passed from RestApiClient
    // m_authToken =
    qDebug() << "m_authToken in MainWindow2222:" << m_authToken;
    m_restApi->setAuthToken(m_authToken);
}

void MainWindow::onLoginFailed(const QString &message)
{
    QMessageBox::warning(this, "Login Failed", message);
}

// mainwindow.cpp
void MainWindow::onAttendanceTestSuccess(const QJsonObject &responseData)
{
    QString prettyJson = QJsonDocument(responseData).toJson(QJsonDocument::Indented);
    qDebug() << "[TEST] SUCCESS: /attendance endpoint test returned:\n" << prettyJson;
    QMessageBox::information(this, "Attendance Test Success",
                             "Successfully fetched /attendance endpoint.\nSee Application Output for details.");
    // Burada 'responseData' içindeki 'data' alanını ayrıca işleyebilirsiniz.
    // Örneğin: QJsonObject attendanceDetails = responseData.value("data").toObject();
    //          int totalStudents = attendanceDetails.value("total_students").toInt();
    //          QJsonArray attendanceList = attendanceDetails.value("attendance_list").toArray();
    //          qDebug() << "Total students from /attendance:" << totalStudents;
}

void MainWindow::onAttendanceTestFailed(const QString &errorMsg)
{
    qDebug() << "[TEST] FAILURE: /attendance endpoint test failed:" << errorMsg;
    QMessageBox::warning(this, "Attendance Test Failed",
                         "Failed to fetch /attendance endpoint.\nError: " + errorMsg);
}

void MainWindow::initialize()
{
    // Set window properties
    setWindowTitle("AirClass Desktop");

    // Pass UI elements to managers and controllers
    if (ui->studentListWidget && ui->requestListWidget) {
        m_attendanceManager->setStudentListWidget(ui->studentListWidget);
        m_attendanceManager->setRequestListWidget(ui->requestListWidget);
    }

    // Add Download Attendance and Refresh buttons to attendance group box
    if (ui->attendanceGroupBox) {
        // Check if the buttons layout already exists
        QHBoxLayout* existingLayout = ui->attendanceGroupBox->findChild<QHBoxLayout*>();
        if (!existingLayout) {
            QHBoxLayout *attendanceButtonsLayout = new QHBoxLayout();

            // Create Download Attendance button if it doesn't exist
            if (!ui->attendanceGroupBox->findChild<QPushButton*>("downloadAttendanceButton")) {
                QPushButton *downloadAttendanceButton = new QPushButton(ui->attendanceGroupBox);
                downloadAttendanceButton->setObjectName("downloadAttendanceButton");
                downloadAttendanceButton->setText("Download Attendance");
                downloadAttendanceButton->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #27ae60; }");
                connect(downloadAttendanceButton, &QPushButton::clicked, this, &MainWindow::downloadAttendanceReport);
                attendanceButtonsLayout->addWidget(downloadAttendanceButton);
            }

            // Create Refresh button if it doesn't exist
            if (!ui->attendanceGroupBox->findChild<QPushButton*>("refreshAttendanceButton")) {
                QPushButton *refreshAttendanceButton = new QPushButton(ui->attendanceGroupBox);
                refreshAttendanceButton->setObjectName("refreshAttendanceButton");
                refreshAttendanceButton->setText("Refresh");
                refreshAttendanceButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #2980b9; }");
                connect(refreshAttendanceButton, &QPushButton::clicked, this, [this]() {
                    if (!m_currentRoomId.isEmpty()) {
                        m_restApi->listAttendance(m_currentRoomId);
                    }
                });
                attendanceButtonsLayout->addWidget(refreshAttendanceButton);
            }

            // Add the buttons layout to the attendance group box
            ui->attendanceGroupBox->layout()->addItem(attendanceButtonsLayout);
        }
    }

    // Initialize PDF view
    if (ui->pdfView) {
        // Configure PDF view settings
        ui->pdfView->setPageMode(QPdfView::PageMode::SinglePage);
        ui->pdfView->setZoomMode(QPdfView::ZoomMode::Custom);

        // Create and setup drawing layer
        m_drawingLayer = new DrawingLayer(ui->pdfView);
        m_drawingLayer->setGeometry(ui->pdfView->geometry());
        m_drawingLayer->show();

        // Install event filter to handle PDF view resize events
        ui->pdfView->installEventFilter(this);

        // Connect PDF view signals for page changes and zoom
        auto nav = ui->pdfView->pageNavigator();
        connect(nav, &QPdfPageNavigator::currentPageChanged, this, [this](int page) {
            if (m_drawingLayer) {
                m_drawingLayer->setCurrentPage(page);
            }
        });

        connect(ui->pdfView, &QPdfView::zoomFactorChanged, this, [this](qreal factor) {
            if (m_drawingLayer) {
                m_drawingLayer->updateScale(factor);
            }
        });

        // Connect scroll bars
        connect(ui->pdfView->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
            if (m_drawingLayer) {
                QPoint currentScroll = QPoint(value, ui->pdfView->verticalScrollBar()->value());
                m_drawingLayer->updateScrollPosition(currentScroll);
            }
        });

        connect(ui->pdfView->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
            if (m_drawingLayer) {
                QPoint currentScroll = QPoint(ui->pdfView->horizontalScrollBar()->value(), value);
                m_drawingLayer->updateScrollPosition(currentScroll);
            }
        });

        // Create drawing controls layout
        QHBoxLayout *drawingControlsLayout = new QHBoxLayout();
        drawingControlsLayout->setObjectName("drawingControlsLayout");

        // Common button style
        QString buttonStyle = R"(
            QPushButton {
                background-color: white;
                color: #333;
                border: 1px solid #e0e0e0;
                padding: 8px;
                border-radius: 8px;
                font-size: 18px;
                min-width: 44px;
                min-height: 44px;
                max-width: 44px;
                max-height: 44px;
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            }
            QPushButton:hover {
                background-color: #f8f9fa;
                border-color: #bdbdbd;
                box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15);
            }
            QPushButton:pressed {
                background-color: #f1f3f4;
                border-color: #9e9e9e;
                box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);
            }
        )";

        // Create clear button with emoji
        QPushButton *clearButton = new QPushButton("🗑️", this);
        clearButton->setObjectName("clearDrawingButton");
        clearButton->setToolTip("Clear Drawing");
        clearButton->setStyleSheet(buttonStyle);
        connect(clearButton, &QPushButton::clicked, this, [this]() {
            if (m_drawingLayer) {
                m_drawingLayer->clearCurrentPage();
            }
        });
        drawingControlsLayout->addWidget(clearButton);

        // Create drawing toggle button with emoji
        QPushButton *drawButton = new QPushButton("✏️", this);
        drawButton->setObjectName("drawButton");
        drawButton->setToolTip("Toggle Drawing");
        drawButton->setCheckable(true);
        drawButton->setChecked(true);
        drawButton->setStyleSheet(buttonStyle + R"(
            QPushButton:checked {
                background-color: #e8f5e9;
                border-color: #66bb6a;
                color: #2e7d32;
            }
            QPushButton:checked:hover {
                background-color: #c8e6c9;
                border-color: #43a047;
            }
            QPushButton:checked:pressed {
                background-color: #a5d6a7;
                border-color: #2e7d32;
            }
        )");
        connect(drawButton, &QPushButton::clicked, this, [this, drawButton]() {
            if (m_drawingLayer) {
                m_drawingLayer->setDrawingEnabled(drawButton->isChecked());
            }
        });
        drawingControlsLayout->addWidget(drawButton);

        // Create color button with emoji
        QPushButton *colorButton = new QPushButton("🎨", this);
        colorButton->setObjectName("colorButton");
        colorButton->setToolTip("Select Color");
        colorButton->setStyleSheet(buttonStyle);
        connect(colorButton, &QPushButton::clicked, this, [this]() {
            if (m_drawingLayer) {
                QColor color = QColorDialog::getColor(m_drawingLayer->getPenColor(), this, "Select Pen Color");
                if (color.isValid()) {
                    m_drawingLayer->setPenColor(color);
                }
            }
        });
        drawingControlsLayout->addWidget(colorButton);

        // Create pen width slider
        QSlider *widthSlider = new QSlider(Qt::Horizontal, this);
        widthSlider->setObjectName("penWidthSlider");
        widthSlider->setRange(1, 10);
        widthSlider->setValue(2);
        widthSlider->setFixedWidth(120);
        widthSlider->setStyleSheet(R"(
            QSlider::groove:horizontal {
                border: 1px solid #e0e0e0;
                height: 6px;
                background: white;
                margin: 2px 0;
                border-radius: 3px;
            }
            QSlider::handle:horizontal {
                background: white;
                border: 1px solid #bdbdbd;
                width: 16px;
                height: 16px;
                margin: -5px 0;
                border-radius: 8px;
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            }
            QSlider::handle:horizontal:hover {
                background: #f8f9fa;
                border-color: #9e9e9e;
                box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15);
            }
            QSlider::handle:horizontal:pressed {
                background: #f1f3f4;
                border-color: #757575;
                box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);
            }
        )");
        connect(widthSlider, &QSlider::valueChanged, this, [this](int value) {
            if (m_drawingLayer) {
                m_drawingLayer->setPenWidth(value);
            }
        });
        drawingControlsLayout->addWidget(widthSlider);

        // Add drawing controls to presentation layout
        if (ui->presentationLayout) {
            ui->presentationLayout->insertLayout(1, drawingControlsLayout);
        }

        // Pass it to presentation manager
        m_presentationManager->setPdfView(ui->pdfView);

        // Connect PDF view signals
        connect(m_presentationManager, &PresentationManager::error, this, [this](const QString &message) {
            QMessageBox::warning(this, tr("PDF Error"), message);
        });

        connect(m_presentationManager, &PresentationManager::presentationLoaded, this, [this](const QString &title, int pages) {
            statusBar()->showMessage(tr("Loaded presentation: %1 (%2 pages)").arg(title).arg(pages), 3000);

            // Switch to presentation view after successful load
            if (ui->stackedWidget) {
                ui->stackedWidget->setCurrentIndex(1);
            }
        });
    }

    // Register gesture commands
    m_gestureProcessor->registerGestureCommand("next_slide", [this]() {
        qDebug() << "Executing next slide command";
        m_presentationManager->nextSlide(1);
    });

    m_gestureProcessor->registerGestureCommand("prev_slide", [this]() {
        qDebug() << "Executing previous slide command";
        m_presentationManager->previousSlide(1);
    });

    m_gestureProcessor->registerGestureCommand("zoom_in", [this]() {
        qDebug() << "Executing zoom in command";
        m_presentationManager->zoomIn();
    });

    m_gestureProcessor->registerGestureCommand("zoom_out", [this]() {
        qDebug() << "Executing zoom out command";
        m_presentationManager->zoomOut();
    });

    m_gestureProcessor->registerGestureCommand("zoom_reset", [this]() {
        qDebug() << "Executing zoom reset command";
        m_presentationManager->setZoomLevel(1.0, QPointF(-1, -1));
    });

    m_gestureProcessor->registerGestureCommand("up", [this]() {
        qDebug() << "Executing up command";
        if (ui->pdfView) {
            QScrollBar* vbar = ui->pdfView->verticalScrollBar();
            vbar->setValue(vbar->value() - vbar->pageStep());
        }
    });

    m_gestureProcessor->registerGestureCommand("down", [this]() {
        qDebug() << "Executing down command";
        if (ui->pdfView) {
            QScrollBar* vbar = ui->pdfView->verticalScrollBar();
            vbar->setValue(vbar->value() + vbar->pageStep());
        }
    });

    m_gestureProcessor->registerGestureCommand("left", [this]() {
        qDebug() << "Executing left command";
        if (ui->pdfView) {
            QScrollBar* hbar = ui->pdfView->horizontalScrollBar();
            hbar->setValue(hbar->value() - hbar->pageStep());
        }
    });

    m_gestureProcessor->registerGestureCommand("right", [this]() {
        qDebug() << "Executing right command";
        if (ui->pdfView) {
            QScrollBar* hbar = ui->pdfView->horizontalScrollBar();
            hbar->setValue(hbar->value() + hbar->pageStep());
        }
    });

    qDebug() << "Application initialized successfully";
}

void MainWindow::setupConnections()
{
    qDebug() << "Setting up connections in MainWindow...";
    // WebSocket connections
    connect(m_webSocketClient, &WebSocketClient::connected, this, &MainWindow::onConnected);
    qDebug() << "WebSocket connected signal connected in MainWindow";
    connect(m_webSocketClient, &WebSocketClient::disconnected, this, &MainWindow::onDisconnected);
    qDebug() << "WebSocket disconnected signal connected in MainWindow";
    connect(m_webSocketClient, &WebSocketClient::messageReceived, this, &MainWindow::handleServerMessage);
    qDebug() << "WebSocket message received signal connected in MainWindow";

    // Connect gesture and page navigation signals
    connect(m_webSocketClient, &WebSocketClient::gestureReceived, this, [this](const QString &gestureType, const QString &clientId, const QString &timestamp) {
        qDebug() << "Gesture received in MainWindow:" << gestureType << "from client:" << clientId;

        QJsonObject gestureData;
        gestureData["gesture_type"] = gestureType;
        gestureData["client_id"] = clientId;
        gestureData["timestamp"] = timestamp;
        handleGestureCommand(gestureData);
    });

    qDebug() << "WebSocket gesture received signal connected in MainWindow";

    connect(m_webSocketClient, &WebSocketClient::pageNavigationReceived, this, [this](const QString &action, const QString &clientId, const QString &timestamp) {
        Q_UNUSED(timestamp)  // <-- Bunu ekleyin
        qDebug() << "Page navigation received in MainWindow:" << action << "from client:" << clientId;

        if (action == "next") {
            qDebug() << "Executing next slide command";
            m_presentationManager->nextSlide(1);
        } else if (action == "previous") {
            qDebug() << "Executing previous slide command";
            m_presentationManager->previousSlide(1);
        }
    });

    qDebug() << "WebSocket page navigation received signal connected in MainWindow";

    // Connect UI buttons to actions
    if (ui->nextButton) {
        connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::on_nextButton_clicked);
    }

    if (ui->prevButton) {
        connect(ui->prevButton, &QPushButton::clicked, this, &MainWindow::on_prevButton_clicked);
    }

    if (ui->zoomInButton) {
        connect(ui->zoomInButton, &QPushButton::clicked, this, &MainWindow::on_zoomInButton_clicked);
    }

    if (ui->zoomOutButton) {
        connect(ui->zoomOutButton, &QPushButton::clicked, this, &MainWindow::on_zoomOutButton_clicked);
    }

    qDebug() << "UI button connections set up in MainWindow";

    // Connect new UI buttons
    if (ui->generateAttendanceCodeButton) {
        // wait 2 seconds before connecting to ensure UI is fully initialized
        QTimer::singleShot(2000, this, []() {
            qDebug() << "Connecting generateAttendanceCodeButton in MainWindow";
            // connect(ui->generateAttendanceCodeButton, &QPushButton::clicked,
            //         this, &MainWindow::on_generateAttendanceCodeButton_clicked);
            qDebug() << "Generate Attendance Code button connected in MainWindow";
        });

        // qDebug() << "Connecting generateAttendanceCodeButton in MainWindow";
        // connect(ui->generateAttendanceCodeButton, &QPushButton::clicked,
        //         this, &MainWindow::on_generateAttendanceCodeButton_clicked);
        // qDebug() << "Generate Attendance Code button connected in MainWindow";
    }

    qDebug() << "Generate Attendance Code button connected in MainWindow";

    if (ui->refreshAttendanceButton) {
        qDebug() << "Connecting refreshAttendanceButton in MainWindow";
        connect(ui->refreshAttendanceButton, &QPushButton::clicked,
                this, &MainWindow::on_refreshAttendanceButton_clicked);
        qDebug() << "Refresh Attendance button connected in MainWindow";
    }

    qDebug() << "Refresh Attendance button connected in MainWindow";

    if (ui->approveRequestButton) {
        connect(ui->approveRequestButton, &QPushButton::clicked,
                this, &MainWindow::on_approveRequestButton_clicked);
    }

    qDebug() << "Approve Request button connected in MainWindow";

    if (ui->rejectRequestButton) {
        connect(ui->rejectRequestButton, &QPushButton::clicked,
                this, &MainWindow::on_rejectRequestButton_clicked);
    }



    // Connect attendance manager signals
    qDebug() << "Connecting attendance manager signals in MainWindow";

    // Connect menu actions
    connect(ui->actionDashboard, &QAction::triggered, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
    });

    connect(ui->actionPresentation, &QAction::triggered, [this]() {
        ui->stackedWidget->setCurrentIndex(1);
    });

    connect(ui->actionStudents, &QAction::triggered, [this]() {
        ui->stackedWidget->setCurrentIndex(2);
    });

    connect(ui->actionGenerate_QR_Code, &QAction::triggered, [this]() {
        ui->stackedWidget->setCurrentIndex(3);
    });

    qDebug() << "Connections set up successfully in MainWindow";


    QAction *actionGestureGuide = new QAction(tr("Gesture Guide"), this); // Use tr() for translations
    ui->menuView->addAction(actionGestureGuide);

    connect(actionGestureGuide, &QAction::triggered, [this]() {
        GestureGuide *gestureGuideDialog = new GestureGuide(this); // Pass 'this' as parent
        gestureGuideDialog->setAttribute(Qt::WA_DeleteOnClose);    // Auto-delete when closed
        gestureGuideDialog->exec();                                // Show the dialog modally
    });

    qDebug() << "Gesture Guide action added to menu";

    // Add timeout functionality
    QAction *actionTimeout = new QAction("Timeout", this);
    actionTimeout->setShortcut(QKeySequence("Ctrl+T")); // Add keyboard shortcut
    ui->menuView->addAction(actionTimeout);

    connect(actionTimeout, &QAction::triggered, [this]() {
        showTimerDialog();
    });

    // Connect presentation manager signals
    connect(m_presentationManager, &PresentationManager::error, [this](const QString &errorMessage) {
        QMessageBox::warning(this, tr("PDF Error"), errorMessage);
    });

    connect(m_presentationManager, &PresentationManager::pageChanged, [this](int currentPage, int totalPages) {
        ui->pageIndicatorLabel->setText(tr("Page %1 of %2").arg(currentPage + 1).arg(totalPages));
    });

}


// Update onConnected method to use session code
void MainWindow::onConnected()
{
    qDebug() << "Connected to server";
    statusBar()->showMessage(tr("Connected to server"), 3000);

    ui->connectionStatusLabel->setText("Connected");
    ui->connectionStatusLabel->setStyleSheet("color: green;");
    ui->disconnectButton->setEnabled(true);
    ui->connectButton->setEnabled(false);

    // Send identification message with session code
    QJsonObject message;
    message["type"] = "identify";
    message["role"] = "desktop";
    message["roomId"] = m_currentRoomId; // Use session code as room ID
    message["sessionId"] = m_sessionId;
    message["sessionName"] = m_sessionName;

    QJsonDocument doc(message);
    m_webSocketClient->sendMessage(doc.toJson(QJsonDocument::Compact));

    m_restApi->startPollingRequests(m_currentRoomId);
}

void MainWindow::onDisconnected()
{
    qDebug() << "Disconnected from server1";
    //ui->connectionStatusLabel->setText("Disconnected");
    qDebug() << "Dc 1";
    //ui->connectionStatusLabel->setStyleSheet("color: red;");
    qDebug() << "Dc 2";
    //ui->disconnectButton->setEnabled(false);
    qDebug() << "Dc 3";
    //ui->connectButton->setEnabled(true);
    qDebug() << "Dc 4";
    m_restApi->stopPollingRequests();
    qDebug() << "Dc 5";
    // Use the QMainWindow's statusBar() method instead of accessing ui->statusBar
    statusBar()->showMessage(tr("Disconnected from server"), 3000);

}

void MainWindow::handleServerMessage(const QString &message)
{
    qDebug() << "Received message from server:" << message;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON message received";
        return;
    }

    QJsonObject jsonObj = doc.object();
    QString messageType = jsonObj["type"].toString();

    qDebug() << "Processing message type:" << messageType;

    if (messageType == "page_navigation") {
        QString action = jsonObj["action"].toString();
        qDebug() << "Page navigation action:" << action;

        if (action == "next") {
            qDebug() << "Executing next slide command from WebSocket";
            m_presentationManager->nextSlide(1);
        } else if (action == "previous") {
            qDebug() << "Executing previous slide command from WebSocket";
            m_presentationManager->previousSlide(1);
        }
    } else if (messageType == "gesture") {
        qDebug() << "Handling gesture message";
        handleGestureCommand(jsonObj);
    } else if (messageType == "room_joined") {
        QString qrCodeData = jsonObj["qr_code"].toString();
        if (!qrCodeData.isEmpty()) {
            QByteArray decodedData = QByteArray::fromBase64(qrCodeData.toUtf8());
            QPixmap pixmap;
            pixmap.loadFromData(decodedData);

            if (ui->qrCodePreviewLabel) {
                ui->qrCodePreviewLabel->setPixmap(pixmap);
                ui->qrCodePreviewLabel->show();
            }
        }
    }
    else if (messageType == "attendance_update") {
        handleAttendanceUpdate(jsonObj["data"].toObject());
    } else if (messageType == "attendance_qr_code") {
        handleAttendanceQRCode(jsonObj["data"].toObject());
    } else if (messageType == "speak_request") {
        handleSpeakRequest(jsonObj["data"].toObject());
    } else if (messageType == "request_update") {
        handleRequestUpdate(jsonObj["data"].toObject());
    }

}

void MainWindow::onAttendanceListReceived(const QJsonArray &records)
{
    qDebug() << "onAttendanceListReceived called with" << records.size() << "records";

    // Switch to students view
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(2); // Switch to students view
        qDebug() << "Switched to students view";
    }

    // Clear existing attendance list
    if (ui->studentListWidget) {
        ui->studentListWidget->clear();
        qDebug() << "Cleared student list widget";

        // Add attendance records to the list
        for (const QJsonValue &value : records) {
            QJsonObject record = value.toObject();
            QString studentName = record["student_name"].toString();
            QString studentEmail = record["student_email"].toString();
            QString attendanceCode = record["attendance_code"].toString();
            QString attendanceDate = record["attendance_date"].toString();
            QString createdAt = record["created_at"].toString();

            qDebug() << "Processing record:" << studentName << studentEmail << attendanceCode << createdAt;

            // Format the display text with all information
            QString displayText = QString("%1 (%2)\nKod: %3 | Tarih: %4 | Saat: %5")
                .arg(studentName)
                .arg(studentEmail)
                .arg(attendanceCode)
                .arg(attendanceDate)
                .arg(createdAt.split(" ")[1]); // Just show the time part

            QListWidgetItem *item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, record["id"].toInt());

            // Style the item
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            item->setBackground(QBrush(QColor("#f0f9ff"))); // Light blue background
            item->setForeground(QBrush(QColor("#2c3e50"))); // Dark text color

            ui->studentListWidget->addItem(item);
            qDebug() << "Added item to list:" << displayText;
        }

        // Update attendance count
        if (ui->attendanceCountLabel) {
            //ui->attendanceCountLabel->setText(QString("Öğrenci sayısı: %1").arg(records.size()));
            qDebug() << "Updated attendance count label";
        }
    } else {
        qDebug() << "ERROR: studentListWidget is null!";
    }
}

void MainWindow::on_refreshAttendanceButton_clicked()
{
    qDebug() << "Refresh button clicked, current room ID:" << m_currentRoomId;

    if (!m_currentRoomId.isEmpty()) {
        // Show loading indicator
        if (ui->studentListWidget) {
            ui->studentListWidget->clear();
            QListWidgetItem *loadingItem = new QListWidgetItem("Yükleniyor...");
            loadingItem->setTextAlignment(Qt::AlignCenter);
            ui->studentListWidget->addItem(loadingItem);
            qDebug() << "Added loading indicator to student list widget";
        } else {
            qDebug() << "ERROR: studentListWidget is null in refresh handler!";
        }

        // Refresh attendance list
        m_restApi->listAttendance(m_currentRoomId);
        qDebug() << "Called listAttendance with room ID:" << m_currentRoomId;
    } else {
        QMessageBox::warning(this, "Hata", "Aktif bir sınıf bulunamadı.");
        qDebug() << "Warning shown: No active room found";
    }
}

void MainWindow::onAttendanceCodeGenerated(const QString &code, const QString &expiresAt, const QString &expiresIn)
{
    qDebug() << "onAttendanceCodeGenerated called with code:" << code;
    qDebug() << "Expires at:" << expiresAt;
    qDebug() << "Expires in:" << expiresIn;

    // Immediately request QR code after code generation
    if (!m_currentRoomId.isEmpty()) {
        qDebug() << "Requesting QR code for room:" << m_currentRoomId;
        m_restApi->getQrCode(m_currentRoomId);
        
        // Switch to QR code view immediately
        if (ui->stackedWidget) {
            ui->stackedWidget->setCurrentIndex(3); // QR code page index
            qDebug() << "Switched to QR code page";
        }
    } else {
        qDebug() << "ERROR: No current room ID available for QR code request";
    }
}

void MainWindow::on_generateAttendanceCodeButton_clicked()
{
    if (!m_currentRoomId.isEmpty()) {
        qDebug() << "Generating attendance code for room:" << m_currentRoomId;
        
        // Generate attendance code via REST API
        m_restApi->generateAttendanceCode(m_currentRoomId, 300); // 5 minutes = 300 seconds
        
        // Request QR code after a short delay to ensure the code is generated
        QTimer::singleShot(500, this, [this]() {
            qDebug() << "Requesting QR code for room:" << m_currentRoomId;
            m_restApi->getQrCode(m_currentRoomId);
        });

        // Switch to QR Code view
        if (ui->stackedWidget) {
            ui->stackedWidget->setCurrentIndex(3); // QR code page index
            qDebug() << "Switched to QR code page";
        }
    } else {
        QMessageBox::warning(this, "Error", "No active room selected");
    }
}

void MainWindow::on_downloadAttendanceButton_clicked()
{
    downloadAttendanceReport();
}

void MainWindow::onRequestsReceived(const QJsonArray &requests)
{
    qDebug() << "onRequestsReceived called with" << requests.size() << "requests";

    // Clear existing request list
    if (ui->requestListWidget) {
        ui->requestListWidget->clear();
        qDebug() << "Cleared request list widget";

        // Add request records to the list
        for (const QJsonValue &value : requests) {
            QJsonObject request = value.toObject();
            QString userName = request["user_name"].toString();
            QString status = request["status"].toString();
            QString requestId = QString::number(request["id"].toInt());

            qDebug() << "Processing request:" << userName << status << requestId;

            QListWidgetItem *item = new QListWidgetItem(userName);
            item->setData(Qt::UserRole, requestId); // Store request ID for later use

            // Style the item based on status
            if (status == "pending") {
                item->setBackground(QColor("#fff3cd")); // Light yellow for pending
                item->setForeground(QColor("#856404")); // Dark yellow text
            } else if (status == "approved") {
                item->setBackground(QColor("#d4edda")); // Light green for approved
                item->setForeground(QColor("#155724")); // Dark green text
            } else if (status == "rejected") {
                item->setBackground(QColor("#f8d7da")); // Light red for rejected
                item->setForeground(QColor("#721c24")); // Dark red text
            }

            ui->requestListWidget->addItem(item);
            qDebug() << "Added request to list:" << userName;
        }
    } else {
        qDebug() << "ERROR: requestListWidget is null!";
    }
}

void MainWindow::onNewSpeakRequest(const QString &studentId, const QString &studentName, const QString &requestId)
{
    // Show notification
    showSpeakRequestNotification(studentName, requestId);

    // Play notification sound
    playNotificationSound();

    qDebug() << "New speak request from:" << studentName << "ID:" << studentId;
}

void MainWindow::onRequestUpdated(const QString &requestId, const QString &action)
{
    Q_UNUSED(action)  // <-- Bunu ekleyin

    // Hide notification if it's for the current request
    if (m_currentRequestId == requestId) {
        hideSpeakRequestNotification();
    }

    // Remove from request list widget
    if (ui->requestListWidget) {
        for (int i = 0; i < ui->requestListWidget->count(); ++i) {
            QListWidgetItem *item = ui->requestListWidget->item(i);
            if (item && item->data(Qt::UserRole).toString() == requestId) {
                delete ui->requestListWidget->takeItem(i);
                break;
            }
        }
    }
}

void MainWindow::on_approveRequestButton_clicked()
{
    if (!m_currentRequestId.isEmpty()) {
        m_restApi->updateRequest(m_currentRoomId, m_currentRequestId, "approve");
        hideSpeakRequestNotification();
    }
}

void MainWindow::on_rejectRequestButton_clicked()
{
    if (!m_currentRequestId.isEmpty()) {
        m_restApi->updateRequest(m_currentRoomId, m_currentRequestId, "reject");
        hideSpeakRequestNotification();
    }
}

void MainWindow::handleAttendanceUpdate(const QJsonObject &data)
{
    QString studentName = data["student_name"].toString();
    QString studentId = data["student_id"].toString();

    // Add to attendance list
    QListWidgetItem *item = new QListWidgetItem(studentName);
    item->setData(Qt::UserRole, studentId);

    if (ui->studentListWidget) {
        ui->studentListWidget->addItem(item);
    }

    // Update count
    int count = ui->studentListWidget ? ui->studentListWidget->count() : 0;
    if (ui->attendanceCountLabel) {
        ui->attendanceCountLabel->setText(QString("Students present: %1").arg(count));
    }
}

void MainWindow::handleAttendanceQRCode(const QJsonObject &data)
{
    qDebug() << "handleAttendanceQRCode called with data:" << data;

    QString qrBase64 = data["qr_code"].toString();
    QString code = data["code"].toString();
    QString expiry = data["expiry"].toString();

    qDebug() << "QR Code data received - Code:" << code << "Expiry:" << expiry;

    // Display the QR code
    QByteArray imageData = QByteArray::fromBase64(qrBase64.toUtf8());
    QPixmap pixmap;
    if (pixmap.loadFromData(imageData)) {
        qDebug() << "Successfully loaded QR code image";
        if (ui->qrCodePreviewLabel) {
            ui->qrCodePreviewLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->qrCodePreviewLabel->show();
            qDebug() << "Set QR code to label";
        } else {
            qDebug() << "ERROR: qrCodePreviewLabel is null!";
        }
    } else {
        qDebug() << "ERROR: Failed to load QR code image from base64 data";
    }

    // Switch to QR Code view
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(3); // QR code page index
        qDebug() << "Switched to QR code page";
    } else {
        qDebug() << "ERROR: stackedWidget is null!";
    }
}

void MainWindow::handleSpeakRequest(const QJsonObject &data)
{
    QString studentName = data["student_name"].toString();
    QString requestId = data["id"].toString();

    showSpeakRequestNotification(studentName, requestId);
    playNotificationSound();
}

void MainWindow::handleRequestUpdate(const QJsonObject &data)
{
    QString requestId = data["request_id"].toString();
    QString action = data["action"].toString();

    onRequestUpdated(requestId, action);
}

void MainWindow::playNotificationSound()
{
    // Qt 6 için güncellendi
    if (!m_notificationPlayer) {
        m_notificationPlayer = new QMediaPlayer(this);
        QAudioOutput *audioOutput = new QAudioOutput(this);
        m_notificationPlayer->setAudioOutput(audioOutput);
    }

    m_notificationPlayer->setSource(QUrl("qrc:/sounds/notification.wav"));

    if (m_notificationPlayer->audioOutput()) {
        m_notificationPlayer->audioOutput()->setVolume(0.7f);
    }

    m_notificationPlayer->play();
}

void MainWindow::showSpeakRequestNotification(const QString &studentName, const QString &requestId)
{
    m_currentRequestId = requestId;
    m_currentRequestStudentName = studentName;

    if (ui->notificationWidget) {
        ui->speakRequestLabel->setText(QString("🙋 %1 wants to speak").arg(studentName));
        ui->notificationWidget->setVisible(true);

        // Animate the notification
        ui->notificationWidget->setStyleSheet(
            "QWidget#notificationWidget {"
            "    background-color: #ffc107;"
            "    border-radius: 8px;"
            "    margin: 5px;"
            "    border: 2px solid #ff9800;"
            "}"
        );
    }
}

void MainWindow::hideSpeakRequestNotification()
{
    if (ui->notificationWidget) {
        ui->notificationWidget->setVisible(false);
    }

    m_currentRequestId.clear();
    m_currentRequestStudentName.clear();
}

void MainWindow::downloadAttendanceReport()
{
    if (m_currentRoomId.isEmpty()) {
        QMessageBox::warning(this, "Error", "No active room selected.");
        return;
    }

    // First, get the latest attendance data
    m_restApi->listAttendance(m_currentRoomId);

    // Connect a one-time slot to handle the response
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = connect(m_restApi, &RestApiClient::attendanceListReceived,
                          this, [this, connection](const QJsonArray &records) {
                              // Disconnect this one-time connection
                              QObject::disconnect(*connection);
                              delete connection;

                              // Get save file path
                              QString fileName = QFileDialog::getSaveFileName(this,
                                                                              "Save Attendance Report",
                                                                              QString("attendance_report_%1_%2.pdf")
                                                                                  .arg(m_userName)
                                                                                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
                                                                              "PDF Files (*.pdf);;All Files (*)");

                              if (fileName.isEmpty()) {
                                  return;
                              }

                              // Create PDF document with better settings
                              QPdfWriter pdfWriter(fileName);
                              pdfWriter.setPageSize(QPageSize(QPageSize::A4));
                              pdfWriter.setPageMargins(QMarginsF(20, 20, 20, 20)); // Marjinleri azalttım
                              pdfWriter.setResolution(300); // Yüksek çözünürlük

                              QPainter painter(&pdfWriter);

                              // Antialiasing ve render hints ekle
                              painter.setRenderHint(QPainter::Antialiasing, true);
                              painter.setRenderHint(QPainter::TextAntialiasing, true);
                              painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

                              // Set up fonts with better sizing
                              QFont titleFont("Arial", 16, QFont::Bold);
                              QFont headerFont("Arial", 11, QFont::Bold);
                              QFont contentFont("Arial", 9, QFont::Normal);

                              // PDF boyutlarını al
                              int pageWidth = pdfWriter.width();
                              int pageHeight = pdfWriter.height();
                              int margin = 40; // 20mm margin in device units

                              // Calculate column widths based on actual page width - daha dengeli dağılım
                              int totalWidth = pageWidth - (2 * margin);
                              int idWidth = totalWidth * 0.12;        // Student ID için daha az yer
                              int nameWidth = totalWidth * 0.40;      // İsim için daha fazla yer
                              int timestampWidth = totalWidth * 0.33; // Timestamp için yeterli yer
                              int codeWidth = totalWidth * 0.15;      // Code için standart yer

                              // Draw title
                              painter.setFont(titleFont);
                              painter.setPen(Qt::black);
                              QRect titleRect(margin, margin, totalWidth, 60);
                              painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, "Attendance Report");

                              // Draw session info
                              painter.setFont(contentFont);
                              QString sessionInfo = QString("Session: %1 (Room ID: %2)")
                                                        .arg(m_sessionName)
                                                        .arg(m_currentRoomId);
                              QRect sessionRect(margin, margin + 70, totalWidth, 40);
                              painter.drawText(sessionRect, Qt::AlignHCenter | Qt::AlignVCenter, sessionInfo);

                              // Draw date and time
                              QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss");
                              QRect dateRect(margin, margin + 110, totalWidth, 40);
                              painter.drawText(dateRect, Qt::AlignHCenter | Qt::AlignVCenter, dateTime);

                              // Table starting position
                              int tableStartY = margin + 170;
                              int headerHeight = 50;  // Header için daha fazla yükseklik
                              int rowHeight = 45;     // Satırlar için çok daha fazla yükseklik

                              // Draw table header
                              painter.setFont(headerFont);

                              // Header background
                              painter.fillRect(margin, tableStartY, totalWidth, headerHeight, QColor(240, 240, 240));

                              // Header borders
                              painter.setPen(QPen(Qt::black, 1));
                              painter.drawRect(margin, tableStartY, idWidth, headerHeight);
                              painter.drawRect(margin + idWidth, tableStartY, nameWidth, headerHeight);
                              painter.drawRect(margin + idWidth + nameWidth, tableStartY, timestampWidth, headerHeight);
                              painter.drawRect(margin + idWidth + nameWidth + timestampWidth, tableStartY, codeWidth, headerHeight);

                              // Header text with proper alignment - daha fazla padding
                              painter.setPen(Qt::black);
                              painter.drawText(QRect(margin + 10, tableStartY + 10, idWidth - 20, headerHeight - 20),
                                               Qt::AlignCenter | Qt::AlignVCenter, "Student ID");
                              painter.drawText(QRect(margin + idWidth + 10, tableStartY + 10, nameWidth - 20, headerHeight - 20),
                                               Qt::AlignCenter | Qt::AlignVCenter, "Student Name");
                              painter.drawText(QRect(margin + idWidth + nameWidth + 10, tableStartY + 10, timestampWidth - 20, headerHeight - 20),
                                               Qt::AlignCenter | Qt::AlignVCenter, "Timestamp");
                              painter.drawText(QRect(margin + idWidth + nameWidth + timestampWidth + 10, tableStartY + 10, codeWidth - 20, headerHeight - 20),
                                               Qt::AlignCenter | Qt::AlignVCenter, "Code");

                              // Draw content rows
                              painter.setFont(contentFont);
                              int currentY = tableStartY + headerHeight;

                              for (int i = 0; i < records.size(); ++i) {
                                  QJsonObject record = records[i].toObject();

                                  // Check if we need a new page
                                  if (currentY + rowHeight > pageHeight - margin) {
                                      pdfWriter.newPage();
                                      currentY = margin;

                                      // Redraw header on new page
                                      painter.setFont(headerFont);
                                      painter.fillRect(margin, currentY, totalWidth, headerHeight, QColor(240, 240, 240));
                                      painter.setPen(QPen(Qt::black, 1));
                                      painter.drawRect(margin, currentY, idWidth, headerHeight);
                                      painter.drawRect(margin + idWidth, currentY, nameWidth, headerHeight);
                                      painter.drawRect(margin + idWidth + nameWidth, currentY, timestampWidth, headerHeight);
                                      painter.drawRect(margin + idWidth + nameWidth + timestampWidth, currentY, codeWidth, headerHeight);

                                      painter.setPen(Qt::black);
                                      painter.drawText(QRect(margin + 10, currentY + 10, idWidth - 20, headerHeight - 20),
                                                       Qt::AlignCenter | Qt::AlignVCenter, "Student ID");
                                      painter.drawText(QRect(margin + idWidth + 10, currentY + 10, nameWidth - 20, headerHeight - 20),
                                                       Qt::AlignCenter | Qt::AlignVCenter, "Student Name");
                                      painter.drawText(QRect(margin + idWidth + nameWidth + 10, currentY + 10, timestampWidth - 20, headerHeight - 20),
                                                       Qt::AlignCenter | Qt::AlignVCenter, "Timestamp");
                                      painter.drawText(QRect(margin + idWidth + nameWidth + timestampWidth + 10, currentY + 10, codeWidth - 20, headerHeight - 20),
                                                       Qt::AlignCenter | Qt::AlignVCenter, "Code");

                                      currentY += headerHeight;
                                      painter.setFont(contentFont);
                                  }

                                  // Alternate row background
                                  if (i % 2 == 1) {
                                      painter.fillRect(margin, currentY, totalWidth, rowHeight, QColor(248, 249, 250));
                                  }

                                  // Draw cell borders
                                  painter.setPen(QPen(Qt::black, 1));
                                  painter.drawRect(margin, currentY, idWidth, rowHeight);
                                  painter.drawRect(margin + idWidth, currentY, nameWidth, rowHeight);
                                  painter.drawRect(margin + idWidth + nameWidth, currentY, timestampWidth, rowHeight);
                                  painter.drawRect(margin + idWidth + nameWidth + timestampWidth, currentY, codeWidth, rowHeight);

                                  // Draw cell content with padding and proper alignment - çok daha fazla padding
                                  painter.setPen(Qt::black);

                                  // Student ID - center aligned
                                  painter.drawText(QRect(margin + 10, currentY + 10, idWidth - 20, rowHeight - 20),
                                                   Qt::AlignCenter | Qt::AlignVCenter,
                                                   record["student_id"].toString());

                                  // Student Name - left aligned with more padding for readability
                                  QString studentName = record["student_name"].toString();
                                  painter.drawText(QRect(margin + idWidth + 15, currentY + 10, nameWidth - 30, rowHeight - 20),
                                                   Qt::AlignLeft | Qt::AlignVCenter,
                                                   studentName);

                                  // Timestamp - center aligned
                                  QString timestamp = record["timestamp"].toString();
                                  // Format timestamp if needed
                                  if (!timestamp.isEmpty()) {
                                      QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
                                      if (dt.isValid()) {
                                          timestamp = dt.toString("dd.MM.yyyy HH:mm");
                                      }
                                  }
                                  painter.drawText(QRect(margin + idWidth + nameWidth + 10, currentY + 10, timestampWidth - 20, rowHeight - 20),
                                                   Qt::AlignCenter | Qt::AlignVCenter, timestamp);

                                  // Code - center aligned
                                  painter.drawText(QRect(margin + idWidth + nameWidth + timestampWidth + 10, currentY + 10, codeWidth - 20, rowHeight - 20),
                                                   Qt::AlignCenter | Qt::AlignVCenter,
                                                   record["code"].toString());

                                  currentY += rowHeight;
                              }

                              // Draw summary at the bottom
                              currentY += 30;
                              painter.setFont(headerFont);
                              QString totalText = QString("Total Attendance: %1 students").arg(records.size());
                              painter.drawText(QRect(margin, currentY, totalWidth, 30),
                                               Qt::AlignRight | Qt::AlignVCenter, totalText);

                              // Draw footer with generation info
                              currentY += 50;
                              painter.setFont(contentFont);
                              QString footerText = QString("Report generated on %1 by %2")
                                                       .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss"))
                                                       .arg(m_userName);
                              painter.drawText(QRect(margin, currentY, totalWidth, 30),
                                               Qt::AlignCenter | Qt::AlignVCenter, footerText);

                              painter.end();

                              QMessageBox::information(this, "Success",
                                                       QString("Attendance report saved successfully to:\n%1").arg(fileName));
                          });
}

// Update handleGestureCommand to handle all gestures
void MainWindow::handleGestureCommand(const QJsonObject &data)
{
    QString gestureType = data["gesture_type"].toString();
    QString clientId = data["client_id"].toString();

    qDebug() << "Handling gesture:" << gestureType << "from client:" << clientId;

    // Get the gesture map file path
    QString gestureMapPath = GestureGuide::getGestureMapPath();
    qDebug() << "Looking for gesture map file at:" << gestureMapPath;

    QFile file(gestureMapPath);
    if (!file.exists()) {
        qDebug() << "Gesture map file does not exist at:" << gestureMapPath;
        return;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line;
        bool found = false;

        while (!in.atEnd()) {
            line = in.readLine().trimmed();
            if (!line.isEmpty() && line.startsWith(gestureType)) {
                found = true;
                QStringList parts = line.split(" ", Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    QString action = parts[1];
                    int repeatCount = (parts.size() >= 3) ? parts[2].toInt() : 1;

                    qDebug() << "Found gesture mapping:" << gestureType << "=>" << action << "(" << repeatCount << "times)";

                    // Handle different actions with repeat count
                    if (action == "next_slide") {
                        qDebug() << "Executing next_slide action" << repeatCount << "times";
                        m_presentationManager->nextSlide(repeatCount);
                    }
                    else if (action == "prev_slide") {
                        qDebug() << "Executing prev_slide action" << repeatCount << "times";
                        m_presentationManager->previousSlide(repeatCount);
                    }
                    else if (action == "drawing") {
                        qDebug() << "Executing drawing action";
                        if (m_drawingLayer) {
                            m_drawingLayer->setDrawingEnabled(true);
        if (data.contains("position")) {
            QJsonObject position = data["position"].toObject();
            double x = position["x"].toDouble();
            double y = position["y"].toDouble();
                                m_drawingLayer->drawRemotePoint(QPointF(x, y), true);
                            }
                        }
                    }
                    else if (action == "point") {
                        qDebug() << "Executing point action";
                        if (m_drawingLayer && data.contains("position")) {
                            QJsonObject position = data["position"].toObject();
                            double x = position["x"].toDouble();
                            double y = position["y"].toDouble();
                            m_drawingLayer->showPointer(x, y);
                        }
                    }
                    else if (action == "accept") {
                        qDebug() << "Executing accept action";
                        on_approveButton_clicked();
                    }
                    else if (action == "decline") {
                        qDebug() << "Executing decline action";
                        on_rejectButton_clicked();
                    }
                    else if (action == "attendance") {
                        qDebug() << "Executing attendance action";
                        if (!m_currentRoomId.isEmpty()) {
                            // Use repeat count as duration in seconds if specified
                            int duration = repeatCount * 60; // Convert minutes to seconds
                            m_restApi->generateAttendanceCode(m_currentRoomId, duration);
                        }
                    }
                    else if (action == "attention") {
                        qDebug() << "Executing attention action";
                        playNotificationSound();
                        QMessageBox::information(this, "Attention", "A student is requesting attention.");
                    }
                    else if (action == "break") {
                        qDebug() << "Executing break action for" << repeatCount << "minutes";
                        startBreakTimer(repeatCount); // Use the repeat count as minutes
                    }
                    else if (action == "zoom_in") {
                        qDebug() << "Executing zoom_in action" << repeatCount << "times";
                        for (int i = 0; i < repeatCount; i++) {
                            m_presentationManager->zoomIn();
                        }
                    }
                    else if (action == "zoom_out") {
                        qDebug() << "Executing zoom_out action" << repeatCount << "times";
                        for (int i = 0; i < repeatCount; i++) {
                            m_presentationManager->zoomOut();
                        }
                    }
                    else if (action == "zoom_reset") {
                        qDebug() << "Executing zoom_reset action";
                        m_presentationManager->setZoomLevel(1.0, QPointF(-1, -1));
                    }
                    else if (action == "up") {
                        qDebug() << "Executing up action" << repeatCount << "times";
        if (ui->pdfView) {
                            QScrollBar* vbar = ui->pdfView->verticalScrollBar();
                            vbar->setValue(vbar->value() - (vbar->pageStep() * repeatCount));
            }
        }
                    else if (action == "down") {
                        qDebug() << "Executing down action" << repeatCount << "times";
        if (ui->pdfView) {
                            QScrollBar* vbar = ui->pdfView->verticalScrollBar();
                            vbar->setValue(vbar->value() + (vbar->pageStep() * repeatCount));
            }
        }
                    else if (action == "left") {
                        qDebug() << "Executing left action" << repeatCount << "times";
        if (ui->pdfView) {
                            QScrollBar* hbar = ui->pdfView->horizontalScrollBar();
                            hbar->setValue(hbar->value() - (hbar->pageStep() * repeatCount));
            }
        }
                    else if (action == "right") {
                        qDebug() << "Executing right action" << repeatCount << "times";
        if (ui->pdfView) {
                            QScrollBar* hbar = ui->pdfView->horizontalScrollBar();
                            hbar->setValue(hbar->value() + (hbar->pageStep() * repeatCount));
                        }
                    }
                }
                break;
            }
        }
        file.close();

        if (!found) {
            qDebug() << "Gesture not found in map file:" << gestureType;
        }
    } else {
        qDebug() << "Could not open gesture map file at:" << gestureMapPath << "Error:" << file.errorString();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Presentation"), "", tr("PDF or PowerPoint Files (*.pdf *.pptx)"));

    if (filePath.isEmpty())
        return;

    QString pathToLoad = filePath;

    // Eğer PPTX ise PDF'e dönüştür
    if (filePath.endsWith(".pptx", Qt::CaseInsensitive)) {
        QString outputDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString fileNameOnly = QFileInfo(filePath).completeBaseName();

        QString sofficePath = "C:/Program Files/LibreOffice/program/soffice.exe";
        if (!QFile::exists(sofficePath)) {
            QMessageBox::warning(this, tr("LibreOffice Not Found"),
                tr("LibreOffice is required to convert PPTX files. Please install LibreOffice first."));
            return;
        }

        QProcess process;
        process.setWorkingDirectory(outputDir);

        QStringList arguments;
        arguments << "--headless"
                 << "--convert-to" << "pdf"
                 << "--outdir" << outputDir
                 << filePath;

        try {
            process.start("\"" + sofficePath + "\"", arguments);
            bool finished = process.waitForFinished(30000); // 30 second timeout

            if (!finished) {
                QMessageBox::warning(this, tr("Conversion Timeout"),
                    tr("PPTX to PDF conversion timed out. Please try again with a smaller file."));
                process.kill();
                return;
            }

            if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
                QString error = QString::fromUtf8(process.readAllStandardError());
                QMessageBox::warning(this, tr("Conversion Failed"),
                    tr("LibreOffice failed to convert PPTX to PDF.\nError: %1").arg(error));
                return;
            }

            // PDF çıktısını dosya adından tahmin et
            QDir dir(outputDir);
            QStringList pdfFiles = dir.entryList(QStringList() << (fileNameOnly + ".pdf"), QDir::Files, QDir::Time);
            if (!pdfFiles.isEmpty()) {
                QString newestPdf = dir.absoluteFilePath(pdfFiles.first());
                pathToLoad = newestPdf;
                qDebug() << "PDF bulundu:" << pathToLoad;
            } else {
                QMessageBox::warning(this, tr("Conversion Failed"), tr("Converted PDF file not found."));
                return;
            }
        } catch (const std::exception& e) {
            QMessageBox::warning(this, tr("Conversion Error"),
                tr("Error during PPTX conversion: %1").arg(e.what()));
            return;
        }
    }

    // PDF'i yüklemeye çalış
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool loaded = false;

    try {
        // Ensure PDF view is properly initialized
        if (!ui->pdfView) {
            throw std::runtime_error("PDF view not initialized");
        }

        // Load the presentation
        loaded = m_presentationManager->loadPresentation(pathToLoad);

        if (loaded) {
            // Add to recent presentations
            addRecentPresentation(pathToLoad);
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, tr("Loading Error"),
            tr("Error loading presentation: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::warning(this, tr("Loading Error"),
            tr("Unknown error occurred while loading presentation."));
    }

    QApplication::restoreOverrideCursor();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionConnect_triggered()
{
    QString serverUrl;
    if (ui->serverUrlEdit) {
        serverUrl = ui->serverUrlEdit->text();
    }

    if (serverUrl.isEmpty()) {
        QMessageBox::warning(this, tr("Connection Error"), tr("Please enter a server URL"));
        return;
    }
    ui->connectionStatusLabel->setText("Connecting...");
    ui->connectionStatusLabel->setStyleSheet("color: orange;");
    m_webSocketClient->connectToServer(serverUrl);
}

void MainWindow::on_actionDisconnect_triggered()
{
    m_webSocketClient->disconnect();
}

void MainWindow::on_nextButton_clicked()
{
    qDebug() << "NextButton";
    m_presentationManager->nextSlide(1);
}

void MainWindow::on_prevButton_clicked()
{
    m_presentationManager->previousSlide(1);
}

void MainWindow::on_zoomInButton_clicked()
{
    m_presentationManager->zoomIn();
}

void MainWindow::on_zoomOutButton_clicked()
{
    m_presentationManager->zoomOut();
}

void MainWindow::on_saveImageButton_clicked()
{
    // QLabel::pixmap() Qt 5'te QPixmap, Qt 6'da const QPixmap* döndürür.
    // Qt sürümünüze göre doğru kullanımı sağlamalısınız.
    // Hata mesajınız Qt 6 kullandığınızı ve pixmap() fonksiyonunun
    // doğrudan bir QPixmap nesnesi döndürdüğünü gösteriyor.
    // Bu durumda const QPixmap* p = ... satırı hatalıdır.

    // Doğru yaklaşım, eğer QLabel::pixmap() QPixmap döndürüyorsa:
    QPixmap pixmapToSave = ui->qrCodePreviewLabel->pixmap();

    // Eğer QLabel::pixmap() gerçekten const QPixmap* döndürüyorsa (Qt 6'nın bazı eski versiyonları veya özel durumlar),
    // o zaman önceki kodunuzdaki gibi bir kontrol ve dereference gerekirdi:
    // const QPixmap* p = ui->qrCodePreviewLabel->pixmap();
    // if (!p || p->isNull()) {
    //     QMessageBox::warning(this, "Error", "No QR code to save");
    //     return;
    // }
    // QPixmap pixmapToSave = *p;

    // Ancak hata mesajınız 'cannot convert QPixmap to const QPixmap*' dediği için,
    // ui->qrCodePreviewLabel->pixmap() bir QPixmap nesnesi döndürüyor demektir.
    // Bu yüzden doğrudan atama yapacağız ve null/isNull kontrolünü pixmapToSave üzerinde yapacağız.

    if (pixmapToSave.isNull()) {
        QMessageBox::warning(this, "Error", "No QR code to save");
        return;
    }

    QString defaultFileName = QString("QRCode_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString defaultPath = documentsPath + "/" + defaultFileName;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save QR Code"), defaultPath,
                                                    tr("PNG Images (*.png);;JPEG Images (*.jpg *.jpeg);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    if (!pixmapToSave.save(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to save QR code image to " + fileName + ".\nPlease check permissions and path.");
    } else {
        QMessageBox::information(this, "Success", "QR code image saved to " + fileName);
    }
}


void MainWindow::on_printButton_clicked()
{
    // QLabel::pixmap() Qt 6'da const QPixmap* yerine QPixmap döndürür (hata mesajınıza göre).
    QPixmap pixmapToPrint = ui->qrCodePreviewLabel->pixmap();

    if (pixmapToPrint.isNull()) {
        QMessageBox::warning(this, "Error", "No QR code to print");
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    // Qt 6'da QPageSize sınıfını kullanmak daha doğrudur.
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);

        // printer.pageRect() QRectF döndürür.
        QRectF pageRectF = printer.pageRect(QPrinter::DevicePixel);

        // QPixmap'i sayfa boyutuna göre orantılı olarak ölçeklendir
        QPixmap scaledPixmap = pixmapToPrint.scaled(pageRectF.size().toSize(), // QSizeF'i QSize'a dönüştür
                                                    Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation);

        // Ölçeklendirilmiş pixmap'i sayfada ortalamak için x ve y koordinatlarını hesapla
        qreal x = (pageRectF.width() - scaledPixmap.width()) / 2.0;
        qreal y = (pageRectF.height() - scaledPixmap.height()) / 2.0;

        // QPainter::drawPixmap tam sayı koordinatlar bekleyebilir, bu yüzden qRound kullanın.
        painter.drawPixmap(qRound(x), qRound(y), scaledPixmap);

        qDebug() << "Printing QR Code...";
    } else {
        qDebug() << "Print dialog cancelled.";
    }
}
void MainWindow::on_displayFullScreenButton_clicked()
{
    // Get QR code from REST API
    if (!m_currentRoomId.isEmpty()) {
        m_restApi->getQrCode(m_currentRoomId);
    } else {
        QMessageBox::warning(this, "Error", "No active room/session found.");
    }
    // Show full screen dialog with QR code
    // if (ui->stackedWidget) {
    //     ui->stackedWidget->setCurrentIndex(3); // Switch to QR Code view
    // }
    // showFullScreen();
    // if (ui->qrCodePreviewLabel) {
    //     ui->qrCodePreviewLabel->setPixmap(QPixmap()); // Clear previous QR code
    // }

}

void MainWindow::onQrCodeReceived(const QString &qrBase64)
{
    qDebug() << "onQrCodeReceived called with base64 data length:" << qrBase64.length();

    // Convert base64 to QPixmap
    QByteArray imageData = QByteArray::fromBase64(qrBase64.toUtf8());
    QPixmap pixmap;
    if (!pixmap.loadFromData(imageData)) {
        qDebug() << "ERROR: Failed to load QR code image from base64 data";
        QMessageBox::warning(this, "Error", "Failed to load QR code image");
        return;
    }

    qDebug() << "Successfully loaded QR code image, original size:" << pixmap.size();

    // Scale QR code to 300x300 while preserving aspect ratio
    QPixmap scaledPixmap = pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    qDebug() << "Scaled QR code to size:" << scaledPixmap.size();

    // Show QR code in preview label
    if (ui->qrCodePreviewLabel) {
        ui->qrCodePreviewLabel->setPixmap(scaledPixmap);
        ui->qrCodePreviewLabel->setScaledContents(false); // Let the label respect the pixmap's aspect ratio
        ui->qrCodePreviewLabel->setAlignment(Qt::AlignCenter); // Center the QR code in the label
        ui->qrCodePreviewLabel->show();
        qDebug() << "QR code set to preview label";
    } else {
        qDebug() << "ERROR: qrCodePreviewLabel is null!";
        return;
    }

    // Switch to QR code view if not already there
    if (ui->stackedWidget) {
        if (ui->stackedWidget->currentIndex() != 3) {
            ui->stackedWidget->setCurrentIndex(3); // QR code page index
            qDebug() << "Switched to QR code page";
        }
    } else {
        qDebug() << "ERROR: stackedWidget is null!";
    }
}

void MainWindow::addRecentPresentation(const QString &filePath)
{
    if (filePath.isEmpty()) return;
    m_recentPresentations.removeAll(filePath); // Remove if already exists
    m_recentPresentations.prepend(filePath); // Add to top
    if (m_recentPresentations.size() > 10) // Limit to 10
        m_recentPresentations = m_recentPresentations.mid(0, 10);
    // Update UI
    ui->recentFilesList->clear();
    for (const QString &path : m_recentPresentations) {
        QListWidgetItem *item = new QListWidgetItem(QFileInfo(path).fileName());
        item->setToolTip(path);
        item->setData(Qt::UserRole, path);
        ui->recentFilesList->addItem(item);
    }
}

void MainWindow::showFullScreen()
{
    QMainWindow::showFullScreen();
}

QPdfView* MainWindow::getPdfView() const
{
    // Find the PDF view widget in the UI
    QPdfView* pdfView = findChild<QPdfView*>();
    if (!pdfView) {
        qWarning() << "PDF view not found in MainWindow";
    }
    return pdfView;
}

// Add a method to update session info
void MainWindow::updateSessionInfo(const QString &sessionId, const QString &sessionName, const QString &sessionCode)
{
    m_sessionId = sessionId;
    m_sessionName = sessionName;
    m_sessionCode = sessionCode;

    // Update window title
    if (!m_sessionName.isEmpty()) {
        setWindowTitle(QString("AirClass Desktop - %1 (%2)").arg(m_sessionName, m_sessionCode));
    } else {
        setWindowTitle("AirClass Desktop");
    }

    // Update top bar with session info
    if (ui->topBar) {
        // Clear existing widgets in topBar
        QLayoutItem *child;
        while ((child = ui->horizontalLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        // Add session info with code
        QLabel *sessionLabel = new QLabel(QString("Session: %1 | Code: %2").arg(m_sessionName, m_sessionCode), ui->topBar);
        sessionLabel->setStyleSheet("color: #4a90e2; font-weight: bold; margin-right: 15px;");
        ui->horizontalLayout->addWidget(sessionLabel);

        // Add close session button right after session info
        QPushButton *closeSessionBtn = new QPushButton("Close Session", ui->topBar);
        closeSessionBtn->setObjectName("closeSessionButton");
        closeSessionBtn->setStyleSheet("QPushButton { color: #e74c3c; font-weight: bold; background: transparent; border: 1px solid #e74c3c; border-radius: 6px; padding: 6px 16px; } QPushButton:hover { background: #fbeee6; }");
        connect(closeSessionBtn, &QPushButton::clicked, this, &MainWindow::closeSession);
        ui->horizontalLayout->addWidget(closeSessionBtn);

        // Add user info
        QLabel *userLabel = new QLabel(QString("User: %1").arg(m_userName), ui->topBar);
        userLabel->setStyleSheet("color: #333; margin-right: 15px;");
        ui->horizontalLayout->addWidget(userLabel);

        // Add spacer
        ui->horizontalLayout->addStretch();

        // Re-add logout button
        QPushButton *logoutButton = new QPushButton("Log out", ui->topBar);
        logoutButton->setObjectName("logoutButton");
        logoutButton->setStyleSheet("QPushButton { color: #e74c3c; font-weight: bold; background: transparent; border: 1px solid #e74c3c; border-radius: 6px; padding: 6px 16px; } QPushButton:hover { background: #fbeee6; }");

        connect(logoutButton, &QPushButton::clicked, this, [this]() {
            // Hide all menus
            ui->menuFile->menuAction()->setVisible(false);
            ui->menuConnection->menuAction()->setVisible(false);
            ui->menuView->menuAction()->setVisible(false);

            // Clear auth token and session info
            m_authToken.clear();
            m_sessionId.clear();
            m_sessionName.clear();
            m_sessionCode.clear();
            m_currentRoomId.clear();
            // m_restApi->setAuthToken("");

            // Show login dialog
            showLoginDialog();
        });

        ui->horizontalLayout->addWidget(logoutButton);
    }

    // Store session info in settings
    QSettings settings("AirClass", "Desktop");
    settings.setValue("session_id", sessionId);
    settings.setValue("session_name", sessionName);
    settings.setValue("session_code", sessionCode);
}

void MainWindow::closeSession()
{
    if (!m_sessionId.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Close Session",
            "Are you sure you want to close the current session?",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            // First send the close session request
            m_restApi->closeSession(m_sessionId);

            // Disconnect from WebSocket server
            if (m_webSocketClient) {
                m_webSocketClient->disconnect();
            }

            // Clear session data
            m_sessionId.clear();
            m_sessionName.clear();
            m_sessionCode.clear();
            m_currentRoomId.clear();

            // Clear UI elements
            ui->requestListWidget->clear();
            if (ui->studentListWidget) {
                ui->studentListWidget->clear();
            }

            // Hide all menus
            ui->menuFile->menuAction()->setVisible(false);
            ui->menuConnection->menuAction()->setVisible(false);
            ui->menuView->menuAction()->setVisible(false);

            // Hide logout button
            if (QWidget *logoutButton = findChild<QWidget*>("logoutButton")) {
                logoutButton->setVisible(false);
            }

            // Show success message
            QMessageBox::information(this, "Session Closed", "Session has been closed successfully.");

            // Hide the main window
            this->hide();

            // Show session dialog
            SessionDialog *sessionDialog = new SessionDialog(m_restApi, this);
            connect(sessionDialog, &SessionDialog::sessionSelected,
                    this, &MainWindow::onSessionSelected);
            connect(sessionDialog, &SessionDialog::finished, this, [this](int result) {
                if (result == QDialog::Rejected) {
                    // If session dialog is closed without selecting a session, show login dialog
                    showLoginDialog();
                }
            });
            sessionDialog->exec();
        }
    }
}

void MainWindow::onSessionClosed(const QString &sessionId)
{
    if (sessionId == m_sessionId) {
        // Force a refresh of the classroom list to ensure we have the latest status
        m_restApi->listClassrooms();
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    // Clean up dynamically allocated objects
    if (m_webSocketClient) {
        delete m_webSocketClient;
        m_webSocketClient = nullptr;
    }

    if (m_presentationManager) {
        delete m_presentationManager;
        m_presentationManager = nullptr;
    }

    if (m_attendanceManager) {
        delete m_attendanceManager;
        m_attendanceManager = nullptr;
    }

    if (m_gestureProcessor) {
        delete m_gestureProcessor;
        m_gestureProcessor = nullptr;
    }

    if (m_uiController) {
        delete m_uiController;
        m_uiController = nullptr;
    }

    if (m_notificationPlayer) {
        delete m_notificationPlayer;
        m_notificationPlayer = nullptr;
    }

    // Note: Don't delete m_restApi as it's owned by QApplication
}

void MainWindow::setupDrawingControls()
{
    // Create drawing controls layout
    QHBoxLayout *drawingControlsLayout = new QHBoxLayout();
    drawingControlsLayout->setObjectName("drawingControlsLayout");

    // Common button style
    QString buttonStyle = R"(
        QPushButton {
            background-color: white;
            color: #333;
            border: 1px solid #e0e0e0;
            padding: 8px;
            border-radius: 8px;
            font-size: 18px;
            min-width: 44px;
            min-height: 44px;
            max-width: 44px;
            max-height: 44px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }
        QPushButton:hover {
            background-color: #f8f9fa;
            border-color: #bdbdbd;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15);
        }
        QPushButton:pressed {
            background-color: #f1f3f4;
            border-color: #9e9e9e;
            box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);
        }
    )";

    // Create clear button with emoji
    QPushButton *clearButton = new QPushButton("🗑️", this);
    clearButton->setObjectName("clearDrawingButton");
    clearButton->setToolTip("Clear Drawing");
    clearButton->setStyleSheet(buttonStyle);
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        if (m_drawingLayer) {
            m_drawingLayer->clearCurrentPage();
        }
    });
    drawingControlsLayout->addWidget(clearButton);

    // Create drawing toggle button with emoji
    QPushButton *drawButton = new QPushButton("✏️", this);
    drawButton->setObjectName("drawButton");
    drawButton->setToolTip("Toggle Drawing");
    drawButton->setCheckable(true);
    drawButton->setChecked(true);
    drawButton->setStyleSheet(buttonStyle + R"(
        QPushButton:checked {
            background-color: #e8f5e9;
            border-color: #66bb6a;
            color: #2e7d32;
        }
        QPushButton:checked:hover {
            background-color: #c8e6c9;
            border-color: #43a047;
        }
        QPushButton:checked:pressed {
            background-color: #a5d6a7;
            border-color: #2e7d32;
        }
    )");
    connect(drawButton, &QPushButton::clicked, this, [this, drawButton]() {
        if (m_drawingLayer) {
            m_drawingLayer->setDrawingEnabled(drawButton->isChecked());
        }
    });
    drawingControlsLayout->addWidget(drawButton);

    // Create color button with emoji
    QPushButton *colorButton = new QPushButton("🎨", this);
    colorButton->setObjectName("colorButton");
    colorButton->setToolTip("Select Color");
    colorButton->setStyleSheet(buttonStyle);
    connect(colorButton, &QPushButton::clicked, this, [this]() {
        if (m_drawingLayer) {
            QColor color = QColorDialog::getColor(m_drawingLayer->getPenColor(), this, "Select Pen Color");
            if (color.isValid()) {
                m_drawingLayer->setPenColor(color);
            }
        }
    });
    drawingControlsLayout->addWidget(colorButton);

    // Create pen width slider
    QSlider *widthSlider = new QSlider(Qt::Horizontal, this);
    widthSlider->setObjectName("penWidthSlider");
    widthSlider->setRange(1, 10);
    widthSlider->setValue(2);
    widthSlider->setFixedWidth(120);
    widthSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            border: 1px solid #e0e0e0;
            height: 6px;
            background: white;
            margin: 2px 0;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: white;
            border: 1px solid #bdbdbd;
            width: 16px;
            height: 16px;
            margin: -5px 0;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }
        QSlider::handle:horizontal:hover {
            background: #f8f9fa;
            border-color: #9e9e9e;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15);
        }
        QSlider::handle:horizontal:pressed {
            background: #f1f3f4;
            border-color: #757575;
            box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);
        }
    )");
    connect(widthSlider, &QSlider::valueChanged, this, [this](int value) {
        if (m_drawingLayer) {
            m_drawingLayer->setPenWidth(value);
        }
    });
    drawingControlsLayout->addWidget(widthSlider);

    // Add drawing controls to presentation layout
    if (ui->presentationLayout) {
        ui->presentationLayout->insertLayout(1, drawingControlsLayout);
    }
}

void MainWindow::on_clearDrawingButton_clicked()
{
    if (m_drawingLayer) {
        m_drawingLayer->clearCurrentPage();
    }
}

void MainWindow::on_colorButton_clicked()
{
    if (m_drawingLayer) {
        QColor color = QColorDialog::getColor(m_drawingLayer->getPenColor(), this, "Select Pen Color");
        if (color.isValid()) {
            m_drawingLayer->setPenColor(color);
        }
    }
}

// Add this method to handle PDF view resize events
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->pdfView && event->type() == QEvent::Resize) {
        if (m_drawingLayer) {
            m_drawingLayer->setGeometry(ui->pdfView->geometry());
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

// void MainWindow::on_timerStartButton_clicked()
// {
//     if (!m_isTimerRunning) {
//         // Set initial time if timer is not already set
//         if (m_remainingSeconds == 0) {
//             m_remainingSeconds = (ui->timerMinutesSpinBox->value() * 60) + ui->timerSecondsSpinBox->value();
//         }

//         if (m_remainingSeconds > 0) {
//             m_timer->start(1000); // Start timer with 1 second interval
//         m_isTimerRunning = true;
//         ui->timerStartButton->setEnabled(false);
//         ui->timerStopButton->setEnabled(true);
//             ui->timerMinutesSpinBox->setEnabled(false);
//             ui->timerSecondsSpinBox->setEnabled(false);
//         }

//         //
//     }
// }

void MainWindow::on_timerStopButton_clicked()
{
    if (m_isTimerRunning) {
        m_timer->stop();
        m_isTimerRunning = false;
        ui->timerStartButton->setEnabled(true);
        ui->timerStopButton->setEnabled(false);
        ui->timerMinutesSpinBox->setEnabled(true);
        ui->timerSecondsSpinBox->setEnabled(true);
    }
}

void MainWindow::on_timerResetButton_clicked()
{
    m_timer->stop();
    m_isTimerRunning = false;
    m_remainingSeconds = ui->timerMinutesSpinBox->value() * 60;
    updateTimerDisplay();
    ui->timerStartButton->setEnabled(true);
    ui->timerStopButton->setEnabled(false);
}

void MainWindow::on_timerMinutesSpinBox_valueChanged(int value)
{
    if (!m_isTimerRunning) {
        m_remainingSeconds = (value * 60) + ui->timerSecondsSpinBox->value();
        updateTimerDisplay();
    }
}

void MainWindow::on_timerSecondsSpinBox_valueChanged(int value)
{
    if (!m_isTimerRunning) {
        m_remainingSeconds = (ui->timerMinutesSpinBox->value() * 60) + value;
        updateTimerDisplay();
    }
}

void MainWindow::updateTimer()
{
    if (m_remainingSeconds > 0) {
        m_remainingSeconds--;
        updateTimerDisplay();

        if (m_remainingSeconds == 0) {
        m_timer->stop();
        m_isTimerRunning = false;
        ui->timerStartButton->setEnabled(true);
        ui->timerStopButton->setEnabled(false);
            ui->timerMinutesSpinBox->setEnabled(true);
            ui->timerSecondsSpinBox->setEnabled(true);
            QMessageBox::information(this, "Timer", "Süre doldu!");
        }
    }
}

void MainWindow::updateTimerDisplay()
{
    int minutes = m_remainingSeconds / 60;
    int seconds = m_remainingSeconds % 60;
    ui->timerDisplayLabel->setText(QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

// Add new method to handle timer dialog
void MainWindow::showTimerDialog()
{
    QDialog *timerDialog = new QDialog(this);
    timerDialog->setWindowTitle("Timer");
    timerDialog->setFixedSize(300, 250); // Increased height for reset button
    timerDialog->setStyleSheet("background-color: white;");

    QVBoxLayout *layout = new QVBoxLayout(timerDialog);

    // Time display
    QLabel *timeLabel = new QLabel("10:00", timerDialog);
    timeLabel->setStyleSheet("QLabel { font-size: 48px; font-weight: bold; color: #2c3e50; }");
    timeLabel->setAlignment(Qt::AlignCenter);

    // Control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *decreaseButton = new QPushButton("-", timerDialog);
    QPushButton *increaseButton = new QPushButton("+", timerDialog);
    QPushButton *startButton = new QPushButton("Start", timerDialog);
    QPushButton *resetButton = new QPushButton("Reset", timerDialog);

    // Set button sizes
    decreaseButton->setFixedSize(50, 50);
    increaseButton->setFixedSize(50, 50);
    startButton->setFixedSize(100, 40);
    resetButton->setFixedSize(100, 40);

    // Style for control buttons
    QString buttonStyle = R"(
        QPushButton {
            font-size: 24px;
            font-weight: bold;
            border: 2px solid #3498db;
            border-radius: 25px;
            background-color: white;
            color: #3498db;
        }
        QPushButton:hover {
            background-color: #3498db;
            color: white;
        }
    )";

    // Style for action buttons (start and reset)
    QString actionButtonStyle = R"(
        QPushButton {
            font-size: 14px;
            font-weight: bold;
            background-color: #2ecc71;
            color: white;
            border: none;
            border-radius: 5px;
        }
        QPushButton:hover {
            background-color: #27ae60;
        }
    )";

    // Style for reset button
    QString resetButtonStyle = R"(
        QPushButton {
            font-size: 14px;
            font-weight: bold;
            background-color: #95a5a6;
            color: white;
            border: none;
            border-radius: 5px;
        }
        QPushButton:hover {
            background-color: #7f8c8d;
        }
    )";

    decreaseButton->setStyleSheet(buttonStyle);
    increaseButton->setStyleSheet(buttonStyle);
    startButton->setStyleSheet(actionButtonStyle);
    resetButton->setStyleSheet(resetButtonStyle);

    buttonLayout->addStretch();
    buttonLayout->addWidget(decreaseButton);
    buttonLayout->addWidget(increaseButton);
    buttonLayout->addStretch();

    layout->addWidget(timeLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(startButton, 0, Qt::AlignCenter);
    layout->addWidget(resetButton, 0, Qt::AlignCenter);

    // Timer setup - using pointers to maintain state
    QTimer *timer = new QTimer(timerDialog);
    int *remainingSeconds = new int(600); // Başlangıç değeri 10 dakika
    bool *isRunning = new bool(false);
    int *initialSeconds = new int(600); // Başlangıç değerini saklamak için

    // Update display function
    auto updateDisplay = [=]() {
        int minutes = *remainingSeconds / 60;
        int secs = *remainingSeconds % 60;
        timeLabel->setText(QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0')));
    };

    // Connect buttons
    connect(decreaseButton, &QPushButton::clicked, [=]() {
        if (!*isRunning && *remainingSeconds >= 60) {  // Minimum 1 dakika
            *remainingSeconds -= 60;
            *initialSeconds = *remainingSeconds; // Sıfırlama için başlangıç değerini güncelle
            updateDisplay();
        }
    });

    connect(increaseButton, &QPushButton::clicked, [=]() {
        if (!*isRunning && *remainingSeconds < 3600) {  // Maximum 60 dakika
            *remainingSeconds += 60;
            *initialSeconds = *remainingSeconds; // Sıfırlama için başlangıç değerini güncelle
            updateDisplay();
        }
    });

    connect(resetButton, &QPushButton::clicked, [=]() {
        timer->stop();
        *isRunning = false;
        *remainingSeconds = *initialSeconds; // Son ayarlanan değere sıfırla
        startButton->setText("Start");
        startButton->setStyleSheet(actionButtonStyle);
        decreaseButton->setEnabled(true);
        increaseButton->setEnabled(true);
        updateDisplay();
    });

    connect(startButton, &QPushButton::clicked, [=]() {
        if (*isRunning) {
            timer->stop();
            *isRunning = false;
            startButton->setText("Start");
            startButton->setStyleSheet(actionButtonStyle);
            decreaseButton->setEnabled(true);
            increaseButton->setEnabled(true);
            resetButton->setEnabled(true);
        } else {
            if (*remainingSeconds > 0) {
                timer->start(1000);
                *isRunning = true;
                startButton->setText("Stop");
                decreaseButton->setEnabled(false);
                increaseButton->setEnabled(false);
                resetButton->setEnabled(false);
                startButton->setStyleSheet(R"(
                    QPushButton {
                        font-size: 14px;
                        font-weight: bold;
                        background-color: #e74c3c;
                        color: white;
                        border: none;
                        border-radius: 5px;
                    }
                    QPushButton:hover {
                        background-color: #c0392b;
                    }
                )");
            }
        }
    });

    connect(timer, &QTimer::timeout, [=]() {
        if (*remainingSeconds > 0) {
            *remainingSeconds -= 1;
            updateDisplay();

            if (*remainingSeconds == 0) {
                timer->stop();
                *isRunning = false;
                startButton->setText("Start");
                startButton->setStyleSheet(actionButtonStyle);
                decreaseButton->setEnabled(true);
                increaseButton->setEnabled(true);
                resetButton->setEnabled(true);
                QMessageBox::information(timerDialog, "Timer", "Süre doldu!");
            }
        }
    });

    // Initial display
    updateDisplay();

    // Cleanup when dialog is closed
    connect(timerDialog, &QDialog::finished, [=]() {
        timer->stop();
        delete remainingSeconds;
        delete isRunning;
        delete initialSeconds;
    });

    timerDialog->setLayout(layout);
    timerDialog->exec();
}

void MainWindow::on_actionDashboard_triggered()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_actionPresentation_triggered()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::on_actionStudents_triggered()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::on_actionGenerate_QR_Code_triggered()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(3);
    }
}

void MainWindow::on_connectButton_clicked()
{
    on_actionConnect_triggered();
}

void MainWindow::on_disconnectButton_clicked()
{
    on_actionDisconnect_triggered();
}

void MainWindow::on_openPresentationButton_clicked()
{
    on_actionOpen_triggered();
}

void MainWindow::on_approveButton_clicked()
{
    if (!m_currentRequestId.isEmpty()) {
        m_restApi->updateRequest(m_currentRoomId, m_currentRequestId, "approve");
        hideSpeakRequestNotification();
    }
}

void MainWindow::on_rejectButton_clicked()
{
    if (!m_currentRequestId.isEmpty()) {
        m_restApi->updateRequest(m_currentRoomId, m_currentRequestId, "reject");
        hideSpeakRequestNotification();
    }
}

void MainWindow::on_closeSessionButton_clicked()
{
    closeSession();
}

void MainWindow::on_actionGestureGuide_triggered()
{
    // Create and show gesture guide dialog
    QDialog *gestureGuideDialog = new QDialog(this);
    gestureGuideDialog->setWindowTitle("Gesture Guide");
    gestureGuideDialog->setMinimumWidth(600);
    gestureGuideDialog->setMinimumHeight(800);

    QVBoxLayout *layout = new QVBoxLayout(gestureGuideDialog);

    // Add title
    QLabel *titleLabel = new QLabel("Available Gestures", gestureGuideDialog);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Create scroll area for gestures
    QScrollArea *scrollArea = new QScrollArea(gestureGuideDialog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    QWidget *scrollContent = new QWidget(scrollArea);
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);

    // Required Gestures Section
    QLabel *requiredTitle = new QLabel("Fixed Gestures", scrollContent);
    requiredTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2980b9; margin-top: 10px;");
    scrollLayout->addWidget(requiredTitle);

    // Add fixed gestures
    addGestureItem(scrollLayout, "three_gun", "Next Slide", "Move to the next slide", "3_gun.png");
    addGestureItem(scrollLayout, "inv_three_gun", "Previous Slide", "Move to the previous slide", "inv_3_gun.png");
    addGestureItem(scrollLayout, "one", "Drawing Mode", "Enter drawing mode", "1.png");
    addGestureItem(scrollLayout, "two_up", "Point Mode", "Enter pointing mode", "2_up.png");
    addGestureItem(scrollLayout, "like", "Accept", "Accept current request", "like.png");
    addGestureItem(scrollLayout, "dislike", "Decline", "Decline current request", "dislike.png");
    addGestureItem(scrollLayout, "take_picture", "Attendance", "Take attendance", "take_picture.png");
    addGestureItem(scrollLayout, "palm", "Attention", "Request attention", "palm.png");
    addGestureItem(scrollLayout, "zoom_in", "Zoom In", "Zoom in presentation", "zoom_in.png");
    addGestureItem(scrollLayout, "zoom_out", "Zoom Out", "Zoom out presentation", "zoom_out.png");
    addGestureItem(scrollLayout, "zoom_reset", "Zoom Reset", "Reset zoom level", "zoom_reset.png");
    addGestureItem(scrollLayout, "up", "Scroll Up", "Scroll presentation up", "up.png");
    addGestureItem(scrollLayout, "down", "Scroll Down", "Scroll presentation down", "down.png");
    addGestureItem(scrollLayout, "left", "Scroll Left", "Scroll presentation left", "left.png");
    addGestureItem(scrollLayout, "right", "Scroll Right", "Scroll presentation right", "right.png");

    // Customizable Gestures Section
    QLabel *customTitle = new QLabel("Customizable Gestures", scrollContent);
    customTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2980b9; margin-top: 20px;");
    scrollLayout->addWidget(customTitle);

    // Add customizable gestures
    addGestureItem(scrollLayout, "timeout", "Break Timer", "Start break timer (default: 10 minutes)", "timeout.png");
    addGestureItem(scrollLayout, "call", "Call", "Make a call gesture", "call.png");
    addGestureItem(scrollLayout, "ok", "OK", "Show OK sign", "ok.png");
    addGestureItem(scrollLayout, "heart", "Heart", "Make a heart gesture", "heart.png");
    addGestureItem(scrollLayout, "heart2", "Heart 2", "Alternative heart gesture", "heart2.png");
    addGestureItem(scrollLayout, "mid_finger", "Middle", "Show middle finger", "mid_finger.png");
    addGestureItem(scrollLayout, "four", "Four", "Show four fingers", "4.png");
    addGestureItem(scrollLayout, "rock", "Break Timer 2", "Alternative break timer (default: 10 minutes)", "rock.png");
    addGestureItem(scrollLayout, "thumb_index", "Thumb Index", "Point with thumb and index", "thumb_index.png");
    addGestureItem(scrollLayout, "holy", "Holy", "Holy gesture", "holy.png");
    addGestureItem(scrollLayout, "three", "Three", "Show three fingers", "3.png");
    addGestureItem(scrollLayout, "three2", "Three 2", "Alternative three fingers", "3_2.png");

    // Add usage notes
    QLabel *notesLabel = new QLabel("Usage Notes:", scrollContent);
    notesLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #e74c3c; margin-top: 20px;");
    scrollLayout->addWidget(notesLabel);

    QLabel *notesContent = new QLabel(
        "• Fixed gestures always perform the same action\n"
        "• Customizable gestures can be configured with different actions\n"
        "• Break timer gestures (timeout, rock) default to 10 minutes\n"
        "• Drawing and pointing modes use normalized coordinates (0-1)\n"
        "• Some gestures may require specific hand positions", 
        scrollContent);
    notesContent->setStyleSheet("font-size: 14px; color: #7f8c8d; margin-left: 20px;");
    notesContent->setWordWrap(true);
    scrollLayout->addWidget(notesContent);

    // Add spacer at the bottom
    scrollLayout->addStretch();

    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea);

    // Add close button
    QPushButton *closeButton = new QPushButton("Close", gestureGuideDialog);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    connect(closeButton, &QPushButton::clicked, gestureGuideDialog, &QDialog::accept);
    layout->addWidget(closeButton, 0, Qt::AlignCenter);

    gestureGuideDialog->setLayout(layout);
    gestureGuideDialog->exec();
    delete gestureGuideDialog;
}

void MainWindow::addGestureItem(QVBoxLayout *layout, const QString &gesture, const QString &name, 
                               const QString &description, const QString &imagePath)
{
    QWidget *itemWidget = new QWidget();
    QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);

    // Add gesture image
    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(":/gestureGuidePictures/" + imagePath);
    if (!pixmap.isNull()) {
        pixmap = pixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(pixmap);
    } else {
        imageLabel->setText("[Image]");
    }
    imageLabel->setFixedSize(60, 60);
    itemLayout->addWidget(imageLabel);

    // Add gesture info
    QVBoxLayout *infoLayout = new QVBoxLayout();
    
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50;");
    infoLayout->addWidget(nameLabel);

    QLabel *gestureLabel = new QLabel(gesture);
    gestureLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; font-family: monospace;");
    infoLayout->addWidget(gestureLabel);

    QLabel *descLabel = new QLabel(description);
    descLabel->setStyleSheet("font-size: 14px; color: #34495e;");
    infoLayout->addWidget(descLabel);

    itemLayout->addLayout(infoLayout);
    itemLayout->addStretch();

    // Style the item widget
    itemWidget->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 5px;"
        "   margin: 5px;"
        "   padding: 10px;"
        "}"
        "QWidget:hover {"
        "   background-color: #f5f6fa;"
        "   border-color: #3498db;"
        "}"
    );

    layout->addWidget(itemWidget);
}

void MainWindow::onSessionSelected(const QString &sessionId, const QString &sessionName, const QString &sessionCode)
{
    m_sessionId = sessionId;
    m_sessionName = sessionName;
    m_sessionCode = sessionCode;

    // Update window title and UI
    updateSessionInfo(sessionId, sessionName, sessionCode);

    // Show all menus
    ui->menuFile->menuAction()->setVisible(true);
    ui->menuConnection->menuAction()->setVisible(true);
    ui->menuView->menuAction()->setVisible(true);

    // Show logout button
    if (QWidget *logoutButton = findChild<QWidget*>("logoutButton")) {
        logoutButton->setVisible(true);
    }

    // Connect to WebSocket server
    m_webSocketClient->connectToServer("ws://localhost:8082");

    // Show the main window
    this->show();

    // Initialize the UI and setup connections
    initialize();
    setupConnections();

    // Get requests once instead of continuous polling
    m_restApi->listRequests(m_currentRoomId);
}


void MainWindow::on_actionTimeout_triggered()
{
    showTimerDialog();
}

void MainWindow::startBreakTimer(int minutes)
{
    m_remainingBreakSeconds = minutes * 60;

    // Create a more visually appealing timer label
    m_breakTimerLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 36px;"
        "   font-weight: bold;"
        "   color: white;"
        "   background-color: rgba(46, 204, 113, 0.9);"
        "   border-radius: 15px;"
        "   padding: 15px 30px;"
        "   margin: 10px;"
        "}"
    );
    m_breakTimerLabel->setAlignment(Qt::AlignCenter);
    
    // Position the timer at the top center of the window
    QSize labelSize(300, 80);
    m_breakTimerLabel->setFixedSize(labelSize);
    m_breakTimerLabel->move((width() - labelSize.width()) / 2, 20);
    m_breakTimerLabel->raise();
    m_breakTimerLabel->show();

    // Start the timer
    if (!m_breakTimer) {
        m_breakTimer = new QTimer(this);
        connect(m_breakTimer, &QTimer::timeout, this, &MainWindow::onBreakTimerTick);
    }
    m_breakTimer->start(1000);
    
    // Update display
    updateBreakTimerDisplay();
}

void MainWindow::onBreakTimerTick()
{
    if (m_remainingBreakSeconds > 0) {
        m_remainingBreakSeconds--;
        updateBreakTimerDisplay();
    } else {
        m_breakTimer->stop();
        m_breakTimerLabel->hide();
        QMessageBox::information(this, "Break Time", "Break time is over!");
    }
}

void MainWindow::updateBreakTimerDisplay()
{
    int minutes = m_remainingBreakSeconds / 60;
    int seconds = m_remainingBreakSeconds % 60;
    m_breakTimerLabel->setText(QString("Break Time: %1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::on_timerStartButton_clicked()
{
    if (!m_isTimerRunning) {
        int minutes = ui->timerMinutesSpinBox->value();
        int seconds = ui->timerSecondsSpinBox->value();
        m_remainingSeconds = (minutes * 60) + seconds;

        if (m_remainingSeconds <= 0) {
            QMessageBox::warning(this, "Timer Error", "Please set a duration greater than 0 seconds.");
            return;
        }

        qDebug() << "[MainWindow] Timer başlatıldı. Süre:" << m_remainingSeconds << "saniye. QR Kod yoklama isteği gönderiliyor.";

        if (m_restApi && !m_currentRoomId.isEmpty()) {
            // classroomIdLong ve conversionOk ile ilgili satırlar kaldırıldı,
            // çünkü RestApiClient::generateAttendanceCode QString alıyor ve
            // kendi içinde toLongLong dönüşümünü yapıyor.
            m_restApi->generateAttendanceCode(m_currentRoomId, m_remainingSeconds);
        } else {
            QMessageBox::warning(this, "API Error", "Cannot generate attendance code. REST API client or Room ID is not available.");
            return;
        }

        // Timer'ı ve UI butonlarını API'den başarılı yanıt (onAttendanceCodeReady) geldikten sonra
        // başlatmak/güncellemek daha iyi bir yaklaşımdır.
        // Şimdilik burada bırakıyorum, API yanıtını beklemek daha sağlam olur.
        m_timer->start(1000);
        m_isTimerRunning = true;
        ui->timerStartButton->setEnabled(false);
        ui->timerStopButton->setEnabled(true);
        ui->timerMinutesSpinBox->setEnabled(false);
        ui->timerSecondsSpinBox->setEnabled(false);
    }
}

void MainWindow::onAttendanceCodeReady(const QString &code, const QString &expiresAt, const QString &expiresIn)
{
    qDebug() << "[MainWindow] Yoklama kodu alındı:" << code << "Geçerlilik:" << expiresAt << "(" << expiresIn << ")";

    QString qrDataUrl = QString("https://api.qrserver.com/v1/create-qr-code/?size=400x400&data=%1").arg(code);

    // "Most vexing parse" sorununu çözmek için süslü parantez veya çift parantez kullanın:
    QNetworkRequest qrRequest{QUrl(qrDataUrl)}; // TERCİH EDİLEN YÖNTEM (Uniform Initialization)
    // VEYA:
    // QNetworkRequest qrRequest((QUrl(qrDataUrl))); // Alternatif yöntem

    qDebug() << "[MainWindow] QR kod resmi indiriliyor:" << qrDataUrl;
    m_qrNetworkManager->get(qrRequest); // Şimdi qrRequest bir QNetworkRequest nesnesi olarak algılanacak

    QMessageBox::information(this, "Attendance Code", QString("Code: %1\nExpires: %2").arg(code, expiresAt));

    if (ui->stackedWidget && ui->qrCodeView) {
        ui->stackedWidget->setCurrentWidget(ui->qrCodeView);
    } else {
        qWarning() << "[MainWindow] qrCodeView widget'ı bulunamadı veya stackedWidget null!";
    }
}
void MainWindow::onAttendanceCodeFailed(const QString &errorMsg)
{
    qWarning() << "[MainWindow] Yoklama kodu oluşturma başarısız:" << errorMsg;
    QMessageBox::critical(this, "Attendance Code Error", "Failed to generate attendance code: " + errorMsg);

    // Timer'ı durdur ve UI'ı sıfırla (eğer on_timerStartButton_clicked içinde başlatıldıysa)
    if (m_isTimerRunning) {
        m_timer->stop();
        m_isTimerRunning = false;
        ui->timerStartButton->setEnabled(true);
        ui->timerStopButton->setEnabled(false);
        ui->timerMinutesSpinBox->setEnabled(true);
        ui->timerSecondsSpinBox->setEnabled(true);
    }
}

void MainWindow::onQrCodeImageDownloaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            qDebug() << "[MainWindow] QR kod resmi başarıyla indirildi ve yüklendi.";
            if (ui->qrCodePreviewLabel) {
                // QLabel'in boyutunu koruyarak resmi sığdır
                ui->qrCodePreviewLabel->setPixmap(pixmap.scaled(
                    ui->qrCodePreviewLabel->size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    ));
            } else {
                qWarning() << "[MainWindow] qrCodePreviewLabel bulunamadı!";
            }
        } else {
            qWarning() << "[MainWindow] İndirilen QR kod resmi QPixmap'e yüklenemedi.";
            QMessageBox::warning(this, "QR Code Error", "Failed to load QR code image data.");
            if (ui->qrCodePreviewLabel) {
                ui->qrCodePreviewLabel->setText("Failed to load QR image.");
            }
        }
    } else {
        qWarning() << "[MainWindow] QR kod resmi indirme hatası:" << reply->errorString();
        QMessageBox::warning(this, "QR Code Error", "Failed to download QR code image: " + reply->errorString());
        if (ui->qrCodePreviewLabel) {
            ui->qrCodePreviewLabel->setText("Error downloading QR.");
        }
    }
    reply->deleteLater();
}
