#-------------------------------------------------
#
# Project created by QtCreator 2011-09-14T12:04:12
#
#-------------------------------------------------

#mac:
#{
#    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
#}

QT       += core gui network phonon sql

TARGET = iAlert
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    camera.cpp \
    logitechcameras.cpp \
    qupnp.cpp \
    qxmpp.cpp \
    videowidget.cpp \
    registry.cpp

HEADERS  += mainwindow.h \
    camera.h \
    logitechcameras.h \
    qupnp.h \
    qxmpp.h \
    videowidget.h \
    registry.h


FORMS    += mainwindow.ui

LIBS += -lupnp -lgloox

































