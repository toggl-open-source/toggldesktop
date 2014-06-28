include(../common.pri)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
LIBS += -luser32

# When built as a library, the default is to include the component's
# MFC support. To disable this (e.g. in order to use this component
# with Visual Studio Express, which does not include MFC), comment out
# the following line.
qtwinmigrate-uselib:DEFINES *= _AFXDLL

qtwinmigrate-uselib:!qtwinmigrate-buildlib {
    LIBS += -L$$QTWINMIGRATE_LIBDIR -l$$QTWINMIGRATE_LIBNAME
} else {
    SOURCES += $$PWD/qwinwidget.cpp $$PWD/qwinhost.cpp $$PWD/qmfcapp.cpp
    HEADERS += $$PWD/qwinwidget.h $$PWD/qwinhost.h $$PWD/qmfcapp.h
}

win32 {
    qtwinmigrate-buildlib:DEFINES += QT_QTWINMIGRATE_EXPORT
    else:qtwinmigrate-uselib:DEFINES += QT_QTWINMIGRATE_IMPORT
}
contains(QT_MAJOR_VERSION, 5): QT += widgets gui-private