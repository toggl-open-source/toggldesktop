export qt_arch=x86_64
if [ "$(uname -m)" == "i386" ]; then
  export qt_arch=i386
fi
if [ "$(uname -m)" == "i686" ]; then
  export qt_arch=i386
fi

if [ -z "$QPATH" ]; then
	export QPATH=/home/buildbot/Qt/5.3/gcc_64/
fi
if [ -z "$QLIBPATH" ]; then
	export QLIBPATH=/home/buildbot/Qt/5.3/gcc_64/lib
fi

set -e

echo "Creating package"

out=out/linux/toggldesktop

rm -rf toggldesktop*.tar.gz $out

mkdir -p $out/lib $out/platforms

cp $QPATH/plugins/platforms/libqxcb.so $out/platforms/.

(chrpath -r "\$ORIGIN/lib" $out/platforms/* || true)

go run src/ui/linux/copy_deps.go --executable $(pwd)/$out/platforms/libqxcb.so --destination $(pwd)/$out/lib

cp src/ui/linux/TogglDesktop/build/release/TogglDesktop $out/.

go run src/ui/linux/copy_deps.go --executable $(pwd)/$out/TogglDesktop --destination $(pwd)/$out/lib

chrpath -r "\$ORIGIN/lib" $out/TogglDesktop

# SVG plugins dont come out as deps need to copy manually
cp $QLIBPATH/libQt5Svg.so.5 $(pwd)/$out/lib/
cp $QLIBPATH/libQt5Xml.so.5 $(pwd)/$out/lib/
mkdir -p $(pwd)/$out/imageformats
mkdir -p $(pwd)/$out/iconengines
cp $QPATH/plugins/imageformats/libqsvg.so $(pwd)/$out/imageformats/
cp $QPATH/plugins/iconengines/libqsvgicon.so $(pwd)/$out/iconengines/

# copy icons
cp -r src/ui/linux/TogglDesktop/icons $out/icons

# SSL library needs to be copied manually
# else local system installed library will get packaged?!?!
rm -rf $out/lib/libssl* $out/lib/libcrypto*
cp third_party/openssl/libssl.so.1.0.0 $out/lib/
cp third_party/openssl/libcrypto.so.1.0.0 $out/lib/

cp src/ssl/cacert.pem $out/.

chmod -x $out/lib/*
chmod -w $out/lib/*

cd $out/..

tar cvfz toggldesktop_$(uname -m).tar.gz toggldesktop

echo "Packaging done"
