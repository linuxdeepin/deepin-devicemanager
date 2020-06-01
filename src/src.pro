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

TRANSLATIONS += ../translations/*.ts

INCLUDEPATH +=     \
            /usr/include/cups/ \
            /usr/include/libdframeworkdbus-2.0/ \


include(../3rdparty/QtXlsxWriter/src/xlsx/qtxlsx.pri)
include(../3rdparty/docx/docx.pri)


CONFIG(debug, debug|release) {
    TRANSLATIONS = $$files($$PWD/../translations/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    #将qm文件添加到安装包
    dtk_translations.path = /usr/share/$$TARGET/translations
    dtk_translations.files = $$PWD/../translations/*.qm
    INSTALLS += dtk_translations
}
#LIBS += -L/usr/lib/x86_64-linux-gnu/ -lhd
#LIBS += -L../3rdparty/hd -lhd

SOURCES += \
    deviceattributedefine.cpp \
    edidparser.cpp \
    main.cpp \
    mainwindow.cpp \
    deviceinfoparser.cpp \
    DeviceManager/DeviceInfo.cpp \
    DeviceManager/DeviceMouse.cpp \
    DeviceManager/DeviceCPU.cpp \
    DeviceManager/DeviceStorage.cpp \
    DeviceManager/DeviceGPU.cpp \
    DeviceManager/DeviceMemory.cpp \
    DeviceManager/DeviceMonitor.cpp \
    DeviceManager/DeviceBios.cpp \
    DeviceManager/DeviceBluetooth.cpp \
    DeviceManager/DeviceImage.cpp \
    DeviceManager/DeviceOthers.cpp \
    DeviceManager/DevicePower.cpp \
    DeviceManager/DeviceNetwork.cpp \
    DeviceManager/DeviceAudio.cpp \
    DeviceManager/DeviceOtherPCI.cpp \
    DeviceManager/DeviceKeyboard.cpp \
    Widget/audiodevicewidget.cpp \
    Widget/bluetoothwidget.cpp \
    Widget/bottomroundframe.cpp \
    Widget/camerawidget.cpp \
    Widget/cdromwidget.cpp \
    Widget/cloumnwidget.cpp \
    Widget/computeroverviewwidget.cpp \
    Widget/cpuwidget.cpp \
    Widget/deviceinfowidgetbase.cpp \
    Widget/devicelistview.cpp \
    Widget/diskwidget.cpp \
    Widget/displayadapterwidget.cpp \
    Widget/keyboardwidget.cpp \
    Widget/logpasswordauth.cpp \
    Widget/logtreeview.cpp \
    Widget/logviewheaderview.cpp \
    Widget/logviewitemdelegate.cpp \
    Widget/memorywidget.cpp \
    Widget/monitorwidget.cpp \
    Widget/motherboardwidget.cpp \
    Widget/mousewidget.cpp \
    Widget/networkadapterwidget.cpp \
    Widget/otherdevicewidget.cpp \
    Widget/otherpcidevice.cpp \
    Widget/powerwidget.cpp \
    Widget/printerwidget.cpp \
    Widget/usbdevicewidget.cpp \
    ThreadPool/ThreadPool.cpp \
    DeviceManager/DevicePrint.cpp \
    DeviceManager/DeviceComputer.cpp \
    DeviceManager/DeviceCdrom.cpp \
    LoadInfo/CmdTool.cpp \
    LoadInfo/DeviceFactory.cpp \
    LoadInfo/X86Generator.cpp \
    LoadInfo/MipsGenerator.cpp \
    LoadInfo/ArmGenerator.cpp \
    LoadInfo/PanguGenerator.cpp \
    LoadInfo/KLUGenerator.cpp \
    LoadInfo/DeviceGenerator.cpp \
    DeviceManager/DeviceManager.cpp

HEADERS += \
    DeviceManager/DeviceInfo.h \
    DeviceManager/DeviceMouse.h \
    DeviceManager/DeviceCpu.h \
    DeviceManager/DeviceGpu.h \
    DeviceManager/DeviceMemory.h \
    DeviceManager/DeviceMonitor.h \
    DeviceManager/DeviceBios.h \
    DeviceManager/DeviceImage.h \
    DeviceManager/DeviceOthers.h \
    DeviceManager/DevicePower.h \
    DeviceManager/DeviceNetwork.h \
    DeviceManager/DeviceAudio.h \
    DeviceManager/DeviceKeyboard.h \
    DeviceManager/DevicePrint.h \
    DeviceManager/DeviceBluetooth.h \
    DeviceManager/DeviceOtherPCI.h \
    DeviceManager/DeviceStorage.h \
    Widget/usbdevicewidget.h \
    commondefine.h \
    deviceattributedefine.h \
    deviceinfoparser.h \
    hd.h \
    mainwindow.h \
    singletondef.h \
    Widget/audiodevicewidget.h \
    Widget/bluetoothwidget.h \
    Widget/bottomroundframe.h \
    Widget/camerawidget.h \
    Widget/cdromwidget.h \
    Widget/cloumnwidget.h \
    Widget/computeroverviewwidget.h \
    Widget/cpuwidget.h \
    Widget/deviceinfowidgetbase.h \
    Widget/devicelistview.h \
    Widget/diskwidget.h \
    Widget/displayadapterwidget.h \
    Widget/keyboardwidget.h \
    Widget/logpasswordauth.h \
    Widget/logtreeview.h \
    Widget/logviewheaderview.h \
    Widget/logviewitemdelegate.h \
    Widget/memorywidget.h \
    Widget/monitorwidget.h \
    Widget/motherboardwidget.h \
    Widget/mousewidget.h \
    Widget/networkadapterwidget.h \
    Widget/otherdevicewidget.h \
    Widget/otherpcidevice.h \
    Widget/powerwidget.h \
    Widget/printerwidget.h \
    ThreadPool/ThreadPool.h \
    DeviceManager/DeviceComputer.h \
    DeviceManager/DeviceCdrom.h \
    LoadInfo/CmdTool.h \
    LoadInfo/DeviceFactory.h \
    LoadInfo/X86Generator.h \
    LoadInfo/MipsGenerator.h \
    LoadInfo/ArmGenerator.h \
    LoadInfo/PanguGenerator.h \
    LoadInfo/KLUGenerator.h \
    LoadInfo/DeviceGenerator.h \
    DeviceManager/DeviceManager.h

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target


RESOURCES += ../assets/qrc/appimage.qrc

DISTFILES += \
    ../README.md \
    ../debian/control \
    ../debian/rules \
    ../translations/*.ts \
    ../translations/*.qm

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
