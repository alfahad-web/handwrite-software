/****************************************************************************
** Meta object code from reading C++ file 'GcodeController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/gcode/GcodeController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GcodeController.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15GcodeControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto GcodeController::qt_create_metaobjectdata<qt_meta_tag_ZN15GcodeControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GcodeController",
        "generatedGcodeChanged",
        "",
        "gcodeStaleChanged",
        "pageLineMapChanged",
        "onLayoutInvalidated",
        "regenerate",
        "saveGcodeFile",
        "openGcodeFile",
        "copyToClipboard",
        "gcodeForPageRange",
        "startPage",
        "endPageExclusive",
        "regeneratePage",
        "pageIndex",
        "pageProgramLineCount",
        "generatedGcode",
        "gcodeStale",
        "pageCount"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'generatedGcodeChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'gcodeStaleChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pageLineMapChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onLayoutInvalidated'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'regenerate'
        QtMocHelpers::MethodData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'saveGcodeFile'
        QtMocHelpers::MethodData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openGcodeFile'
        QtMocHelpers::MethodData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'copyToClipboard'
        QtMocHelpers::MethodData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'gcodeForPageRange'
        QtMocHelpers::MethodData<QString(int, int) const>(10, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::Int, 11 }, { QMetaType::Int, 12 },
        }}),
        // Method 'regeneratePage'
        QtMocHelpers::MethodData<bool(int)>(13, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 14 },
        }}),
        // Method 'pageProgramLineCount'
        QtMocHelpers::MethodData<int(int) const>(15, 2, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::Int, 14 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'generatedGcode'
        QtMocHelpers::PropertyData<QString>(16, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable, 0),
        // property 'gcodeStale'
        QtMocHelpers::PropertyData<bool>(17, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'pageCount'
        QtMocHelpers::PropertyData<int>(18, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<GcodeController, qt_meta_tag_ZN15GcodeControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GcodeController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15GcodeControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15GcodeControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15GcodeControllerE_t>.metaTypes,
    nullptr
} };

void GcodeController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GcodeController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->generatedGcodeChanged(); break;
        case 1: _t->gcodeStaleChanged(); break;
        case 2: _t->pageLineMapChanged(); break;
        case 3: _t->onLayoutInvalidated(); break;
        case 4: _t->regenerate(); break;
        case 5: _t->saveGcodeFile(); break;
        case 6: _t->openGcodeFile(); break;
        case 7: _t->copyToClipboard(); break;
        case 8: { QString _r = _t->gcodeForPageRange((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->regeneratePage((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { int _r = _t->pageProgramLineCount((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (GcodeController::*)()>(_a, &GcodeController::generatedGcodeChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (GcodeController::*)()>(_a, &GcodeController::gcodeStaleChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (GcodeController::*)()>(_a, &GcodeController::pageLineMapChanged, 2))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->generatedGcode(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->gcodeStale(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->pageCount(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setGcodeText(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *GcodeController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GcodeController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15GcodeControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GcodeController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void GcodeController::generatedGcodeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GcodeController::gcodeStaleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void GcodeController::pageLineMapChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
