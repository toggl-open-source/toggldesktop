#!/bin/bash
appname=`basename "$(test -L "$0" && readlink "$0" || echo "$0")" | sed s,\.sh$,,`
dirname=`dirname "$(test -L "$0" && readlink "$0" || echo "$0")"`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

LD_LIBRARY_PATH=$dirname/lib
export LD_LIBRARY_PATH

XDG=$XDG_CURRENT_DESKTOP

if [[ "$XDG" = "Unity" ]]; then
  $dirname/$appname "$@" &
else
  # Xubuntu, i3 and Cinnamon tray icon fix
  dbus-launch $dirname/$appname "$@" &
fi;