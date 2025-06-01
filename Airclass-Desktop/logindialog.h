#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "restapiclient.h"

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(RestApiClient* restApi, QWidget *parent = nullptr);
    ~LoginDialog();
    void setFullScreen(bool fullScreen);

signals:
    // void loginSuccessful(const QString &username, const QString &token);
    void loginSuccessful(const QString &username, const QString &token); // Add token parameter
    // void loginSuccessful(const QString &username, const QString &token); // YENİ HALİ


    void registerSuccessful(const QString &username, const QString &token);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onToRegisterClicked();
    void onToLoginClicked();
    // void onLoginSuccess(const QJsonObject &data);
    // void onLoginSuccess(const QJsonObject &data, const QString &token); // Add token parameter
    void onLoginFailed(const QString &message);
    void onRegisterSuccess(const QJsonObject &data);
    void onRegisterFailed(const QString &message);
    void onError(const QString &message);
    void onLoginSuccess(const QJsonObject &userData, const QString &token); // YENİ SLOT: userData ve token alır

private:
    void setupLoginPage();
    void setupRegisterPage();
    QString getLocalIpAddress();

    QStackedWidget *stackedWidget;
    QWidget *loginPage;
    QWidget *registerPage;

    QLineEdit *nameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *toRegisterButton;

    QLineEdit *regNameEdit;
    QLineEdit *regEmailEdit;
    QLineEdit *regPasswordEdit;
    QLineEdit *regConfirmPasswordEdit;
    QPushButton *registerButton;
    QPushButton *toLoginButton;

    RestApiClient* m_restApi;
    QString m_localIp;
    bool m_isFullScreen;
};

#endif // LOGINDIALOG_H
