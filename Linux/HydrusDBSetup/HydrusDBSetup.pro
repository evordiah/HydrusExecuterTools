QT += core
QT -= gui
QT += sql

DESTDIR += ./bin
CONFIG += c++11

TARGET = HydrusDBSetup
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

QMAKE_RPATHDIR += $ORIGIN/lib

SOURCES += main.cpp \
    databasesqlcommands.cpp

HEADERS += \
    databasesqlcommands.h

INCLUDEPATH += ./tclap

LIBS += -lQt5Core     \
        -lQt5Sql      \
	-lpq 

