/****************************************************************************
** Meta object code from reading C++ file 'HandwritingCanvasItem.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ui/HandwritingCanvasItem.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HandwritingCanvasItem.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN21HandwritingCanvasItemE_t {};
} // unnamed namespace

template <> constexpr inline auto HandwritingCanvasItem::qt_create_metaobjectdata<qt_meta_tag_ZN21HandwritingCanvasItemE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "HandwritingCanvasItem",
        "controllerChanged",
        "",
        "glyphDragActiveChanged",
        "runPreparationStarted",
        "runPreparationFinished",
        "onInvalidated",
        "onRunTick",
        "controller",
        "WriterController*",
        "glyphDragActive"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'controllerChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'glyphDragActiveChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runPreparationStarted'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runPreparationFinished'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onInvalidated'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRunTick'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'controller'
        QtMocHelpers::PropertyData<WriterController*>(8, 0x80000000 | 9, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 0),
        // property 'glyphDragActive'
        QtMocHelpers::PropertyData<bool>(10, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<HandwritingCanvasItem, qt_meta_tag_ZN21HandwritingCanvasItemE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject HandwritingCanvasItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickPaintedItem::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21HandwritingCanvasItemE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21HandwritingCanvasItemE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN21HandwritingCanvasItemE_t>.metaTypes,
    nullptr
} };

void HandwritingCanvasItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<HandwritingCanvasItem *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->controllerChanged(); break;
        case 1: _t->glyphDragActiveChanged(); break;
        case 2: _t->runPreparationStarted(); break;
        case 3: _t->runPreparationFinished(); break;
        case 4: _t->onInvalidated(); break;
        case 5: _t->onRunTick(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (HandwritingCanvasItem::*)()>(_a, &HandwritingCanvasItem::controllerChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (HandwritingCanvasItem::*)()>(_a, &HandwritingCanvasItem::glyphDragActiveChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (HandwritingCanvasItem::*)()>(_a, &HandwritingCanvasItem::runPreparationStarted, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (HandwritingCanvasItem::*)()>(_a, &HandwritingCanvasItem::runPreparationFinished, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<WriterController**>(_v) = _t->controller(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->glyphDragActive(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setController(*reinterpret_cast<WriterController**>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *HandwritingCanvasItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HandwritingCanvasItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21HandwritingCanvasItemE_t>.strings))
        return static_cast<void*>(this);
    return QQuickPaintedItem::qt_metacast(_clname);
}

int HandwritingCanvasItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickPaintedItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void HandwritingCanvasItem::controllerChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void HandwritingCanvasItem::glyphDragActiveChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void HandwritingCanvasItem::runPreparationStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void HandwritingCanvasItem::runPreparationFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
