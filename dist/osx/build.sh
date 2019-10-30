#!/usr/bin/env bash

set -e

# Force using sdk 10.11
export  LDFLAGS="-mmacosx-version-min=10.11"
export   CFLAGS="$LDFLAGS"
export CXXFLAGS="$LDFLAGS"

export TAG_NAME="v7.9.99"
version=${TAG_NAME/v/}

function app_path() {
    echo $(xcodebuild -scheme TogglDesktop -workspace src/ui/osx/TogglDesktop.xcworkspace -configuration Release -showBuildSettings \
                | grep -w 'BUILT_PRODUCTS_DIR' \
                | cut -d'=' -f 2 \
                | sed -e 's/^[ \t]*//')/TogglDesktop.app
}

function dependencies() {
    make deps
}

function cocoapods() {
    make init_cocoapod
}

function app() {
    make app_release
}

function plist() {
    # Get app path
    APP_PATH=$(app_path)

    # Update the plist file (version, enable update check, UI logging to file etc)
    mkdir -p tmp 
    #go run src/branding/osx/plist.go -path="$APP_PATH" -version=$version
    awk '/CFBundleVersion/{print;getline;$0="\t<string>'"$version"'</string>"}1' $APP_PATH/Contents/Info.plist > tmp/Info.plist
    mv tmp/Info.plist $APP_PATH/Contents/Info.plist
    awk '/CFBundleShortVersionString/{print;getline;$0="\t<string>'"$version"'</string>"}1' $APP_PATH/Contents/Info.plist > tmp/Info.plist
    # Overwrite built apps plist file
    mv tmp/Info.plist $APP_PATH/Contents/Info.plist
    
    rmdir tmp
}

function codesign() {
    security unlock-keychain -p '' /Users/$USER/Library/Keychains/login.keychain
    ./src/branding/osx/codesign.sh $APP_PATH
}

function notarize() {
    BUNDLE_APP=$(dirname $(app_path))
    make -C ./src/branding/osx BUNDLE_APP="${BUNDLE_APP}" notarize
}

function dmg() {
    ./src/branding/osx/dmg.sh $APP_PATH
}

function rename_dmg() {
    export timestamp=$(date "+%Y-%m-%d-%H-%M-%S") 
    export escaped_version=$(echo $version | sed 's/\./_/g') 
    export installer=TogglDesktop-$escaped_version-$timestamp.dmg
    export installer_name=TogglDesktop-$escaped_version.dmg

    mv TogglDesktop.dmg $installer 
}

function debuginfo() {
    # Compress main app debug info
    export dsym_dylib=TogglDesktopLibrary.dylib-$escaped_version-$timestamp-dsym.tar.gz 
    rm -rf $dsym_dylib 
    tar cvfz $dsym_dylib $APP_PATH/../TogglDesktopLibrary.dylib.dSYM

    # Compress dynamic library debug info
    export dsym=TogglDesktop-$escaped_version-$timestamp-dsym.tar.gz 
    rm -rf $dsym 
    tar cvfz $dsym $APP_PATH/../TogglDesktop.app.dSYM
}


dependencies
cocoapods
app
plist
codesign
#notarize
dmg
rename_dmg
debuginfo

# Update AppCast
# Upload the new version to Github releases
# Update releases.json
# Post to Slack
