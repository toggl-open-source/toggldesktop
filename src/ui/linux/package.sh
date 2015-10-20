export qt_arch=x86_64

set -e

echo "Creating package"

echo "QPATH is $QPATH"
echo "QLIBPATH is $QLIBPATH"

out=out/linux/toggldesktop

rm -rf toggldesktop*.tar.gz $out

mkdir -p $out/lib $out/platforms

cp src/lib/linux/TogglDesktopLibrary/build/release/libTogglDesktopLibrary.so.1 $out/.

cp src/ui/linux/README $out/.

cp third_party/bugsnag-qt/build/release/libbugsnag-qt.so.1 $out/.

cp third_party/poco/lib/Linux/x86_64/libPocoCrypto.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoData.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoDataSQLite.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoFoundation.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoJSON.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoNet.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoNetSSL.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoUtil.so.31 $out/.
cp third_party/poco/lib/Linux/x86_64/libPocoXML.so.31 $out/.

cp src/ui/linux/TogglDesktop/build/release/TogglDesktop $out/.

chrpath -r "\$ORIGIN" $out/TogglDesktop

cp $QPATH/plugins/platforms/libqxcb.so $out/platforms/.

(chrpath -r "\$ORIGIN/lib" $out/platforms/* || true)

echo "-- DEPENDENCIES for libqxcb --"
ldd $(pwd)/$out/platforms/libqxcb.so
go run src/ui/linux/copy_deps.go --executable $(pwd)/$out/platforms/libqxcb.so --destination $(pwd)/$out/lib

echo "-- DEPENDENCIES for TogglDesktop --"
ldd $(pwd)/$out/TogglDesktop
go run src/ui/linux/copy_deps.go --executable $(pwd)/$out/TogglDesktop --destination $(pwd)/$out/lib

chrpath -r "\$ORIGIN/lib" $out/TogglDesktop

# SVG plugins dont come out as deps need to copy manually
ls -la $QLIBPATH
cp $QLIBPATH/libQt5Svg.so.5 $(pwd)/$out/lib/
cp $QLIBPATH/libQt5Xml.so.5 $(pwd)/$out/lib/
mkdir -p $(pwd)/$out/imageformats
mkdir -p $(pwd)/$out/iconengines
ls -la $QPATH/plugins/
ls -la $QPATH/plugins/imageformats/
ls -la $QPATH/plugins/iconengines/
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
