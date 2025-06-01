#ifndef UDPDISCOVERYSERVER_H
#define UDPDISCOVERYSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpDiscoveryServer : public QObject
{
    Q_OBJECT

public:
    explicit UdpDiscoveryServer(QObject *parent = nullptr);
    ~UdpDiscoveryServer();

signals:
    void broadcastAlindi(const QString &ip);

private:
    QUdpSocket *udpSocket;
    quint16 listenPort;

    QString getLocalIPv4() const;
    QString getLocalIPv4ForSender(const QHostAddress &sender) const;  // ✅ Eklenen doğru fonksiyon
    void handleReadyRead();
};


#endif // UDPDISCOVERYSERVER_H
