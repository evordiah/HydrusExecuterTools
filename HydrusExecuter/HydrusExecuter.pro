QT += core
QT -= gui
QT += sql
CONFIG += c++11

TARGET = HydrusExecuter
CONFIG += console
CONFIG -= app_bundle

DESTDIR += ./bin

unix:QMAKE_RPATHDIR = $ORIGIN/lib

TEMPLATE = app

SOURCES += main.cpp \
    HydrusExcuter.cpp \
    hydrusexcuterevenly.cpp \
    hydrusexcuterpre.cpp \
    taskcontroller.cpp

HEADERS += \
    HydrusExcuter.h \
    hydrusexcuterevenly.h \
    hydrusexcuterpre.h \
    taskcontroller.h

LIBS +=  -lpthread -lQt5Core -lQt5Sql \
         -L../lib -lHydrusFiles

INCLUDEPATH += ./tclap \
               ../HydrusFiles
             

