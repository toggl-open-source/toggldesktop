#!/bin/bash
appname=`basename "$(test -L "$0" && readlink "$0" || echo "$0")" | sed s,\.sh$,,`
dirname=`dirname "$(test -L "$0" && readlink "$0" || echo "$0")"/`
if [ -f "$dirname/bin/TogglDesktop" ]; then
  dirname="$dirname/bin"
fi
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

if [ -f "$dirname/QtWebEngineProcess" ]; then
  QTWEBENGINEPROCESS_PATH=$dirname/QtWebEngineProcess
  export QTWEBENGINEPROCESS_PATH
fi
export QTWEBENGINE_CHROMIUM_FLAGS="--disable-logging"

# Xubuntu, i3 and Cinnamon tray icon fix
XDG=$XDG_CURRENT_DESKTOP

# Check if dbus-launch actually exists
DBUS_LAUNCH=$(which dbus-launch 2>/dev/null)

if [[ "$XDG" = "X-Cinnamon" || "$XDG" = "XFCE" || "$XDG" = "Pantheon" || "$XDG" = "i3" || "$XDG" = "LXDE" || "$XDG" = "MATE" || "$XDG" = "Budgie:GNOME" ]]; then
  DBUS_SESSION_BUS_ADDRESS=""
  $DBUS_LAUNCH $dirname/$appname "$@" &
else
  $dirname/$appname "$@" &
fi;
