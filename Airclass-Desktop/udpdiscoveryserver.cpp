#include "udpdiscoveryserver.h"
#include <QNetworkInterface>
#include <QDebug>

UdpDiscoveryServer::UdpDiscoveryServer(QObject *parent)
    : QObject(parent), udpSocket(new QUdpSocket(this)), listenPort(9999)
{
    if (!udpSocket->bind(QHostAddress::AnyIPv4, listenPort)) {
        qCritical() << "❌ Port bağlama başarısız:" << udpSocket->errorString();
        return;
    }

    qDebug() << "[UDP] Raspberry Pi'den gelen broadcast bekleniyor...";
    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpDiscoveryServer::handleReadyRead);
}

UdpDiscoveryServer::~UdpDiscoveryServer()
{
    if (udpSocket->isOpen()) {
        udpSocket->close();
    }
}


QString UdpDiscoveryServer::getLocalIPv4ForSender(const QHostAddress &sender) const
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp)
            && interface.flags().testFlag(QNetworkInterface::IsRunning)
            && !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            for (const QNetworkAddressEntry& entry : interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol &&
                    entry.netmask().isNull() == false &&
                    entry.ip().isInSubnet(QHostAddress::parseSubnet(sender.toString() + "/" + entry.netmask().toString()))) {
                    return entry.ip().toString();
                }
            }
        }
    }
    return {};
}


void UdpDiscoveryServer::handleReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(udpSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        qDebug() << "[UDP] Broadcast alındı:" << datagram;

        QString localIP = getLocalIPv4ForSender(sender);
        if (localIP.isEmpty()) {
            qCritical() << "❌ IP alınamadı (sender ile aynı subnet bulunamadı).";
            return;
        }

        QByteArray response = localIP.toUtf8();
        udpSocket->writeDatagram(response, sender, senderPort);
        qDebug() << "[UDP] IP gönderildi:" << localIP << " -->" << sender.toString();

        emit broadcastAlindi(sender.toString());
    }
}


