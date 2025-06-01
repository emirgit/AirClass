/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtPdfWidgets/QPdfView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionExit;
    QAction *actionConnect;
    QAction *actionDisconnect;
    QAction *actionDashboard;
    QAction *actionPresentation;
    QAction *actionStudents;
    QAction *actionGenerate_QR_Code;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QWidget *topBar;
    QHBoxLayout *horizontalLayout;
    QLabel *connectionStatusLabel;
    QSpacerItem *horizontalSpacer;
    QLabel *attendanceCountLabel;
    QSpacerItem *horizontalSpacer_2;
    QLabel *pageIndicatorLabel;
    QLabel *notificationLabel;
    QStackedWidget *stackedWidget;
    QWidget *dashboardView;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QGridLayout *gridLayout;
    QGroupBox *serverGroupBox;
    QFormLayout *formLayout;
    QLabel *label_2;
    QLineEdit *serverUrlEdit;
    QLabel *label_3;
    QLineEdit *roomIdEdit;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QGroupBox *recentPresentationsGroupBox;
    QVBoxLayout *verticalLayout_3;
    QListWidget *recentFilesList;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *openPresentationButton;
    QGroupBox *classInfoGroupBox;
    QFormLayout *formLayout_2;
    QLabel *label_4;
    QLineEdit *courseNameEdit;
    QLabel *label_5;
    QLineEdit *instructorNameEdit;
    QGroupBox *gesturesGroupBox;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_6;
    QListWidget *gestureListWidget;
    QWidget *presentationView;
    QVBoxLayout *presentationLayout;
    QPdfView *pdfView;
    QHBoxLayout *presentationControlsLayout;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *zoomOutButton;
    QPushButton *zoomInButton;
    QWidget *studentsView;
    QVBoxLayout *studentsLayout;
    QLabel *studentsLabel;
    QHBoxLayout *studentsListLayout;
    QGroupBox *attendanceGroupBox;
    QVBoxLayout *verticalLayout_5;
    QListWidget *studentListWidget;
    QGroupBox *requestsGroupBox;
    QVBoxLayout *verticalLayout_6;
    QListWidget *requestListWidget;
    QHBoxLayout *requestButtonsLayout;
    QPushButton *approveButton;
    QPushButton *rejectButton;
    QWidget *qrCodeView;
    QVBoxLayout *qrCodeLayout;
    QLabel *qrCodeLabel;
    QHBoxLayout *qrCodeMainLayout;
    QVBoxLayout *qrCodeOptionsLayout;
    QGroupBox *qrContentGroupBox;
    QVBoxLayout *qrContentLayout;
    QRadioButton *sessionRadioButton;
    QRadioButton *customRadioButton;
    QTextEdit *customContentTextEdit;
    QGroupBox *qrSettingsGroupBox;
    QFormLayout *qrSettingsLayout;
    QLabel *sizeLevelLabel;
    QComboBox *sizeComboBox;
    QLabel *errorLevelLabel;
    QComboBox *errorCorrectionComboBox;
    QLabel *colorLabel;
    QHBoxLayout *colorLayout;
    QFrame *foregroundColorFrame;
    QPushButton *chooseColorButton;
    QHBoxLayout *generateButtonLayout;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *generateQRButton;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *qrCodePreviewLayout;
    QGroupBox *qrPreviewGroupBox;
    QVBoxLayout *previewLayout;
    QLabel *qrCodePreviewLabel;
    QGroupBox *shareGroupBox;
    QHBoxLayout *shareOptionsLayout;
    QPushButton *saveImageButton;
    QPushButton *printButton;
    QPushButton *displayFullScreenButton;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuConnection;
    QMenu *menuView;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1024, 768);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName("actionOpen");
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName("actionExit");
        actionConnect = new QAction(MainWindow);
        actionConnect->setObjectName("actionConnect");
        actionDisconnect = new QAction(MainWindow);
        actionDisconnect->setObjectName("actionDisconnect");
        actionDashboard = new QAction(MainWindow);
        actionDashboard->setObjectName("actionDashboard");
        actionPresentation = new QAction(MainWindow);
        actionPresentation->setObjectName("actionPresentation");
        actionStudents = new QAction(MainWindow);
        actionStudents->setObjectName("actionStudents");
        actionGenerate_QR_Code = new QAction(MainWindow);
        actionGenerate_QR_Code->setObjectName("actionGenerate_QR_Code");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        topBar = new QWidget(centralwidget);
        topBar->setObjectName("topBar");
        horizontalLayout = new QHBoxLayout(topBar);
        horizontalLayout->setObjectName("horizontalLayout");
        connectionStatusLabel = new QLabel(topBar);
        connectionStatusLabel->setObjectName("connectionStatusLabel");

        horizontalLayout->addWidget(connectionStatusLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        attendanceCountLabel = new QLabel(topBar);
        attendanceCountLabel->setObjectName("attendanceCountLabel");

        horizontalLayout->addWidget(attendanceCountLabel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        pageIndicatorLabel = new QLabel(topBar);
        pageIndicatorLabel->setObjectName("pageIndicatorLabel");

        horizontalLayout->addWidget(pageIndicatorLabel);


        verticalLayout->addWidget(topBar);

        notificationLabel = new QLabel(centralwidget);
        notificationLabel->setObjectName("notificationLabel");
        notificationLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout->addWidget(notificationLabel);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        dashboardView = new QWidget();
        dashboardView->setObjectName("dashboardView");
        verticalLayout_2 = new QVBoxLayout(dashboardView);
        verticalLayout_2->setObjectName("verticalLayout_2");
        label = new QLabel(dashboardView);
        label->setObjectName("label");
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        label->setFont(font);
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_2->addWidget(label);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        serverGroupBox = new QGroupBox(dashboardView);
        serverGroupBox->setObjectName("serverGroupBox");
        formLayout = new QFormLayout(serverGroupBox);
        formLayout->setObjectName("formLayout");
        label_2 = new QLabel(serverGroupBox);
        label_2->setObjectName("label_2");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label_2);

        serverUrlEdit = new QLineEdit(serverGroupBox);
        serverUrlEdit->setObjectName("serverUrlEdit");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, serverUrlEdit);

        label_3 = new QLabel(serverGroupBox);
        label_3->setObjectName("label_3");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_3);

        roomIdEdit = new QLineEdit(serverGroupBox);
        roomIdEdit->setObjectName("roomIdEdit");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, roomIdEdit);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        connectButton = new QPushButton(serverGroupBox);
        connectButton->setObjectName("connectButton");

        horizontalLayout_3->addWidget(connectButton);

        disconnectButton = new QPushButton(serverGroupBox);
        disconnectButton->setObjectName("disconnectButton");
        disconnectButton->setEnabled(false);

        horizontalLayout_3->addWidget(disconnectButton);


        formLayout->setLayout(2, QFormLayout::ItemRole::FieldRole, horizontalLayout_3);


        gridLayout->addWidget(serverGroupBox, 0, 0, 1, 1);

        recentPresentationsGroupBox = new QGroupBox(dashboardView);
        recentPresentationsGroupBox->setObjectName("recentPresentationsGroupBox");
        verticalLayout_3 = new QVBoxLayout(recentPresentationsGroupBox);
        verticalLayout_3->setObjectName("verticalLayout_3");
        recentFilesList = new QListWidget(recentPresentationsGroupBox);
        recentFilesList->setObjectName("recentFilesList");

        verticalLayout_3->addWidget(recentFilesList);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        openPresentationButton = new QPushButton(recentPresentationsGroupBox);
        openPresentationButton->setObjectName("openPresentationButton");

        horizontalLayout_4->addWidget(openPresentationButton);


        verticalLayout_3->addLayout(horizontalLayout_4);


        gridLayout->addWidget(recentPresentationsGroupBox, 0, 1, 1, 1);

        classInfoGroupBox = new QGroupBox(dashboardView);
        classInfoGroupBox->setObjectName("classInfoGroupBox");
        formLayout_2 = new QFormLayout(classInfoGroupBox);
        formLayout_2->setObjectName("formLayout_2");
        label_4 = new QLabel(classInfoGroupBox);
        label_4->setObjectName("label_4");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::LabelRole, label_4);

        courseNameEdit = new QLineEdit(classInfoGroupBox);
        courseNameEdit->setObjectName("courseNameEdit");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::FieldRole, courseNameEdit);

        label_5 = new QLabel(classInfoGroupBox);
        label_5->setObjectName("label_5");

        formLayout_2->setWidget(1, QFormLayout::ItemRole::LabelRole, label_5);

        instructorNameEdit = new QLineEdit(classInfoGroupBox);
        instructorNameEdit->setObjectName("instructorNameEdit");

        formLayout_2->setWidget(1, QFormLayout::ItemRole::FieldRole, instructorNameEdit);


        gridLayout->addWidget(classInfoGroupBox, 1, 0, 1, 1);

        gesturesGroupBox = new QGroupBox(dashboardView);
        gesturesGroupBox->setObjectName("gesturesGroupBox");
        verticalLayout_4 = new QVBoxLayout(gesturesGroupBox);
        verticalLayout_4->setObjectName("verticalLayout_4");
        label_6 = new QLabel(gesturesGroupBox);
        label_6->setObjectName("label_6");

        verticalLayout_4->addWidget(label_6);

        gestureListWidget = new QListWidget(gesturesGroupBox);
        gestureListWidget->setObjectName("gestureListWidget");

        verticalLayout_4->addWidget(gestureListWidget);


        gridLayout->addWidget(gesturesGroupBox, 1, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);

        stackedWidget->addWidget(dashboardView);
        presentationView = new QWidget();
        presentationView->setObjectName("presentationView");
        presentationLayout = new QVBoxLayout(presentationView);
        presentationLayout->setObjectName("presentationLayout");
        pdfView = new QPdfView(presentationView);
        pdfView->setObjectName("pdfView");
        pdfView->setMinimumSize(QSize(600, 400));

        presentationLayout->addWidget(pdfView);

        presentationControlsLayout = new QHBoxLayout();
        presentationControlsLayout->setObjectName("presentationControlsLayout");
        prevButton = new QPushButton(presentationView);
        prevButton->setObjectName("prevButton");

        presentationControlsLayout->addWidget(prevButton);

        nextButton = new QPushButton(presentationView);
        nextButton->setObjectName("nextButton");

        presentationControlsLayout->addWidget(nextButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        presentationControlsLayout->addItem(horizontalSpacer_3);

        zoomOutButton = new QPushButton(presentationView);
        zoomOutButton->setObjectName("zoomOutButton");

        presentationControlsLayout->addWidget(zoomOutButton);

        zoomInButton = new QPushButton(presentationView);
        zoomInButton->setObjectName("zoomInButton");

        presentationControlsLayout->addWidget(zoomInButton);


        presentationLayout->addLayout(presentationControlsLayout);

        stackedWidget->addWidget(presentationView);
        studentsView = new QWidget();
        studentsView->setObjectName("studentsView");
        studentsLayout = new QVBoxLayout(studentsView);
        studentsLayout->setObjectName("studentsLayout");
        studentsLabel = new QLabel(studentsView);
        studentsLabel->setObjectName("studentsLabel");
        studentsLabel->setFont(font);
        studentsLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        studentsLayout->addWidget(studentsLabel);

        studentsListLayout = new QHBoxLayout();
        studentsListLayout->setObjectName("studentsListLayout");
        attendanceGroupBox = new QGroupBox(studentsView);
        attendanceGroupBox->setObjectName("attendanceGroupBox");
        verticalLayout_5 = new QVBoxLayout(attendanceGroupBox);
        verticalLayout_5->setObjectName("verticalLayout_5");
        studentListWidget = new QListWidget(attendanceGroupBox);
        studentListWidget->setObjectName("studentListWidget");

        verticalLayout_5->addWidget(studentListWidget);


        studentsListLayout->addWidget(attendanceGroupBox);

        requestsGroupBox = new QGroupBox(studentsView);
        requestsGroupBox->setObjectName("requestsGroupBox");
        verticalLayout_6 = new QVBoxLayout(requestsGroupBox);
        verticalLayout_6->setObjectName("verticalLayout_6");
        requestListWidget = new QListWidget(requestsGroupBox);
        requestListWidget->setObjectName("requestListWidget");

        verticalLayout_6->addWidget(requestListWidget);

        requestButtonsLayout = new QHBoxLayout();
        requestButtonsLayout->setObjectName("requestButtonsLayout");
        approveButton = new QPushButton(requestsGroupBox);
        approveButton->setObjectName("approveButton");

        requestButtonsLayout->addWidget(approveButton);

        rejectButton = new QPushButton(requestsGroupBox);
        rejectButton->setObjectName("rejectButton");

        requestButtonsLayout->addWidget(rejectButton);


        verticalLayout_6->addLayout(requestButtonsLayout);


        studentsListLayout->addWidget(requestsGroupBox);


        studentsLayout->addLayout(studentsListLayout);

        stackedWidget->addWidget(studentsView);
        qrCodeView = new QWidget();
        qrCodeView->setObjectName("qrCodeView");
        qrCodeLayout = new QVBoxLayout(qrCodeView);
        qrCodeLayout->setObjectName("qrCodeLayout");
        qrCodeLabel = new QLabel(qrCodeView);
        qrCodeLabel->setObjectName("qrCodeLabel");
        qrCodeLabel->setFont(font);
        qrCodeLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        qrCodeLayout->addWidget(qrCodeLabel);

        qrCodeMainLayout = new QHBoxLayout();
        qrCodeMainLayout->setObjectName("qrCodeMainLayout");
        qrCodeOptionsLayout = new QVBoxLayout();
        qrCodeOptionsLayout->setObjectName("qrCodeOptionsLayout");
        qrContentGroupBox = new QGroupBox(qrCodeView);
        qrContentGroupBox->setObjectName("qrContentGroupBox");
        qrContentLayout = new QVBoxLayout(qrContentGroupBox);
        qrContentLayout->setObjectName("qrContentLayout");
        sessionRadioButton = new QRadioButton(qrContentGroupBox);
        sessionRadioButton->setObjectName("sessionRadioButton");
        sessionRadioButton->setChecked(true);

        qrContentLayout->addWidget(sessionRadioButton);

        customRadioButton = new QRadioButton(qrContentGroupBox);
        customRadioButton->setObjectName("customRadioButton");

        qrContentLayout->addWidget(customRadioButton);

        customContentTextEdit = new QTextEdit(qrContentGroupBox);
        customContentTextEdit->setObjectName("customContentTextEdit");
        customContentTextEdit->setEnabled(false);

        qrContentLayout->addWidget(customContentTextEdit);


        qrCodeOptionsLayout->addWidget(qrContentGroupBox);

        qrSettingsGroupBox = new QGroupBox(qrCodeView);
        qrSettingsGroupBox->setObjectName("qrSettingsGroupBox");
        qrSettingsLayout = new QFormLayout(qrSettingsGroupBox);
        qrSettingsLayout->setObjectName("qrSettingsLayout");
        sizeLevelLabel = new QLabel(qrSettingsGroupBox);
        sizeLevelLabel->setObjectName("sizeLevelLabel");

        qrSettingsLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, sizeLevelLabel);

        sizeComboBox = new QComboBox(qrSettingsGroupBox);
        sizeComboBox->addItem(QString());
        sizeComboBox->addItem(QString());
        sizeComboBox->addItem(QString());
        sizeComboBox->setObjectName("sizeComboBox");

        qrSettingsLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, sizeComboBox);

        errorLevelLabel = new QLabel(qrSettingsGroupBox);
        errorLevelLabel->setObjectName("errorLevelLabel");

        qrSettingsLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, errorLevelLabel);

        errorCorrectionComboBox = new QComboBox(qrSettingsGroupBox);
        errorCorrectionComboBox->addItem(QString());
        errorCorrectionComboBox->addItem(QString());
        errorCorrectionComboBox->addItem(QString());
        errorCorrectionComboBox->addItem(QString());
        errorCorrectionComboBox->setObjectName("errorCorrectionComboBox");

        qrSettingsLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, errorCorrectionComboBox);

        colorLabel = new QLabel(qrSettingsGroupBox);
        colorLabel->setObjectName("colorLabel");

        qrSettingsLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, colorLabel);

        colorLayout = new QHBoxLayout();
        colorLayout->setObjectName("colorLayout");
        foregroundColorFrame = new QFrame(qrSettingsGroupBox);
        foregroundColorFrame->setObjectName("foregroundColorFrame");
        foregroundColorFrame->setMinimumSize(QSize(24, 24));
        foregroundColorFrame->setFrameShape(QFrame::Shape::StyledPanel);

        colorLayout->addWidget(foregroundColorFrame);

        chooseColorButton = new QPushButton(qrSettingsGroupBox);
        chooseColorButton->setObjectName("chooseColorButton");

        colorLayout->addWidget(chooseColorButton);


        qrSettingsLayout->setLayout(2, QFormLayout::ItemRole::FieldRole, colorLayout);


        qrCodeOptionsLayout->addWidget(qrSettingsGroupBox);

        generateButtonLayout = new QHBoxLayout();
        generateButtonLayout->setObjectName("generateButtonLayout");
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        generateButtonLayout->addItem(horizontalSpacer_4);

        generateQRButton = new QPushButton(qrCodeView);
        generateQRButton->setObjectName("generateQRButton");
        generateQRButton->setMinimumSize(QSize(120, 0));

        generateButtonLayout->addWidget(generateQRButton);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        generateButtonLayout->addItem(horizontalSpacer_5);


        qrCodeOptionsLayout->addLayout(generateButtonLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        qrCodeOptionsLayout->addItem(verticalSpacer);


        qrCodeMainLayout->addLayout(qrCodeOptionsLayout);

        qrCodePreviewLayout = new QVBoxLayout();
        qrCodePreviewLayout->setObjectName("qrCodePreviewLayout");
        qrPreviewGroupBox = new QGroupBox(qrCodeView);
        qrPreviewGroupBox->setObjectName("qrPreviewGroupBox");
        previewLayout = new QVBoxLayout(qrPreviewGroupBox);
        previewLayout->setObjectName("previewLayout");
        qrCodePreviewLabel = new QLabel(qrPreviewGroupBox);
        qrCodePreviewLabel->setObjectName("qrCodePreviewLabel");
        qrCodePreviewLabel->setMinimumSize(QSize(300, 300));
        qrCodePreviewLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        previewLayout->addWidget(qrCodePreviewLabel);


        qrCodePreviewLayout->addWidget(qrPreviewGroupBox);

        shareGroupBox = new QGroupBox(qrCodeView);
        shareGroupBox->setObjectName("shareGroupBox");
        shareOptionsLayout = new QHBoxLayout(shareGroupBox);
        shareOptionsLayout->setObjectName("shareOptionsLayout");
        saveImageButton = new QPushButton(shareGroupBox);
        saveImageButton->setObjectName("saveImageButton");
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("document-save")));
        saveImageButton->setIcon(icon);

        shareOptionsLayout->addWidget(saveImageButton);

        printButton = new QPushButton(shareGroupBox);
        printButton->setObjectName("printButton");
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("document-print")));
        printButton->setIcon(icon1);

        shareOptionsLayout->addWidget(printButton);

        displayFullScreenButton = new QPushButton(shareGroupBox);
        displayFullScreenButton->setObjectName("displayFullScreenButton");
        QIcon icon2(QIcon::fromTheme(QString::fromUtf8("view-fullscreen")));
        displayFullScreenButton->setIcon(icon2);

        shareOptionsLayout->addWidget(displayFullScreenButton);


        qrCodePreviewLayout->addWidget(shareGroupBox);


        qrCodeMainLayout->addLayout(qrCodePreviewLayout);


        qrCodeLayout->addLayout(qrCodeMainLayout);

        stackedWidget->addWidget(qrCodeView);

        verticalLayout->addWidget(stackedWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1024, 36));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuConnection = new QMenu(menubar);
        menuConnection->setObjectName("menuConnection");
        menuView = new QMenu(menubar);
        menuView->setObjectName("menuView");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuConnection->menuAction());
        menubar->addAction(menuView->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuConnection->addAction(actionConnect);
        menuConnection->addAction(actionDisconnect);
        menuView->addAction(actionDashboard);
        menuView->addAction(actionPresentation);
        menuView->addAction(actionStudents);
        menuView->addAction(actionGenerate_QR_Code);

        retranslateUi(MainWindow);
        QObject::connect(actionExit, &QAction::triggered, MainWindow, qOverload<>(&QMainWindow::close));
        QObject::connect(connectButton, SIGNAL(clicked()), MainWindow, SLOT(on_actionConnect_triggered()));
        QObject::connect(disconnectButton, SIGNAL(clicked()), MainWindow, SLOT(on_actionDisconnect_triggered()));
        QObject::connect(openPresentationButton, SIGNAL(clicked()), MainWindow, SLOT(on_actionOpen_triggered()));
        QObject::connect(actionGenerate_QR_Code, SIGNAL(triggered()), MainWindow, SLOT(on_actionQRCode_triggered()));
        QObject::connect(customRadioButton, &QRadioButton::toggled, customContentTextEdit, &QTextEdit::setEnabled);

        stackedWidget->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "AirClass Desktop", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open Presentation...", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
#if QT_CONFIG(shortcut)
        actionExit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionConnect->setText(QCoreApplication::translate("MainWindow", "Connect to Server", nullptr));
        actionDisconnect->setText(QCoreApplication::translate("MainWindow", "Disconnect", nullptr));
        actionDashboard->setText(QCoreApplication::translate("MainWindow", "Dashboard", nullptr));
        actionPresentation->setText(QCoreApplication::translate("MainWindow", "Presentation", nullptr));
        actionStudents->setText(QCoreApplication::translate("MainWindow", "Students", nullptr));
        actionGenerate_QR_Code->setText(QCoreApplication::translate("MainWindow", "Generate QR Code", nullptr));
        connectionStatusLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "color: red;", nullptr));
        connectionStatusLabel->setText(QCoreApplication::translate("MainWindow", "Disconnected", nullptr));
        attendanceCountLabel->setText(QCoreApplication::translate("MainWindow", "Students present: 0", nullptr));
        pageIndicatorLabel->setText(QCoreApplication::translate("MainWindow", "Page 0 of 0", nullptr));
        notificationLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #f0f7ff; color: #0055aa; padding: 5px; border-radius: 3px;", nullptr));
        notificationLabel->setText(QString());
        label->setText(QCoreApplication::translate("MainWindow", "AirClass Dashboard", nullptr));
        serverGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Server Connection", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Server URL:", nullptr));
        serverUrlEdit->setText(QCoreApplication::translate("MainWindow", "ws://localhost:8080", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Room ID:", nullptr));
        roomIdEdit->setText(QCoreApplication::translate("MainWindow", "airclass-123", nullptr));
        connectButton->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        disconnectButton->setText(QCoreApplication::translate("MainWindow", "Disconnect", nullptr));
        recentPresentationsGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Recent Presentations", nullptr));
        openPresentationButton->setText(QCoreApplication::translate("MainWindow", "Open Presentation...", nullptr));
        classInfoGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Class Information", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Course Name:", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Instructor Name:", nullptr));
        gesturesGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Gesture Controls", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Registered Gestures:", nullptr));
        prevButton->setText(QCoreApplication::translate("MainWindow", "Previous", nullptr));
        nextButton->setText(QCoreApplication::translate("MainWindow", "Next", nullptr));
        zoomOutButton->setText(QCoreApplication::translate("MainWindow", "Zoom Out", nullptr));
        zoomInButton->setText(QCoreApplication::translate("MainWindow", "Zoom In", nullptr));
        studentsLabel->setText(QCoreApplication::translate("MainWindow", "Students", nullptr));
        attendanceGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Attendance", nullptr));
        requestsGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Speak Requests", nullptr));
        approveButton->setText(QCoreApplication::translate("MainWindow", "Approve", nullptr));
        rejectButton->setText(QCoreApplication::translate("MainWindow", "Reject", nullptr));
        qrCodeLabel->setText(QCoreApplication::translate("MainWindow", "QR Code Generator", nullptr));
        qrContentGroupBox->setTitle(QCoreApplication::translate("MainWindow", "QR Code Content", nullptr));
        sessionRadioButton->setText(QCoreApplication::translate("MainWindow", "Current Session Information", nullptr));
        customRadioButton->setText(QCoreApplication::translate("MainWindow", "Custom Content", nullptr));
        customContentTextEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Enter custom content for QR code here...", nullptr));
        qrSettingsGroupBox->setTitle(QCoreApplication::translate("MainWindow", "QR Code Settings", nullptr));
        sizeLevelLabel->setText(QCoreApplication::translate("MainWindow", "Size:", nullptr));
        sizeComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "Small", nullptr));
        sizeComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "Medium", nullptr));
        sizeComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "Large", nullptr));

        errorLevelLabel->setText(QCoreApplication::translate("MainWindow", "Error Correction:", nullptr));
        errorCorrectionComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "Low (L)", nullptr));
        errorCorrectionComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "Medium (M)", nullptr));
        errorCorrectionComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "Quality (Q)", nullptr));
        errorCorrectionComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "High (H)", nullptr));

        colorLabel->setText(QCoreApplication::translate("MainWindow", "Foreground Color:", nullptr));
        foregroundColorFrame->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: black; border: 1px solid gray;", nullptr));
        chooseColorButton->setText(QCoreApplication::translate("MainWindow", "Choose...", nullptr));
        generateQRButton->setText(QCoreApplication::translate("MainWindow", "Generate QR Code", nullptr));
        qrPreviewGroupBox->setTitle(QCoreApplication::translate("MainWindow", "QR Code Preview", nullptr));
        qrCodePreviewLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "border: 1px solid #cccccc;", nullptr));
        qrCodePreviewLabel->setText(QCoreApplication::translate("MainWindow", "QR Code will appear here", nullptr));
        shareGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Share Options", nullptr));
        saveImageButton->setText(QCoreApplication::translate("MainWindow", "Save Image", nullptr));
        printButton->setText(QCoreApplication::translate("MainWindow", "Print", nullptr));
        displayFullScreenButton->setText(QCoreApplication::translate("MainWindow", "Full Screen", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuConnection->setTitle(QCoreApplication::translate("MainWindow", "Connection", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
