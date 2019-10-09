QT += core
QT -= gui
QT += sql

CONFIG += c++11

TEMPLATE = app

DESTDIR += ./bin
TARGET = HydrusDBSetup
CONFIG += console
CONFIG -= app_bundle

SOURCES += main.cpp \
    databasesqlcommands.cpp

HEADERS += \
    databasesqlcommands.h

INCLUDEPATH += ./tclap

