#-------------------------------------------------
#
# Project created by QtCreator 2019-09-07T12:58:06
#
#-------------------------------------------------

QT  += core gui svg dbus xml
QT  += gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-devicemanager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the$
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#load device info from file
#DEFINES += TEST_DATA_FROM_FILE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# QMAKE_CXXFLAGS += `cups-config --cflags` `cups-config --libs`

LIBS += -lcups

CONFIG += c++11
CONFIG += link_pkgconfig
PKGCONFIG += dtkwidget dframeworkdbus

TRANSLATIONS += translations/*.ts

INCLUDEPATH +=     \
            /usr/include/cups/ \
            /usr/include/libdframeworkdbus-2.0/ \


include(thirdlib/QtXlsxWriter/src/xlsx/qtxlsx.pri)
include(thirdlib/docx/docx.pri)

#LIBS += -L/usr/lib/x86_64-linux-gnu/ -lhd
#LIBS += -L../thirdlib/hd -lhd

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/deviceinfoparser.cpp \
    src/deviceinfowidgetbase.cpp \
    src/computeroverviewwidget.cpp \
    src/hwinfohandler.cpp \
    src/motherboardwidget.cpp \
    src/cpuwidget.cpp \
    src/memorywidget.cpp \
    src/diskwidget.cpp \
    src/displayadapterwidget.cpp \
    src/monitorwidget.cpp \
    src/audiodevicewidget.cpp \
    src/networkadapterwidget.cpp \
    src/bluetoothwidget.cpp \
    src/camerawidget.cpp \
    src/keyboardwidget.cpp \
    src/mousewidget.cpp \
    src/usbdevicewidget.cpp \
    src/otherinputdevicewidget.cpp \
    src/powerwidget.cpp \
    src/otherpcidevice.cpp \
    src/tablewidgetalwaysfocus.cpp \
    src/logpasswordauth.cpp \
    src/printerwidget.cpp \
    src/devicelistview.cpp \
    src/deviceattributedefine.cpp

HEADERS += \
    src/mainwindow.h \
    src/version.h \
    src/deviceinfoparser.h \
    src/deviceattributedefine.h \
    src/deviceinfowidgetbase.h \
    src/computeroverviewwidget.h \
    src/singletondef.h \
    src/hwinfohandler.h \
    src/motherboardwidget.h \
    src/cpuwidget.h \
    src/memorywidget.h \
    src/diskwidget.h \
    src/displayadapterwidget.h \
    src/monitorwidget.h \
    src/audiodevicewidget.h \
    src/networkadapterwidget.h \
    src/bluetoothwidget.h \
    src/camerawidget.h \
    src/keyboardwidget.h \
    src/otherinputdevicewidget.h \
    src/powerwidget.h \
    src/mousewidget.h \
    src/usbdevicewidget.h \
    src/otherpcidevice.h \
    src/tablewidgetalwaysfocus.h \
    src/logpasswordauth.h \
    src/printerwidget.h \
    src/devicelistview.h


# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += app.qrc

DISTFILES += \
    images/security.ico \
    images/splash.png \
    translations/dde-devicemanager.ts \
    translations/dde-devicemanager_zh_CN.ts \


#policy.path = /usr/share/polkit-1/actions
#policy.files = policy/com.deepin.pkexec.dde-devicemanager.policy

# target
target.path = /usr/bin/

# icons
icons.files += images/cpu.svg
icons.path = /usr/share/dde-devicemanager/

# desktop
desktop.files += dde-devicemanager.desktop
desktop.path = /usr/share/applications/

translations.path = /usr/share/dde-devicemanager/translations
translations.files = translations/*.qm

INSTALLS += target icons desktop translations
