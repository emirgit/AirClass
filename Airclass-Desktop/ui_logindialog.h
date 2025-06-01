/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginDialog
{
public:
    QVBoxLayout *verticalLayout;
    QStackedWidget *stackedWidget;
    QWidget *loginPage;
    QVBoxLayout *verticalLayout_2;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QLabel *dividerLabel;
    QPushButton *toRegisterButton;
    QWidget *registerPage;
    QVBoxLayout *verticalLayout_3;
    QLabel *regTitleLabel;
    QLabel *regSubtitleLabel;
    QLabel *regNameLabel;
    QLineEdit *regNameEdit;
    QLabel *regEmailLabel;
    QLineEdit *regEmailEdit;
    QLabel *regPasswordLabel;
    QLineEdit *regPasswordEdit;
    QLabel *regConfirmLabel;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;
    QLabel *regDividerLabel;
    QPushButton *toLoginButton;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName("LoginDialog");
        LoginDialog->resize(400, 600);
        verticalLayout = new QVBoxLayout(LoginDialog);
        verticalLayout->setObjectName("verticalLayout");
        stackedWidget = new QStackedWidget(LoginDialog);
        stackedWidget->setObjectName("stackedWidget");
        loginPage = new QWidget();
        loginPage->setObjectName("loginPage");
        verticalLayout_2 = new QVBoxLayout(loginPage);
        verticalLayout_2->setObjectName("verticalLayout_2");
        titleLabel = new QLabel(loginPage);
        titleLabel->setObjectName("titleLabel");

        verticalLayout_2->addWidget(titleLabel);

        subtitleLabel = new QLabel(loginPage);
        subtitleLabel->setObjectName("subtitleLabel");

        verticalLayout_2->addWidget(subtitleLabel);

        nameLabel = new QLabel(loginPage);
        nameLabel->setObjectName("nameLabel");

        verticalLayout_2->addWidget(nameLabel);

        nameEdit = new QLineEdit(loginPage);
        nameEdit->setObjectName("nameEdit");

        verticalLayout_2->addWidget(nameEdit);

        passwordLabel = new QLabel(loginPage);
        passwordLabel->setObjectName("passwordLabel");

        verticalLayout_2->addWidget(passwordLabel);

        passwordEdit = new QLineEdit(loginPage);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setEchoMode(QLineEdit::Password);

        verticalLayout_2->addWidget(passwordEdit);

        loginButton = new QPushButton(loginPage);
        loginButton->setObjectName("loginButton");

        verticalLayout_2->addWidget(loginButton);

        dividerLabel = new QLabel(loginPage);
        dividerLabel->setObjectName("dividerLabel");
        dividerLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(dividerLabel);

        toRegisterButton = new QPushButton(loginPage);
        toRegisterButton->setObjectName("toRegisterButton");

        verticalLayout_2->addWidget(toRegisterButton);

        stackedWidget->addWidget(loginPage);
        registerPage = new QWidget();
        registerPage->setObjectName("registerPage");
        verticalLayout_3 = new QVBoxLayout(registerPage);
        verticalLayout_3->setObjectName("verticalLayout_3");
        regTitleLabel = new QLabel(registerPage);
        regTitleLabel->setObjectName("regTitleLabel");

        verticalLayout_3->addWidget(regTitleLabel);

        regSubtitleLabel = new QLabel(registerPage);
        regSubtitleLabel->setObjectName("regSubtitleLabel");

        verticalLayout_3->addWidget(regSubtitleLabel);

        regNameLabel = new QLabel(registerPage);
        regNameLabel->setObjectName("regNameLabel");

        verticalLayout_3->addWidget(regNameLabel);

        regNameEdit = new QLineEdit(registerPage);
        regNameEdit->setObjectName("regNameEdit");

        verticalLayout_3->addWidget(regNameEdit);

        regEmailLabel = new QLabel(registerPage);
        regEmailLabel->setObjectName("regEmailLabel");

        verticalLayout_3->addWidget(regEmailLabel);

        regEmailEdit = new QLineEdit(registerPage);
        regEmailEdit->setObjectName("regEmailEdit");

        verticalLayout_3->addWidget(regEmailEdit);

        regPasswordLabel = new QLabel(registerPage);
        regPasswordLabel->setObjectName("regPasswordLabel");

        verticalLayout_3->addWidget(regPasswordLabel);

        regPasswordEdit = new QLineEdit(registerPage);
        regPasswordEdit->setObjectName("regPasswordEdit");
        regPasswordEdit->setEchoMode(QLineEdit::Password);

        verticalLayout_3->addWidget(regPasswordEdit);

        regConfirmLabel = new QLabel(registerPage);
        regConfirmLabel->setObjectName("regConfirmLabel");

        verticalLayout_3->addWidget(regConfirmLabel);

        confirmPasswordEdit = new QLineEdit(registerPage);
        confirmPasswordEdit->setObjectName("confirmPasswordEdit");
        confirmPasswordEdit->setEchoMode(QLineEdit::Password);

        verticalLayout_3->addWidget(confirmPasswordEdit);

        registerButton = new QPushButton(registerPage);
        registerButton->setObjectName("registerButton");

        verticalLayout_3->addWidget(registerButton);

        regDividerLabel = new QLabel(registerPage);
        regDividerLabel->setObjectName("regDividerLabel");
        regDividerLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(regDividerLabel);

        toLoginButton = new QPushButton(registerPage);
        toLoginButton->setObjectName("toLoginButton");

        verticalLayout_3->addWidget(toLoginButton);

        stackedWidget->addWidget(registerPage);

        verticalLayout->addWidget(stackedWidget);


        retranslateUi(LoginDialog);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QCoreApplication::translate("LoginDialog", "AirClass - Login", nullptr));
        titleLabel->setText(QCoreApplication::translate("LoginDialog", "Welcome to AirClass", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("LoginDialog", "Please sign in to continue", nullptr));
        nameLabel->setText(QCoreApplication::translate("LoginDialog", "Full Name", nullptr));
        nameEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "Enter your full name", nullptr));
        passwordLabel->setText(QCoreApplication::translate("LoginDialog", "Password", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "Enter your password", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginDialog", "Sign In", nullptr));
        dividerLabel->setText(QCoreApplication::translate("LoginDialog", "or", nullptr));
        toRegisterButton->setText(QCoreApplication::translate("LoginDialog", "Create New Account", nullptr));
        regTitleLabel->setText(QCoreApplication::translate("LoginDialog", "Create Your Account", nullptr));
        regSubtitleLabel->setText(QCoreApplication::translate("LoginDialog", "Join AirClass to get started", nullptr));
        regNameLabel->setText(QCoreApplication::translate("LoginDialog", "Full Name", nullptr));
        regNameEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "Enter your full name", nullptr));
        regEmailLabel->setText(QCoreApplication::translate("LoginDialog", "Email Address", nullptr));
        regEmailEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "Enter your email", nullptr));
        regPasswordLabel->setText(QCoreApplication::translate("LoginDialog", "Password", nullptr));
        regPasswordEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "Create a password", nullptr));
        regConfirmLabel->setText(QCoreApplication::translate("LoginDialog", "Confirm Password", nullptr));
        confirmPasswordEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "Re-enter your password", nullptr));
        registerButton->setText(QCoreApplication::translate("LoginDialog", "Create Account", nullptr));
        regDividerLabel->setText(QCoreApplication::translate("LoginDialog", "or", nullptr));
        toLoginButton->setText(QCoreApplication::translate("LoginDialog", "Back to Sign In", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
