TEMPLATE=lib
CONFIG += qt dll qtwinmigrate-buildlib
mac:CONFIG += absolute_library_soname
win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release build_all
include(../src/qtwinmigrate.pri)
TARGET = $$QTWINMIGRATE_LIBNAME
DESTDIR = $$QTWINMIGRATE_LIBDIR
win32 {
    DLLDESTDIR = $$[QT_INSTALL_BINS]
    QMAKE_DISTCLEAN += $$[QT_INSTALL_BINS]\\$${QTWINMIGRATE_LIBNAME}.dll
}
target.path = $$DESTDIR
INSTALLS += target
