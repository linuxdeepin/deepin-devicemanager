QT  += core gui svg dbus xml
QT  += gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deepin-devicemanager

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the$
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#load device info from file
#DEFINES += TEST_DATA_FROM_FILE

#QT_SCALE_FACTOR=1.5

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# QMAKE_CXXFLAGS += `cups-config --cflags` `cups-config --libs`

LIBS += -lcups

CONFIG  += c++11
CONFIG  += link_pkgconfig
PKGCONFIG   += dtkwidget dframeworkdbus

TRANSLATIONS += translations/*.ts

INCLUDEPATH +=     \
            /usr/include/cups/ \
            /usr/include/libdframeworkdbus-2.0/ \


include(../thirdlib/QtXlsxWriter/src/xlsx/qtxlsx.pri)
include(../thirdlib/docx/docx.pri)

CONFIG(debug, debug|release) {
    TRANSLATIONS = $$files($$PWD/translations/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    #将qm文件添加到安装包
    dtk_translations.path = /usr/share/$$TARGET/translations
    dtk_translations.files = $$PWD/translations/*.qm
    INSTALLS += dtk_translations
}

#LIBS += -L/usr/lib/x86_64-linux-gnu/ -lhd
#LIBS += -L../thirdlib/hd -lhd

SOURCES += \
    cdromwidget.cpp \
    cloumnwidget.cpp \
    logtreeview.cpp \
    logviewheaderview.cpp \
    logviewitemdelegate.cpp \
    main.cpp \
    mainwindow.cpp \
    deviceinfoparser.cpp \
    deviceinfowidgetbase.cpp \
    computeroverviewwidget.cpp \
    motherboardwidget.cpp \
    cpuwidget.cpp \
    memorywidget.cpp \
    diskwidget.cpp \
    displayadapterwidget.cpp \
    monitorwidget.cpp \
    audiodevicewidget.cpp \
    networkadapterwidget.cpp \
    camerawidget.cpp \
    keyboardwidget.cpp \
    mousewidget.cpp \
    otherdevicewidget.cpp \
    usbdevicewidget.cpp \
    powerwidget.cpp \
    otherpcidevice.cpp \
    logpasswordauth.cpp \
    printerwidget.cpp \
    devicelistview.cpp \
    deviceattributedefine.cpp \
    bluetoothwidget.cpp

HEADERS += \
    cdromwidget.h \
    cloumnwidget.h \
    commondefine.h \
    logtreeview.h \
    logviewheaderview.h \
    logviewitemdelegate.h \
    mainwindow.h \
    otherdevicewidget.h \
    deviceinfoparser.h \
    deviceattributedefine.h \
    deviceinfowidgetbase.h \
    computeroverviewwidget.h \
    singletondef.h \
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
    powerwidget.h \
    mousewidget.h \
    usbdevicewidget.h \
    otherpcidevice.h \
    logpasswordauth.h \
    printerwidget.h \
    devicelistview.h

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target


RESOURCES += appimage.qrc

DISTFILES += \
    ../README.md \
    ../debian/control \
    ../debian/rules \
    ../translations/*.ts \
    ../translations/desktop_zh_CN.qm \
    ../translations/desktop.qm \
    ../translations/policy_zh_CN.qm \
    ../translations/policy.qm \
    ../translations/desktop_zh_CN.ts \
    ../translations/deepin-devicemanager_zh_CN.ts \
    ../translations/deepin-devicemanager.ts \
    ../translations/desktop.ts \
    ../translations/policy_zh_CN.ts \
    ../translations/policy.ts

# target
target.path = /usr/bin/

# icons
#icons.files += ../images/cpu.svg
#icons.path = /usr/share/deepin-devicemanager/
icons.path = /usr/share/icons/hicolor/scalable/apps
icons.files = ../icons/deepin/builtin/deepin-devicemanager.svg

# desktop
desktop.files += deepin-devicemanager.desktop
desktop.path = /usr/share/applications/

translations.path = /usr/share/deepin-devicemanager/translations
translations.files = ../translations/*.qm

INSTALLS += target icons desktop translations
