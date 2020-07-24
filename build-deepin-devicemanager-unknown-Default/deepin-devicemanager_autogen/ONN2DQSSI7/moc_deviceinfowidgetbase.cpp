/****************************************************************************
** Meta object code from reading C++ file 'deviceinfowidgetbase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../deepin-devicemanager/src/Widget/deviceinfowidgetbase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'deviceinfowidgetbase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DeivceInfoBrower_t {
    QByteArrayData data[3];
    char stringdata0[32];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DeivceInfoBrower_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DeivceInfoBrower_t qt_meta_stringdata_DeivceInfoBrower = {
    {
QT_MOC_LITERAL(0, 0, 16), // "DeivceInfoBrower"
QT_MOC_LITERAL(1, 17, 13), // "fillClipboard"
QT_MOC_LITERAL(2, 31, 0) // ""

    },
    "DeivceInfoBrower\0fillClipboard\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeivceInfoBrower[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void DeivceInfoBrower::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DeivceInfoBrower *_t = static_cast<DeivceInfoBrower *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->fillClipboard(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject DeivceInfoBrower::staticMetaObject = {
    { &Dtk::Widget::DTextBrowser::staticMetaObject, qt_meta_stringdata_DeivceInfoBrower.data,
      qt_meta_data_DeivceInfoBrower,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DeivceInfoBrower::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeivceInfoBrower::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DeivceInfoBrower.stringdata0))
        return static_cast<void*>(this);
    return Dtk::Widget::DTextBrowser::qt_metacast(_clname);
}

int DeivceInfoBrower::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Dtk::Widget::DTextBrowser::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_DeviceInfoWidgetBase_t {
    QByteArrayData data[7];
    char stringdata0[88];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DeviceInfoWidgetBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DeviceInfoWidgetBase_t qt_meta_stringdata_DeviceInfoWidgetBase = {
    {
QT_MOC_LITERAL(0, 0, 20), // "DeviceInfoWidgetBase"
QT_MOC_LITERAL(1, 21, 20), // "OnCurrentItemClicked"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 11), // "QModelIndex"
QT_MOC_LITERAL(4, 55, 5), // "index"
QT_MOC_LITERAL(5, 61, 14), // "onExportToFile"
QT_MOC_LITERAL(6, 76, 11) // "changeTheme"

    },
    "DeviceInfoWidgetBase\0OnCurrentItemClicked\0"
    "\0QModelIndex\0index\0onExportToFile\0"
    "changeTheme"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeviceInfoWidgetBase[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x0a /* Public */,
       5,    0,   32,    2, 0x0a /* Public */,
       6,    0,   33,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Bool,
    QMetaType::Void,

       0        // eod
};

void DeviceInfoWidgetBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DeviceInfoWidgetBase *_t = static_cast<DeviceInfoWidgetBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnCurrentItemClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 1: { bool _r = _t->onExportToFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 2: _t->changeTheme(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DeviceInfoWidgetBase::staticMetaObject = {
    { &Dtk::Widget::DWidget::staticMetaObject, qt_meta_stringdata_DeviceInfoWidgetBase.data,
      qt_meta_data_DeviceInfoWidgetBase,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DeviceInfoWidgetBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeviceInfoWidgetBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DeviceInfoWidgetBase.stringdata0))
        return static_cast<void*>(this);
    return Dtk::Widget::DWidget::qt_metacast(_clname);
}

int DeviceInfoWidgetBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Dtk::Widget::DWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
