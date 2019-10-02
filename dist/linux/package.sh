#!/bin/bash

set -e
PLUGINS="imageformats/libqsvg.so iconengines/libqsvgicon.so platforms/libqxcb.so"

if [ -z "$builddir" ]; then
    builddir="build-tarball"
    mkdir -p "$builddir"
fi
fullbuilddir=$PWD/$builddir
errorlog=$fullbuilddir/error.log

if [ ! -z "$TOGGL_VERSION" ]; then
    VERSION_DEFINE="-DTOGGL_VERSION=$TOGGL_VERSION"
fi

function build() {
    echo "=========== Will build in $fullbuilddir" >&2

    pushd $builddir >/dev/null

    echo "=========== Configuring" >&2
    cmake -DTOGGL_PRODUCTION_BUILD=ON -DTOGGL_ALLOW_UPDATE_CHECK=ON $VERSION_DEFINE -DUSE_BUNDLED_LIBRARIES=ON -DCMAKE_INSTALL_PREFIX="$PWD/package" ..
    echo "=========== Building..." >&2
    make -j4
    echo "=========== Installing" >&2
    make install
    popd > /dev/null
}

function compose() {
    pushd "$builddir/package" >/dev/null

    echo "=========== Composing the package" >&2
    rm -fr include lib/cmake
    if [ ! -z "$CMAKE_PREFIX_PATH" ]; then
        export LD_LIBRARY_PATH="$CMAKE_PREFIX_PATH/../"
    fi

    ldd bin/TogglDesktop
    corelib=$(ldd bin/TogglDesktop | grep -e libQt5Core  | sed 's/.* => \(.*\)[(]0x.*/\1/')
    echo "corelib: " $corelib
    libdir=$(dirname "$corelib")
    echo "libdir: " $libdir
    qmake=$(find $libdir/../bin/ $libdir/../../bin/ -name qmake -or -name qmake-qt5 | head -n1)
    echo "qmake: " $qmake

    cp -Lrfu $(ldd bin/TogglDesktop | grep -e libQt -e ssl -e libicu -e double-conversion -e jpeg -e re2 -e avcodec -e avformat -e avutil -e webp | sed 's/.* => \(.*\)[(]0x.*/\1/') lib
    ls "$qmake" >/dev/null

    libexecdir=$($qmake -query QT_INSTALL_LIBEXECS)
    plugindir=$($qmake -query QT_INSTALL_PLUGINS)
    translationdir=$($qmake -query QT_INSTALL_TRANSLATIONS)
    datadir=$($qmake -query QT_INSTALL_DATA)

    cp "$libexecdir/QtWebEngineProcess" bin
    mkdir -p lib
    for i in $PLUGINS; do
        newpath=lib/qt5/plugins/$(dirname $i)/
        file=$(basename $i)
        mkdir -p $newpath
        cp -Lrfu $plugindir/$i $newpath
        patchelf --set-rpath '$ORIGIN/../../../' $newpath/$file
        ldd $newpath/$file | grep -e libQt -e ssl
        echo "========"
        ldd $newpath/$file | grep -e libQt -e ssl | sed 's/.* => \(.*\)[(]0x.*/\1/'
        echo "========"
        for j in $(ldd $newpath/$file | grep -e libQt -e ssl | sed 's/.* => \(.*\)[(]0x.*/\1/'); do
            if [ ! -f lib/$(basename "$j") ]; then
                cp -vLrfu $j lib
            fi
        done
    done

    for i in $(ls lib/*.so); do
        for j in $(ldd $i | grep -e libQt | sed 's/.* => \(.*\)[(]0x.*/\1/'); do
            if [ ! -f lib/$(basename "$j") ]; then
                cp -vLrfu $j lib
            fi
        done
    done

    patchelf --set-rpath '$ORIGIN/../lib/' bin/TogglDesktop
    patchelf --set-rpath '$ORIGIN/../lib/' bin/QtWebEngineProcess

    for i in $(ls lib/*.so*); do
        patchelf --set-rpath '$ORIGIN' $i
    done

    mkdir -p lib/qt5/translations lib/qt5/resources
    cp -Lrfu "$translationdir/qtwebengine_locales" lib/qt5/translations
    cp -Lrfu "$datadir/resources/"* lib/qt5/resources

    mv "bin/TogglDesktop.sh" "."

    cat <<EOF >bin/qt.conf
    [Paths]
    Prefix=..
    Plugins=lib/qt5/plugins
    Data=lib/qt5
    Translations=lib/qt5/translations
EOF

    # echo "=========== Stripping" >&2
    # for i in bin/QtWebEngineProcess $(find . -name \*.so); do
    #     strip --strip-unneeded $i;
    # done

    echo "=========== Packaging" >&2
    tar cvfz ../../toggldesktop_$(uname -m).tar.gz * >/dev/null

    popd >/dev/null
    echo "=========== Result is: $PWD/toggldesktop_$(uname -m).tar.gz ==========="
}

if [[ "$#" -ne 1 ]]; then
    build
    compose
else
    $1
fi
