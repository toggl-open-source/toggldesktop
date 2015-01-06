#!/usr/bin/env bash

fix() {
	pocolib=$1
	lib=$2
	oldpath=$(otool -L $pocolib|fgrep $lib|awk -F' ' '{print $1}'|grep -v @loader_path|grep -v ":")
	if [ -z "$oldpath" ]; then
		return
	fi
	install_name_tool -change $oldpath @loader_path/$lib $pocolib

	# get shared library id name
	file=$(otool -D $pocolib |grep -v ":")
	# get base name of the path we got
	basename=${file##*/}
	# change shared library id name
	install_name_tool -id @loader_path/$basename $pocolib
}

fix_poco_paths() {
	f=$1
	fix $f libPocoUtil.30.dylib
	fix $f libPocoData.30.dylib
	fix $f libPocoNetSSL.30.dylib
	fix $f libPocoXML.30.dylib
	fix $f libPocoDataSQLite.30.dylib
	fix $f libPocoNet.30.dylib
	fix $f libPocoFoundation.30.dylib
	fix $f libPocoCrypto.30.dylib
}

for f in src/ui/osx/TogglDesktop/build/Release/TogglDesktop.app/Contents/Frameworks/*.dylib
do
	fix_poco_paths $f
done
