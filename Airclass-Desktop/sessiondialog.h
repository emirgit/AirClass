#ifndef SESSIONDIALOG_H
#define SESSIONDIALOG_H

#include <QDialog>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include "restapiclient.h"

class SessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionDialog(RestApiClient* restApi, QWidget *parent = nullptr);
    ~SessionDialog();

    QString getSelectedSessionId() const { return m_selectedSessionId; }
    QString getSelectedSessionCode() const { return m_selectedSessionCode; }
    QString getSelectedSessionName() const { return m_selectedSessionName; }

signals:
    void sessionSelected(const QString &sessionId, const QString &sessionCode, const QString &sessionName);

private slots:
    void onActiveSessionReceived(const QJsonObject &session);
    void onSessionsReceived(const QJsonArray &sessions);
    void onSessionCreated(const QJsonObject &session);
    void onSessionActivated(const QJsonObject &session);
    void onError(const QString &message);
    
    void onCreateNewClicked();
    void onSelectExistingClicked();
    void onBackClicked();
    void onContinueClicked();
    void onSessionListItemClicked(QListWidgetItem *item);
    void onCreateSessionClicked();
    void onRefreshClicked();

private:
    void setupMainPage();
    void setupSessionListPage();
    void setupCreateSessionPage();
    void checkActiveSession();
    void populateSessionList(const QJsonArray &sessions);

    RestApiClient* m_restApi;
    QStackedWidget *m_stackedWidget;
    
    // Main page
    QWidget *m_mainPage;
    QLabel *m_activeSessionLabel;
    QPushButton *m_continueButton;
    QPushButton *m_createNewButton;
    QPushButton *m_selectExistingButton;
    
    // Session list page
    QWidget *m_sessionListPage;
    QListWidget *m_sessionListWidget;
    QPushButton *m_selectButton;
    QPushButton *m_backButton;
    QPushButton *m_refreshButton;
    
    // Create session page
    QWidget *m_createSessionPage;
    QLineEdit *m_sessionCodeEdit;
    QLineEdit *m_sessionNameEdit;
    QPushButton *m_createButton;
    QPushButton *m_backFromCreateButton;
    
    // Selected session info
    QString m_selectedSessionId;
    QString m_selectedSessionCode;
    QString m_selectedSessionName;
    bool m_hasActiveSession;
    
    // Session data storage
    QJsonArray m_sessions;
};

#endif // SESSIONDIALOG_H