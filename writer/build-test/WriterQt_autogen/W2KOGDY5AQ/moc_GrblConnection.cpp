/****************************************************************************
** Meta object code from reading C++ file 'GrblConnection.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/cnc/GrblConnection.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GrblConnection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
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
struct qt_meta_tag_ZN14GrblConnectionE_t {};
} // unnamed namespace

template <> constexpr inline auto GrblConnection::qt_create_metaobjectdata<qt_meta_tag_ZN14GrblConnectionE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GrblConnection",
        "connectedChanged",
        "",
        "consoleLogChanged",
        "availablePortsChanged",
        "portNameChanged",
        "streamingChanged",
        "streamProgressChanged",
        "streamFinished",
        "success",
        "positionChanged",
        "machineStateChanged",
        "onReadyRead",
        "onWakeTimer",
        "refreshPorts",
        "connectPort",
        "disconnectPort",
        "sendLine",
        "line",
        "sendUserCommand",
        "commandHistoryOlder",
        "currentDraft",
        "commandHistoryNewer",
        "resetCommandHistoryBrowse",
        "streamProgram",
        "program",
        "cancelStream",
        "abortStreamAndRecover",
        "clearLog",
        "logMessage",
        "msg",
        "sendRealtimeCommand",
        "cmd",
        "setWorkOriginHere",
        "applyMotionTuning",
        "junctionDeviation",
        "accelX",
        "accelY",
        "connected",
        "consoleLog",
        "availablePorts",
        "portLabels",
        "portName",
        "streaming",
        "streamProgress",
        "serialAvailable",
        "posX",
        "posY",
        "posZ",
        "positionKnown",
        "machineState",
        "commandBlocked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectedChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'consoleLogChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'availablePortsChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'portNameChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'streamingChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'streamProgressChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'streamFinished'
        QtMocHelpers::SignalData<void(bool)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 9 },
        }}),
        // Signal 'positionChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'machineStateChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onReadyRead'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onWakeTimer'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'refreshPorts'
        QtMocHelpers::MethodData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'connectPort'
        QtMocHelpers::MethodData<bool()>(15, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'disconnectPort'
        QtMocHelpers::MethodData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'sendLine'
        QtMocHelpers::MethodData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Method 'sendUserCommand'
        QtMocHelpers::MethodData<void(const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Method 'commandHistoryOlder'
        QtMocHelpers::MethodData<QString(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 21 },
        }}),
        // Method 'commandHistoryNewer'
        QtMocHelpers::MethodData<QString()>(22, 2, QMC::AccessPublic, QMetaType::QString),
        // Method 'resetCommandHistoryBrowse'
        QtMocHelpers::MethodData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'streamProgram'
        QtMocHelpers::MethodData<void(const QString &)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 25 },
        }}),
        // Method 'cancelStream'
        QtMocHelpers::MethodData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'abortStreamAndRecover'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'clearLog'
        QtMocHelpers::MethodData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'logMessage'
        QtMocHelpers::MethodData<void(const QString &)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Method 'sendRealtimeCommand'
        QtMocHelpers::MethodData<void(const QString &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 },
        }}),
        // Method 'setWorkOriginHere'
        QtMocHelpers::MethodData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'applyMotionTuning'
        QtMocHelpers::MethodData<void(double, double, double)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 35 }, { QMetaType::Double, 36 }, { QMetaType::Double, 37 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'connected'
        QtMocHelpers::PropertyData<bool>(38, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'consoleLog'
        QtMocHelpers::PropertyData<QString>(39, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'availablePorts'
        QtMocHelpers::PropertyData<QStringList>(40, QMetaType::QStringList, QMC::DefaultPropertyFlags, 2),
        // property 'portLabels'
        QtMocHelpers::PropertyData<QStringList>(41, QMetaType::QStringList, QMC::DefaultPropertyFlags, 2),
        // property 'portName'
        QtMocHelpers::PropertyData<QString>(42, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'streaming'
        QtMocHelpers::PropertyData<bool>(43, QMetaType::Bool, QMC::DefaultPropertyFlags, 4),
        // property 'streamProgress'
        QtMocHelpers::PropertyData<double>(44, QMetaType::Double, QMC::DefaultPropertyFlags, 5),
        // property 'serialAvailable'
        QtMocHelpers::PropertyData<bool>(45, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'posX'
        QtMocHelpers::PropertyData<double>(46, QMetaType::Double, QMC::DefaultPropertyFlags, 7),
        // property 'posY'
        QtMocHelpers::PropertyData<double>(47, QMetaType::Double, QMC::DefaultPropertyFlags, 7),
        // property 'posZ'
        QtMocHelpers::PropertyData<double>(48, QMetaType::Double, QMC::DefaultPropertyFlags, 7),
        // property 'positionKnown'
        QtMocHelpers::PropertyData<bool>(49, QMetaType::Bool, QMC::DefaultPropertyFlags, 7),
        // property 'machineState'
        QtMocHelpers::PropertyData<QString>(50, QMetaType::QString, QMC::DefaultPropertyFlags, 8),
        // property 'commandBlocked'
        QtMocHelpers::PropertyData<bool>(51, QMetaType::Bool, QMC::DefaultPropertyFlags, 8),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<GrblConnection, qt_meta_tag_ZN14GrblConnectionE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GrblConnection::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14GrblConnectionE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14GrblConnectionE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14GrblConnectionE_t>.metaTypes,
    nullptr
} };

void GrblConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GrblConnection *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectedChanged(); break;
        case 1: _t->consoleLogChanged(); break;
        case 2: _t->availablePortsChanged(); break;
        case 3: _t->portNameChanged(); break;
        case 4: _t->streamingChanged(); break;
        case 5: _t->streamProgressChanged(); break;
        case 6: _t->streamFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->positionChanged(); break;
        case 8: _t->machineStateChanged(); break;
        case 9: _t->onReadyRead(); break;
        case 10: _t->onWakeTimer(); break;
        case 11: _t->refreshPorts(); break;
        case 12: { bool _r = _t->connectPort();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 13: _t->disconnectPort(); break;
        case 14: _t->sendLine((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->sendUserCommand((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: { QString _r = _t->commandHistoryOlder((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 17: { QString _r = _t->commandHistoryNewer();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 18: _t->resetCommandHistoryBrowse(); break;
        case 19: _t->streamProgram((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 20: _t->cancelStream(); break;
        case 21: _t->abortStreamAndRecover(); break;
        case 22: _t->clearLog(); break;
        case 23: _t->logMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 24: _t->sendRealtimeCommand((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 25: _t->setWorkOriginHere(); break;
        case 26: _t->applyMotionTuning((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::connectedChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::consoleLogChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::availablePortsChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::portNameChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::streamingChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::streamProgressChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)(bool )>(_a, &GrblConnection::streamFinished, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::positionChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (GrblConnection::*)()>(_a, &GrblConnection::machineStateChanged, 8))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->connected(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->consoleLog(); break;
        case 2: *reinterpret_cast<QStringList*>(_v) = _t->availablePorts(); break;
        case 3: *reinterpret_cast<QStringList*>(_v) = _t->portLabels(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->portName(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->streaming(); break;
        case 6: *reinterpret_cast<double*>(_v) = _t->streamProgress(); break;
        case 7: *reinterpret_cast<bool*>(_v) = _t->serialAvailable(); break;
        case 8: *reinterpret_cast<double*>(_v) = _t->posX(); break;
        case 9: *reinterpret_cast<double*>(_v) = _t->posY(); break;
        case 10: *reinterpret_cast<double*>(_v) = _t->posZ(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->positionKnown(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->machineState(); break;
        case 13: *reinterpret_cast<bool*>(_v) = _t->commandBlocked(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 4: _t->setPortName(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *GrblConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GrblConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14GrblConnectionE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GrblConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 27;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void GrblConnection::connectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GrblConnection::consoleLogChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void GrblConnection::availablePortsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GrblConnection::portNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void GrblConnection::streamingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void GrblConnection::streamProgressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void GrblConnection::streamFinished(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void GrblConnection::positionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void GrblConnection::machineStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}
QT_WARNING_POP
