#!/bin/bash
appname=`basename "$(test -L "$0" && readlink "$0" || echo "$0")" | sed s,\.sh$,,`
dirname=`dirname "$(test -L "$0" && readlink "$0" || echo "$0")"`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

QTWEBENGINEPROCESS_PATH=$dirname/lib/QtWebEngineProcess
export QTWEBENGINEPROCESS_PATH

# Xubuntu, i3 and Cinnamon tray icon fix
XDG=$XDG_CURRENT_DESKTOP

if [[ "$XDG" = "X-Cinnamon" || "$XDG" = "XFCE" || "$XDG" = "Pantheon" || "$XDG" = "i3" || "$XDG" = "LXDE" || "$XDG" = "MATE" || "$XDG" = "Budgie:GNOME" ]]; then
  DBUS_SESSION_BUS_ADDRESS=""
  dbus-launch $dirname/$appname "$@" &
else
  $dirname/$appname "$@" &
fi;
