TARGET      	= trivial

SOURCES		= trivial.cpp

win32 {
   RC_FILE		= trivial.rc
} else:mac {
   QMAKE_INFO_PLIST = Info.plist
   REZ_FILES += trivial.r
   rsrc_files.files = trivial.rsrc
   rsrc_files.path = Contents/Resources
   QMAKE_BUNDLE_DATA += rsrc_files
}

include(../../src/qtbrowserplugin.pri)
