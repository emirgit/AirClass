#include "logindialog.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QStyle>
#include <QIcon>
#include <QFont>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QSettings>

LoginDialog::LoginDialog(RestApiClient* restApi, QWidget *parent)
    : QDialog(parent)
    , m_restApi(restApi)
{
    setWindowTitle("AirClass - Welcome");
    // Set window flags to enable maximize button and window controls
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    // Set minimum size for the window
    setMinimumSize(400, 600);

    // Center the dialog on the screen
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Store window state
    m_isFullScreen = false;

    // Get local IP address
    m_localIp = getLocalIpAddress();
    qDebug() << "Local IP address:" << m_localIp;

    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        QLineEdit {
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            background-color: #f8f9fa;
            font-size: 14px;
            margin-bottom: 12px;
            color: #333333;
            min-height: 20px;
        }
        QLineEdit:focus {
            border-color: #4a90e2;
            background-color: #ffffff;
        }
        QLineEdit::placeholder {
            color: #999999;
        }
        QPushButton {
            padding: 10px 20px;
            background-color: #4a90e2;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
            margin: 5px;
            min-height: 40px;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton#secondary {
            background-color: transparent;
            color: #4a90e2;
            border: 2px solid #4a90e2;
        }
        QPushButton#secondary:hover {
            background-color: #f0f7ff;
        }
        QLabel {
            color: #333333;
            font-size: 14px;
        }
        QLabel#title {
            font-size: 32px;
            font-weight: bold;
            color: #2c3e50;
            margin-bottom: 20px;
        }
        QLabel#subtitle {
            font-size: 18px;
            color: #666666;
            margin-bottom: 15px;
        }
        QLabel#info {
            font-size: 14px;
            color: #666666;
            margin-top: 5px;
        }
    )");

    // Connect to RestAPI signals
    // connect(m_restApi, &RestApiClient::loginSuccess, this, &LoginDialog::onLoginSuccess);
    connect(m_restApi, QOverload<const QJsonObject &, const QString &>::of(&RestApiClient::loginSuccess),
            this, QOverload<const QJsonObject &, const QString &>::of(&LoginDialog::onLoginSuccess));

    connect(m_restApi, &RestApiClient::loginFailed, this, &LoginDialog::onLoginFailed);
    connect(m_restApi, &RestApiClient::registerSuccess, this, &LoginDialog::onRegisterSuccess);
    connect(m_restApi, &RestApiClient::registerFailed, this, &LoginDialog::onRegisterFailed);
    connect(m_restApi, &RestApiClient::error, this, &LoginDialog::onError);

    stackedWidget = new QStackedWidget(this);
    setupLoginPage();
    setupRegisterPage();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(15);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);
}

LoginDialog::~LoginDialog()
{
    // We don't own m_restApi, so don't delete it
    if (m_restApi) {
        disconnect(m_restApi, nullptr, this, nullptr);
    }
}

QString LoginDialog::getLocalIpAddress()
{
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            return address.toString();
        }
    }
    return "127.0.0.1"; // Fallback to localhost
}

