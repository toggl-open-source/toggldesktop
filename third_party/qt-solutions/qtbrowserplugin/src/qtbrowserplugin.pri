TEMPLATE = lib
CONFIG  += dll
win32 {
  # Uncomment the following line to build a plugin that can be used also in 
  # Internet Explorer, through ActiveX.
  # CONFIG += qaxserver
} else {
  CONFIG += plugin
}

INCLUDEPATH     += $$PWD
DEPENDPATH      += $$PWD

SOURCES	       += $$PWD/qtbrowserplugin.cpp
HEADERS        += $$PWD/qtbrowserplugin.h qtnpapi.h

win32 {
  SOURCES     += $$PWD/qtbrowserplugin_win.cpp
  !isEmpty(TARGET) {
    TARGET = np$$TARGET
  }
  LIBS        += -luser32

  qaxserver {
    DEF_FILE       += $$PWD/qtbrowserpluginax.def
  } else {
    DEF_FILE       += $$PWD/qtbrowserplugin.def
  }

  firefox {
    exists("c:/program files/mozilla firefox/plugins") {
      DLLDESTDIR += "c:/program files/mozilla firefox/plugins"
    } else {
      message("Firefox not found at default location")
    }
  }
  opera {
    exists("c:/program files/opera/program/plugins") {
      DLLDESTDIR += "c:/program files/opera/program/plugins"
    } else {
      message("Opera not found at default location")
    }
  }
  netscape {
    exists("c:/program files/netscape/netscape browser/plugins") {
      DLLDESTDIR += "c:/program files/netscape/netscape browser/plugins"
    } else {
      message("Netscape not found at default location")
    }
  }
} else:mac {
  CONFIG += plugin_bundle
  SOURCES     += $$PWD/qtbrowserplugin_mac.cpp
  #target.path = /Library/Internet\ Plugins
  #INSTALLS += target
} else {
  SOURCES     += $$PWD/qtbrowserplugin_x11.cpp
  INCLUDEPATH += /usr/X11R6/include
  
  # Avoiding symbol clash with other instances of the Qt library
  # (ref. developingplugins.html in the doc.):
  #
  # For Qt 4.4 and later, just configure Qt to use a separate namespace:
  # configure -qtnamespace SomeNamespace
  #
  # For Qt 4.3: Uncomment the line below.
  # It makes the dynamic linker prefer our own Qt symbols for the plugin,
  # provided that our Qt is statically built and linked into the
  # plugin. Note that to force the linker to prefer the static Qt
  # libraries (.a files), the dynamic libraries (.so) files must not
  # be present in the lib directory.
  #
  # QMAKE_LFLAGS += -Wl,-Bsymbolic
}
