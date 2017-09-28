# -------------------------------------------------
# Project created by QtCreator 2009-11-30T00:37:36
# -------------------------------------------------
QT += network
TARGET = controller
TEMPLATE = app
SOURCES += main.cpp \
    controller.cpp \
    qtwin.cpp \
    enc.cpp \
    portmapper.cpp
HEADERS += controller.h \
    qtwin.h \
    enc.h \
    portmapper.h
FORMS += controller.ui \
    portmapper.ui
CONFIG += embed_manifest_exe
CONFIG -= exception
