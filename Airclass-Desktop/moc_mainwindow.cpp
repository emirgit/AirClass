/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "onConnected",
        "",
        "onDisconnected",
        "handleServerMessage",
        "message",
        "",
        "data",
        "onLoginFailed",
        "onAttendanceListReceived",
        "records",
        "onAttendanceCodeGenerated",
        "code",
        "qrBase64",
        "expiry",
        "onRequestsReceived",
        "requests",
        "onNewSpeakRequest",
        "studentId",
        "studentName",
        "requestId",
        "onRequestUpdated",
        "action",
        "onSessionClosed",
        "sessionId",
        "onQrCodeReceived",
        "closeSession",
        "on_actionOpen_triggered",
        "on_actionExit_triggered",
        "on_actionConnect_triggered",
        "on_actionDisconnect_triggered",
        "on_nextButton_clicked",
        "on_prevButton_clicked",
        "on_zoomInButton_clicked",
        "on_zoomOutButton_clicked",
        "on_saveImageButton_clicked",
        "on_printButton_clicked",
        "on_displayFullScreenButton_clicked",
        "on_generateAttendanceCodeButton_clicked",
        "on_refreshAttendanceButton_clicked",
        "on_downloadAttendanceButton_clicked",
        "on_approveRequestButton_clicked",
        "on_rejectRequestButton_clicked",
        "addRecentPresentation",
        "filePath",
        "handleAttendanceUpdate",
        "handleAttendanceQRCode",
        "handleSpeakRequest",
        "handleRequestUpdate",
        "on_timerStartButton_clicked",
        "on_timerStopButton_clicked",
        "on_timerResetButton_clicked",
        "on_timerMinutesSpinBox_valueChanged",
        "value",
        "on_timerSecondsSpinBox_valueChanged",
        "on_clearDrawingButton_clicked",
        "on_eraserButton_clicked",
        "on_colorButton_clicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onConnected'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDisconnected'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleServerMessage'
        QtMocHelpers::SlotData<void(const QString &)>(4, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'onLoginSuccess'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QJsonObject, 7 },
        }}),
        // Slot 'onLoginFailed'
        QtMocHelpers::SlotData<void(const QString &)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'onAttendanceListReceived'
        QtMocHelpers::SlotData<void(const QJsonArray &)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QJsonArray, 10 },
        }}),
        // Slot 'onAttendanceCodeGenerated'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 12 }, { QMetaType::QString, 13 }, { QMetaType::QString, 14 },
        }}),
        // Slot 'onRequestsReceived'
        QtMocHelpers::SlotData<void(const QJsonArray &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QJsonArray, 16 },
        }}),
        // Slot 'onNewSpeakRequest'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 18 }, { QMetaType::QString, 19 }, { QMetaType::QString, 20 },
        }}),
        // Slot 'onRequestUpdated'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 20 }, { QMetaType::QString, 22 },
        }}),
        // Slot 'onSessionClosed'
        QtMocHelpers::SlotData<void(const QString &)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 24 },
        }}),
        // Slot 'onQrCodeReceived'
        QtMocHelpers::SlotData<void(const QString &)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Slot 'closeSession'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionOpen_triggered'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionExit_triggered'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionConnect_triggered'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionDisconnect_triggered'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_nextButton_clicked'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_prevButton_clicked'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_zoomInButton_clicked'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_zoomOutButton_clicked'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_saveImageButton_clicked'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_printButton_clicked'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_displayFullScreenButton_clicked'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_generateAttendanceCodeButton_clicked'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_refreshAttendanceButton_clicked'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_downloadAttendanceButton_clicked'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_approveRequestButton_clicked'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_rejectRequestButton_clicked'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'addRecentPresentation'
        QtMocHelpers::SlotData<void(const QString &)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 44 },
        }}),
        // Slot 'handleAttendanceUpdate'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(45, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QJsonObject, 7 },
        }}),
        // Slot 'handleAttendanceQRCode'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QJsonObject, 7 },
        }}),
        // Slot 'handleSpeakRequest'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(47, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QJsonObject, 7 },
        }}),
        // Slot 'handleRequestUpdate'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QJsonObject, 7 },
        }}),
        // Slot 'on_timerStartButton_clicked'
        QtMocHelpers::SlotData<void()>(49, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_timerStopButton_clicked'
        QtMocHelpers::SlotData<void()>(50, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_timerResetButton_clicked'
        QtMocHelpers::SlotData<void()>(51, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_timerMinutesSpinBox_valueChanged'
        QtMocHelpers::SlotData<void(int)>(52, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 53 },
        }}),
        // Slot 'on_timerSecondsSpinBox_valueChanged'
        QtMocHelpers::SlotData<void(int)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 53 },
        }}),
        // Slot 'on_clearDrawingButton_clicked'
        QtMocHelpers::SlotData<void()>(55, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_eraserButton_clicked'
        QtMocHelpers::SlotData<void()>(56, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_colorButton_clicked'
        QtMocHelpers::SlotData<void()>(57, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onConnected(); break;
        case 1: _t->onDisconnected(); break;
        case 2: _t->handleServerMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->onLoginSuccess((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 4: _t->onLoginFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->onAttendanceListReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 6: _t->onAttendanceCodeGenerated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 7: _t->onRequestsReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 8: _t->onNewSpeakRequest((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 9: _t->onRequestUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->onSessionClosed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->onQrCodeReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->closeSession(); break;
        case 13: _t->on_actionOpen_triggered(); break;
        case 14: _t->on_actionExit_triggered(); break;
        case 15: _t->on_actionConnect_triggered(); break;
        case 16: _t->on_actionDisconnect_triggered(); break;
        case 17: _t->on_nextButton_clicked(); break;
        case 18: _t->on_prevButton_clicked(); break;
        case 19: _t->on_zoomInButton_clicked(); break;
        case 20: _t->on_zoomOutButton_clicked(); break;
        case 21: _t->on_saveImageButton_clicked(); break;
        case 22: _t->on_printButton_clicked(); break;
        case 23: _t->on_displayFullScreenButton_clicked(); break;
        case 24: _t->on_generateAttendanceCodeButton_clicked(); break;
        case 25: _t->on_refreshAttendanceButton_clicked(); break;
        case 26: _t->on_downloadAttendanceButton_clicked(); break;
        case 27: _t->on_approveRequestButton_clicked(); break;
        case 28: _t->on_rejectRequestButton_clicked(); break;
        case 29: _t->addRecentPresentation((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 30: _t->handleAttendanceUpdate((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 31: _t->handleAttendanceQRCode((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 32: _t->handleSpeakRequest((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 33: _t->handleRequestUpdate((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 34: _t->on_timerStartButton_clicked(); break;
        case 35: _t->on_timerStopButton_clicked(); break;
        case 36: _t->on_timerResetButton_clicked(); break;
        case 37: _t->on_timerMinutesSpinBox_valueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 38: _t->on_timerSecondsSpinBox_valueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 39: _t->on_clearDrawingButton_clicked(); break;
        case 40: _t->on_eraserButton_clicked(); break;
        case 41: _t->on_colorButton_clicked(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 42)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 42;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 42)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 42;
    }
    return _id;
}
QT_WARNING_POP
