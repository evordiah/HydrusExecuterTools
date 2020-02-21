#-------------------------------------------------
#
# Project created by QtCreator 2020-02-09T13:41:10
#
#-------------------------------------------------

QT       += sql

QT       -= gui

TARGET = HydrusFiles
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ALevelObject.cpp \
    AtmosphObject.cpp \
    FFmt.cpp \
    HydrusParameterFilesManager.cpp \
    NodInfoObject.cpp \
    ObsNodeObject.cpp \
    ProfileObject.cpp \
    SelectorObject.cpp \
    SoluteObject.cpp \
    TLevelObject.cpp \
    BalanceObject.cpp

HEADERS += \
    ALevelObject.h \
    AtmosphObject.h \
    FFmt.h \
    HydrusParameterFilesManager.h \
    IHydrusParameterFileObject.h \
    NodInfoObject.h \
    ObsNodeObject.h \
    ProfileObject.h \
    SelectorObject.h \
    SoluteObject.h \
    TLevelObject.h \
    BalanceObject.h

DESTDIR = ../lib

unix {
    target.path = ../lib
    INSTALLS += target
}

