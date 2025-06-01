/****************************************************************************
** Meta object code from reading C++ file 'restapiclient.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "restapiclient.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'restapiclient.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13RestApiClientE_t {};
} // unnamed namespace

template <> constexpr inline auto RestApiClient::qt_create_metaobjectdata<qt_meta_tag_ZN13RestApiClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RestApiClient",
        "loginSuccess",
        "",
        "data",
        "loginFailed",
        "message",
        "registerSuccess",
        "registerFailed",
        "sessionsReceived",
        "sessions",
        "activeSessionReceived",
        "session",
        "sessionCreated",
        "sessionActivated",
        "sessionClosed",
        "sessionId",
        "classroomsReceived",
        "classrooms",
        "classroomCreated",
        "classroom",
        "classroomUpdated",
        "classroomDeleted",
        "attendanceListReceived",
        "records",
        "attendanceMarked",
        "record",
        "attendanceCodeGenerated",
        "code",
        "qrBase64",
        "expiry",
        "requestsReceived",
        "requests",
        "requestCreated",
        "request",
        "requestUpdated",
        "requestId",
        "action",
        "newSpeakRequest",
        "studentId",
        "studentName",
        "error",
        "qrCodeReceived",
        "handleNetworkReply",
        "QNetworkReply*",
        "reply",
        "pollRequests"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'loginSuccess'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 3 },
        }}),
        // Signal 'loginFailed'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'registerSuccess'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 3 },
        }}),
        // Signal 'registerFailed'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'sessionsReceived'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonArray, 9 },
        }}),
        // Signal 'activeSessionReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 11 },
        }}),
        // Signal 'sessionCreated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 11 },
        }}),
        // Signal 'sessionActivated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 11 },
        }}),
        // Signal 'sessionClosed'
        QtMocHelpers::SignalData<void(const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 },
        }}),
        // Signal 'classroomsReceived'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonArray, 17 },
        }}),
        // Signal 'classroomCreated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 19 },
        }}),
        // Signal 'classroomUpdated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 19 },
        }}),
        // Signal 'classroomDeleted'
        QtMocHelpers::SignalData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'attendanceListReceived'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonArray, 23 },
        }}),
        // Signal 'attendanceMarked'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 25 },
        }}),
        // Signal 'attendanceCodeGenerated'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 }, { QMetaType::QString, 28 }, { QMetaType::QString, 29 },
        }}),
        // Signal 'requestsReceived'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonArray, 31 },
        }}),
        // Signal 'requestCreated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 33 },
        }}),
        // Signal 'requestUpdated'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 35 }, { QMetaType::QString, 36 },
        }}),
        // Signal 'newSpeakRequest'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &)>(37, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 38 }, { QMetaType::QString, 39 }, { QMetaType::QString, 35 },
        }}),
        // Signal 'error'
        QtMocHelpers::SignalData<void(const QString &)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'qrCodeReceived'
        QtMocHelpers::SignalData<void(const QString &)>(41, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 28 },
        }}),
        // Slot 'handleNetworkReply'
        QtMocHelpers::SlotData<void(QNetworkReply *)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 43, 44 },
        }}),
        // Slot 'pollRequests'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RestApiClient, qt_meta_tag_ZN13RestApiClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RestApiClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13RestApiClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13RestApiClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13RestApiClientE_t>.metaTypes,
    nullptr
} };

void RestApiClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RestApiClient *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->loginSuccess((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 1: _t->loginFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->registerSuccess((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 3: _t->registerFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->sessionsReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 5: _t->activeSessionReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 6: _t->sessionCreated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 7: _t->sessionActivated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 8: _t->sessionClosed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->classroomsReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 10: _t->classroomCreated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 11: _t->classroomUpdated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 12: _t->classroomDeleted(); break;
        case 13: _t->attendanceListReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 14: _t->attendanceMarked((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 15: _t->attendanceCodeGenerated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 16: _t->requestsReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 17: _t->requestCreated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 18: _t->requestUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 19: _t->newSpeakRequest((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 20: _t->error((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->qrCodeReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->handleNetworkReply((*reinterpret_cast< std::add_pointer_t<QNetworkReply*>>(_a[1]))); break;
        case 23: _t->pollRequests(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 22:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QNetworkReply* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::loginSuccess, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & )>(_a, &RestApiClient::loginFailed, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::registerSuccess, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & )>(_a, &RestApiClient::registerFailed, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonArray & )>(_a, &RestApiClient::sessionsReceived, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::activeSessionReceived, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::sessionCreated, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::sessionActivated, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & )>(_a, &RestApiClient::sessionClosed, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonArray & )>(_a, &RestApiClient::classroomsReceived, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::classroomCreated, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::classroomUpdated, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)()>(_a, &RestApiClient::classroomDeleted, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonArray & )>(_a, &RestApiClient::attendanceListReceived, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::attendanceMarked, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & , const QString & , const QString & )>(_a, &RestApiClient::attendanceCodeGenerated, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonArray & )>(_a, &RestApiClient::requestsReceived, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QJsonObject & )>(_a, &RestApiClient::requestCreated, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & , const QString & )>(_a, &RestApiClient::requestUpdated, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & , const QString & , const QString & )>(_a, &RestApiClient::newSpeakRequest, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & )>(_a, &RestApiClient::error, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (RestApiClient::*)(const QString & )>(_a, &RestApiClient::qrCodeReceived, 21))
            return;
    }
}

const QMetaObject *RestApiClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RestApiClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13RestApiClientE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RestApiClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void RestApiClient::loginSuccess(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void RestApiClient::loginFailed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void RestApiClient::registerSuccess(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void RestApiClient::registerFailed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void RestApiClient::sessionsReceived(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void RestApiClient::activeSessionReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void RestApiClient::sessionCreated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void RestApiClient::sessionActivated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void RestApiClient::sessionClosed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void RestApiClient::classroomsReceived(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void RestApiClient::classroomCreated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}

// SIGNAL 11
void RestApiClient::classroomUpdated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void RestApiClient::classroomDeleted()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void RestApiClient::attendanceListReceived(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void RestApiClient::attendanceMarked(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}

// SIGNAL 15
void RestApiClient::attendanceCodeGenerated(const QString & _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1, _t2, _t3);
}

// SIGNAL 16
void RestApiClient::requestsReceived(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void RestApiClient::requestCreated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1);
}

// SIGNAL 18
void RestApiClient::requestUpdated(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1, _t2);
}

// SIGNAL 19
void RestApiClient::newSpeakRequest(const QString & _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1, _t2, _t3);
}

// SIGNAL 20
void RestApiClient::error(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1);
}

// SIGNAL 21
void RestApiClient::qrCodeReceived(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1);
}
QT_WARNING_POP
