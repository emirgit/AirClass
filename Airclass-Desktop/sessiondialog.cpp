#include "sessiondialog.h"
#include <QMessageBox>
#include <QJsonValue>
#include <QDebug>

SessionDialog::SessionDialog(RestApiClient* restApi, QWidget *parent)
    : QDialog(parent)
    , m_restApi(restApi)
    , m_hasActiveSession(false)
{
    setWindowTitle("Session Selection");
    setModal(true);
    setMinimumSize(640, 640);
    
    // Set style
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QListWidget {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 5px;
            font-size: 14px;
            color: black;
        }
        QListWidget::item {
            padding: 10px;
            margin: 2px 0;
            border-radius: 4px;
            color: black;
        }
        QListWidget::item:selected {
            background-color: #4a90e2;
            color: white;
        }
        QListWidget::item:hover {
            background-color: #e3f2fd;
            color: black;
        }
        QPushButton {
            padding: 10px 20px;
            background-color: #4a90e2;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            min-height: 40px;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:disabled {
            background-color: #cccccc;
        }
        QPushButton#secondary {
            background-color: #6c757d;
        }
        QPushButton#secondary:hover {
            background-color: #5a6268;
        }
        QLineEdit {
            padding: 10px;
            border: 2px solid #e0e0e0;
            border-radius: 4px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #4a90e2;
        }
        QLabel#title {
            font-size: 24px;
            font-weight: bold;
            color: #333;
            margin: 10px 0;
        }
        QLabel#subtitle {
            font-size: 16px;
            color: #666;
            margin: 5px 0;
        }
        QLabel#activeSession {
            font-size: 18px;
            font-weight: bold;
            color: #4a90e2;
            padding: 20px;
            background-color: #e3f2fd;
            border-radius: 8px;
            margin: 20px 0;
        }
    )");
    
    // Connect signals
    connect(m_restApi, &RestApiClient::activeSessionReceived, this, &SessionDialog::onActiveSessionReceived);
    connect(m_restApi, &RestApiClient::sessionsReceived, this, &SessionDialog::onSessionsReceived);
    connect(m_restApi, &RestApiClient::sessionCreated, this, &SessionDialog::onSessionCreated);
    connect(m_restApi, &RestApiClient::sessionActivated, this, &SessionDialog::onSessionActivated);
    connect(m_restApi, &RestApiClient::error, this, &SessionDialog::onError);
    
    // Setup UI
    m_stackedWidget = new QStackedWidget(this);
    
    setupMainPage();
    setupSessionListPage();
    setupCreateSessionPage();
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_stackedWidget);
    
    // Check for active session on startup
    checkActiveSession();
}

SessionDialog::~SessionDialog()
{
    if (m_restApi) {
        disconnect(m_restApi, nullptr, this, nullptr);
    }
}

void SessionDialog::setupMainPage()
{
    m_mainPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_mainPage);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);
    
    QLabel *titleLabel = new QLabel("Session Management");
    titleLabel->setObjectName("title");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    m_activeSessionLabel = new QLabel("Checking for active session...");
    m_activeSessionLabel->setObjectName("activeSession");
    m_activeSessionLabel->setAlignment(Qt::AlignCenter);
    m_activeSessionLabel->setWordWrap(true);
    
    m_continueButton = new QPushButton("Continue with Current Session");
    m_continueButton->setEnabled(false);
    connect(m_continueButton, &QPushButton::clicked, this, &SessionDialog::onContinueClicked);
    
    QLabel *orLabel = new QLabel("— OR —");
    orLabel->setAlignment(Qt::AlignCenter);
    orLabel->setStyleSheet("color: #999; margin: 10px 0;");
    
    m_createNewButton = new QPushButton("Create New Session");
    connect(m_createNewButton, &QPushButton::clicked, this, &SessionDialog::onCreateNewClicked);
    
    m_selectExistingButton = new QPushButton("Select from Existing Sessions");
    m_selectExistingButton->setObjectName("secondary");
    connect(m_selectExistingButton, &QPushButton::clicked, this, &SessionDialog::onSelectExistingClicked);
    
    layout->addWidget(titleLabel);
    layout->addWidget(m_activeSessionLabel);
    layout->addWidget(m_continueButton);
    layout->addWidget(orLabel);
    layout->addWidget(m_createNewButton);
    layout->addWidget(m_selectExistingButton);
    layout->addStretch();
    
    m_stackedWidget->addWidget(m_mainPage);
}

