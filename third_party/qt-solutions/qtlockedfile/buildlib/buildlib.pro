TEMPLATE=lib
CONFIG += qt dll qtlockedfile-buildlib
mac:CONFIG += absolute_library_soname
win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release build_all
include(../src/qtlockedfile.pri)
TARGET = $$QTLOCKEDFILE_LIBNAME
DESTDIR = $$QTLOCKEDFILE_LIBDIR
win32 {
    DLLDESTDIR = $$[QT_INSTALL_BINS]
    QMAKE_DISTCLEAN += $$[QT_INSTALL_BINS]\\$${QTLOCKEDFILE_LIBNAME}.dll
}
target.path = $$DESTDIR
INSTALLS += target
