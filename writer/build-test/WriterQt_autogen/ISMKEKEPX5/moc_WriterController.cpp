/****************************************************************************
** Meta object code from reading C++ file 'WriterController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/app/WriterController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WriterController.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16WriterControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto WriterController::qt_create_metaobjectdata<qt_meta_tag_ZN16WriterControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WriterController",
        "viewModeChanged",
        "",
        "fontFolderPathChanged",
        "fontStatusChanged",
        "projectFilePathChanged",
        "documentDirtyChanged",
        "layoutInvalidated",
        "settingsOpenChanged",
        "runActiveChanged",
        "runPausedChanged",
        "runArmedChanged",
        "runStartPageChanged",
        "pageCountChanged",
        "runPathChanged",
        "runArmVisualsChanged",
        "lineHeightCollisionWarning",
        "fontFolderMissing",
        "path",
        "projectIoError",
        "message",
        "historyChanged",
        "pickFontFolder",
        "reloadFonts",
        "startRun",
        "pauseRun",
        "resumeRun",
        "stopRun",
        "stopRunPreserveCnc",
        "advanceRunToPage",
        "page",
        "clearRunArm",
        "refreshPageMap",
        "finishPageRun",
        "onRunApproachComplete",
        "notifyLineHeightCollision",
        "exceeds",
        "generateGcode",
        "pushUndoSnapshot",
        "undo",
        "redo",
        "newWriterProject",
        "openWriterProject",
        "saveWriterProject",
        "saveWriterProjectAs",
        "loadWriterProjectFromPath",
        "viewMode",
        "fontFolderPath",
        "fontStatus",
        "projectFilePath",
        "documentDirty",
        "document",
        "DocumentModel*",
        "settings",
        "AppSettings*",
        "gcode",
        "GcodeController*",
        "grbl",
        "GrblConnection*",
        "settingsOpen",
        "runActive",
        "runPaused",
        "runArmed",
        "runStartPage",
        "pageCount",
        "runStartDistanceCm",
        "runEndDistanceCm",
        "executingPage",
        "canUndo",
        "canRedo"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'viewModeChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fontFolderPathChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fontStatusChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'projectFilePathChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'documentDirtyChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'layoutInvalidated'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'settingsOpenChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runActiveChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runPausedChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runArmedChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runStartPageChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pageCountChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runPathChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runArmVisualsChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lineHeightCollisionWarning'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fontFolderMissing'
        QtMocHelpers::SignalData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Signal 'projectIoError'
        QtMocHelpers::SignalData<void(const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 20 },
        }}),
        // Signal 'historyChanged'
        QtMocHelpers::SignalData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'pickFontFolder'
        QtMocHelpers::MethodData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'reloadFonts'
        QtMocHelpers::MethodData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'startRun'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'pauseRun'
        QtMocHelpers::MethodData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resumeRun'
        QtMocHelpers::MethodData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stopRun'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stopRunPreserveCnc'
        QtMocHelpers::MethodData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'advanceRunToPage'
        QtMocHelpers::MethodData<void(int)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 30 },
        }}),
        // Method 'clearRunArm'
        QtMocHelpers::MethodData<void()>(31, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshPageMap'
        QtMocHelpers::MethodData<void()>(32, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'finishPageRun'
        QtMocHelpers::MethodData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'onRunApproachComplete'
        QtMocHelpers::MethodData<void()>(34, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'notifyLineHeightCollision'
        QtMocHelpers::MethodData<void(bool)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 36 },
        }}),
        // Method 'generateGcode'
        QtMocHelpers::MethodData<bool()>(37, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'pushUndoSnapshot'
        QtMocHelpers::MethodData<void()>(38, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'undo'
        QtMocHelpers::MethodData<bool()>(39, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'redo'
        QtMocHelpers::MethodData<bool()>(40, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'newWriterProject'
        QtMocHelpers::MethodData<void()>(41, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openWriterProject'
        QtMocHelpers::MethodData<void()>(42, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'saveWriterProject'
        QtMocHelpers::MethodData<void()>(43, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'saveWriterProjectAs'
        QtMocHelpers::MethodData<void()>(44, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'loadWriterProjectFromPath'
        QtMocHelpers::MethodData<bool(const QString &)>(45, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 18 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'viewMode'
        QtMocHelpers::PropertyData<QString>(46, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'fontFolderPath'
        QtMocHelpers::PropertyData<QString>(47, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'fontStatus'
        QtMocHelpers::PropertyData<QString>(48, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'projectFilePath'
        QtMocHelpers::PropertyData<QString>(49, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'documentDirty'
        QtMocHelpers::PropertyData<bool>(50, QMetaType::Bool, QMC::DefaultPropertyFlags, 4),
        // property 'document'
        QtMocHelpers::PropertyData<DocumentModel*>(51, 0x80000000 | 52, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'settings'
        QtMocHelpers::PropertyData<AppSettings*>(53, 0x80000000 | 54, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'gcode'
        QtMocHelpers::PropertyData<GcodeController*>(55, 0x80000000 | 56, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'grbl'
        QtMocHelpers::PropertyData<GrblConnection*>(57, 0x80000000 | 58, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'settingsOpen'
        QtMocHelpers::PropertyData<bool>(59, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 6),
        // property 'runActive'
        QtMocHelpers::PropertyData<bool>(60, QMetaType::Bool, QMC::DefaultPropertyFlags, 7),
        // property 'runPaused'
        QtMocHelpers::PropertyData<bool>(61, QMetaType::Bool, QMC::DefaultPropertyFlags, 8),
        // property 'runArmed'
        QtMocHelpers::PropertyData<bool>(62, QMetaType::Bool, QMC::DefaultPropertyFlags, 9),
        // property 'runStartPage'
        QtMocHelpers::PropertyData<int>(63, QMetaType::Int, QMC::DefaultPropertyFlags, 10),
        // property 'pageCount'
        QtMocHelpers::PropertyData<int>(64, QMetaType::Int, QMC::DefaultPropertyFlags, 11),
        // property 'runStartDistanceCm'
        QtMocHelpers::PropertyData<double>(65, QMetaType::Double, QMC::DefaultPropertyFlags, 12),
        // property 'runEndDistanceCm'
        QtMocHelpers::PropertyData<double>(66, QMetaType::Double, QMC::DefaultPropertyFlags, 12),
        // property 'executingPage'
        QtMocHelpers::PropertyData<int>(67, QMetaType::Int, QMC::DefaultPropertyFlags, 12),
        // property 'canUndo'
        QtMocHelpers::PropertyData<bool>(68, QMetaType::Bool, QMC::DefaultPropertyFlags, 17),
        // property 'canRedo'
        QtMocHelpers::PropertyData<bool>(69, QMetaType::Bool, QMC::DefaultPropertyFlags, 17),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WriterController, qt_meta_tag_ZN16WriterControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WriterController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16WriterControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16WriterControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16WriterControllerE_t>.metaTypes,
    nullptr
} };

void WriterController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WriterController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->viewModeChanged(); break;
        case 1: _t->fontFolderPathChanged(); break;
        case 2: _t->fontStatusChanged(); break;
        case 3: _t->projectFilePathChanged(); break;
        case 4: _t->documentDirtyChanged(); break;
        case 5: _t->layoutInvalidated(); break;
        case 6: _t->settingsOpenChanged(); break;
        case 7: _t->runActiveChanged(); break;
        case 8: _t->runPausedChanged(); break;
        case 9: _t->runArmedChanged(); break;
        case 10: _t->runStartPageChanged(); break;
        case 11: _t->pageCountChanged(); break;
        case 12: _t->runPathChanged(); break;
        case 13: _t->runArmVisualsChanged(); break;
        case 14: _t->lineHeightCollisionWarning(); break;
        case 15: _t->fontFolderMissing((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->projectIoError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->historyChanged(); break;
        case 18: _t->pickFontFolder(); break;
        case 19: _t->reloadFonts(); break;
        case 20: _t->startRun(); break;
        case 21: _t->pauseRun(); break;
        case 22: _t->resumeRun(); break;
        case 23: _t->stopRun(); break;
        case 24: _t->stopRunPreserveCnc(); break;
        case 25: _t->advanceRunToPage((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->clearRunArm(); break;
        case 27: _t->refreshPageMap(); break;
        case 28: _t->finishPageRun(); break;
        case 29: _t->onRunApproachComplete(); break;
        case 30: _t->notifyLineHeightCollision((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 31: { bool _r = _t->generateGcode();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 32: _t->pushUndoSnapshot(); break;
        case 33: { bool _r = _t->undo();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 34: { bool _r = _t->redo();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 35: _t->newWriterProject(); break;
        case 36: _t->openWriterProject(); break;
        case 37: _t->saveWriterProject(); break;
        case 38: _t->saveWriterProjectAs(); break;
        case 39: { bool _r = _t->loadWriterProjectFromPath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::viewModeChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::fontFolderPathChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::fontStatusChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::projectFilePathChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::documentDirtyChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::layoutInvalidated, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::settingsOpenChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::runActiveChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::runPausedChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::runArmedChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::runStartPageChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::pageCountChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::runPathChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::runArmVisualsChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::lineHeightCollisionWarning, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)(const QString & )>(_a, &WriterController::fontFolderMissing, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)(const QString & )>(_a, &WriterController::projectIoError, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (WriterController::*)()>(_a, &WriterController::historyChanged, 17))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 6:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< AppSettings* >(); break;
        case 5:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< DocumentModel* >(); break;
        case 7:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< GcodeController* >(); break;
        case 8:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< GrblConnection* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->viewMode(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->fontFolderPath(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->fontStatus(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->projectFilePath(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->documentDirty(); break;
        case 5: *reinterpret_cast<DocumentModel**>(_v) = _t->document(); break;
        case 6: *reinterpret_cast<AppSettings**>(_v) = _t->settings(); break;
        case 7: *reinterpret_cast<GcodeController**>(_v) = _t->gcode(); break;
        case 8: *reinterpret_cast<GrblConnection**>(_v) = _t->grbl(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->settingsOpen(); break;
        case 10: *reinterpret_cast<bool*>(_v) = _t->runActive(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->runPaused(); break;
        case 12: *reinterpret_cast<bool*>(_v) = _t->runArmed(); break;
        case 13: *reinterpret_cast<int*>(_v) = _t->runStartPage(); break;
        case 14: *reinterpret_cast<int*>(_v) = _t->pageCount(); break;
        case 15: *reinterpret_cast<double*>(_v) = _t->runStartDistanceCm(); break;
        case 16: *reinterpret_cast<double*>(_v) = _t->runEndDistanceCm(); break;
        case 17: *reinterpret_cast<int*>(_v) = _t->executingPage(); break;
        case 18: *reinterpret_cast<bool*>(_v) = _t->canUndo(); break;
        case 19: *reinterpret_cast<bool*>(_v) = _t->canRedo(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setViewMode(*reinterpret_cast<QString*>(_v)); break;
        case 9: _t->setSettingsOpen(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *WriterController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WriterController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16WriterControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WriterController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 40)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 40;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 40)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 40;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void WriterController::viewModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void WriterController::fontFolderPathChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void WriterController::fontStatusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void WriterController::projectFilePathChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void WriterController::documentDirtyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void WriterController::layoutInvalidated()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void WriterController::settingsOpenChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void WriterController::runActiveChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void WriterController::runPausedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void WriterController::runArmedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void WriterController::runStartPageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void WriterController::pageCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void WriterController::runPathChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void WriterController::runArmVisualsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void WriterController::lineHeightCollisionWarning()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void WriterController::fontFolderMissing(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1);
}

// SIGNAL 16
void WriterController::projectIoError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void WriterController::historyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}
QT_WARNING_POP
