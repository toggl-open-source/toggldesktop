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
    loginviewcontroller.cpp \
    updateview.cpp \
    timeentryview.cpp \
    autocompleteview.cpp \
    genericview.cpp \
    settingsview.cpp \
    toggl_api.cpp \
    settingswindowcontroller.cpp \
    aboutwindowcontroller.cpp \
    feedbackwindowcontroller.cpp \
    timeentrycell.cpp \
    timeentrycellwithheader.cpp \
    timeentrylistviewcontroller.cpp \
    timereditviewcontroller.cpp \
    timeentryeditviewcontroller.cpp \
    mainwindowcontroller.cpp \
    errorviewcontroller.cpp

HEADERS  += \
    loginviewcontroller.h \
    updateview.h \
    timeentryview.h \
    autocompleteview.h \
    genericview.h \
    settingsview.h \
    toggl_api.h \
    settingswindowcontroller.h \
    aboutwindowcontroller.h \
    feedbackwindowcontroller.h \
    timeentrycell.h \
    timeentrycellwithheader.h \
    timeentrylistviewcontroller.h \
    timereditviewcontroller.h \
    timeentryeditviewcontroller.h \
    mainwindowcontroller.h \
    errorviewcontroller.h

FORMS    += \
    loginviewcontroller.ui \
    settingswindowcontroller.ui \
    aboutwindowcontroller.ui \
    feedbackwindowcontroller.ui \
    timeentrycell.ui \
    timeentrycellwithheader.ui \
    timeentrylistviewcontroller.ui \
    timereditviewcontroller.ui \
    timeentryeditviewcontroller.ui \
    mainwindowcontroller.ui \
    errorviewcontroller.ui

QMAKE_RPATHDIR += $$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug

LIBS += -L$$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug -lTogglDesktopLibrary

INCLUDEPATH += $$PWD/../../../libkopsik/include

QMAKE_RPATHDIR += .

OTHER_FILES += \
    ../../../ssl/cacert.pem
