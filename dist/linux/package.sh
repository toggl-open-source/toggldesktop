#!/bin/bash

scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
reporoot="${scriptdir}/../.."

set -e
PLUGINS="imageformats/libqsvg.so iconengines/libqsvgicon.so platforms/libqxcb.so"

if [ -z "$builddir" ]; then
    builddir="build-tarball"
    mkdir -p "$builddir"
fi
fullbuilddir=$PWD/$builddir
errorlog=$fullbuilddir/error.log

if [ -z "$TOGGL_VERSION" ]; then
    TOGGL_VERSION=${TAG_NAME/v/}
fi
if [ -z "$TOGGL_VERSION" ]; then
    TOGGL_VERSION="7.0.0"
fi

if [ ! -z "$TOGGL_VERSION" ]; then
    VERSION_DEFINE="-DTOGGL_VERSION=${TOGGL_VERSION}"
fi
if [ -z "$BUILD_TESTS" ]; then
    BUILD_TESTS="OFF"
else
    BUILD_TESTS="ON"
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
    cmake -DTOGGL_BUILD_TESTS=$BUILD_TESTS -DTOGGL_PRODUCTION_BUILD=ON -DTOGGL_ALLOW_UPDATE_CHECK=ON $VERSION_DEFINE -DUSE_BUNDLED_LIBRARIES=ON -DCMAKE_INSTALL_PREFIX="$PREFIX" -DTOGGL_INTERNAL_LIB_DIR="$LIBDIR" ..
    echo "=========== Building..." >&2
    make -j4
    echo "=========== Installing" >&2
    make install
    popd > /dev/null
}

function listdeps() {
    local file=""
    if [[ "$2" == "" ]]; then
        file=$1
    else
        file=$2
    fi
    ldd $file | grep "=>" | sed "s/.*=> //" | sed "s/ [(]0x[0-9a-fA-F]*[)]//" | grep -v "^/usr/lib" | grep -v "^/lib" | grep -v "^$PREFIX/[^/]*" | grep -v "not found" | sed '/^ *$/d'

}

function compose() {
    pushd "$PREFIX" >/dev/null

    mkdir -p "$BINDIR" "$LIBDIR" "$LIBDIR"

    echo "=========== Composing the package" >&2
    rm -fr include lib/cmake
    if [ ! -z "$CMAKE_PREFIX_PATH" ]; then
        export LD_LIBRARY_PATH="$CMAKE_PREFIX_PATH/../"
    fi

    ldd "$BINDIR"/TogglDesktop
    corelib=$(ldd "$BINDIR"/TogglDesktop | grep -e libQt5Core  | sed 's/.* => \(.*\)[(]0x.*/\1/')
    echo "corelib: " $corelib
    libdir=$(dirname "$corelib")
    echo "libdir: " $libdir
    qmake=$(find $libdir/../bin/ $libdir/../../bin/ -name qmake -or -name qmake-qt5 | head -n1)
    echo "qmake: " $qmake

    plugindir=$($qmake -query QT_INSTALL_PLUGINS)
    translationdir=$($qmake -query QT_INSTALL_TRANSLATIONS)
    datadir=$($qmake -query QT_INSTALL_DATA)

    for i in `listdeps "${BINDIR}/TogglDesktop"`; do
        cp -Lrfu $i "$LIBDIR"
    done

    if [[ -f /usr/lib/x86_64-linux-gnu/libssl.so.1.1 ]]; then
        cp -Lrfu /usr/lib/x86_64-linux-gnu/libssl.so.1.1 "$LIBDIR"
        cp -Lrfu /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1 "$LIBDIR"
    fi
    if [[ -f /usr/lib/x86_64-linux-gnu/libstdc++.so.6 ]]; then
        cp -Lrfu /usr/lib/x86_64-linux-gnu/libstdc++.so.6 "$LIBDIR"
    fi
    
    for i in `ls -1 "${LIBDIR}"/*.so`; do
        for j in `listdeps $i`; do
            cp -Lrfu $j "$LIBDIR"
        done
    done

    for i in $PLUGINS; do
        newpath="$LIBDIR/qt5/plugins/$(dirname $i)/"
        file=$(basename $i)
        mkdir -p $newpath
        cp -Lrfu $plugindir/$i $newpath
        patchelf --set-rpath '$ORIGIN/../../../' $newpath/$file
        for j in `listdeps $newpath/$file`; do
            cp -Lrfu $j "$LIBDIR"
        done
    done

    for i in `ls -1 "$LIBDIR"/*.so`; do
        for j in `listdeps $i`; do
            cp -Lrfu $j "$LIBDIR"
        done
    done

    for i in `ls -1 "${BINDIR}"/* | grep -v "sh$"`; do
        patchelf --set-rpath '$ORIGIN/../lib' $i
    done
    for i in `ls -1 "${LIBDIR}"/*.so*`; do
        patchelf --set-rpath '$ORIGIN' $i
    done

    cat <<EOF >"bin/qt.conf"
[Paths]
Prefix=../lib
Plugins=qt5/plugins
Data=qt5
Translations=qt5/translations
EOF

    cp "${reporoot}/src/ssl/cacert.pem" bin

    popd >/dev/null
}

function package() {
    pushd "$PREFIX" >/dev/null
    
    # echo "=========== Stripping" >&2
    # for i in $(find . -name \*.so); do
    #     strip --strip-unneeded $i;
    # done

    echo "=========== Packaging" >&2
    mv "$BINDIR/TogglDesktop.sh" "$BINDIR"/..
    cd ..
    mv "$PREFIX" toggldesktop
    tar cvfz $reporoot/toggldesktop_$(uname -m).tar.gz toggldesktop >/dev/null

    popd >/dev/null
    echo "=========== Result is: $reporoot/toggldesktop_$(uname -m).tar.gz ==========="
}

if [[ "$#" -ne 1 ]]; then
    build
    compose
    package
else
    $1
fi
