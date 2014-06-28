TEMPLATE=subdirs
CONFIG += ordered
include(common.pri)
qtwinmigrate-uselib:SUBDIRS=buildlib
SUBDIRS+=examples

!win32:error("This component will only work on Windows. No makefile generated.")
