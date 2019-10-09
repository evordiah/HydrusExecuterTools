QT += core
QT -= gui
QT += sql
CONFIG += c++11

TARGET = HydrusExecuter
CONFIG += console
CONFIG -= app_bundle

DESTDIR += ./bin

QMAKE_RPATHDIR += $ORIGIN/lib

TEMPLATE = app

SOURCES += main.cpp \
    A_LevelObject.cpp \
    HydrusResultCompresser.cpp \
    NodeInfoObject.cpp \
    T_LevelObject.cpp \
    SelectorParse.cpp \
    AtmosphParser.cpp \
    ProfileParser.cpp \
    importhydrusoutputfile.cpp \
    A_LevelParser.cpp \
    BalanceObject.cpp \
    BalanceParse.cpp \
    Nod_InfParser.cpp \
    RuninfoParser.cpp \
    T_LevelParser.cpp \
    profiledatabaseobject.cpp \
    atmosphdatabaseobject.cpp \
    exporthydrusinputfile.cpp \
    selectordatabaseobject.cpp \
    aleveldatabaseobject.cpp \
    tleveldatabaseobject.cpp \
    nodinfodatabaseobject.cpp \
    balancedatabaseobject.cpp \
    HydrusExcuter.cpp \
    selectorobject.cpp \
    profileobject.cpp \
    atmosphobject.cpp \
    HydrusInputCompresser.cpp \
    hydrusexcuterevenly.cpp \
    hydrusexcuterpre.cpp \
    obs_nodeobject.cpp \
    obs_nodeparser.cpp \
    obsnodedatabaseobject.cpp

HEADERS += \
    A_LevelObject.h \
    HydrusResultCompresser.h \
    NodeInfoObject.h \
    T_LevelObject.h \
    SelectorParse.h \
    AtmosphParser.h \
    ProfileParser.h \
    importhydrusoutputfile.h \
    A_LevelParser.h \
    BalanceObject.h \
    BalanceParse.h \
    Nod_InfParser.h \
    RuninfoParser.h \
    T_LevelParser.h \
    profiledatabaseobject.h \
    atmosphdatabaseobject.h \
    exporthydrusinputfile.h \
    selectordatabaseobject.h \
    aleveldatabaseobject.h \
    tleveldatabaseobject.h \
    nodinfodatabaseobject.h \
    balancedatabaseobject.h \
    HydrusExcuter.h \
    selectorobject.h \
    profileobject.h \
    atmosphobject.h \
    HydrusInputCompresser.h \
    hydrusexcuterevenly.h \
    hydrusexcuterpre.h \
    obs_nodeobject.h \
    obs_nodeparser.h \
    obsnodedatabaseobject.h

unix: LIBS += -lpthread -lQt5Core -lQt5Sql -lpq 

INCLUDEPATH += ./tclap

