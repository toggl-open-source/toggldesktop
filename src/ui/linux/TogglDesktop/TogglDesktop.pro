#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T00:08:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TogglDesktop
TEMPLATE = app

SOURCES += main.cpp\
    updateview.cpp \
    timeentryview.cpp \
    autocompleteview.cpp \
    genericview.cpp \
    settingsview.cpp \
    toggl_api.cpp \
    mainwindowcontroller.cpp \
    errorviewcontroller.cpp \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlocalpeer.cpp \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlockedfile.cpp \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlockedfile_unix.cpp \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlockedfile_win.cpp \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsingleapplication.cpp \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsinglecoreapplication.cpp \
    loginwidget.cpp \
    timeentrylistwidget.cpp \
    timerwidget.cpp \
    timeentrycellwidget.cpp

HEADERS  += \
    updateview.h \
    timeentryview.h \
    autocompleteview.h \
    genericview.h \
    settingsview.h \
    toggl_api.h \
    mainwindowcontroller.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlocalpeer.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlockedfile.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/QtLockedFile \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsingleapplication.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/QtSingleApplication \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsinglecoreapplication.h \
    loginviewcontroller.h \
    loginwidget.h \
    errorviewcontroller.h \
    ../../../libkopsik/include/kopsik_api.h \
    timeentrylistwidget.h \
    timerwidget.h \
    timeentrycellwidget.h

FORMS    += \
    mainwindowcontroller.ui \
    errorviewcontroller.ui \
    loginwidget.ui \
    timeentrylistwidget.ui \
    timerwidget.ui \
    timeentrycellwidget.ui

QMAKE_RPATHDIR += $$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug

LIBS += -L$$PWD/../../../libkopsik/Library/build-TogglDesktopLibrary-Desktop-Debug -lTogglDesktopLibrary

INCLUDEPATH += $$PWD/../../../libkopsik/include
INCLUDEPATH += $$PWD/../../../../third_party/qt-solutions/qtsingleapplication/src/

QMAKE_RPATHDIR += .

OTHER_FILES += \
    ../../../ssl/cacert.pem \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsingleapplication.pri \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsinglecoreapplication.pri
