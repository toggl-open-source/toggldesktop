#-------------------------------------------------
#
# Project created by QtCreator 2012-01-18T16:34:48
#
#-------------------------------------------------

QT       += webkit

TARGET = qt-oauth-lib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += oauth2.cpp \
    logindialog.cpp

HEADERS += oauth2.h \
    logindialog.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    logindialog.ui


