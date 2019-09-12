#-------------------------------------------------
#
# Project created by QtCreator 2019-09-07T12:58:06
#
#-------------------------------------------------

QT       += core gui svg
QT       += gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeviceManager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += link_pkgconfig
PKGCONFIG += xcb xcb-util dtkwidget dtkwm

INCLUDEPATH += ../dmidecode/

//CFLAGS += -D_FILE_OFFSET_BITS=64
QMAKE_CFLAGS += -D_FILE_OFFSET_BITS=64

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    devicelistwidget.cpp \
    generalinfowidget.cpp \
    deviceinfowidget.cpp \
    deviceinfoparser.cpp \
    ../dmidecode/dmioem.c \
    ../dmidecode/dmiopt.c \
    ../dmidecode/util.c \
    ../dmidecode/dmidecode.c

HEADERS += \
    mainwindow.h \
    devicelistwidget.h \
    version.h \
    generalinfowidget.h \
    deviceinfowidget.h \
    deviceinfoparser.h \
    deviceinfodefine.h \
    ../dmidecode/version.h \
    ../dmidecode/types.h \
    ../dmidecode/config.h \
    ../dmidecode/dmidecode.h \
    ../dmidecode/dmiopt.h \
    ../dmidecode/dmioem.h \
    ../dmidecode/util.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    app.qrc
