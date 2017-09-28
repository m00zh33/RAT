# -------------------------------------------------
# Project created by QtCreator 2009-11-22T21:23:03
# -------------------------------------------------
QT += network
QT -= gui
TARGET = pytheas
CONFIG -= app_bundle
CONFIG -= exceptions
CONFIG -= exception
TEMPLATE = app
SOURCES += main.cpp \
    com.cpp \
    enc.cpp
HEADERS += com.h \
    enc.h
#QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9
#CONFIG += embed_manifest_exe
Debug:DEFINES+=DEBUG

QMAKE_LFLAGS += ole32.lib
