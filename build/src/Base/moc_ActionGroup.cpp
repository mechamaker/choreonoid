/****************************************************************************
** Meta object code from reading C++ file 'ActionGroup.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Base/ActionGroup.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ActionGroup.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_cnoid__ActionGroup_t {
    QByteArrayData data[6];
    char stringdata0[58];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_cnoid__ActionGroup_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_cnoid__ActionGroup_t qt_meta_stringdata_cnoid__ActionGroup = {
    {
QT_MOC_LITERAL(0, 0, 18), // "cnoid::ActionGroup"
QT_MOC_LITERAL(1, 19, 9), // "onHovered"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 8), // "QAction*"
QT_MOC_LITERAL(4, 39, 6), // "action"
QT_MOC_LITERAL(5, 46, 11) // "onTriggered"

    },
    "cnoid::ActionGroup\0onHovered\0\0QAction*\0"
    "action\0onTriggered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_cnoid__ActionGroup[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08 /* Private */,
       5,    1,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void cnoid::ActionGroup::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ActionGroup *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onHovered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 1: _t->onTriggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject cnoid::ActionGroup::staticMetaObject = { {
    &QActionGroup::staticMetaObject,
    qt_meta_stringdata_cnoid__ActionGroup.data,
    qt_meta_data_cnoid__ActionGroup,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *cnoid::ActionGroup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *cnoid::ActionGroup::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_cnoid__ActionGroup.stringdata0))
        return static_cast<void*>(this);
    return QActionGroup::qt_metacast(_clname);
}

int cnoid::ActionGroup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QActionGroup::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
