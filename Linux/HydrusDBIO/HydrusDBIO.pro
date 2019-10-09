QT += core
QT -= gui
QT += sql
CONFIG += c++11

DESTDIR += ./bin
TARGET = HydrusDBIO
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

QMAKE_RPATHDIR += $ORIGIN/lib

SOURCES += main.cpp \
    A_LevelObject.cpp \
    HydrusResultCompresser.cpp \
    NodeInfoObject.cpp \
    T_LevelObject.cpp \
    HydrusInputCompresser.cpp \
    SelectorParse.cpp \
    AtmosphParser.cpp \
    ProfileParser.cpp \
    importhydrusinputfile.cpp \
    importhydrusoutputfile.cpp \
    importer.cpp \
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
    exporter.cpp \
    exporthydrusoutputfile.cpp \
    aleveldatabaseobject.cpp \
    tleveldatabaseobject.cpp \
    nodinfodatabaseobject.cpp \
    balancedatabaseobject.cpp \
    atmosphobject.cpp \
    profileobject.cpp \
    selectorobject.cpp \
    obsnodedatabaseobject.cpp \
    obs_nodeobject.cpp \
    obs_nodeparser.cpp

HEADERS += \
    A_LevelObject.h \
    HydrusResultCompresser.h \
    NodeInfoObject.h \
    T_LevelObject.h \
    HydrusInputCompresser.h \
    SelectorParse.h \
    AtmosphParser.h \
    ProfileParser.h \
    importhydrusinputfile.h \
    importhydrusoutputfile.h \
    importer.h \
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
    exporter.h \
    exporthydrusoutputfile.h \
    aleveldatabaseobject.h \
    tleveldatabaseobject.h \
    nodinfodatabaseobject.h \
    balancedatabaseobject.h \
    atmosphobject.h \
    profileobject.h \
    selectorobject.h \
    obsnodedatabaseobject.h \
    obs_nodeobject.h \
    obs_nodeparser.h

LIBS += -L./          \
        -lQt5Core     \
        -lQt5Sql      \
	    -lpq      
INCLUDEPATH += ./tclap


