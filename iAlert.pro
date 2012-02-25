#-------------------------------------------------
#
# Project created by QtCreator 2011-09-14T12:04:12
#
#-------------------------------------------------

QT       += core gui network phonon sql webkit

TARGET = iAlert
TEMPLATE = app
ICON = icon.icns

SOURCES += main.cpp\
        mainwindow.cpp \
    camera.cpp \
    logitechcameras.cpp \
    qupnp.cpp \
    qxmpp.cpp \
    videowidget.cpp \
    registry.cpp \
    httpserver.cpp \
    storagedialog.cpp

HEADERS  += mainwindow.h \
    camera.h \
    logitechcameras.h \
    qupnp.h \
    qxmpp.h \
    videowidget.h \
    registry.h \
    httpserver.h \
    storagedialog.h

FORMS    += mainwindow.ui \
    storagedialog.ui

LIBS += -lupnp -lgloox

OTHER_FILES += \
    TODO.txt \
    License.txt \
    README

RESOURCES += \
    resources.qrc















