void SessionDialog::setupSessionListPage()
{
    m_sessionListPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_sessionListPage);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Create a container widget for better styling
    QWidget *container = new QWidget;
    container->setObjectName("sessionListContainer");
    container->setStyleSheet(R"(
        QWidget#sessionListContainer {
            background-color: white;
            border-radius: 10px;
            padding: 20px;
        }
    )");
    
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(15);
    
    // Header section with icon
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon(":/icons/session.png").pixmap(32, 32));
    QLabel *titleLabel = new QLabel("Select a Session");
    titleLabel->setObjectName("title");
    titleLabel->setStyleSheet(R"(
        QLabel#title {
            font-size: 24px;
            font-weight: bold;
            color: #2c3e50;
            margin: 0;
        }
    )");
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    // Description
    QLabel *subtitleLabel = new QLabel("Choose a session from the list below to activate");
    subtitleLabel->setStyleSheet(R"(
        QLabel {
            color: #666;
            font-size: 14px;
            margin: 5px 0;
        }
    )");
    subtitleLabel->setWordWrap(true);
    
    // Session list with custom styling
    m_sessionListWidget = new QListWidget;
    m_sessionListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sessionListWidget->setStyleSheet(R"(
        QListWidget {
            background-color: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 5px;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 15px;
            margin: 5px;
            border-radius: 6px;
            background-color: #f8f9fa;
            border: 1px solid #e0e0e0;
        }
        QListWidget::item:hover {
            background-color: #e3f2fd;
            border: 1px solid #3498db;
        }
        QListWidget::item:selected {
            background-color: #3498db;
            color: white;
            border: 1px solid #2980b9;
        }
    )");
    
    connect(m_sessionListWidget, &QListWidget::itemClicked, this, &SessionDialog::onSessionListItemClicked);
    connect(m_sessionListWidget, &QListWidget::itemDoubleClicked, 
            [this](QListWidgetItem *) { onContinueClicked(); });
    
    // Empty state message
    QLabel *emptyLabel = new QLabel("No sessions found. Create a new one!");
    emptyLabel->setStyleSheet(R"(
        QLabel {
            color: #666;
            font-size: 14px;
            padding: 20px;
            text-align: center;
        }
    )");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->hide();
    
    // Buttons with improved styling
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    m_backButton = new QPushButton("Back");
    m_backButton->setObjectName("secondary");
    m_backButton->setStyleSheet(R"(
        QPushButton {
            padding: 10px 20px;
            background-color: #95a5a6;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #7f8c8d;
        }
    )");
    
    m_refreshButton = new QPushButton("Refresh");
    m_refreshButton->setObjectName("secondary");
    m_refreshButton->setStyleSheet(R"(
        QPushButton {
            padding: 10px 20px;
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
    )");
    
    m_selectButton = new QPushButton("Select Session");
    m_selectButton->setEnabled(false);
    m_selectButton->setStyleSheet(R"(
        QPushButton {
            padding: 10px 20px;
            background-color: #2ecc71;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            min-width: 120px;
        }
        QPushButton:hover {
            background-color: #27ae60;
        }
        QPushButton:disabled {
            background-color: #bdc3c7;
        }
    )");
    
    connect(m_backButton, &QPushButton::clicked, this, &SessionDialog::onBackClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &SessionDialog::onRefreshClicked);
    connect(m_selectButton, &QPushButton::clicked, this, &SessionDialog::onContinueClicked);
    
    buttonLayout->addWidget(m_backButton);
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_selectButton);
    
    // Add widgets to container layout
    containerLayout->addLayout(headerLayout);
    containerLayout->addWidget(subtitleLabel);
    containerLayout->addWidget(m_sessionListWidget);
    containerLayout->addWidget(emptyLabel);
    containerLayout->addLayout(buttonLayout);
    
    // Add container to main layout
    layout->addWidget(container);
    
    m_stackedWidget->addWidget(m_sessionListPage);
}

