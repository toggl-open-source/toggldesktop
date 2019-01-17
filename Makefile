$(shell mkdir -p build/test build/test src/lib/osx/build)

pwd=$(shell pwd)
uname=$(shell uname)
architecture=$(shell uname -m)
timestamp=$(shell date "+%Y-%m-%d-%H-%M-%S")

rootdir=$(shell pwd)
pocodir=third_party/poco
openssldir=third_party/openssl
jsoncppdir=third_party/jsoncpp/dist
pocoversion=$(shell cat third_party/poco/libversion)
tbbdir=third_party/tbb

GTEST_ROOT=third_party/googletest-read-only

source_dirs=src/*.cc src/*.h src/test/*.cc src/test/*.h \
	src/ui/linux/TogglDesktop/aboutdialog.h src/ui/linux/TogglDesktop/aboutdialog.cpp \
	src/ui/linux/TogglDesktop/autocompleteview.h src/ui/linux/TogglDesktop/autocompleteview.cpp \
	src/ui/linux/TogglDesktop/clickablelabel.h src/ui/linux/TogglDesktop/clickablelabel.cpp \
	src/ui/linux/TogglDesktop/errorviewcontroller.h src/ui/linux/TogglDesktop/errorviewcontroller.cpp \
	src/ui/linux/TogglDesktop/feedbackdialog.h src/ui/linux/TogglDesktop/feedbackdialog.cpp \
	src/ui/linux/TogglDesktop/genericview.h src/ui/linux/TogglDesktop/genericview.cpp \
	src/ui/linux/TogglDesktop/loginwidget.h src/ui/linux/TogglDesktop/loginwidget.cpp \
	src/ui/linux/TogglDesktop/main.cpp \
	src/ui/linux/TogglDesktop/mainwindowcontroller.h src/ui/linux/TogglDesktop/mainwindowcontroller.cpp \
	src/ui/linux/TogglDesktop/preferencesdialog.h src/ui/linux/TogglDesktop/preferencesdialog.cpp \
	src/ui/linux/TogglDesktop/settingsview.h src/ui/linux/TogglDesktop/settingsview.cpp \
	src/ui/linux/TogglDesktop/singleapplication.h src/ui/linux/TogglDesktop/singleapplication.cpp \
	src/ui/linux/TogglDesktop/timeentrycellwidget.h src/ui/linux/TogglDesktop/timeentrycellwidget.cpp \
	src/ui/linux/TogglDesktop/timeentryeditorwidget.h src/ui/linux/TogglDesktop/timeentryeditorwidget.cpp \
	src/ui/linux/TogglDesktop/timeentrylistwidget.h src/ui/linux/TogglDesktop/timeentrylistwidget.cpp \
	src/ui/linux/TogglDesktop/timeentryview.h src/ui/linux/TogglDesktop/timeentryview.cpp \
	src/ui/linux/TogglDesktop/timerwidget.h src/ui/linux/TogglDesktop/timerwidget.cpp \
	src/ui/linux/TogglDesktop/toggl.h src/ui/linux/TogglDesktop/toggl.cpp

ifndef QMAKE
QMAKE=qmake
endif

ifndef MSBUILD
MSBUILD=/cygdrive/c/Program\ Files/MSBuild/12.0/Bin/MSBuild.exe
endif

ifeq ($(uname), Darwin)
xcodebuild_command=xcodebuild \
				  -scheme TogglDesktop \
				  -project src/ui/osx/TogglDesktop/TogglDesktop.xcodeproj  \
				  -configuration Release
executable=$(shell $(xcodebuild_command) \
			 -showBuildSettings \
 			| grep -w 'BUILT_PRODUCTS_DIR' \
 			| cut -d'=' -f 2)/TogglDesktop.app/Contents/MacOS/TogglDesktop
pocolib=$(pocodir)/lib/Darwin/x86_64/
osname=mac
endif	

ifeq ($(uname), Linux)
executable=./src/ui/linux/TogglDesktop/build/release/TogglDesktop
pocolib=$(pocodir)/lib/Linux/$(architecture)
osname=linux
endif

ifneq (, $(findstring CYGWIN, $(uname) ))
executable=./src/ui/windows/TogglDesktop/TogglDesktop/bin/Release_VS/TogglDesktop.exe
pocolib=$(pocodir)/lib/CYGWIN/i686
osname=windows
endif

pococflags = \
	-I$(pocodir)/Foundation/include \
	-I$(pocodir)/Util/include \
	-I$(pocodir)/Data/include \
	-I$(pocodir)/Data/SQLite/include \
	-I$(pocodir)/Crypto/include \
	-I$(pocodir)/Net/include \
	-I$(pocodir)/NetSSL_OpenSSL/include

pocolibs = -L$(pocolib)

ifeq ($(osname), windows)
pococonfigure = --sqlite-thread-safe=0
else
pococonfigure = --cflags=-fPIC --sqlite-thread-safe=1
endif

pocolibs += \
	-lPocoDataSQLite \
	-lPocoData \
	-lPocoNet \
	-lPocoNetSSL \
	-lPocoCrypto \
	-lPocoUtil \
	-lPocoXML \
	-lPocoJSON \
	-lPocoFoundation

ifeq ($(osname), windows)
opensslincdirname = inc32
else
opensslincdirname = include
endif

opensslcflags = -I$(openssldir)/$(opensslincdirname)
openssllibs = -L$(openssldir)
opensslconfigure = --include-path=$(pwd)/$(openssldir)/$(opensslincdirname) --library-path=$(pwd)/$(openssldir)

ifeq ($(osname), mac)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -Wunreachable-code -DLUA_USE_MACOSX \
	-mmacosx-version-min=10.11 \
	$(pococflags) $(opensslcflags) \
	-I$(GTEST_ROOT)/include \
	-I$(GTEST_ROOT) \
	-I$(jsoncppdir) \
	-Ithird_party/lua/install/include \
	-DNDEBUG
endif

ifeq ($(osname), linux)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -static \
	$(opensslcflags) \
	-I$(GTEST_ROOT)/include \
	-I$(GTEST_ROOT) \
	$(pococflags) \
	-I$(jsoncppdir) \
	-Ithird_party/lua/install/include \
	-DNDEBUG
endif

ifeq ($(osname), windows)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -static \
	$(opensslcflags) \
	-I$(GTEST_ROOT)/include \
	-I$(GTEST_ROOT) \
	$(pococflags) \
	-I$(jsoncppdir) \
	-Ithird_party/lua/install/include \
	-DNDEBUG
endif

ifeq ($(osname), mac)
libs=-framework Carbon \
	$(pocolibs) \
	-lpthread \
	$(openssllibs) \
	-lssl \
	-lcrypto \
  -Lthird_party/lua/install/lib \
	-llua \
	-ldl
endif

ifeq ($(osname), linux)
libs=-lX11 \
	$(pocolibs) \
	-lpthread \
	$(openssllibs) \
	-lssl \
	-lcrypto \
	-lrt \
  -Lthird_party/lua/install/lib \
	-llua \
	-ldl
endif

ifeq ($(osname), windows)
libs= $(pocolibs) \
	-lpthread \
	$(openssllibs) \
	-lrt \
	-lpsapi \
  -Lthird_party/lua/install/lib \
	-llua
endif

cxx=g++ -fprofile-arcs -ftest-coverage -std=gnu++0x

ifeq ($(osname), windows)
default: fmt app
endif

default: fmt app

csapi: generate_cs_api fmt

generate_cs_api:
	go run src/script/generate_cs_api.go

clean: clean_ui clean_lib clean_test
	rm -rf build coverage

ifeq ($(osname), linux)
clean_lib:
	rm -rf src/lib/linux/TogglDesktopLibrary/build && \
	(cd src/lib/linux/TogglDesktopLibrary && $(QMAKE) && make clean)
endif

ifeq ($(osname), mac)
clean_lib:
	rm -rf src/lib/osx/build
endif

ifeq ($(osname), windows)
clean_lib:
	$(MSBUILD) src/lib/windows/TogglDesktopDLL/TogglDesktopDLL.vcxproj /p:Configuration=Release /target:Clean
endif

ifeq ($(osname), linux)
clean_ui: clean-bugsnag-qt
	(cd src/ui/linux/TogglDesktop && $(QMAKE) && make clean) && \
	rm -rf src/ui/linux/TogglDesktop/build
endif

ifeq ($(osname), mac)
clean_ui:
	rm -rf src/ui/osx/TogglDesktop/build third_party/TFDatePicker/TFDatePicker/build TogglDesktop*.dmg TogglDesktop*.tar.gz
endif

ifeq ($(osname), windows)
clean_ui:
	$(MSBUILD) src/ui/windows/TogglDesktop/TogglDesktop.sln /p:Configuration=Release /t:TogglDesktop:Clean
endif

clean_test:
	rm -rf test/*
	mkdir -p test

lint:
	./third_party/cpplint/cpplint.py $(source_dirs)

cppclean:
	./third_party/cppclean/cppclean -q --include-path=src/lib/include --include-path=third_party/poco/Crypto/include --include-path=third_party/poco/Net/include --include-path=third_party/poco/NetSSL_OpenSSL/include --include-path=third_party/poco/Foundation/include --include-path=third_party/poco/Util/include --include-path=third_party/poco/Data/include --include-path=third_party/poco/Data/SQLite/include --include-path=third_party/poco/Data/SQLite/src --include-path=third_party/lua/install/include src/*.*

qa: lint fmt cppclean test

fmt: fmt_lib fmt_ui

app: lib ui

ifeq ($(osname), mac)
lib:
	xcodebuild -project src/lib/osx/TogglDesktopLibrary.xcodeproj
endif

ifeq ($(osname), linux)
lib:
	cd src/lib/linux/TogglDesktopLibrary && $(QMAKE) && make && \
	cd ../../../../ && \
	cp $(openssldir)/*so* src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoCrypto.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoData.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoDataSQLite.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoFoundation.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoNet.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoNetSSL.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoUtil.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoXML.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoJSON.so.$(pocoversion) src/lib/linux/TogglDesktopLibrary/build/release
endif

ifeq ($(osname), windows)
lib:
	$(MSBUILD) src/ui/windows/TogglDesktop/TogglDesktop.sln /p:Configuration=Release /t:TogglDesktopDLL
endif

ifeq ($(osname), mac)
ui:
	$(xcodebuild_command)
endif

ifeq ($(osname), linux)
ui: bugsnag-qt
	cd src/ui/linux/TogglDesktop && $(QMAKE) && make && \
	cd ../../../../ && \
	cp src/ssl/cacert.pem src/ui/linux/TogglDesktop/build/release
endif

ifeq ($(osname), windows)
ui:
	$(MSBUILD) src/ui/windows/TogglDesktop/TogglDesktop.sln /p:Configuration=Release
endif

clean-bugsnag-qt:
	rm -rf third_party/bugsnag-qt/build && \
	cd third_party/bugsnag-qt && $(QMAKE) && make clean

bugsnag-qt: clean-bugsnag-qt
	cd third_party/bugsnag-qt && $(QMAKE) && make

run: app
	$(executable)

clean_deps:
	cd $(pocodir) && (make clean || true)
	rm -rf $(pocodir)/**/.dep
	cd $(openssldir) && (make clean || true)
	cd third_party/lua && make clean

