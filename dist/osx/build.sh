#!/usr/bin/env bash

set -e

# Force using sdk 10.11
export  LDFLAGS="-mmacosx-version-min=10.11"
export   CFLAGS="$LDFLAGS"
export CXXFLAGS="$LDFLAGS"

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
    
    cat $APP_PATH/Contents/Info.plist
    rmdir tmp
}

function sign() {
    security unlock-keychain -p 'password' ~/Library/Keychains/build.keychain
    APP=$(app_path)
    EXECUTABLE=$APP/Contents/MacOS/TogglDesktop
    CERTIFICATE="Developer ID Application: TOGGL OU"

    echo "== check that gatekeeper is enabled =="
    spctl --status|grep "disabled" && echo "cannot continue"

    codesign --force --options runtime --deep --sign "${CERTIFICATE}" $APP/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app
    codesign --force --options runtime --deep --sign "${CERTIFICATE}" $APP/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app/Contents/MacOS/fileop

for filename in $APP/Contents/Frameworks/*; do
        codesign -d --force --options runtime -vvvv --verify --strict -s "${CERTIFICATE}"  -r='designated => anchor apple generic and certificate leaf[subject.OU] = "B227VTMZ94"' $filename
    done

    codesign -d --force --options runtime -vvvv --verify --strict -s "${CERTIFICATE}" -r='designated => anchor apple generic and identifier "com.toggl.toggldesktop.TogglDesktop" and certificate leaf[subject.OU] = "B227VTMZ94"' $EXECUTABLE

    codesign -d --force --options runtime -vvvv --verify --strict -s "${CERTIFICATE}" -r='designated => anchor apple generic and identifier "com.toggl.toggldesktop.TogglDesktop" and certificate leaf[subject.OU] = "B227VTMZ94"' $APP

    codesign --deep --verify --strict --verbose=4 $APP
}

function notarize() {
    APP_PATH=$(app_path)
    BUNDLE_APP=$(dirname "${APP_PATH}")
    EXPORT_PATH=${BUNDLE_APP}/Submissions
    BUNDLE_ZIP=${EXPORT_PATH}/TogglDesktop.zip
    UPLOAD_INFO_PLIST=${EXPORT_PATH}/UploadInfo.plist
    REQUEST_INFO_PLIST=${EXPORT_PATH}/RequestInfo.plist
    AUDIT_INFO_JSON=${EXPORT_PATH}/AuditInfo.json
    DEVELOPER_USERNAME=${APPLE_APPID_USER}
    DEVELOPER_PASSWORD=${APPLE_APPID_PASSWORD}

    echo "Notarization" "Building a ZIP archive…"
    /usr/bin/ditto -c -k --keepParent ${APP_PATH} ${BUNDLE_ZIP}
    echo "Notarization" "Uploading for notarization…"
    /usr/bin/xcrun altool --notarize-app --primary-bundle-id "com.toggl.toggldesktop.TogglDesktop.zip" -itc_provider "B227VTMZ94" -u ${DEVELOPER_USERNAME} -p ${DEVELOPER_PASSWORD} -f ${BUNDLE_ZIP} --output-format xml > ${UPLOAD_INFO_PLIST} || cat ${UPLOAD_INFO_PLIST}
    echo "Notarization" "Waiting while notarized…"
    while true; do
        /usr/bin/xcrun altool --notarization-info `/usr/libexec/PlistBuddy -c "Print :notarization-upload:RequestUUID" ${UPLOAD_INFO_PLIST}` -u ${DEVELOPER_USERNAME} -p ${DEVELOPER_PASSWORD} --output-format xml > ${REQUEST_INFO_PLIST} || cat ${REQUEST_INFO_PLIST}
        if [[ `/usr/libexec/PlistBuddy -c "Print :notarization-info:Status" ${REQUEST_INFO_PLIST}` != "in progress" ]]; then
            break
        fi
        echo '\n***** Notarization - waiting 60s'
        sleep 60
    done
    echo "Notarization" "Downloading log file…"
    /usr/bin/curl -o ${AUDIT_INFO_JSON} `/usr/libexec/PlistBuddy -c "Print :notarization-info:LogFileURL" ${REQUEST_INFO_PLIST}`
    if [ `/usr/libexec/PlistBuddy -c "Print :notarization-info:Status" ${REQUEST_INFO_PLIST}` != "success" ]; then \
        false; \
    fi
    echo "Notarization", "Stapling…"
    /usr/bin/xcrun stapler staple ${APP_PATH}
    echo "Notarization", "✅ Done!"
}

function dmg() {
    APP_PATH=$(app_path)
    npm install --global create-dmg
    brew install graphicsmagick imagemagick
    create-dmg $APP_PATH
    mv *.dmg $installer
    pwd
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

function appcast() {
    signature=$(./src/ui/osx/Pods/Sparkle/bin/old_dsa_scripts/sign_update $installer ./dsa_priv.pem)
    filesize=$(cat $installer | wc -c | sed 's/ //g') 
    functionilesize=(${filesize// / })
    appUrl=https://github.com/toggl-open-source/toggldesktop/releases/download/v$version/$installer_name

    echo $signature
    echo $filesize
    echo $appUrl

    mkdir -p branding
    mkdir -p tmp
    
    # Generate AppCast
    # Save to dist/osx/appcast
    go run dist/osx/appcast.go -platform="darwin" -version=$version -date=$timestamp -appUrl=$appUrl -filesize="${filesize}" -signature=$signature -verbose=true 
    cat dist/osx/appcast/darwin_dev_appcast.xml
}

function upload() {
    # Upload the new version to Github releases
    PLATFORM="darwin" VERSION=$version INSTALLER_FILENAME=$installer_name INSTALLER=$installer GITHUB_USER="token" GITHUB_TOKEN=${GITHUB_TOKEN} go run dist/osx/upload_to_github.go -platform="darwin" 

    # Update releases.json
    echo "Update releases.json and download links"
    # ./dist/osx/update_releases.sh osx dev $version
}

if [[ "$#" -ne 1 ]]; then
    cocoapods
    app
    plist
    sign
    notarize
    debuginfo
    dmg
    appcast
    upload
else
    $1
fi

# Update AppCast
# Upload the new version to Github releases
# Update releases.json
# Post to Slack