void SessionDialog::setupCreateSessionPage()
{
    m_createSessionPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_createSessionPage);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);
    
    // Create a container widget for better styling
    QWidget *formContainer = new QWidget;
    formContainer->setObjectName("formContainer");
    formContainer->setStyleSheet(R"(
        QWidget#formContainer {
            background-color: white;
            border-radius: 10px;
            padding: 20px;
            min-width: 400px;
            max-width: 500px;
        }
    )");
    
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(15);
    
    // Title with icon
    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon(":/icons/session.png").pixmap(32, 32));
    QLabel *titleLabel = new QLabel("Create New Session");
    titleLabel->setObjectName("title");
    titleLabel->setStyleSheet(R"(
        QLabel#title {
            font-size: 24px;
            font-weight: bold;
            color: #2c3e50;
            margin: 0;
        }
    )");
    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    
    // Description
    QLabel *descriptionLabel = new QLabel("Enter a name for your new session. This will help you identify it later.");
    descriptionLabel->setStyleSheet(R"(
        QLabel {
            color: #666;
            font-size: 14px;
            margin: 5px 0;
        }
    )");
    descriptionLabel->setWordWrap(true);
    
    // Session name input
    QLabel *nameLabel = new QLabel("Session Name:");
    nameLabel->setStyleSheet(R"(
        QLabel {
            color: #2c3e50;
            font-size: 14px;
            font-weight: bold;
            margin-top: 10px;
        }
    )");
    
    m_sessionNameEdit = new QLineEdit;
    m_sessionNameEdit->setPlaceholderText("e.g., Computer Science 101");
    m_sessionNameEdit->setStyleSheet(R"(
        QLineEdit {
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            font-size: 14px;
            background-color: white;
            min-height: 20px;
            color: black;
        }
        QLineEdit:focus {
            border-color: #3498db;
        }
    )");
    
    // Tips section
    QLabel *tipsLabel = new QLabel("💡 Tips:");
    tipsLabel->setStyleSheet("font-weight: bold; color: #2c3e50; margin-top: 10px;");
    
    QLabel *tip1 = new QLabel("• Use a descriptive name that reflects the session's purpose");
    QLabel *tip2 = new QLabel("• Include course code or subject if applicable");
    QLabel *tip3 = new QLabel("• Keep it concise but informative");
    
    QString tipStyle = "color: #666; margin-left: 20px;";
    tip1->setStyleSheet(tipStyle);
    tip2->setStyleSheet(tipStyle);
    tip3->setStyleSheet(tipStyle);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    m_backFromCreateButton = new QPushButton("Back");
    m_backFromCreateButton->setObjectName("secondary");
    m_backFromCreateButton->setStyleSheet(R"(
        QPushButton {
            padding: 10px 20px;
            background-color: #95a5a6;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #7f8c8d;
        }
    )");
    
    m_createButton = new QPushButton("Create Session");
    m_createButton->setStyleSheet(R"(
        QPushButton {
            padding: 10px 20px;
            background-color: #2ecc71;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            min-width: 120px;
        }
        QPushButton:hover {
            background-color: #27ae60;
        }
    )");
    
    buttonLayout->addWidget(m_backFromCreateButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_createButton);
    
    // Add widgets to form layout
    formLayout->addLayout(titleLayout);
    formLayout->addWidget(descriptionLabel);
    formLayout->addWidget(nameLabel);
    formLayout->addWidget(m_sessionNameEdit);
    formLayout->addWidget(tipsLabel);
    formLayout->addWidget(tip1);
    formLayout->addWidget(tip2);
    formLayout->addWidget(tip3);
    formLayout->addStretch();
    formLayout->addLayout(buttonLayout);
    
    // Add form container to main layout
    layout->addWidget(formContainer);
    layout->addStretch();
    
    // Connect signals
    connect(m_backFromCreateButton, &QPushButton::clicked, this, &SessionDialog::onBackClicked);
    connect(m_createButton, &QPushButton::clicked, this, &SessionDialog::onCreateSessionClicked);
    
    m_stackedWidget->addWidget(m_createSessionPage);
}

