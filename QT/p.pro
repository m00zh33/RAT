# -------------------------------------------------
# Project created by QtCreator 2010-06-28T17:39:18
# -------------------------------------------------
QT += network
QT -= gui
QT += sql
TARGET = p
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    httpd.cpp \
    https.cpp \
    httpparsergeneric.cpp \
    httpparserget.cpp \
    httpparserhead.cpp \
    httpparserpost.cpp \
    httpparserbitspost.cpp \
    qsslserver.cpp \
    httphandlergeneric.cpp \
    httphandlerclock.cpp \
    httphandlerip.cpp \
    httphandlersession.cpp \
    httphandlersecure.cpp \
    httphandlersecurelist.cpp \
    httphandlernoop.cpp \
    httphandlersecurecmd.cpp \
    httphandlerreport.cpp \
    httpbitsgeneric.cpp \
    httpbitsdropbox.cpp \
    httphandlerotpgen.cpp \
    encryptio.cpp \
    httpbitsdir.cpp \
    httphandlerp.cpp \
    netcat.cpp \
    httphandlersecuredir.cpp \
    httpparseroptions.cpp \
    httpparserprofind.cpp
HEADERS += httpd.h \
    https.h \
    httpparsergeneric.h \
    common.h \
    httpparserget.h \
    httpparserhead.h \
    httpparserpost.h \
    httpparserbitspost.h \
    qsslserver.h \
    httphandlergeneric.h \
    httphandlerclock.h \
    httphandlerip.h \
    httphandlersession.h \
    httphandlersecure.h \
    httphandlersecurelist.h \
    httphandlernoop.h \
    httphandlersecurecmd.h \
    httphandlerreport.h \
    httpbitsgeneric.h \
    httpbitsdropbox.h \
    httphandlerotpgen.h \
    EnRUPT.h \
    encryptio.h \
    httpbitsdir.h \
    httphandlerp.h \
    netcat.h \
    httphandlersecuredir.h \
    httpparseroptions.h \
    httpparserprofind.h

OTHER_FILES += \
    debug/htstatic/400.html \
    htstatic/404.html \
    htstatic/400.html \
    htstatic/403.html \
    htstatic/500.html \
    htstatic/416.html \
    htstatic/login.html \
    htstatic/list.html \
    htstatic/cmd.html \
    htstatic/dirrequest.html \
    htstatic/dirdrives.html \
    htstatic/dirpath.html \
    htstatic/dirstatus.html

#QMAKE_CXXFLAGS += -Wall
#QMAKE_LFLAGS += /ltgc
#-pedantic
