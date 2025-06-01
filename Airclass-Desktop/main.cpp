#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QStyleFactory>
#include <QEventLoop>
#include <QDebug>
#include "mainwindow.h"
#include "restapiclient.h"
#include "udpdiscoveryserver.h"
#include "websocketclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    QApplication::setPalette(darkPalette);

    QPixmap pixmap(":/images/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();

    // Başlangıç bileşenleri
    RestApiClient* restApi = new RestApiClient(&app);
    UdpDiscoveryServer* udpServer = new UdpDiscoveryServer(&app);
    WebSocketClient* wsClient = new WebSocketClient(&app);

    restApi->setBaseUrl("http://159.89.19.111/airclass-api"); // Adjust this URL according to your server
/*
     QString discoveredIp;
     QEventLoop waitLoop;

     QObject::connect(udpServer, &UdpDiscoveryServer::broadcastAlindi, [&](const QString &ip){
         discoveredIp = ip;
         qDebug() << "[MAIN] Broadcast IP alındı:" << ip;

         wsClient->connectToDiscoveredIP(discoveredIp);  // Connect to the broadcast sender's IP
         waitLoop.quit();
     });

     qDebug() << "[MAIN] Broadcast bekleniyor...";
     waitLoop.exec();  // ⏳ Burada Raspberry Pi'den yayın bekleniyor
*/
    // Arayüz başlat
    MainWindow mainWindow(restApi, wsClient);  // Eğer MainWindow WebSocketClient alacaksa
    mainWindow.setWindowFlags(Qt::Window);
    mainWindow.resize(1024, 768);

    QTimer::singleShot(500, [&]() {
        mainWindow.showNormal();
        splash.finish(&mainWindow);
    });

    return app.exec();
}
