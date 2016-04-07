/****************************************************************************
** Meta object code from reading C++ file 'opencvwork.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.1.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../opencvwork.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'opencvwork.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.1.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_OpencvWork_t {
    QByteArrayData data[12];
    char stringdata[141];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_OpencvWork_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_OpencvWork_t qt_meta_stringdata_OpencvWork = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 14),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 14),
QT_MOC_LITERAL(4, 42, 11),
QT_MOC_LITERAL(5, 54, 11),
QT_MOC_LITERAL(6, 66, 10),
QT_MOC_LITERAL(7, 77, 10),
QT_MOC_LITERAL(8, 88, 12),
QT_MOC_LITERAL(9, 101, 12),
QT_MOC_LITERAL(10, 114, 14),
QT_MOC_LITERAL(11, 129, 10)
    },
    "OpencvWork\0openImgActionA\0\0openImgActionB\0"
    "showSrcInCV\0showDstInCV\0returnMask\0"
    "updateMask\0mouseMoveDst\0mouseMoveSrc\0"
    "PossionEditing\0PatchMatch\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OpencvWork[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x08,
       3,    0,   65,    2, 0x08,
       4,    0,   66,    2, 0x08,
       5,    0,   67,    2, 0x08,
       6,    0,   68,    2, 0x08,
       7,    0,   69,    2, 0x08,
       8,    0,   70,    2, 0x08,
       9,    0,   71,    2, 0x08,
      10,    0,   72,    2, 0x08,
      11,    0,   73,    2, 0x08,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void OpencvWork::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OpencvWork *_t = static_cast<OpencvWork *>(_o);
        switch (_id) {
        case 0: _t->openImgActionA(); break;
        case 1: _t->openImgActionB(); break;
        case 2: _t->showSrcInCV(); break;
        case 3: _t->showDstInCV(); break;
        case 4: _t->returnMask(); break;
        case 5: _t->updateMask(); break;
        case 6: _t->mouseMoveDst(); break;
        case 7: _t->mouseMoveSrc(); break;
        case 8: _t->PossionEditing(); break;
        case 9: _t->PatchMatch(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject OpencvWork::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_OpencvWork.data,
      qt_meta_data_OpencvWork,  qt_static_metacall, 0, 0}
};


const QMetaObject *OpencvWork::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OpencvWork::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OpencvWork.stringdata))
        return static_cast<void*>(const_cast< OpencvWork*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int OpencvWork::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
