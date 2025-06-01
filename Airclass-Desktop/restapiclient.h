#ifndef RESTAPICLIENT_H
#define RESTAPICLIENT_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrlQuery>
#include <QProcess>
// #include <QNetworkAccessManager>

class RestApiClient : public QObject
{
    Q_OBJECT
public:
    explicit RestApiClient(QObject *parent = nullptr);
    ~RestApiClient();
    void generateAttendanceCode(const QString &classroomId, int expiresInSeconds);

    void setBaseUrl(const QString &baseUrl);
    void setAuthToken(const QString &token);
    QString getAuthToken() const;

    void sendRequest(const QString &endpointPath, const QString &httpMethod,
                     const QJsonObject &jsonData = QJsonObject(),
                     const QUrlQuery &urlQuery = QUrlQuery());

    void login(const QString &email, const QString &password);
    void registerUser(const QString &name, const QString &email, const QString &password);

    void listClassrooms();
    void createClassroom(const QString &ip, int port);
    void updateClassroom(const QString &roomId, const QString &name, const QString &description);
    void deleteClassroom(const QString &roomId);

    void testAttendanceEndpoint(const QString &classroomId);
    void listAttendance(const QString &roomId);
    void markAttendance(const QString &roomId, const QString &code,
                        const QString &studentId, const QString &studentName);
    // void generateAttendanceCode(const QString &roomId, int durationMinutes);

    void listRequests(const QString &roomId);
    void createRequest(const QString &roomId, const QString &studentId, const QString &studentName);
    void updateRequest(const QString &roomId, const QString &requestId, const QString &action);

    void startPollingRequests(const QString &roomId, int intervalMs = 5000);
    void stopPollingRequests();

    void listSessions();
    void getActiveSession();
    void createSession(const QString &name, const QString &ip, int port);
    void activateSession(const QString &sessionId);
    void closeSession(const QString &sessionId);

    void getQrCode(const QString &roomId);
    //QNetworkAccessManager *m_networkManager; // Added for QR code requests

signals:
    void loginSuccess(const QJsonObject &userData, const QString &token);
    void loginFailed(const QString &message);
    void registerSuccess(const QJsonObject &userData);
    void registerFailed(const QString &message);
    void error(const QString &message);
    void classroomCreated(qint64 classroomId, const QString &code);
    void classroomListReceived(const QJsonArray &classrooms);
    void sessionsReceived(const QJsonArray &sessions);
    void activeSessionReceived(const QJsonObject &session);
    void sessionCreated(const QJsonObject &sessionData);
    void sessionActivated(const QJsonObject &sessionData);
    void sessionClosed(const QString &sessionId);
    void attendanceListReceived(const QJsonArray &attendanceList);
    // void attendanceCodeGenerated(const QString &code, const QString &qrCode, const QString &expiry);
    void requestsReceived(const QJsonArray &requests);
    void newSpeakRequest(const QString &studentId, const QString &studentName, const QString &requestId);
    void requestCreated(const QJsonObject &requestData);
    void requestUpdated(const QString &requestId, const QString &action);
    void qrCodeReceived(const QString &qrBase64);
    void attendanceTestSuccess(const QJsonObject &data);
    void attendanceTestFailed(const QString &message);
    void attendanceCodeGenerated(const QString &code, const QString &expiresAt, const QString &expiresIn); // API'nizin döndüğü yanıta göre bu daha doğru olabilir
    void attendanceCodeGenerationFailed(const QString &errorMsg);


private slots:
    void pollRequests();
    void handleResponse(const QString &endpoint, const QString &method, const QJsonObject &jsonObj);
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessError(QProcess::ProcessError processError);


private:
    QString m_baseUrl;
    QString m_authToken;
    QTimer *m_pollTimer;
    QString m_currentRoomId;
    QStringList m_processedRequestIds;
    //QNetworkAccessManager *m_networkManager; // BU SATIRIN OLDUĞUNDAN EMİN OLUN

};

#endif // RESTAPICLIENT_H