void SessionDialog::checkActiveSession()
{
    m_activeSessionLabel->setText("Checking for active session...");
    m_restApi->getActiveSession(); // Bu, listSessions'ı tetikleyecek ve her iki sinyali de gönderecek
}

void SessionDialog::onActiveSessionReceived(const QJsonObject &session)
{
    /*
    {
  "status": true,
  "message": "Active classrooms retrieved successfully",
  "data": [
    {
      "id": 7,
      "teacher_id": 10,
      "code": "251632",
      "ip": "198.51.100.42",
      "port": 65535,
      "status": 1,
      "created_at": "2025-05-28 21:50:51",
      "updated_at": "2025-05-28 21:50:51",
      "teacher_name": "a",
      "attendance_count": 0
    }
  ]
} here is example response 
    */

    // API'den gelen yanıtta 'data' bir dizi olduğu için, RestApiClient'te
    // bu dizi içinden aktif olan bulunup tek bir obje olarak bu slota gönderiliyor.
    if (session.isEmpty() || session.value("status").toInt() != 1) {
        m_hasActiveSession = false;
        m_activeSessionLabel->setText("No active session found.\nPlease create a new session or select an existing one.");
        qDebug() << "No active session found.";
        m_continueButton->setEnabled(false);
    } else {
        qDebug() << "Active session found:" << session;
        m_hasActiveSession = true;
        m_selectedSessionId = QString::number(session["id"].toInt()); // ID integer olabilir
        m_selectedSessionCode = session["code"].toString();
        m_selectedSessionName = session["name"].toString(); // API response'unda 'name' alanı olmalı

        m_activeSessionLabel->setText(QString("Active Session:\n%1 - %2")
                                   .arg(m_selectedSessionName) // Önce isim daha okunaklı
                                   .arg(m_selectedSessionCode));
        m_continueButton->setEnabled(true);
    }

    // if (session.isEmpty()) {
    //     m_hasActiveSession = false;
    //     m_activeSessionLabel->setText("No active session found.\nPlease create a new session or select an existing one.");
    //     m_continueButton->setEnabled(false);
    // } else {
    //     m_hasActiveSession = true;
    //     m_selectedSessionId = session["id"].toString();
    //     m_selectedSessionCode = session["code"].toString();
    //     m_selectedSessionName = session["name"].toString();
        
    //     m_activeSessionLabel->setText(QString("Active Session:\n%1 - %2")
    //         .arg(m_selectedSessionCode)
    //         .arg(m_selectedSessionName));
    //     m_continueButton->setEnabled(true);
    // }
}

void SessionDialog::onSessionsReceived(const QJsonArray &sessions)
{
    m_sessions = sessions;
    populateSessionList(sessions);
}

void SessionDialog::populateSessionList(const QJsonArray &sessions)
{
    m_sessionListWidget->clear();
    m_selectButton->setEnabled(false);

    
    if (sessions.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("No sessions found. Create a new one!");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setTextAlignment(Qt::AlignCenter);
        m_sessionListWidget->addItem(item);
        return;
    }

    for (const QJsonValue &value : sessions) {
        QJsonObject session = value.toObject();
        QString code = session["code"].toString();
        QString name = session["name"].toString("Unnamed Session"); // name alanı yoksa varsayılan
        int status = session["status"].toInt();
        int studentCount = session["attendance_count"].toInt(); // OpenAPI'ye göre 'attendance_count'

        QString displayText = QString("%1 - %2\n%3 students")
                                  .arg(name)
                                  .arg(code)
                                  .arg(studentCount);

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, QString::number(session["id"].toInt()));
        item->setData(Qt::UserRole + 1, code);
        item->setData(Qt::UserRole + 2, name);

        if (status == 1) {
            item->setText(displayText + "\n[ACTIVE]");
            item->setBackground(QBrush(QColor("#e3f2fd")));
            item->setForeground(QBrush(QColor("#2c3e50")));
        } else {
            item->setText(displayText);
        }

        m_sessionListWidget->addItem(item);
    }
}

