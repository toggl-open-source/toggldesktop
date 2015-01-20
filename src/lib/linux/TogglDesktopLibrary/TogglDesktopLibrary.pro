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
    ../../../autocomplete_item.cc \
    ../../../base_model.cc \
    ../../../batch_update_result.cc \
    ../../../client.cc \
    ../../../idle.cc \
    ../../../analytics.cc \
    ../../../context.cc \
    ../../../custom_error_handler.cc \
    ../../../database.cc \
    ../../../feedback.cc \
    ../../../formatter.cc \
    ../../../get_focused_window_linux.cc \
    ../../../error.cc \
    ../../../gui.cc \
    ../../../https_client.cc \
    $$PWD/../../../../third_party/jsoncpp/dist/jsoncpp.cpp \
    ../../../toggl_api.cc \
    ../../../toggl_api_private.cc \
    ../../../model_change.cc \
    ../../../project.cc \
    ../../../proxy.cc \
    ../../../related_data.cc \
    ../../../settings.cc \
    ../../../tag.cc \
    ../../../task.cc \
    ../../../time_entry.cc \
    ../../../timeline_uploader.cc \
    ../../../user.cc \
    ../../../websocket_client.cc \
    ../../../window_change_recorder.cc \
    ../../../workspace.cc

HEADERS += \
    ../../../autocomplete_item.h \
    ../../../base_model.h \
    ../../../batch_update_result.h \
    ../../../client.h \
    ../../../const.h \
    ../../../idle.h \
    ../../../analytics.h \
    ../../../context.h \
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
    ../../../project.h \
    ../../../proxy.h \
    ../../../related_data.h \
    ../../../settings.h \
    ../../../tag.h \
    ../../../task.h \
    ../../../time_entry.h \
    ../../../timeline_constants.h \
    ../../../timeline_event.h \
    ../../../timeline_notifications.h \
    ../../../timeline_uploader.h \
    ../../../types.h \
    ../../../user.h \
    ../../../websocket_client.h \
    ../../../window_change_recorder.h \
    ../../../workspace.h \
    ../../include/toggl_api.h

pocolib = $$PWD/../../../../third_party/poco/lib/Linux/$$system(uname -m)
openssldir = $$PWD/../../../../third_party/openssl

LIBS += -L$$OUT_PWD/
LIBS += -L$$pocolib -lPocoDataSQLite -lPocoData -lPocoNet -lPocoNetSSL -lPocoCrypto -lPocoUtil -lPocoXML -lPocoFoundation
LIBS += -L$$openssldir -lssl -lcrypto
LIBS += -lX11

INCLUDEPATH += $$PWD/../../../../third_party/poco/Foundation/include
DEPENDPATH += $$PWD/../../../../third_party/poco/Foundation/include

INCLUDEPATH += $$PWD/../../../../third_party/jsoncpp/dist
DEPENDPATH += $$PWD/../../../../third_party/jsoncpp/dist

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

unix:!mac{
	QMAKE_LFLAGS += -Wl,--rpath=\\\$\$\ORIGIN
	QMAKE_LFLAGS += -Wl,--rpath=\\\$\$\ORIGIN/lib
	QMAKE_LFLAGS += -Wl,--rpath=\\\$\$\ORIGIN/lib
	QMAKE_RPATH=
}

