TEMPLATE = app
TARGET	 = QtMfc4

DEFINES -= UNICODE
DEFINES += _AFXDLL WINVER=0x0500
QMAKE_LIBS_QT_ENTRY =

HEADERS = childview.h mainframe.h qtmfc.h stdafx.h
SOURCES = childview.cpp mainframe.cpp qtmfc.cpp stdafx.cpp
FORMS   = optionsdialog.ui
RC_FILE = qtmfc.rc

include(../../../src/qtwinmigrate.pri)
