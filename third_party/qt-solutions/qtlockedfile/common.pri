exists(config.pri):infile(config.pri, SOLUTIONS_LIBRARY, yes): CONFIG += qtlockedfile-uselib
TEMPLATE += fakelib
QTLOCKEDFILE_LIBNAME = $$qtLibraryTarget(QtSolutions_LockedFile-head)
TEMPLATE -= fakelib
QTLOCKEDFILE_LIBDIR = $$PWD/lib
unix:qtlockedfile-uselib:!qtlockedfile-buildlib:QMAKE_RPATHDIR += $$QTLOCKEDFILE_LIBDIR
