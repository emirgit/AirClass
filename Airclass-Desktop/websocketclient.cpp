#include "websocketclient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent),
    m_serverUrl("ws://localhost:8082"), // m_serverUrl önce başlatılmaya çalışılıyor
    m_connected(false),                  // m_connected sonra
    m_reconnectAttempts(0),
    m_clientId("desktop-pi-01")  // Statik client ID

{
    // Connect WebSocket signals to slots
    connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(&m_webSocket, &QWebSocket::errorOccurred, this, &WebSocketClient::onError);
    connect(&m_webSocket, &QWebSocket::pong, this, &WebSocketClient::onPong);

    // Setup ping timer with a shorter interval for more reliable connection
    m_pingTimer.setInterval(PING_INTERVAL);
    connect(&m_pingTimer, &QTimer::timeout, this, &WebSocketClient::sendPing);

    // Setup reconnect timer
    m_reconnectTimer.setInterval(RECONNECT_INTERVAL);
    m_reconnectTimer.setSingleShot(true); // Ensure it only runs once per attempt
    connect(&m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::tryReconnect);

    // Start ping timer immediately to keep connection alive
    // m_pingTimer.start();
    // m_reconnectTimer.setSingleShot(true); // Ensure it only runs once per attempt
    // m_reconnectTimer.setInterval(RECONNECT_INTERVAL);
    // // m_reconnectTimer.start(); // Start the reconnect timer
    // // qDebug() << "WebSocketClient initialized, ping timer started with interval:" << PING_INTERVAL / 1000 << "seconds";
    // // m_webSocket.setPingInterval(PING_INTERVAL); // Set ping interval for WebSocket
    // // m_webSocket.setPongTimeout(PING_INTERVAL + 5000); // Set a timeout for pong response
    // // qDebug() << "WebSocketClient initialized, pong timeout set to:" << (PING_INTERVAL + 5000) / 1000 << "seconds";

    // m_serverUrl = "ws://localhost:8082"; // Initialize server URL as empty
    // Connect the WebSocket to the server URL if provided
    //qDebug() << "Connecting to server atilkter:" << m_serverUrl;
    //m_webSocket.open(QUrl(m_serverUrl));
    //qDebug() << "WebSocketClient initialized, connecting to server at:" << m_serverUrl;
    // send deneme mesage
    //m_webSocket.sendTextMessage("WebSocketClient initialized, connecting to server at: " + m_serverUrl);
    //qDebug() << "WebSocketClient initialized, mesaj gonderildi:" << m_serverUrl;
}

WebSocketClient::~WebSocketClient()
{
    m_pingTimer.stop();
    m_reconnectTimer.stop();
    if (m_connected) {
        m_webSocket.close();
    }
}

void WebSocketClient::connectToServer(const QString &url)
{

    if (isConnected()) {
        qDebug() << "Already connected to server at:" << m_serverUrl;
        emit connected(); // Re-emit connected signal to update UI
        return;
    }

    m_serverUrl = url;
    m_reconnectAttempts = 0; // Reset reconnection attempts
    m_reconnectTimer.stop(); // Stop any existing reconnection timer

    qDebug() << "Connecting to server at:" << m_serverUrl;
    m_webSocket.open(QUrl(m_serverUrl));

    // qDebug() << "Attempting to connect to server at:" << url;
    // if (m_connected  || m_webSocket.state() == QAbstractSocket::ConnectedState) {
    //     qDebug() << "Already connected to server";
    //     emit connected(); // Re-emit connected signal to update UI
    //     return;
    // }

    // qDebug() << "Connecting to server at222:" << url;

    // m_serverUrl = url;
    // qDebug() << "Connecting to server at:" << url;

    // // Reset reconnection attempts when explicitly connecting
    // m_reconnectAttempts = 0;
    // m_reconnectTimer.stop(); // Stop any existing reconnection timer
    // m_webSocket.open(QUrl(url));
    // qDebug() << "WebSocketClient CONNN to server at BAKKK:" << url;
}

void WebSocketClient::disconnect()
{
    m_pingTimer.stop();
    m_reconnectTimer.stop();
    m_reconnectAttempts = MAX_RECONNECT_ATTEMPTS; // Prevent auto-reconnect
    if (!m_connected) {
        qDebug() << "Not connected to any server";
        return;
    }

    qDebug() << "Disconnecting from server";
    m_webSocket.close();
}

void WebSocketClient::sendMessage(const QString &message)
{
    // if (!m_connected) {
    if (!isConnected()) {
        qWarning() << "Cannot send message: Not connected to server";
        emit error("Cannot send message: Not connected to server");
        return;
    }

    qDebug() << "Sending message to server:" << message;
    m_webSocket.sendTextMessage(message);
}

bool WebSocketClient::isConnected() const
{
    return m_connected && m_webSocket.state() == QAbstractSocket::ConnectedState;
}

