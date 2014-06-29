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
    ../../../context.cc \
    ../../../CustomErrorHandler.cc \
    ../../../database.cc \
    ../../../feedback.cc \
    ../../../formatter.cc \
    ../../../get_focused_window_linux.cc \
    ../../../gui.cc \
    ../../../https_client.cc \
    ../../../json.cc \
    ../../../kopsik_api.cc \
    ../../../kopsik_api_private.cc \
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
    ../../../context.h \
    ../../../CustomErrorHandler.h \
    ../../../database.h \
    ../../../feedback.h \
    ../../../formatter.h \
    ../../../get_focused_window.h \
    ../../../gui.h \
    ../../../https_client.h \
    ../../../json.h \
    ../../../kopsik_api_private.h \
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
    ../../include/kopsik_api.h

pocolib = $$PWD/../../../../third_party/poco-1.4.6p2-all/lib/Linux/x86_64
jsondir = $$PWD/../../../../third_party/libjson
openssldir = $$PWD/../../../../third_party/openssl

LIBS += -L$$OUT_PWD/
LIBS += -L$$pocolib -lPocoDataSQLite -lPocoData -lPocoNet -lPocoNetSSL -lPocoCrypto -lPocoUtil -lPocoXML -lPocoFoundation
LIBS += -L$$jsondir -ljson
LIBS += -L$$openssldir -lssl -lcrypto
LIBS += -lX11

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Foundation/include
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Foundation/include

INCLUDEPATH += $$PWD/../../../../third_party/libjson
DEPENDPATH += $$PWD/../../../../third_party/libjson

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Util/include
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Util/include

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Data/include
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Data/include

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Data/SQLite/include
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Data/SQLite/include

INCLUDEPATH += $$PWD/../../../../third_party/openssl/include
DEPENDPATH += $$PWD/../../../../third_party/openssl/include

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Crypto/include
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Crypto/include

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Net/include
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Net/include

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/NetSSL_OpenSSL/include
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/NetSSL_OpenSSL/include

INCLUDEPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Data/SQLite/src
DEPENDPATH += $$PWD/../../../../third_party/poco-1.4.6p2-all/Data/SQLite/src
