QT += core
QT -= gui

CONFIG += c++11

TARGET = deepin-devicemanager-authenticateProxy
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += $$PWD/main.cpp
DEFINES += QT_DEPRECATED_WARNINGS

target.path = /usr/bin

policy.path = /usr/share/polkit-1/actions
policy.files = policy/com.deepin.pkexec.deepin-devicemanager-authenticateProxy.policy

INSTALLS += target policy


