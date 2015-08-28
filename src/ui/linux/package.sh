export qt_arch=x86_64
if [ "$(uname -m)" == "i386" ]; then
  export qt_arch=i386
fi
if [ "$(uname -m)" == "i686" ]; then
  export qt_arch=i386
fi

if [ -z "$QPATH" ]; then
	export QPATH=/home/tanel/Qt5.5.0/5.5/gcc_64/
fi
if [ -z "$QLIBPATH" ]; then
	export QLIBPATH=/home/tanel/Qt5.5.0/5.5/gcc_64/lib
fi

set -e

echo "Creating package"

out=out/linux/toggldesktop

rm -rf toggldesktop*.tar.gz $out

mkdir -p $out

cp src/lib/linux/TogglDesktopLibrary/build/release/libTogglDesktopLibrary.so.1 $out

cp third_party/bugsnag-qt/build/release/libbugsnag-qt.so.1.0.0 $out

cp src/ui/linux/TogglDesktop/build/release/TogglDesktop $out/.

# copy icons
cp -r src/ui/linux/TogglDesktop/icons $out/icons

# SSL library needs to be copied manually
# else local system installed library will get packaged?!?!
rm -rf $out/lib/libssl* $out/lib/libcrypto*

cp src/ssl/cacert.pem $out/.

cd $out/..

tar cvfz toggldesktop_$(uname -m).tar.gz toggldesktop

echo "Packaging done"
