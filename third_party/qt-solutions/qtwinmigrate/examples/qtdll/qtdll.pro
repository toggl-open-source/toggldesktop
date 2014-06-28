TEMPLATE     = lib
CONFIG	    += dll
SOURCES	     = main.cpp

TARGET	     = qtdialog
DLLDESTDIR   = $$[QT_INSTALL_PREFIX]/bin

include(../../src/qtwinmigrate.pri)
