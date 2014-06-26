#!/usr/bin/env bash

fix() {
	wheretofix=$1
	lib=$2
	oldpath=$(otool -L $wheretofix|fgrep $lib|awk -F' ' '{print $1}'|grep -v @loader_path|grep -v ":")
	if [ -z "$oldpath" ]; then
		return
	fi
	install_name_tool -change $oldpath @loader_path/$lib $wheretofix
	install_name_tool -id @loader_path/$lib $wheretofix
}

fix_poco_paths() {
	f=$1
	fix $f libPocoUtil.16.dylib
	fix $f libPocoData.16.dylib
	fix $f libPocoNetSSL.16.dylib
	fix $f libPocoXML.16.dylib
	fix $f libPocoDataSQLite.16.dylib
	fix $f libPocoNet.16.dylib
	fix $f libPocoFoundation.16.dylib
	fix $f libPocoCrypto.16.dylib
}

for f in src/ui/osx/TogglDesktop/build/Release/TogglDesktop.app/Contents/Frameworks/*.dylib
do
	fix_poco_paths $f
done