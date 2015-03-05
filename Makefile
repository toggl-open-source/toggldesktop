$(shell mkdir -p build/test build/test src/lib/osx/build)

pwd=$(shell pwd)
uname=$(shell uname)
architecture=$(shell uname -m)
timestamp=$(shell date "+%Y-%m-%d-%H-%M-%S")

pocodir=third_party/poco
openssldir=third_party/openssl
jsoncppdir=third_party/jsoncpp/dist

GTEST_ROOT=third_party/googletest-read-only

source_dirs=src/*.cc src/*.h src/test/*.cc src/test/*.h \
	src/ui/linux/TogglDesktop/toggl.h src/ui/linux/TogglDesktop/toggl.cpp \
	src/ui/linux/TogglDesktop/aboutdialog.h src/ui/linux/TogglDesktop/aboutdialog.cpp \
	src/ui/linux/TogglDesktop/autocompleteview.h src/ui/linux/TogglDesktop/autocompleteview.cpp \
	src/ui/linux/TogglDesktop/errorviewcontroller.h src/ui/linux/TogglDesktop/errorviewcontroller.cpp \
	src/ui/linux/TogglDesktop/feedbackdialog.h src/ui/linux/TogglDesktop/feedbackdialog.cpp \
	src/ui/linux/TogglDesktop/genericview.h src/ui/linux/TogglDesktop/genericview.cpp \
	src/ui/linux/TogglDesktop/idlenotificationdialog.h src/ui/linux/TogglDesktop/idlenotificationdialog.cpp \
	src/ui/linux/TogglDesktop/loginwidget.h src/ui/linux/TogglDesktop/loginwidget.cpp \
	src/ui/linux/TogglDesktop/main.cpp \
	src/ui/linux/TogglDesktop/mainwindowcontroller.h src/ui/linux/TogglDesktop/mainwindowcontroller.cpp \
	src/ui/linux/TogglDesktop/preferencesdialog.h src/ui/linux/TogglDesktop/preferencesdialog.cpp \
	src/ui/linux/TogglDesktop/settingsview.h src/ui/linux/TogglDesktop/settingsview.cpp \
	src/ui/linux/TogglDesktop/timeentrycellwidget.h src/ui/linux/TogglDesktop/timeentrycellwidget.cpp \
	src/ui/linux/TogglDesktop/timeentryeditorwidget.h src/ui/linux/TogglDesktop/timeentryeditorwidget.cpp \
	src/ui/linux/TogglDesktop/timeentryview.h src/ui/linux/TogglDesktop/timeentryview.cpp \
	src/ui/linux/TogglDesktop/timerwidget.h src/ui/linux/TogglDesktop/timerwidget.cpp \
	src/ui/linux/TogglDesktop/clickablelabel.h src/ui/linux/TogglDesktop/clickablelabel.cpp

ifndef QMAKE
QMAKE=qmake
endif

ifeq ($(uname), Darwin)
executable=./src/ui/osx/TogglDesktop/build/Release/TogglDesktop.app/Contents/MacOS/TogglDesktop
pocolib=$(pocodir)/lib/Darwin/x86_64/
osname=mac
endif

ifeq ($(uname), Linux)
executable=./src/ui/linux/TogglDesktop/build/release/TogglDesktop
pocolib=$(pocodir)/lib/Linux/$(architecture)
osname=linux
endif

ifeq ($(uname), Darwin)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -Wunreachable-code -DLUA_USE_MACOSX \
	-I$(openssldir)/include \
	-I$(GTEST_ROOT)/include \
	-I$(GTEST_ROOT) \
	-I$(pocodir)/Foundation/include \
	-I$(pocodir)/Util/include \
	-I$(pocodir)/Data/include \
	-I$(pocodir)/Data/SQLite/include \
	-I$(pocodir)/Crypto/include \
	-I$(pocodir)/Net/include \
	-I$(pocodir)/NetSSL_OpenSSL/include \
	-I$(jsoncppdir) \
	-Ithird_party/lua/install/include \
	-DNDEBUG
endif

ifeq ($(uname), Linux)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -static \
	-I$(openssldir)/include \
	-I$(GTEST_ROOT)/include \
	-I$(GTEST_ROOT) \
	-I$(pocodir)/Foundation/include \
	-I$(pocodir)/Util/include \
	-I$(pocodir)/Data/include \
	-I$(pocodir)/Data/SQLite/src \
	-I$(pocodir)/Data/SQLite/include \
	-I$(pocodir)/Crypto/include \
	-I$(pocodir)/Net/include \
	-I$(pocodir)/NetSSL_OpenSSL/include \
	-I$(jsoncppdir) \
	-Ithird_party/lua/install/include \
	-DNDEBUG
endif

ifeq ($(uname), Darwin)
libs=-framework Carbon \
	-L$(pocolib) \
	-lPocoDataSQLite \
	-lPocoData \
	-lPocoNet \
	-lPocoNetSSL \
	-lPocoCrypto \
	-lPocoUtil \
	-lPocoXML \
	-lPocoJSON \
	-lPocoFoundation \
	-lpthread \
	-L$(openssldir) \
	-lssl \
	-lcrypto \
        -Lthird_party/lua/install/lib \
	-llua \
	-ldl
endif

ifeq ($(uname), Linux)
libs=-lX11 \
	-L$(pocolib) \
	-lPocoDataSQLite \
	-lPocoData \
	-lPocoNet \
	-lPocoNetSSL \
	-lPocoCrypto \
	-lPocoUtil \
	-lPocoXML \
	-lPocoJSON \
	-lPocoFoundation \
	-lpthread \
	-L$(openssldir) \
	-lssl \
	-lcrypto \
	-lrt \
        -Lthird_party/lua/install/lib \
	-llua \
	-ldl
endif

cxx=g++ -fprofile-arcs -ftest-coverage

default: app

clean: clean_ui clean_lib clean_test
	rm -rf build coverage

ifeq ($(uname), Linux)
clean_lib:
	rm -rf src/lib/linux/TogglDesktopLibrary/build && \
	(cd src/lib/linux/TogglDesktopLibrary && $(QMAKE) && make clean)
endif
ifeq ($(uname), Darwin)
clean_lib:
	rm -rf src/lib/osx/build
endif

ifeq ($(uname), Linux)
clean_ui: clean-bugsnag-qt
	(cd src/ui/linux/TogglDesktop && $(QMAKE) && make clean) && \
	rm -rf src/ui/linux/TogglDesktop/build
endif
ifeq ($(uname), Darwin)
clean_ui:
	rm -rf src/ui/osx/TogglDesktop/build third_party/TFDatePicker/TFDatePicker/build TogglDesktop*.dmg TogglDesktop*.tar.gz
endif

clean_test:
	rm -rf test/*

lint:
	./third_party/cpplint/cpplint.py $(source_dirs)

cppclean:
	./third_party/cppclean/cppclean -q --include-path=src/lib/include --include-path=third_party/poco/Crypto/include --include-path=third_party/poco/Net/include --include-path=third_party/poco/NetSSL_OpenSSL/include --include-path=third_party/poco/Foundation/include --include-path=third_party/poco/Util/include --include-path=third_party/poco/Data/include --include-path=third_party/poco/Data/SQLite/include --include-path=third_party/poco/Data/SQLite/src --include-path=third_party/lua/install/include src/*.*

qa: lint fmt cppclean test

fmt: fmt_lib fmt_ui

app: lib ui

ifeq ($(uname), Darwin)
lib:
	xcodebuild -project src/lib/osx/TogglDesktopLibrary.xcodeproj && \
	!(otool -L ./src/ui/osx/TogglDesktop/build/Release/TogglDesktop.app/Contents/Frameworks/*.dylib | grep "Users" && echo "Shared library should not contain hardcoded paths!")
endif
ifeq ($(uname), Linux)
lib:
	cd src/lib/linux/TogglDesktopLibrary && $(QMAKE) && make && \
	cd ../../../../ && \
	cp $(openssldir)/*so* src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoCrypto.so.30 src/lib/linux/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoData.so.30 src/lib/linux/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoDataSQLite.so.30 src/lib/linux/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoFoundation.so.30 src/lib/linux/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoNet.so.30 src/lib/linux/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoNetSSL.so.30 src/lib/linux/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoUtil.so.30 src/lib/linux/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoXML.so.30 src/lib/linux/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoJSON.so.30 src/lib/linux/TogglDesktopLibrary/build/release
endif


ifeq ($(uname), Darwin)
ui:
	xcodebuild -project src/ui/osx/TogglDesktop/TogglDesktop.xcodeproj -sdk macosx10.9 && \
	!(otool -L $(executable) | grep "Users" && echo "Executable should not contain hardcoded paths!")
endif
ifeq ($(uname), Linux)
ui: bugsnag-qt
	cd src/ui/linux/TogglDesktop && $(QMAKE) && make && \
	cd ../../../../ && \
	cp src/ssl/cacert.pem src/ui/linux/TogglDesktop/build/release
endif

clean-bugsnag-qt:
	rm -rf third_party/bugsnag-qt/build && \
	cd third_party/bugsnag-qt && $(QMAKE) && make clean

bugsnag-qt: clean-bugsnag-qt
	cd third_party/bugsnag-qt && $(QMAKE) && make

run: app
	$(executable)

sikuli: osx
	(pkill TogglDesktop) || true
	rm -rf kopsik_sikuli.db kopsik_sikuli.log
	$(executable) \
	--api_url http://0.0.0.0:8080 \
	--websocket_url http://0.0.0.0:8088 \
	--db_path kopsik_sikuli.db \
	--log_path kopsik_sikuli.log 

clean_deps:
	cd $(pocodir) && (make clean || true)
	rm -rf $(pocodir)/**/.dep
	cd $(openssldir) && (make clean || true)
	cd third_party/lua && make clean

