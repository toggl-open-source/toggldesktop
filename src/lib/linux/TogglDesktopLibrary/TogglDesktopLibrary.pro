#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T00:39:54
#
#-------------------------------------------------

TARGET = TogglDesktopLibrary
TEMPLATE = lib

DEFINES += TOGGLDESKTOPLIBRARY_LIBRARY

CONFIG(debug, debug|release) {
	DESTDIR = build/debug
} else {
	DESTDIR = build/release
}   

SOURCES += \
    ../../../base_model.cc \
    ../../../batch_update_result.cc \
    ../../../client.cc \
    ../../../idle.cc \
    ../../../analytics.cc \
    ../../../help_article.cc \
    ../../../autotracker.cc \
    ../../../urls.cc \
    ../../../migrations.cc \
    ../../../context.cc \
    ../../../custom_error_handler.cc \
    ../../../database.cc \
    ../../../feedback.cc \
    ../../../formatter.cc \
    ../../../get_focused_window_linux.cc \
    ../../../error.cc \
    ../../../gui.cc \
    ../../../netconf.cc \
    ../../../https_client.cc \
    $$PWD/../../../../third_party/jsoncpp/dist/jsoncpp.cpp \
    ../../../toggl_api.cc \
    ../../../toggl_api_private.cc \
    ../../../model_change.cc \
    ../../../obm_action.cc \
    ../../../project.cc \
    ../../../proxy.cc \
    ../../../related_data.cc \
    ../../../settings.cc \
    ../../../tag.cc \
    ../../../task.cc \
    ../../../timeline_event.cc \
    ../../../time_entry.cc \
    ../../../timeline_uploader.cc \
    ../../../user.cc \
    ../../../websocket_client.cc \
    ../../../window_change_recorder.cc \
    ../../../workspace.cc \
    ../../../../third_party/lua/src/lapi.c \
    ../../../../third_party/lua/src/lauxlib.c \
    ../../../../third_party/lua/src/lbaselib.c \
    ../../../../third_party/lua/src/lbitlib.c \
    ../../../../third_party/lua/src/lcode.c \
    ../../../../third_party/lua/src/lcorolib.c \
    ../../../../third_party/lua/src/lctype.c \
    ../../../../third_party/lua/src/ldblib.c \
    ../../../../third_party/lua/src/ldebug.c \
    ../../../../third_party/lua/src/ldo.c \
    ../../../../third_party/lua/src/ldump.c \
    ../../../../third_party/lua/src/lfunc.c \
    ../../../../third_party/lua/src/lgc.c \
    ../../../../third_party/lua/src/linit.c \
    ../../../../third_party/lua/src/liolib.c \
    ../../../../third_party/lua/src/llex.c \
    ../../../../third_party/lua/src/lmathlib.c \
    ../../../../third_party/lua/src/lmem.c \
    ../../../../third_party/lua/src/loadlib.c \
    ../../../../third_party/lua/src/lobject.c \
    ../../../../third_party/lua/src/lopcodes.c \
    ../../../../third_party/lua/src/loslib.c \
    ../../../../third_party/lua/src/lparser.c \
    ../../../../third_party/lua/src/lstate.c \
    ../../../../third_party/lua/src/lstring.c \
    ../../../../third_party/lua/src/lstrlib.c \
    ../../../../third_party/lua/src/ltable.c \
    ../../../../third_party/lua/src/ltablib.c \
    ../../../../third_party/lua/src/ltm.c \
    ../../../../third_party/lua/src/luac.c \
    ../../../../third_party/lua/src/lundump.c \
    ../../../../third_party/lua/src/lutf8lib.c \
    ../../../../third_party/lua/src/lvm.c \
    ../../../../third_party/lua/src/lzio.c

