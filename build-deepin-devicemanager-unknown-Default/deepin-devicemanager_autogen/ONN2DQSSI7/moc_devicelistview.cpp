/****************************************************************************
** Meta object code from reading C++ file 'devicelistview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../deepin-devicemanager/src/Widget/devicelistview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'devicelistview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DeviceListviewDelegate_t {
    QByteArrayData data[1];
    char stringdata0[23];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DeviceListviewDelegate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DeviceListviewDelegate_t qt_meta_stringdata_DeviceListviewDelegate = {
    {
QT_MOC_LITERAL(0, 0, 22) // "DeviceListviewDelegate"

    },
    "DeviceListviewDelegate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeviceListviewDelegate[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void DeviceListviewDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject DeviceListviewDelegate::staticMetaObject = {
    { &Dtk::Widget::DStyledItemDelegate::staticMetaObject, qt_meta_stringdata_DeviceListviewDelegate.data,
      qt_meta_data_DeviceListviewDelegate,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DeviceListviewDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeviceListviewDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DeviceListviewDelegate.stringdata0))
        return static_cast<void*>(this);
    return Dtk::Widget::DStyledItemDelegate::qt_metacast(_clname);
}

int DeviceListviewDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Dtk::Widget::DStyledItemDelegate::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_DeviceListView_t {
    QByteArrayData data[5];
    char stringdata0[62];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DeviceListView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DeviceListView_t qt_meta_stringdata_DeviceListView = {
    {
QT_MOC_LITERAL(0, 0, 14), // "DeviceListView"
QT_MOC_LITERAL(1, 15, 14), // "onExportToFile"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 14), // "OnlvOpRightBtn"
QT_MOC_LITERAL(4, 46, 15) // "changeThemeIcon"

    },
    "DeviceListView\0onExportToFile\0\0"
    "OnlvOpRightBtn\0changeThemeIcon"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeviceListView[] = {

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
       1,    0,   29,    2, 0x0a /* Public */,
       3,    1,   30,    2, 0x0a /* Public */,
       4,    0,   33,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Bool,
    QMetaType::Void, QMetaType::QPoint,    2,
    QMetaType::Void,

       0        // eod
};

void DeviceListView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DeviceListView *_t = static_cast<DeviceListView *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: { bool _r = _t->onExportToFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 1: _t->OnlvOpRightBtn((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 2: _t->changeThemeIcon(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DeviceListView::staticMetaObject = {
    { &Dtk::Widget::DListView::staticMetaObject, qt_meta_stringdata_DeviceListView.data,
      qt_meta_data_DeviceListView,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DeviceListView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeviceListView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DeviceListView.stringdata0))
        return static_cast<void*>(this);
    return Dtk::Widget::DListView::qt_metacast(_clname);
}

int DeviceListView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Dtk::Widget::DListView::qt_metacall(_c, _id, _a);
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