deps: clean_deps init_submodule openssl poco lua tbb copy_libs

init_submodule:
	cd $(rootdir) && git submodule update --init --recursive

ifeq ($(osname), linux)
lua:
	cd third_party/lua && make linux && make local
endif

ifeq ($(osname), mac)
lua:
	cd third_party/lua && make  $(LEGACYMACOSSDK) macosx && make $(LEGACYMACOSSDK) local
endif

ifeq ($(osname), windows)
lua:
	cd third_party/lua && make generic && make local
endif

openssl:
ifeq ($(osname), mac)
	cd $(openssldir) && ./config -fPIC no-shared no-dso && ./Configure darwin64-x86_64-cc && make $(LEGACYMACOSSDK)
endif
ifeq ($(osname), linux)
	cd $(openssldir) && ./config -fPIC shared no-dso && make clean && make
endif
ifeq ($(osname), windows)
	cd $(openssldir) && ./config shared no-dso && ./Configure Cygwin && make
endif

poco:
	cd $(pocodir) && \
	./configure --omit=Data/ODBC,Data/MySQL,Zip,JSON,MongoDB,PageCompiler,PageCompiler/File2Page,CppUnit --no-tests --no-samples \
	$(pococonfigure) \
	$(opensslconfigure) \
	&& \
	make clean && \
	make $(LEGACYMACOSSDK)

