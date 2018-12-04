#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T00:08:05
#
#-------------------------------------------------
include(../../../../third_party/qxtglobalshortcut5/qxt.pri)
VERSION = 7.0.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

ENVIRONMENT = "development"

DEFINES += APP_ENVIRONMENT=\\\"$$ENVIRONMENT\\\"

QT       += core gui network webengine webenginewidgets widgets dbus

TARGET = TogglDesktop
TEMPLATE = app

CONFIG(debug, debug|release) {
	DESTDIR = build/debug
} else {
	DESTDIR = build/release
}

SOURCES += main.cpp\
    timeentryview.cpp \
    autocompleteview.cpp \
    genericview.cpp \
    settingsview.cpp \
    toggl.cpp \
    mainwindowcontroller.cpp \
    errorviewcontroller.cpp \
    loginwidget.cpp \
    timeentrylistwidget.cpp \
    timerwidget.cpp \
    timeentrycellwidget.cpp \
    timeentryeditorwidget.cpp \
    ../../../../third_party/qt-oauth-lib/logindialog.cpp \
    ../../../../third_party/qt-oauth-lib/oauth2.cpp \
    singleapplication.cpp \
    preferencesdialog.cpp \
    aboutdialog.cpp \
    feedbackdialog.cpp \
    clickablelabel.cpp \
    colorpicker.cpp \
    countryview.cpp \
    overlaywidget.cpp \
    systemtray.cpp \
    idlenotificationwidget.cpp \
    autocompletelistmodel.cpp \
    autocompleteitemview.cpp \
    autocompletecombobox.cpp

HEADERS  += \
    timeentryview.h \
    autocompleteview.h \
    genericview.h \
    settingsview.h \
    toggl.h \
    mainwindowcontroller.h \
    loginwidget.h \
    errorviewcontroller.h \
    timeentrylistwidget.h \
    timerwidget.h \
    timeentrycellwidget.h \
    timeentryeditorwidget.h \
    ../../../../third_party/qt-oauth-lib/logindialog.h \
    ../../../../third_party/qt-oauth-lib/oauth2.h \
    singleapplication.h \
    preferencesdialog.h \
    aboutdialog.h \
    feedbackdialog.h \
    clickablelabel.h \
    colorpicker.h \
    countryview.h \
    overlaywidget.h \
    systemtray.h \
    idlenotificationwidget.h \
    autocompletelistmodel.h \
    autocompleteitemview.h \
    autocompletecombobox.h

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
    colorpicker.ui \
    overlaywidget.ui \
    idlenotificationwidget.ui

QMAKE_RPATHDIR += $$PWD/../../../lib/linux/TogglDesktopLibrary/build/release
QMAKE_RPATHDIR += $$PWD/../../../../third_party/bugsnag-qt/build/release

LIBS += -L$$PWD/../../../lib/linux/TogglDesktopLibrary/build/release -lTogglDesktopLibrary
LIBS += -L$$PWD/../../../../third_party/bugsnag-qt/build/release -lbugsnag-qt
LIBS += -lX11 -lXss

INCLUDEPATH += $$PWD/../../../
INCLUDEPATH += $$PWD/../../../../third_party/bugsnag-qt
INCLUDEPATH += $$PWD/../../../../third_party/qt-oauth-lib

QMAKE_RPATHDIR += lib

OTHER_FILES += \
    ../../../ssl/cacert.pem

#QMAKE_CXXFLAGS += -fsanitize=thread -fPIE
#QMAKE_LFLAGS += -pie -fsanitize=thread

RESOURCES += \
    Resources.qrc