deps: clean_deps openssl poco lua

ifeq ($(uname), Linux)
lua:
	cd third_party/lua && make linux && make local
endif
ifeq ($(uname), Darwin)
lua:
	cd third_party/lua && make macosx && make local
endif

openssl:
ifeq ($(uname), Darwin)
	cd $(openssldir) && ./config -fPIC no-shared no-dso && ./Configure darwin64-x86_64-cc && make
endif
ifeq ($(uname), Linux)
	cd $(openssldir) && ./config -fPIC shared no-dso && make
endif

poco:
	cd $(pocodir) && \
	./configure --omit=Data/ODBC,Data/MySQL,Zip,JSON,MongoDB,PageCompiler,PageCompiler/File2Page --no-tests --no-samples --cflags=-fPIC \
	--sqlite-thread-safe=1 \
	--include-path=$(pwd)/$(openssldir)/include --library-path=$(pwd)/$(openssldir) && \
	make

third_party/google-astyle/build/google-astyle:
	cd third_party/google-astyle && mkdir -p build && g++ *.cpp -o build/google-astyle

fmt_lib: third_party/google-astyle/build/google-astyle
	third_party/google-astyle/build/google-astyle -n $(source_dirs)

fmt_ui:
	./third_party/Xcode-formatter/CodeFormatter/scripts/formatAllSources.sh src/ui/osx/

