#-------------------------------------------------
#
# Project created by QtCreator 2019-09-07T12:58:06
#
#-------------------------------------------------

QT       += core gui svg dbus xml
QT       += gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeviceManager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += TEST_DATA_FROM_FILE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += link_pkgconfig
PKGCONFIG += xcb xcb-util dtkwidget dtkwm dframeworkdbus

INCLUDEPATH +=     \
            /usr/include/libdframeworkdbus-2.0/ \


include(../thirdlib/QtXlsxWriter/src/xlsx/qtxlsx.pri)
include(../thirdlib/docx/docx.pri)

#LIBS += -L/usr/lib/x86_64-linux-gnu/ -lhd
#LIBS += -L../thirdlib/hd -lhd

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    devicelistwidget.cpp \
    deviceinfoparser.cpp \
    deviceinfowidgetbase.cpp \
    computeroverviewwidget.cpp \
    hwinfohandler.cpp \
    motherboardwidget.cpp \
    cpuwidget.cpp \
    memorywidget.cpp \
    diskwidget.cpp \
    displayadapterwidget.cpp \
    monitorwidget.cpp \
    audiodevicewidget.cpp \
    networkadapterwidget.cpp \
    bluetoothwidget.cpp \
    camerawidget.cpp \
    keyboardwidget.cpp \
    mousewidget.cpp \
    usbdevicewidget.cpp \
    otherinputdevicewidget.cpp \
    powerwidget.cpp \
    otherpcidevice.cpp \
    portwidget.cpp \
    tablewidgetalwaysfocus.cpp

HEADERS += \
    mainwindow.h \
    devicelistwidget.h \
    version.h \
    deviceinfoparser.h \
    deviceinfodefine.h \
    deviceattributedefine.h \
    deviceinfowidgetbase.h \
    computeroverviewwidget.h \
    singletondef.h \
    hwinfohandler.h \
    motherboardwidget.h \
    cpuwidget.h \
    memorywidget.h \
    diskwidget.h \
    displayadapterwidget.h \
    monitorwidget.h \
    audiodevicewidget.h \
    networkadapterwidget.h \
    bluetoothwidget.h \
    camerawidget.h \
    keyboardwidget.h \
    otherinputdevicewidget.h \
    powerwidget.h \
    mousewidget.h \
    usbdevicewidget.h \
    otherpcidevice.h \
    portwidget.h \
    tablewidgetalwaysfocus.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    app.qrc
