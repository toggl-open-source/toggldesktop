#!/bin/bash

# This script accepts the following parameters:
#
# * cert_password

CONFIG=$@

for line in $CONFIG; do
  eval "$line"
done

if [[ -z "$SIGNTOOL" ]]; then
    SIGNTOOL="/c/Program Files (x86)/Windows Kits/10/bin/10.0.18362.0/x64/signtool.exe"
fi

PATHS="src/ui/windows/TogglDesktop/TogglDesktop/bin/Release
       src/ui/windows/TogglDesktop/TogglDesktop/bin/x64/Release"

EXTENSIONS="exe dll"

arguments=$(sed 's/\</\*./g' <<< $EXTENSIONS | sed 's/\s/ -or -name /g' | sed 's/^/-name /')

for i in $(find $PATHS $arguments 2>/dev/null); do
    echo "Signing $i"
    "$SIGNTOOL" sign -a -t http://timestamp.verisign.com/scripts/timestamp.dll -f "Certificate.pfx" -p $cert_password $i
done

