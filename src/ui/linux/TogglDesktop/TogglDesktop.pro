#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T00:08:05
#
#-------------------------------------------------

VERSION = 7.0.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT       += core gui network webkitwidgets widgets

TARGET = TogglDesktop
TEMPLATE = app

CONFIG(debug, debug|release) {
	DESTDIR = build/debug
} else {
	DESTDIR = build/release
}

SOURCES += main.cpp\
    updateview.cpp \
    timeentryview.cpp \
    autocompleteview.cpp \
    genericview.cpp \
    settingsview.cpp \
    toggl.cpp \
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
    timeentrycellwidget.cpp \
    timeentryeditorwidget.cpp \
    ../../../../third_party/qt-oauth-lib/logindialog.cpp \
    ../../../../third_party/qt-oauth-lib/oauth2.cpp \
    preferencesdialog.cpp \
    aboutdialog.cpp \
    feedbackdialog.cpp \
    idlenotificationdialog.cpp

HEADERS  += \
    updateview.h \
    timeentryview.h \
    autocompleteview.h \
    genericview.h \
    settingsview.h \
    toggl.h \
    mainwindowcontroller.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlocalpeer.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtlockedfile.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/QtLockedFile \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsingleapplication.h \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/QtSingleApplication \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsinglecoreapplication.h \
    loginwidget.h \
    errorviewcontroller.h \
    timeentrylistwidget.h \
    timerwidget.h \
    timeentrycellwidget.h \
    timeentryeditorwidget.h \
    ../../../../third_party/qt-oauth-lib/logindialog.h \
    ../../../../third_party/qt-oauth-lib/oauth2.h \
    preferencesdialog.h \
    aboutdialog.h \
    feedbackdialog.h \
    idlenotificationdialog.h

FORMS    += \
    mainwindowcontroller.ui \
    errorviewcontroller.ui \
    loginwidget.ui \
    timeentrylistwidget.ui \
    timerwidget.ui \
    timeentrycellwidget.ui \
    timeentryeditorwidget.ui \
    ../../../../third_party/qt-oauth-lib/logindialog.ui \
    preferencesdialog.ui \
    aboutdialog.ui \
    feedbackdialog.ui \
    idlenotificationdialog.ui

QMAKE_RPATHDIR += $$PWD/../../../lib/Library/TogglDesktopLibrary/build/release
QMAKE_RPATHDIR += $$PWD/../../../../third_party/bugsnag-qt/build/release

LIBS += -L$$PWD/../../../lib/Library/TogglDesktopLibrary/build/release -lTogglDesktopLibrary
LIBS += -L$$PWD/../../../../third_party/bugsnag-qt/build/release -lbugsnag-qt
LIBS += -lX11 -lXss

INCLUDEPATH += $$PWD/../../../lib/include
INCLUDEPATH += $$PWD/../../../../third_party/bugsnag-qt
INCLUDEPATH += $$PWD/../../../../third_party/qt-solutions/qtsingleapplication/src/
INCLUDEPATH += $$PWD/../../../../third_party/qt-oauth-lib

QMAKE_RPATHDIR += .

OTHER_FILES += \
    ../../../ssl/cacert.pem \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsingleapplication.pri \
    ../../../../third_party/qt-solutions/qtsingleapplication/src/qtsinglecoreapplication.pri

#QMAKE_CXXFLAGS += -fsanitize=thread -fPIE
#QMAKE_LFLAGS += -pie -fsanitize=thread

RESOURCES += \
    Resurces.qrc