HEADERS += \
    ../../../base_model.h \
    ../../../batch_update_result.h \
    ../../../client.h \
    ../../../const.h \
    ../../../idle.h \
    ../../../analytics.h \
    ../../../help_article.h \
    ../../../autotracker.h \
    ../../../urls.h \
    ../../../context.h \
    ../../../migrations.h \
    ../../../custom_error_handler.h \
    ../../../database.h \
    ../../../feedback.h \
    ../../../formatter.h \
    ../../../get_focused_window.h \
    ../../../error.h \
    ../../../gui.h \
    ../../../https_client.h \
    ../../../toggl_api_private.h \
    ../../../model_change.h \
    ../../../obm_action.h \
    ../../../project.h \
    ../../../proxy.h \
    ../../../related_data.h \
    ../../../settings.h \
    ../../../tag.h \
    ../../../task.h \
    ../../../time_entry.h \
    ../../../timeline_event.h \
    ../../../timeline_notifications.h \
    ../../../timeline_uploader.h \
    ../../../types.h \
    ../../../user.h \
    ../../../websocket_client.h \
    ../../../window_change_recorder.h \
    ../../../workspace.h \
    ../../../../third_party/lua/src/lapi.h \
    ../../../../third_party/lua/src/lauxlib.h \
    ../../../../third_party/lua/src/lcode.h \
    ../../../../third_party/lua/src/lctype.h \
    ../../../../third_party/lua/src/ldebug.h \
    ../../../../third_party/lua/src/ldo.h \
    ../../../../third_party/lua/src/lfunc.h \
    ../../../../third_party/lua/src/lgc.h \
    ../../../../third_party/lua/src/llex.h \
    ../../../../third_party/lua/src/llimits.h \
    ../../../../third_party/lua/src/lmem.h \
    ../../../../third_party/lua/src/lobject.h \
    ../../../../third_party/lua/src/lopcodes.h \
    ../../../../third_party/lua/src/lparser.h \
    ../../../../third_party/lua/src/lprefix.h \
    ../../../../third_party/lua/src/lstate.h \
    ../../../../third_party/lua/src/lstring.h \
    ../../../../third_party/lua/src/ltable.h \
    ../../../../third_party/lua/src/ltm.h \
    ../../../../third_party/lua/src/lua.hpp \
    ../../../../third_party/lua/src/luaconf.h \
    ../../../../third_party/lua/src/lualib.h \
    ../../../../third_party/lua/src/lundump.h \
    ../../../../third_party/lua/src/lvm.h \
    ../../../../third_party/lua/src/lzio.h \
    ../../../../third_party/lua/src/lua.h

pocolib = $$PWD/../../../../third_party/poco/lib/Linux/$$system(uname -m)
openssldir = $$PWD/../../../../third_party/openssl

LIBS += -L$$OUT_PWD/
LIBS += -L$$pocolib -lPocoDataSQLite -lPocoData -lPocoNet -lPocoNetSSL -lPocoCrypto -lPocoUtil -lPocoXML -lPocoFoundation
LIBS += -L$$openssldir -lssl -lcrypto
LIBS += -lX11

INCLUDEPATH += $$PWD/../../../../third_party/jsoncpp/dist
DEPENDPATH += $$PWD/../../../../third_party/jsoncpp/dist

INCLUDEPATH += $$PWD/../../../../third_party/lua/src
DEPENDPATH += $$PWD/../../../../third_party/lua/src

INCLUDEPATH += $$PWD/../../../../third_party/poco/Foundation/include
DEPENDPATH += $$PWD/../../../../third_party/poco/Foundation/include

INCLUDEPATH += $$PWD/../../../../third_party/poco/Util/include
DEPENDPATH += $$PWD/../../../../third_party/poco/Util/include

INCLUDEPATH += $$PWD/../../../../third_party/poco/Data/include
DEPENDPATH += $$PWD/../../../../third_party/poco/Data/include

INCLUDEPATH += $$PWD/../../../../third_party/poco/Data/SQLite/include
DEPENDPATH += $$PWD/../../../../third_party/poco/Data/SQLite/include

INCLUDEPATH += $$PWD/../../../../third_party/openssl/include
DEPENDPATH += $$PWD/../../../../third_party/openssl/include

INCLUDEPATH += $$PWD/../../../../third_party/poco/Crypto/include
DEPENDPATH += $$PWD/../../../../third_party/poco/Crypto/include

INCLUDEPATH += $$PWD/../../../../third_party/poco/Net/include
DEPENDPATH += $$PWD/../../../../third_party/poco/Net/include

INCLUDEPATH += $$PWD/../../../../third_party/poco/NetSSL_OpenSSL/include
DEPENDPATH += $$PWD/../../../../third_party/poco/NetSSL_OpenSSL/include

INCLUDEPATH += $$PWD/../../../../third_party/poco/Data/SQLite/src
DEPENDPATH += $$PWD/../../../../third_party/poco/Data/SQLite/src

#QMAKE_CXXFLAGS += -fsanitize=thread -fPIE
#QMAKE_LFLAGS += -pie -fsanitize=thread

QMAKE_CXXFLAGS += -std=c++11

unix:!mac{
	QMAKE_LFLAGS += -Wl,--rpath=\\\$\$\ORIGIN
	QMAKE_LFLAGS += -Wl,--rpath=\\\$\$\ORIGIN/lib
	QMAKE_LFLAGS += -Wl,--rpath=\\\$\$\ORIGIN/lib
	QMAKE_RPATH=
}

