QT       += core gui printsupport widgets pdf pdfwidgets websockets network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Add websockets module with version check
greaterThan(QT_MAJOR_VERSION, 4): QT += websockets

# Add Qt include paths
INCLUDEPATH += $$[QT_INSTALL_HEADERS]
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtGui
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtWidgets
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtWebSockets
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtNetwork
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtMultimedia

DEPENDPATH += $$[QT_INSTALL_HEADERS]/QtWebSockets
DEPENDPATH += $$[QT_INSTALL_HEADERS]/QtNetwork
DEPENDPATH += $$[QT_INSTALL_HEADERS]/QtMultimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    attendancemanager.cpp \
    custompdfviewer.cpp \
    gestureprocessor.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    presentationmanager.cpp \
    sessiondialog.cpp \
    student.cpp \
    testdata.cpp \
    uicontroller.cpp \
    websocketclient.cpp \
    restapiclient.cpp \
    drawinglayer.cpp \
    udpdiscoveryserver.cpp \
    gestureguide.cpp


HEADERS += \
    attendancemanager.h \
    custompdfviewer.hpp \
    gestureprocessor.h \
    logindialog.h \
    mainwindow.h \
    presentationmanager.h \
    sessiondialog.h \
    student.h \
    testdata.h \
    ui_mainwindow.h \
    uicontroller.h \
    websocketclient.h \
    restapiclient.h \
    drawinglayer.h \
    udpdiscoveryserver.h \
    gestureguide.h


FORMS += \
    mainwindow.ui

# Translation files
TRANSLATIONS = \
    AirClassMay2_en_US.ts \
    AirClassMay2_tr_TR.ts

# Translation configuration
CONFIG += lrelease embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