void WebSocketClient::onConnected()
{
    m_connected = true;
    m_reconnectAttempts = 0;
    m_reconnectTimer.stop();
    m_pingTimer.start();

    qDebug() << "WebSocket state after connection:" << m_webSocket.state();

    // Kayıt mesajını hazırla ve gönder
    QJsonObject registrationMsg;
    registrationMsg["register"] = "desktop";
    registrationMsg["id"] = m_clientId;

    QJsonDocument doc(registrationMsg);
    QString message = doc.toJson(QJsonDocument::Compact);

    qDebug() << "Sending registration message:" << message;
    m_webSocket.sendTextMessage(message);

    qDebug() << "Connected to WebSocket server. Ping timer started.";
    emit connected();
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "Disconnected, WebSocket state:" << m_webSocket.state();
    m_connected = false;
    m_pingTimer.stop();
    emit disconnected();
    if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        m_reconnectTimer.start();
    }
    else {
        qDebug() << "Maximum reconnection attempts reached. Not attempting to reconnect.";
    }
}

void WebSocketClient::tryReconnect()
{
    if (m_connected) {
        qDebug() << "Reconnection attempt skipped. WebSocket is already connected.";
        m_reconnectTimer.stop();
        return;
    }

    m_reconnectAttempts++;
    qDebug() << "Attempting to reconnect... (Attempt" << m_reconnectAttempts << "of" << MAX_RECONNECT_ATTEMPTS << ")";

    if (m_reconnectAttempts <= MAX_RECONNECT_ATTEMPTS) {
        qDebug() << "Reconnecting to server at:" << m_serverUrl;
        m_webSocket.open(QUrl(m_serverUrl));
    } else {
        qDebug() << "Maximum reconnection attempts reached. Stopping reconnection timer.";
        m_reconnectTimer.stop();
    }
}

void WebSocketClient::sendPing()
{
    if (isConnected()) {
        qDebug() << "Sending ping to server to keep connection alive.";
        m_webSocket.ping();
    } else {
        qDebug() << "Ping not sent. WebSocket is not connected.";
    }
}

void WebSocketClient::onPong(quint64 elapsedTime, const QByteArray &payload)
{
    qDebug() << "Received pong from server. Elapsed time:" << elapsedTime << "ms, Payload:" << payload;
    // Optionally, you can handle the pong response here
    // For example, you could log it or emit a signal
    // emit pongReceived(elapsedTime, payload);
    m_pingTimer.start(); // Restart ping timer if needed
}

void WebSocketClient::onTextMessageReceived(const QString &message)
{
    qDebug() << "Raw message received from server:" << message;

    // Parse the JSON message
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON message received";
        return;
    }

    QJsonObject obj = doc.object();

    // Komut mesajları için
    if (obj.contains("command")) {
        QString command = obj["command"].toString();

        if (command == "two_up" || command == "one_up") {
            // Position bilgisi içeren komutlar için
            QJsonObject position = obj["position"].toObject();
            double x = position["x"].toDouble();
            double y = position["y"].toDouble();

            qDebug() << "Received command:" << command << "with position x:" << x << "y:" << y;
            emit gestureReceived(command, QString::number(x), QString::number(y));
        }
        else {
            // Sadece komut içeren mesajlar için
            qDebug() << "Received command:" << command;
            emit gestureReceived(command, "", "");
        }
        return;
    }

    // Drawing mesajları için
    if (obj.contains("type") && obj["type"].toString() == "drawing") {
        double x = obj["x"].toDouble();
        double y = obj["y"].toDouble();
        bool isStart = obj["isStart"].toBool();
        QString color = obj["color"].toString();
        int width = obj["width"].toInt();

        qDebug() << "Drawing received:" << x << y << isStart << color << width;
        emit drawingReceived(x, y, isStart, color, width);
        return;
    }

    // Page navigation mesajları için
    if (obj.contains("type") && obj["type"].toString() == "page_navigation") {
        QString action = obj["action"].toString();
        QString clientId = obj["client_id"].toString();
        QString timestamp = obj["timestamp"].toString();

        qDebug() << "Page navigation received:" << action << "from client:" << clientId;
        emit pageNavigationReceived(action, clientId, timestamp);
        return;
    }

    // Diğer mesaj tipleri için
    emit messageReceived(message);
}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    const QString msg = m_webSocket.errorString();
    qDebug() << "WebSocket error occurred:" << msg;
    emit this->error(msg);
}

void WebSocketClient::connectToDiscoveredIP(const QString &ip, quint16 port)
{
    if (isConnected()) {
        qDebug() << "Already connected to server at:" << m_serverUrl;
        emit connected();
        return;
    }

    // Construct WebSocket URL with the discovered IP and port
    QString wsUrl = QString("ws://%1:%2").arg(ip).arg(port);
    m_serverUrl = wsUrl;
    m_reconnectAttempts = 0;
    m_reconnectTimer.stop();

    qDebug() << "Connecting to discovered server at:" << m_serverUrl;
    m_webSocket.open(QUrl(m_serverUrl));
}

QString WebSocketClient::serverUrl() const
{
    return m_serverUrl;
}