void LoginDialog::setupLoginPage()
{
    loginPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(loginPage);
    layout->setSpacing(20);
    layout->setAlignment(Qt::AlignCenter);

    // Container widget for centering content
    QWidget *container = new QWidget;
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(20);
    containerLayout->setAlignment(Qt::AlignCenter);
    container->setMaximumWidth(500);

    QLabel *titleLabel = new QLabel("Welcome to AirClass");
    titleLabel->setObjectName("title");
    titleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel("Please sign in to continue");
    subtitleLabel->setObjectName("subtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(subtitleLabel);

    QLabel *nameLabel = new QLabel("Email");
    nameLabel->setObjectName("info");
    containerLayout->addWidget(nameLabel);

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Enter your email");
    containerLayout->addWidget(nameEdit);

    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setObjectName("info");
    containerLayout->addWidget(passwordLabel);

    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    containerLayout->addWidget(passwordEdit);

    loginButton = new QPushButton("Sign In");
    loginButton->setMinimumHeight(45);
    containerLayout->addWidget(loginButton);

    QLabel *divider = new QLabel("or");
    divider->setAlignment(Qt::AlignCenter);
    divider->setStyleSheet("color: #666666; margin: 10px 0;");
    containerLayout->addWidget(divider);

    toRegisterButton = new QPushButton("Create New Account");
    toRegisterButton->setObjectName("secondary");
    toRegisterButton->setMinimumHeight(45);
    containerLayout->addWidget(toRegisterButton);

    layout->addWidget(container);
    stackedWidget->addWidget(loginPage);

    // Connect signals
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(toRegisterButton, &QPushButton::clicked, this, &LoginDialog::onToRegisterClicked);
}

void LoginDialog::setupRegisterPage()
{
    registerPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(registerPage);
    layout->setSpacing(20);
    layout->setAlignment(Qt::AlignCenter);

    // Container widget for centering content
    QWidget *container = new QWidget;
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(20);
    containerLayout->setAlignment(Qt::AlignCenter);
    container->setMaximumWidth(500);

    QLabel *titleLabel = new QLabel("Create Account");
    titleLabel->setObjectName("title");
    titleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel("Join AirClass today");
    subtitleLabel->setObjectName("subtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(subtitleLabel);

    QLabel *nameLabel = new QLabel("Full Name");
    nameLabel->setObjectName("info");
    containerLayout->addWidget(nameLabel);

    regNameEdit = new QLineEdit;
    regNameEdit->setPlaceholderText("Enter your full name");
    containerLayout->addWidget(regNameEdit);

    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setObjectName("info");
    containerLayout->addWidget(emailLabel);

    regEmailEdit = new QLineEdit;
    regEmailEdit->setPlaceholderText("Enter your email");
    containerLayout->addWidget(regEmailEdit);

    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setObjectName("info");
    containerLayout->addWidget(passwordLabel);

    regPasswordEdit = new QLineEdit;
    regPasswordEdit->setPlaceholderText("Enter your password");
    regPasswordEdit->setEchoMode(QLineEdit::Password);
    containerLayout->addWidget(regPasswordEdit);

    QLabel *confirmPasswordLabel = new QLabel("Confirm Password");
    confirmPasswordLabel->setObjectName("info");
    containerLayout->addWidget(confirmPasswordLabel);

    regConfirmPasswordEdit = new QLineEdit;
    regConfirmPasswordEdit->setPlaceholderText("Confirm your password");
    regConfirmPasswordEdit->setEchoMode(QLineEdit::Password);
    containerLayout->addWidget(regConfirmPasswordEdit);

    registerButton = new QPushButton("Create Account");
    registerButton->setMinimumHeight(45);
    containerLayout->addWidget(registerButton);

    QLabel *divider = new QLabel("or");
    divider->setAlignment(Qt::AlignCenter);
    divider->setStyleSheet("color: #666666; margin: 10px 0;");
    containerLayout->addWidget(divider);

    toLoginButton = new QPushButton("Back to Sign In");
    toLoginButton->setObjectName("secondary");
    toLoginButton->setMinimumHeight(45);
    containerLayout->addWidget(toLoginButton);

    layout->addWidget(container);
    stackedWidget->addWidget(registerPage);

    // Connect signals
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(toLoginButton, &QPushButton::clicked, this, &LoginDialog::onToLoginClicked);
}

void LoginDialog::onLoginClicked()
{
    QString email = nameEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }

    m_restApi->login(email, password);
}

void LoginDialog::onRegisterClicked()
{
    QString name = regNameEdit->text().trimmed();
    QString email = regEmailEdit->text().trimmed();
    QString password = regPasswordEdit->text();
    QString confirmPassword = regConfirmPasswordEdit->text();

    if (name.isEmpty() || email.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Error", "Passwords do not match");
        return;
    }

    m_restApi->registerUser(name, email, password);
}

void LoginDialog::onToRegisterClicked()
{
    stackedWidget->setCurrentWidget(registerPage);
}

void LoginDialog::onToLoginClicked()
{
    stackedWidget->setCurrentWidget(loginPage);
}

// void LoginDialog::onLoginSuccess(const QJsonObject &userData) // Bu 'userData' RestApiClient'ten gelen user objesidir
// {
//     qDebug() << "Login successful in LoginDialog. User data:" << QJsonDocument(userData).toJson(QJsonDocument::Compact);
//     QString username = userData.value("name").toString();

//     // Token zaten m_restApi içinde doğru bir şekilde saklanıyor.
//     // LoginDialog'un token'ı tekrar işlemesine veya göndermesine gerek yok.
//     emit loginSuccessful(username); // Sadece kullanıcı adını yayınla
//     accept();
// }

void LoginDialog::onLoginSuccess(const QJsonObject &userData, const QString &token)
{
    qDebug() << "Login successful in LoginDialog. User data:" << QJsonDocument(userData).toJson(QJsonDocument::Compact) << "Token:" << token.left(15) << "...";
    QString username = userData.value("name").toString();
    

    emit loginSuccessful(username, token); // Hem username'i hem de token'ı MainWindow'a yayınla
    accept(); // Dialog'u kapat
}

void LoginDialog::onLoginFailed(const QString &message)
{
    QMessageBox::warning(this, "Login Failed", message);
}

void LoginDialog::onRegisterSuccess(const QJsonObject &data)
{
    QString name = data["name"].toString();
    QMessageBox::information(this, "Registration Successful", 
        QString("Welcome %1! Your account has been created successfully.\nPlease login with your credentials.").arg(name));
    
    // Clear the registration form
    regNameEdit->clear();
    regEmailEdit->clear();
    regPasswordEdit->clear();
    regConfirmPasswordEdit->clear();
    
    // Switch back to login view
    stackedWidget->setCurrentWidget(loginPage);
    
    // Pre-fill the login email field with the registered email
    nameEdit->setText(data["email"].toString());
}

void LoginDialog::onRegisterFailed(const QString &message)
{
    QMessageBox::warning(this, "Registration Failed", message);
}

void LoginDialog::onError(const QString &message)
{
    QMessageBox::critical(this, "Error", message);
}

void LoginDialog::setFullScreen(bool fullScreen)
{
    m_isFullScreen = fullScreen;
    if (m_isFullScreen) {
        showFullScreen();
    } else {
        showNormal();
    }
}
