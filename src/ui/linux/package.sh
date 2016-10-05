set -e

echo "Creating package"

echo "QPATH is $QPATH"
echo "QLIBPATH is $QLIBPATH"

out=out/linux/toggldesktop

# Clear output directories
rm -rf toggldesktop*.tar.gz $out/*
mkdir -p $out/lib $out/platforms $out/imageformats $out/iconengines

# Copy Toggl Desktop shared library
cp src/lib/linux/TogglDesktopLibrary/build/release/libTogglDesktopLibrary.so.1 $out/lib

# Copy README
cp src/ui/linux/README $out/.

# Copy Bugsnag library
cp third_party/bugsnag-qt/build/release/libbugsnag-qt.so.1 $out/lib

# Copy Poco libraries
cp third_party/poco/lib/Linux/x86_64/libPocoCrypto.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoData.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoDataSQLite.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoFoundation.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoJSON.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoNet.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoNetSSL.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoUtil.so.31 $out/lib
cp third_party/poco/lib/Linux/x86_64/libPocoXML.so.31 $out/lib

# Copy executable
cp src/ui/linux/TogglDesktop/build/release/TogglDesktop $out

# Copy startup script
cp src/ui/linux/TogglDesktop.sh $out

# Copy Qt libraries
cp $QLIBPATH/libQt5Svg.so.5 $out/lib
cp $QLIBPATH/libQt5Xml.so.5 $out/lib
cp $QLIBPATH/libQt5XcbQpa.so.5 $out/lib
cp $QLIBPATH/libQt5XcbQpa.so.5 $out/lib
cp $QLIBPATH/libQt5WebKitWidgets.so.5 $out/lib
cp $QLIBPATH/libQt5DBus.so.5 $out/lib
cp $QLIBPATH/libQt5Widgets.so.5 $out/lib
cp $QLIBPATH/libQt5Gui.so.5 $out/lib
cp $QLIBPATH/libQt5Network.so.5 $out/lib
cp $QLIBPATH/libQt5Core.so.5 $out/lib
cp $QLIBPATH/libQt5WebKit.so.5 $out/lib
cp $QLIBPATH/libQt5Sensors.so.5 $out/lib
cp $QLIBPATH/libQt5Positioning.so.5 $out/lib
cp $QLIBPATH/libQt5PrintSupport.so.5 $out/lib
cp $QLIBPATH/libQt5OpenGL.so.5 $out/lib
cp $QLIBPATH/libQt5Sql.so.5 $out/lib
cp $QLIBPATH/libicui18n.so.54 $out/lib
cp $QLIBPATH/libicuuc.so.54 $out/lib
cp $QLIBPATH/libicudata.so.54 $out/lib
cp $QLIBPATH/libQt5Quick.so.5 $out/lib
cp $QLIBPATH/libQt5Qml.so.5 $out/lib
cp $QLIBPATH/libQt5WebChannel.so.5 $out/lib
cp $QLIBPATH/libQt5X11Extras.so.5 $out/lib

# Copy Qt plugins
cp $QPATH/plugins/imageformats/libqsvg.so $out/imageformats
cp $QPATH/plugins/iconengines/libqsvgicon.so $out/iconengines
cp $QPATH/plugins/platforms/libqxcb.so $out/platforms

# Copy icons
cp -r src/ui/linux/TogglDesktop/icons $out/icons

# SSL library needs to be copied manually
# else local system installed library will get packaged?!?!
rm -rf $out/lib/libssl* $out/lib/libcrypto*
cp third_party/openssl/libssl.so.1.0.0 $out/lib/
cp third_party/openssl/libcrypto.so.1.0.0 $out/lib/

# Copy certificate bundle
cp src/ssl/cacert.pem $out/.

# Set permissions
chmod -x $out/lib/*
chmod -w $out/lib/*

# Create a tar ball
cd $out/..
tar cvfz toggldesktop_$(uname -m).tar.gz toggldesktop

echo "Packaging done"