void SessionDialog::onSessionCreated(const QJsonObject &sessionDataFromApi)
{
    // Try both classroom_id and id fields
    int id = sessionDataFromApi.value("id").toInt();

    m_selectedSessionId = QString::number(id);
    m_selectedSessionCode = sessionDataFromApi.value("code").toString();
    m_selectedSessionName = m_sessionNameEdit->text().trimmed(); // Use the name entered by user

    if (id <= 0 || m_selectedSessionCode.isEmpty()) {
        QMessageBox::warning(this, "Creation Error", 
                           "Failed to retrieve valid ID or code for the created session.\n"
                           "Please try again or contact support if the problem persists.");
        return;
    }

    QMessageBox::information(this, "Success",
        QString("Session '%1' created successfully!\nSession Code: %2")
        .arg(m_selectedSessionName, m_selectedSessionCode));

    emit sessionSelected(m_selectedSessionId, m_selectedSessionName, m_selectedSessionCode);
    accept(); // This will close the dialog
}


void SessionDialog::onSessionActivated(const QJsonObject &session)
{
    m_selectedSessionId = QString::number(session["id"].toInt());
    m_selectedSessionCode = session["code"].toString();
    m_selectedSessionName = session["name"].toString();

    emit sessionSelected(m_selectedSessionId, m_selectedSessionName, m_selectedSessionCode);
    accept();
}

void SessionDialog::onError(const QString &message)
{
    QMessageBox::warning(this, "Error", message);
}

void SessionDialog::onCreateNewClicked()
{
    m_stackedWidget->setCurrentWidget(m_createSessionPage);
    m_sessionNameEdit->clear();
    m_sessionNameEdit->setFocus();
}

void SessionDialog::onSelectExistingClicked()
{
    m_stackedWidget->setCurrentWidget(m_sessionListPage);
    m_restApi->listSessions();
}

void SessionDialog::onBackClicked()
{
    m_stackedWidget->setCurrentWidget(m_mainPage);
}

void SessionDialog::onContinueClicked()
{
    if (m_stackedWidget->currentWidget() == m_mainPage && m_hasActiveSession) {
        emit sessionSelected(m_selectedSessionId, m_selectedSessionName, m_selectedSessionCode);
        accept();
    } else if (m_stackedWidget->currentWidget() == m_sessionListPage) {
        QListWidgetItem *currentItem = m_sessionListWidget->currentItem();
        if (currentItem && (currentItem->flags() & Qt::ItemIsSelectable)) {
            QString sessionId = currentItem->data(Qt::UserRole).toString();
            m_restApi->activateSession(sessionId); // Bu metot artık doğru çalışıyor
        }
    }
}

void SessionDialog::onSessionListItemClicked(QListWidgetItem *item)
{
    if (item && item->flags() & Qt::ItemIsSelectable) {
        m_selectButton->setEnabled(true);
    }
}

void SessionDialog::onCreateSessionClicked()
{
    QString sessionName = m_sessionNameEdit->text().trimmed();
    if (sessionName.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a session name.");
        return;
    }
    // API'nin beklediği IP ve Port değerlerini gönderin.
    // Şimdilik varsayılan değerler kullanıyoruz, bunlar yapılandırılabilir olmalı.
    m_restApi->createSession(sessionName, "127.0.0.1", 1234);
}

void SessionDialog::onRefreshClicked()
{
    m_restApi->listSessions();
}