build/jsoncpp.o: $(jsoncppdir)/jsoncpp.cpp
	$(cxx) $(cflags) -c $(jsoncppdir)/jsoncpp.cpp -o build/jsoncpp.o

build/proxy.o: src/proxy.cc
	$(cxx) $(cflags) -c src/proxy.cc -o build/proxy.o

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

build/database.o: src/database.cc
	$(cxx) $(cflags) -c src/database.cc -o build/database.o

build/autocomplete_item.o: src/autocomplete_item.cc
	$(cxx) $(cflags) -c src/autocomplete_item.cc -o build/autocomplete_item.o

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

build/context.o: src/context.cc
	$(cxx) $(cflags) -c src/context.cc -o build/context.o

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
	build/proxy.o \
	build/https_client.o \
	build/websocket_client.o \
	build/base_model.o \
	build/user.o \
	build/workspace.o \
	build/client.o \
	build/project.o \
	build/task.o \
	build/time_entry.o \
	build/tag.o \
	build/related_data.o \
	build/batch_update_result.o \
	build/formatter.o \
	build/model_change.o \
	build/database.o \
	build/autocomplete_item.o \
	build/feedback.o \
	build/error.o \
	build/gui.o \
	build/idle.o \
	build/analytics.o \
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

uitest:
	rm -rf test
	mkdir -p test
	$(executable) \
		--script-path $(pwd)/src/test/uitest.lua \
		--log-path $(pwd)/test/uitest.log \
		--db-path $(pwd)/test/uitest.db

toggl_test: clean_test objects test_objects
	mkdir -p test
	$(cxx) -coverage -o test/toggl_test build/*.o build/test/*.o $(libs)

test_lib: lua toggl_test
ifeq ($(uname), Linux)
	cp src/ssl/cacert.pem test/.
	cp -r $(pocodir)/lib/Linux/$(architecture)/*.so* test/.
	cp -r $(openssldir)/*so* test/.
	cd test && LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./toggl_test --gtest_shuffle
else
	cd test && ./toggl_test
endif

test: test_lib

lcov: test
	lcov -q -d . -c -o app.info
	genhtml -q -o coverage app.info

coverage: lcov

loco:
ifeq ($(uname), Darwin)
	xcodebuild -exportLocalizations -localizationPath src/ui/osx/localization -project src/ui/osx/TogglDesktop/TogglDesktop.xcodeproj -exportLanguage et
endif
