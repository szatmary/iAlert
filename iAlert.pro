#-------------------------------------------------
#
# Project created by QtCreator 2011-09-14T12:04:12
#
#-------------------------------------------------

#mac:
#{
#    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
#}

QT       += core gui network phonon sql webkit

TARGET = iAlert
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    camera.cpp \
    logitechcameras.cpp \
    qupnp.cpp \
    qxmpp.cpp \
    videowidget.cpp \
    registry.cpp \
    httpserver.cpp \
    storagedialog.cpp \
    about.cpp

HEADERS  += mainwindow.h \
    camera.h \
    logitechcameras.h \
    qupnp.h \
    qxmpp.h \
    videowidget.h \
    registry.h \
    httpserver.h \
    storagedialog.h \
    about.h

FORMS    += mainwindow.ui \
    storagedialog.ui \
    about.ui

LIBS += -lupnp -lgloox

OTHER_FILES += \
    TODO.txt \
    License.txt \
    About iAlert.xhtml

RESOURCES += \
    resources.qrc















































