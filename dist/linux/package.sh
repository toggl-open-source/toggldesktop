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

if [ -z "$PREFIX" ]; then
    PREFIX="$PWD/package"
fi
if [ -z "$BINDIR" ]; then
    BINDIR="$PREFIX/bin"
fi
if [ -z "$LIBDIR" ]; then
    LIBDIR="$PREFIX/lib"
fi
if [ -z "$THIRDPARTYDIR" ]; then
    THIRDPARTYDIR="$PREFIX/lib"
fi
if [ -z "$LIBEXECDIR" ]; then
    LIBEXECDIR="$PREFIX/bin"
fi
if [ -z "$SCRIPTDIR" ]; then
    SCRIPTDIR="$PREFIX"
fi

function relpath() {
    if [ -f "$1" ]; then
        realpath --relative-to="$(dirname $1)" "$2"
    else
        realpath --relative-to="$1" "$2"
    fi
}

function build() {
    echo "=========== Will build in $fullbuilddir" >&2

    pushd $builddir >/dev/null

    echo "=========== Configuring" >&2
    cmake -DTOGGL_PRODUCTION_BUILD=ON -DTOGGL_ALLOW_UPDATE_CHECK=ON $VERSION_DEFINE -DUSE_BUNDLED_LIBRARIES=ON -DCMAKE_INSTALL_PREFIX="$PREFIX" -DTOGGL_INTERNAL_LIB_DIR="$THIRDPARTYDIR" ..
    echo "=========== Building..." >&2
    make -j4
    echo "=========== Installing" >&2
    make install
    popd > /dev/null
}

function compose() {
    pushd "$PREFIX" >/dev/null

    mkdir -p "$BINDIR" "$LIBDIR" "$THIRDPARTYDIR" "$LIBEXECDIR" "$SCRIPTDIR"

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

    cp -Lrfu $(ldd bin/TogglDesktop | grep -e libQt -e ssl -e crypto -e libicu -e double-conversion -e jpeg -e re2 -e avcodec -e avformat -e avutil -e webp | sed 's/.* => \(.*\)[(]0x.*/\1/') "$THIRDPARTYDIR"
    ls "$qmake" >/dev/null

    libexecdir=$($qmake -query QT_INSTALL_LIBEXECS)
    plugindir=$($qmake -query QT_INSTALL_PLUGINS)
    translationdir=$($qmake -query QT_INSTALL_TRANSLATIONS)
    datadir=$($qmake -query QT_INSTALL_DATA)

    mkdir -p "$LIBEXECDIR"
    cp "$libexecdir/QtWebEngineProcess" "$LIBEXECDIR"
    mkdir -p "$LIBDIR"
    for i in $PLUGINS; do
        newpath="$THIRDPARTYDIR/qt5/plugins/$(dirname $i)/"
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
                cp -vLrfu $j "$THIRDPARTYDIR"
            fi
        done
    done

    for i in $(ls "$THIRDPARTYDIR"/*.so); do
        for j in $(ldd $i | grep -e libQt | sed 's/.* => \(.*\)[(]0x.*/\1/'); do
            if [ ! -f lib/$(basename "$j") ]; then
                cp -vLrfu $j "$THIRDPARTYDIR"
            fi
        done
    done

    patchelf --set-rpath '$ORIGIN/'$(relpath "$BINDIR/TogglDesktop" "$THIRDPARTYDIR") "$BINDIR/TogglDesktop"
    patchelf --set-rpath '$ORIGIN/'$(relpath "$LIBEXECDIR/QtWebEngineProcess" "$THIRDPARTYDIR") "$LIBEXECDIR/QtWebEngineProcess"
    patchelf --set-rpath '$ORIGIN/'$(relpath "$LIBDIR/libTogglDesktopLibrary.so" "$THIRDPARTYDIR") "$LIBDIR/libTogglDesktopLibrary.so"

    for i in $(ls "$THIRDPARTYDIR"/*.so*); do
        patchelf --set-rpath '$ORIGIN' $i
    done

    mkdir -p "$THIRDPARTYDIR/qt5/translations" "$THIRDPARTYDIR/qt5/resources"
    cp -Lrfu "$translationdir/qtwebengine_locales" "$THIRDPARTYDIR/qt5/translations"
    cp -Lrfu "$datadir/resources/"* "$THIRDPARTYDIR/qt5/resources"

    if [ "$SCRIPTDIR" != "$BINDIR" ]; then
        mv -n "bin/TogglDesktop.sh" "$SCRIPTDIR/TogglDesktop.sh"
    fi

    cat <<EOF >"$LIBEXECDIR/qt.conf"
    [Paths]
    Prefix=../lib
    Plugins=qt5/plugins
    Data=qt5
    Translations=qt5/translations
EOF

}

function package() {
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
    package
else
    $1
fi
