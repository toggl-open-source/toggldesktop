#!/bin/bash

PLUGINS="imageformats/libqsvg.so iconengines/libqsvgicon.so platforms/libqxcb.so"

if [ -z "$builddir" ]; then
    builddir=$(mktemp -d build-XXXXXX)
fi
fullbuilddir=$PWD/$builddir
errorlog=$fullbuilddir/error.log

if [ ! -z "$TOGGL_VERSION" ]; then
    VERSION_DEFINE="-DTOGGL_VERSION=$TOGGL_VERSION"
fi

function CHECK() {
    echo "===================================" >> $errorlog
    echo "Running $@" >> $errorlog
    echo "===================================" >> $errorlog
    if eval $@ 2>> $errorlog ; then
        echo "OK" >&2
    else
        echo "Build failed, see $errorlog for details." >&2
        exit 1
    fi
    echo "" >> $errorlog
}

echo "Will build in $fullbuilddir" >&2

pushd $builddir >/dev/null
mkdir "package"

echo "Configuring" >&2
CHECK cmake -DTOGGL_PRODUCTION_BUILD=ON -DTOGGL_ALLOW_UPDATE_CHECK=ON $VERSION_DEFINE -DUSE_BUNDLED_LIBRARIES=ON -DCMAKE_INSTALL_PREFIX="$PWD/package" .. > cmake.log
echo "Building..." >&2
CHECK make -j8 > build.log
echo "Installing" >&2
CHECK make install > install.log

pushd package >/dev/null

echo "Composing the package" >&2
rm -fr include lib/cmake
if [ ! -z "$CMAKE_PREFIX_PATH" ]; then
    export LD_LIBRARY_PATH="$CMAKE_PREFIX_PATH/../"
fi

ldd bin/TogglDesktop
corelib=$(ldd bin/TogglDesktop | grep -e libQt5Core  | sed 's/.* => \(.*\)[(]0x.*/\1/')
echo "corelib: " $corelib
libdir=$(dirname "$corelib")
echo "libdir: " $libdir
qmake=$(ls $libdir/../bin/{qmake,qmake-qt5} $libdir/../../bin/{qmake,qmake-qt5} 2>/dev/null)
echo "qmake: " $qmake

CHECK cp -Lrn $(ldd bin/TogglDesktop | grep -e libQt -e ssl -e libicu | sed 's/.* => \(.*\)[(]0x.*/\1/') lib
CHECK ls "$qmake" >/dev/null

libexecdir=$($qmake -query QT_INSTALL_LIBEXECS)
plugindir=$($qmake -query QT_INSTALL_PLUGINS)
translationdir=$($qmake -query QT_INSTALL_TRANSLATIONS)
datadir=$($qmake -query QT_INSTALL_DATA)

CHECK cp "$libexecdir/QtWebEngineProcess" bin
for i in $PLUGINS; do
    newpath=lib/qt5/plugins/$(dirname $i)/
    file=$(basename $i)
    CHECK mkdir -p $newpath
    CHECK cp -Lrn $plugindir/$i $newpath
    CHECK patchelf --set-rpath '\$ORIGIN/../../../' $newpath/$file
    ldd $newpath/$file | grep -e libQt -e ssl
    echo "========"
    ldd $newpath/$file | grep -e libQt -e ssl | sed 's/.* => \(.*\)[(]0x.*/\1/'
    echo "========"
    CHECK cp -Lrn $(ldd $newpath/$file | grep -e libQt -e ssl | sed 's/.* => \(.*\)[(]0x.*/\1/') lib
done

for i in $(ls lib/*.so); do
    for j in $(ldd $i | grep -e libQt | sed 's/.* => \(.*\)[(]0x.*/\1/'); do
        CHECK cp -Lrn $j lib
    done
done

CHECK patchelf --set-rpath '\$ORIGIN/../lib/' bin/TogglDesktop
CHECK patchelf --set-rpath '\$ORIGIN/../lib/' bin/QtWebEngineProcess

for i in $(ls lib/*.so); do
    CHECK patchelf --set-rpath '\$ORIGIN' $i
done

CHECK mkdir -p lib/qt5/translations lib/qt5/resources
CHECK cp -Lrn "$translationdir/qtwebengine_locales" lib/qt5/translations
CHECK cp -Lrn "$datadir/resources/"* lib/qt5/resources

CHECK mv "bin/TogglDesktop.sh" "."

CHECK cat <<EOF >bin/qt.conf
[Paths]
Prefix=..
Plugins=lib/qt5/plugins
Data=lib/qt5
Translations=lib/qt5/translations
EOF

echo "Stripping" >&2
for i in bin/QtWebEngineProcess $(find . -name \*.so); do 
    strip --strip-unneeded $i;
done

echo "Packaging" >&2
CHECK tar cvfz ../../toggldesktop_$(uname -m).tar.gz * >/dev/null

popd >/dev/null
popd >/dev/null

echo "Result is: $PWD/toggldesktop_$(uname -m).tar.gz"