copy_libs:
ifeq ($(osname), mac)
	cp $(pocodir)/lib/Darwin/x86_64/libPocoCrypto.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoData.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoDataSQLite.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoFoundation.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoNet.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoNetSSL.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoUtil.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoXML.$(pocoversion).dylib /usr/local/lib/
	cp $(pocodir)/lib/Darwin/x86_64/libPocoJSON.$(pocoversion).dylib /usr/local/lib/
	cp $(openssldir)/libssl.1.1.dylib /usr/local/lib/ 2>/dev/null || :
	cp $(openssldir)/libcrypto.1.1.dylib /usr/local/lib/ 2>/dev/null || :
endif

third_party/google-astyle/build/google-astyle:
	cd third_party/google-astyle && mkdir -p build && g++ *.cpp -o build/google-astyle

fmt_lib: third_party/google-astyle/build/google-astyle
	third_party/google-astyle/build/google-astyle -n $(source_dirs)
	third_party/google-astyle/build/google-astyle -n src/ui/windows/TogglDesktop/TogglDesktopDLLInteropTest//*.cs

fmt_ui:
	./third_party/Xcode-formatter/CodeFormatter/scripts/formatAllSources.sh src/ui/osx/
	third_party/google-astyle/build/google-astyle -n src/ui/windows/TogglDesktop/TogglDesktop/*.cs
	third_party/google-astyle/build/google-astyle -n src/ui/windows/TogglDesktop/TogglDesktop/TogglApi.cs

build/jsoncpp.o: $(jsoncppdir)/jsoncpp.cpp
	$(cxx) $(cflags) -c $(jsoncppdir)/jsoncpp.cpp -o build/jsoncpp.o

build/proxy.o: src/proxy.cc
	$(cxx) $(cflags) -c src/proxy.cc -o build/proxy.o

build/netconf.o: src/netconf.cc
	$(cxx) $(cflags) -c src/netconf.cc -o build/netconf.o

build/https_client.o: src/https_client.cc
	$(cxx) $(cflags) -c src/https_client.cc -o build/https_client.o

build/websocket_client.o: src/websocket_client.cc
	$(cxx) $(cflags) -c src/websocket_client.cc -o build/websocket_client.o

build/base_model.o: src/base_model.cc
	$(cxx) $(cflags) -c src/base_model.cc -o build/base_model.o

build/user.o: src/user.cc
	$(cxx) $(cflags) -c src/user.cc -o build/user.o

build/workspace.o: src/workspace.cc
	$(cxx) $(cflags) -c src/workspace.cc -o build/workspace.o

build/client.o: src/client.cc
	$(cxx) $(cflags) -c src/client.cc -o build/client.o

build/project.o: src/project.cc
	$(cxx) $(cflags) -c src/project.cc -o build/project.o

build/task.o: src/task.cc
	$(cxx) $(cflags) -c src/task.cc -o build/task.o

build/obm_action.o: src/obm_action.cc
	$(cxx) $(cflags) -c src/obm_action.cc -o build/obm_action.o

build/help_article.o: src/help_article.cc
	$(cxx) $(cflags) -c src/help_article.cc -o build/help_article.o

build/time_entry.o: src/time_entry.cc
	$(cxx) $(cflags) -c src/time_entry.cc -o build/time_entry.o

build/tag.o: src/tag.cc
	$(cxx) $(cflags) -c src/tag.cc -o build/tag.o

build/related_data.o: src/related_data.cc
	$(cxx) $(cflags) -c src/related_data.cc -o build/related_data.o

build/batch_update_result.o: src/batch_update_result.cc
	$(cxx) $(cflags) -c src/batch_update_result.cc -o build/batch_update_result.o

build/formatter.o: src/formatter.cc
	$(cxx) $(cflags) -c src/formatter.cc -o build/formatter.o

build/model_change.o: src/model_change.cc
	$(cxx) $(cflags) -c src/model_change.cc -o build/model_change.o

build/migrations.o: src/migrations.cc
	$(cxx) $(cflags) -c src/migrations.cc -o build/migrations.o

build/database.o: src/database.cc
	$(cxx) $(cflags) -c src/database.cc -o build/database.o

build/feedback.o: src/feedback.cc
	$(cxx) $(cflags) -c src/feedback.cc -o build/feedback.o

build/gui.o: src/gui.cc
	$(cxx) $(cflags) -c src/gui.cc -o build/gui.o

build/error.o: src/error.cc
	$(cxx) $(cflags) -c src/error.cc -o build/error.o

build/idle.o: src/idle.cc
	$(cxx) $(cflags) -c src/idle.cc -o build/idle.o

build/analytics.o: src/analytics.cc
	$(cxx) $(cflags) -c src/analytics.cc -o build/analytics.o

build/urls.o: src/urls.cc
	$(cxx) $(cflags) -c src/urls.cc -o build/urls.o

build/timeline_event.o: src/timeline_event.cc
	$(cxx) $(cflags) -c src/timeline_event.cc -o build/timeline_event.o

build/context.o: src/context.cc
	$(cxx) $(cflags) -c src/context.cc -o build/context.o

build/settings.o: src/settings.cc
	$(cxx) $(cflags) -c src/settings.cc -o build/settings.o

build/autotracker.o: src/autotracker.cc
	$(cxx) $(cflags) -c src/autotracker.cc -o build/autotracker.o

build/toggl_api_private.o: src/toggl_api_private.cc
	$(cxx) $(cflags) -c src/toggl_api_private.cc -o build/toggl_api_private.o

build/toggl_api.o: src/toggl_api.cc
	$(cxx) $(cflags) -c src/toggl_api.cc -o build/toggl_api.o

build/test/test_data.o: src/test/test_data.cc
	$(cxx) $(cflags) -c src/test/test_data.cc -o build/test/test_data.o

build/test/toggl_api_test.o: src/test/toggl_api_test.cc
	$(cxx) $(cflags) -c src/test/toggl_api_test.cc -o build/test/toggl_api_test.o

build/test/app_test.o: src/test/app_test.cc
	$(cxx) $(cflags) -c src/test/app_test.cc -o build/test/app_test.o

build/get_focused_window_$(osname).o: src/get_focused_window_$(osname).cc
	$(cxx) $(cflags) -c src/get_focused_window_$(osname).cc -o build/get_focused_window_$(osname).o

build/timeline_uploader.o: src/timeline_uploader.cc
	$(cxx) $(cflags) -c src/timeline_uploader.cc -o build/timeline_uploader.o

build/window_change_recorder.o: src/window_change_recorder.cc
	$(cxx) $(cflags) -c src/window_change_recorder.cc -o build/window_change_recorder.o

build/test/gtest-all.o: $(GTEST_ROOT)/src/gtest-all.cc
	$(cxx) $(cflags) -c $(GTEST_ROOT)/src/gtest-all.cc -o build/test/gtest-all.o

objects: build/jsoncpp.o \
	build/related_data.o \
	build/proxy.o \
	build/netconf.o \
	build/https_client.o \
	build/websocket_client.o \
	build/base_model.o \
	build/user.o \
	build/workspace.o \
	build/client.o \
	build/project.o \
	build/task.o \
	build/obm_action.o \
	build/help_article.o \
	build/time_entry.o \
	build/tag.o \
	build/batch_update_result.o \
	build/formatter.o \
	build/model_change.o \
	build/database.o \
	build/feedback.o \
	build/error.o \
	build/gui.o \
	build/idle.o \
	build/analytics.o \
	build/autotracker.o \
	build/settings.o \
	build/urls.o \
	build/timeline_event.o \
	build/migrations.o \
	build/context.o \
	build/toggl_api_private.o \
	build/toggl_api.o \
	build/get_focused_window_$(osname).o \
	build/timeline_uploader.o \
	build/window_change_recorder.o

test_objects: build/test/gtest-all.o \
	build/test/test_data.o \
	build/test/app_test.o \
	build/test/toggl_api_test.o

uitest: clean_test
ifeq ($(osname), windows)
	$(executable) \
		--script-path src/test/uitest.lua \
		--log-path test/uitest.log \
		--db-path test/uitest.db \
		--environment test
else
	$(executable) \
		--script-path $(pwd)/src/test/uitest.lua \
		--log-path $(pwd)/test/uitest.log \
		--db-path $(pwd)/test/uitest.db
endif

toggl_test: clean_test objects test_objects
	mkdir -p test
	$(cxx) -coverage -o test/toggl_test build/*.o build/test/*.o $(libs)

test_lib: lua toggl_test
	cp src/ssl/cacert.pem test/.
ifeq ($(osname), linux)
	cp -r $(pocodir)/lib/Linux/$(architecture)/* test/.
	cp -r $(openssldir)/*so* test/.
	cd test && LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./toggl_test --gtest_shuffle
endif
ifeq ($(osname), mac)
	cp -r $(pocolib)/* test/.
	install_name_tool -change /usr/local/lib/libPocoCrypto.$(pocoversion).dylib @loader_path/libPocoCrypto.$(pocoversion).dylib test/libPocoNetSSL.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoCrypto.$(pocoversion).dylib @loader_path/libPocoCrypto.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoData.$(pocoversion).dylib @loader_path/libPocoData.$(pocoversion).dylib test/libPocoDataSQLite.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoData.$(pocoversion).dylib @loader_path/libPocoData.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoData.$(pocoversion).dylib @loader_path/libPocoData.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoDataSQLite.$(pocoversion).dylib @loader_path/libPocoDataSQLite.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoCrypto.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoData.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoDataSQLite.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoNet.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoNetSSL.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoUtil.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoXML.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoFoundation.$(pocoversion).dylib @loader_path/libPocoFoundation.$(pocoversion).dylib test/libPocoJSON.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoJSON.$(pocoversion).dylib @loader_path/libPocoJSON.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoNet.$(pocoversion).dylib @loader_path/libPocoNet.$(pocoversion).dylib test/libPocoNetSSL.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoNet.$(pocoversion).dylib @loader_path/libPocoNet.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoNetSSL.$(pocoversion).dylib @loader_path/libPocoNetSSL.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoUtil.$(pocoversion).dylib @loader_path/libPocoUtil.$(pocoversion).dylib test/libPocoNetSSL.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoUtil.$(pocoversion).dylib @loader_path/libPocoUtil.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoXML.$(pocoversion).dylib @loader_path/libPocoXML.$(pocoversion).dylib test/toggl_test
	install_name_tool -change /usr/local/lib/libPocoXML.$(pocoversion).dylib @loader_path/libPocoXML.$(pocoversion).dylib test/libPocoUtil.$(pocoversion).dylib
	install_name_tool -change /usr/local/lib/libPocoJSON.$(pocoversion).dylib @loader_path/libPocoJSON.$(pocoversion).dylib test/libPocoUtil.$(pocoversion).dylib
	cd test && ./toggl_test --gtest_shuffle
endif
ifeq ($(osname), windows)
	cp -r $(pocolib)/* test/.
	cd test && ./toggl_test --gtest_shuffle
endif

test: test_lib

lcov: test
	lcov -q -d . -c -o app.info
	genhtml -q -o coverage app.info

coverage: lcov
	@echo "open coverage/index.html to view coverage report"

loco:
ifeq ($(osname), mac)
	xcodebuild -exportLocalizations -localizationPath src/ui/osx/localization -project src/ui/osx/TogglDesktop/TogglDesktop.xcodeproj -exportLanguage et
endif

package:
	./src/ui/linux/package.sh

authors:
	git log --all --format='%aN <%cE>' | sort -u > AUTHORS

tbb:
ifeq ($(osname), mac)
	cd $(rootdir) && cd third_party/tbb && make tbb_build_prefix=mac
endif
ifeq ($(osname), linux)
	cd $(rootdir) && cd third_party/tbb && make tbb_build_prefix=linux
endif
