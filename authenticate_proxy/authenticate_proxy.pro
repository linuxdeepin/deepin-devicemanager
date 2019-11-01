QT += core
QT -= gui

CONFIG += c++11

TARGET = authenticateProxy
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += $$PWD/main.cpp
DEFINES += QT_DEPRECATED_WARNINGS

target.path = /usr/bin

INSTALLS += target

