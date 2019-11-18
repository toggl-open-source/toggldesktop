#!/bin/bash

set -e
set -x

VERSION="$1"

if [[ "$VERSION" == "" ]]; then
    exit 0
fi

sed -i.bak "s/7\.0\.0\.0/$VERSION\.0/g" src/ui/windows/TogglDesktop/TogglDesktop/Properties/AssemblyInfo.cs
sed -i.bak "s/7\.0\.0\.0/$VERSION\.0/g" src/ui/windows/TogglDesktop/TogglDesktop.Package/Package.appxmanifest
