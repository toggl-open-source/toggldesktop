QT += network gui-private
unix:!macx:{ QT += x11extras }

CONFIG += qxt

QXT += core gui widgets

DEFINES += BUILD_QXT_CORE \
    BUILD_QXT_GUI

DEPENDPATH +=  $$PWD/gui \
    $$PWD/core
INCLUDEPATH += $$PWD/core \
    $$PWD/gui

HEADERS  += $$PWD/core/qxtglobal.h \
    $$PWD/gui/qxtglobalshortcut_p.h \
    $$PWD/gui/qxtglobalshortcut.h \
    $$PWD/gui/qxtwindowsystem.h

    SOURCES  +=	$$PWD/core/qxtglobal.cpp \
        $$PWD/gui/qxtwindowsystem.cpp \
        $$PWD/gui/qxtglobalshortcut.cpp

unix:!macx {
        CONFIG += X11
        SOURCES += $$PWD/gui/qxtwindowsystem_x11.cpp \
           $$PWD/gui/qxtglobalshortcut_x11.cpp
        HEADERS += $$PWD/gui/x11info.h
}
macx: {
        SOURCES += $$PWD/gui/qxtwindowsystem_mac.cpp \
                $$PWD/gui/qxtglobalshortcut_mac.cpp

        HEADERS  += $$PWD/qxtwindowsystem_mac.h

	QMAKE_LFLAGS += -framework Carbon -framework CoreFoundation
}

win32: {
        SOURCES += $$PWD/gui/qxtwindowsystem_win.cpp \
            $$PWD/gui/qxtglobalshortcut_win.cpp
}
