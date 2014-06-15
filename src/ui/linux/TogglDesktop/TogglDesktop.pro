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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug/release/ -lTogglDesktopLibrary
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug/debug/ -lTogglDesktopLibrary
else:unix: LIBS += -L$$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug/ -lTogglDesktopLibrary

INCLUDEPATH += $$PWD/../../../libkopsik/include
DEPENDPATH += $$PWD/../../../libkopsik/include

unix: LIBS += -lX11
