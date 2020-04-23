QT += core
QT -= gui

CONFIG += c++11

TARGET = deepin-devicemanager-authenticateProxy
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += $$PWD/main.cpp
DEFINES += QT_DEPRECATED_WARNINGS

TRANSLATIONS += ../translations/policy/*.ts

CONFIG(debug, debug|release) {
    TRANSLATIONS = $$files($$PWD/../translations/policy/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    #将qm文件添加到安装包
    dtk_translations.path = /usr/share/$$TARGET/translations
    dtk_translations.files = $$PWD/../translations/policy/*.qm
    INSTALLS += dtk_translations
}

target.path = /usr/bin

policy.path = /usr/share/polkit-1/actions
policy.files = policy/com.deepin.pkexec.deepin-devicemanager-authenticateProxy.policy

translations.path = /usr/share/deepin-devicemanager/translations
translations.files = $$PWD/../translations/policy/*.qm

INSTALLS += target policy translations


