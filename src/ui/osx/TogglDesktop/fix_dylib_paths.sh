dylib=src/ui/osx/TogglDesktop/build/Release/TogglDesktop.app/Contents/Frameworks/Kopsik.dylib

fix() {
	lib=$1
	oldpath=$(otool -L $dylib|fgrep $lib|awk -F' ' '{print $1}')
	install_name_tool -change $oldpath @loader_path/$lib $dylib
}

fix libPocoUtil.16.dylib
fix libPocoData.16.dylib
fix libPocoNetSSL.16.dylib
fix libPocoXML.16.dylib
fix libPocoDataSQLite.16.dylib
fix libPocoNet.16.dylib
fix libPocoFoundation.16.dylib
fix libPocoCrypto.16.dylib

