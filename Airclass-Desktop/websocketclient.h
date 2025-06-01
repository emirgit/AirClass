#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>
#include <QtWebSockets/QWebSocket>
#include <QString>
#include <QUrl>
#include <QTimer>
#include <QWebSocket>


#define PING_INTERVAL 30000 // 30 seconds
#define RECONNECT_INTERVAL 5000 // 5 seconds
#define MAX_RECONNECT_ATTEMPTS 50 // Maximum number of reconnection attempts
#define DEFAULT_WEBSOCKET_PORT 8082 // Default WebSocket port

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    ~WebSocketClient();

    void connectToServer(const QString &url);
    void connectToDiscoveredIP(const QString &ip, quint16 port = DEFAULT_WEBSOCKET_PORT);
    void disconnect();
    void sendMessage(const QString &message);
    bool isConnected() const;
    QString serverUrl() const;

signals:
    void connected();
    void disconnected();
    void error(const QString &message);
    void messageReceived(const QString &message);
    void gestureReceived(const QString &command, const QString &x = "", const QString &y = "");
    void drawingReceived(double x, double y, bool isStart, const QString &color, int width);
    void pageNavigationReceived(const QString &action, const QString &clientId, const QString &timestamp);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);
    void onPong(quint64 elapsedTime, const QByteArray &payload);
    void sendPing();
    void tryReconnect();

private:
    QWebSocket m_webSocket;
    bool m_connected;
    QString m_serverUrl;
    QString m_clientId;
    QTimer m_pingTimer;
    QTimer m_reconnectTimer;
    int m_reconnectAttempts;

};

#endif // WEBSOCKETCLIENT_H
