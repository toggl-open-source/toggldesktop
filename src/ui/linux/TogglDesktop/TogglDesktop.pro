#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T00:08:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TogglDesktop
TEMPLATE = app

SOURCES += main.cpp\
        mainwindowcontroller.cpp \
    loginviewcontroller.cpp

HEADERS  += mainwindowcontroller.h \
    loginviewcontroller.h

FORMS    += mainwindowcontroller.ui \
    loginviewcontroller.ui

QMAKE_RPATHDIR += $$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug

LIBS += -L$$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug -lTogglDesktopLibrary

INCLUDEPATH += $$PWD/../../../libkopsik/include

QMAKE_RPATHDIR += .

OTHER_FILES += \
    ../../../ssl/cacert.pem
