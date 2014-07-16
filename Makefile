$(shell mkdir -p build/test build/test)

pwd=$(shell pwd)
uname=$(shell uname)
architecture=$(shell uname -m)
timestamp=$(shell date "+%Y-%m-%d-%H-%M-%S")

pocodir=third_party/poco
openssldir=third_party/openssl
jsondir=third_party/libjson

GTEST_ROOT=third_party/googletest-read-only
GMOCK_DIR=third_party/gmock-1.7.0

source_dirs=src/*.cc src/*.h src/test/* src/libkopsik/include/*.h \
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
	src/ui/linux/TogglDesktop/updateview.h src/ui/linux/TogglDesktop/updateview.cpp

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
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter \
	-I$(openssldir)/include \
	-I$(GTEST_ROOT)/include \
	-I$(GTEST_ROOT) \
	-I$(GMOCK_DIR)/include \
	-I$(GMOCK_DIR) \
	-I$(pocodir)/Foundation/include \
	-I$(pocodir)/Util/include \
	-I$(pocodir)/Data/include \
	-I$(pocodir)/Data/SQLite/include \
	-I$(pocodir)/Crypto/include \
	-I$(pocodir)/Net/include \
	-I$(pocodir)/NetSSL_OpenSSL/include \
	-I$(jsondir) \
	-DNDEBUG
endif

ifeq ($(uname), Linux)
cflags=-g -DNDEBUG -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -static \
	-I$(openssldir)/include \
	-I$(GTEST_ROOT)/include \
	-I$(GTEST_ROOT) \
	-I$(GMOCK_DIR)/include \
	-I$(GMOCK_DIR) \
	-I$(pocodir)/Foundation/include \
	-I$(pocodir)/Util/include \
	-I$(pocodir)/Data/include \
	-I$(pocodir)/Data/SQLite/src \
	-I$(pocodir)/Data/SQLite/include \
	-I$(pocodir)/Crypto/include \
	-I$(pocodir)/Net/include \
	-I$(pocodir)/NetSSL_OpenSSL/include \
	-I$(jsondir) \
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
	-lPocoFoundation \
	-L$(jsondir) \
	-ljson \
	-lpthread \
	-L$(openssldir) \
	-lssl \
	-lcrypto \
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
	-lPocoFoundation \
	-L$(jsondir) \
	-ljson \
	-lpthread \
	-L$(openssldir) \
	-lssl \
	-lcrypto \
	-lrt \
	-ldl
endif

cxx=g++

ifeq ($(uname), Linux)
default: linux
endif
ifeq ($(uname), Darwin)
default: osx
endif

clean: clean_ui clean_lib clean_test
	rm -rf build gitstats

ifeq ($(uname), Linux)
clean_lib:
	rm -rf src/libkopsik/Library/TogglDesktopLibrary/build && \
	(cd src/libkopsik/Library/TogglDesktopLibrary && $(QMAKE) && make clean)
endif
ifeq ($(uname), Darwin)
clean_lib:
	rm -rf src/libkopsik/Kopsik/build
endif

ifeq ($(uname), Linux)
clean_ui:
	(cd third_party/bugsnag-qt && $(QMAKE) && make clean) && \
	rm -rf third_party/bugsnag-qt/build && \
	(cd src/ui/linux/TogglDesktop && $(QMAKE) && make clean) && \
	rm -rf src/ui/linux/TogglDesktop/build
endif
ifeq ($(uname), Darwin)
clean_ui:
	rm -rf src/ui/osx/TogglDesktop/build && \
	rm -rf third_party/TFDatePicker/TFDatePicker/build && \
	rm -rf TogglDesktop*.dmg TogglDesktop*.tar.gz
endif

clean_test:
	rm -rf test

osx: fmt_lib lint fmt_osx osx_ui
	!(otool -L $(executable) | grep "Users" && echo "Executable should not contain hardcoded paths!") && \
	src/ui/osx/TogglDesktop/fix_dylib_paths.sh && \
	!(otool -L ./src/ui/osx/TogglDesktop/build/Release/TogglDesktop.app/Contents/Frameworks/*.dylib | grep "Users" && echo "Shared library should not contain hardcoded paths!")

osx_ui:
	xcodebuild -project src/ui/osx/TogglDesktop/TogglDesktop.xcodeproj

linux: fmt_lib lint linux_lib linux_ui

linux_lib:
	cd src/libkopsik/Library/TogglDesktopLibrary && $(QMAKE) && make && \
	cd ../../../../ && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoCrypto.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoData.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoDataSQLite.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoFoundation.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoNet.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoNetSSL.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoUtil.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release && \
	cp $(pocodir)/lib/Linux/$(architecture)/libPocoXML.so.16 src/libkopsik/Library/TogglDesktopLibrary/build/release && \
	cp $(jsondir)/libjson.so.7.6.1 src/libkopsik/Library/TogglDesktopLibrary/build/release/libjson.so.7

linux_ui:
	cd third_party/bugsnag-qt && $(QMAKE) && make && \
	cd ../../ && \
	cd src/ui/linux/TogglDesktop && $(QMAKE) && make && \
	cd ../../../../ && \
	cp src/ssl/cacert.pem src/ui/linux/TogglDesktop/build/release

ifeq ($(uname), Linux)
run: linux
	$(executable)
endif
ifeq ($(uname), Darwin)
run: osx
	$(executable)
endif

sikuli: osx
	(pkill TogglDesktop) || true
	rm -rf kopsik_sikuli.db
	rm -rf kopsik_sikuli.log
	$(executable) \
	--api_url http://0.0.0.0:8080 \
	--websocket_url http://0.0.0.0:8088 \
	--db_path kopsik_sikuli.db \
	--log_path kopsik_sikuli.log 

lint:
	./third_party/cpplint/cpplint.py $(source_dirs)

clean_deps:
	cd $(jsondir) && make clean && SHARED=1 make clean
	cd $(pocodir) && (make clean || true)
	rm -rf $(pocodir)/**/.dep
	cd $(openssldir) && (make clean || true)

deps: clean_deps openssl poco json

ifeq ($(uname), Linux)
json:
	cd $(jsondir) && \
	SHARED=1 make && \
	ln -sf libjson.so.7.6.1 libjson.so && \
	ln -sf libjson.so.7.6.1 libjson.so.7
endif
ifeq ($(uname), Darwin)
json:
	cd $(jsondir) && \
	SHARED=0 make
endif

openssl:
ifeq ($(uname), Darwin)
	cd $(openssldir) && ./config -fPIC no-shared no-dso && ./Configure darwin64-x86_64-cc && make
endif
ifeq ($(uname), Linux)
	cd $(openssldir) && ./config -fPIC no-shared no-dso && make
endif

poco:
	cd $(pocodir) && \
	./configure --omit=Data/ODBC,Data/MySQL,Zip --no-tests --no-samples --fPIC \
	--include-path=$(pwd)/$(openssldir)/include --library-path=$(pwd)/$(openssldir) && \
	make

stats:
	rm -rf gitstats
	./third_party/gitstats/gitstats -c merge_authors="Tanel","Tanel Lebedev" . gitstats

simian:
	java -jar third_party/simian/bin/simian-2.3.35.jar src/*

third_party/google-astyle/build/google-astyle:
	cd third_party/google-astyle && mkdir -p build && g++ *.cpp -o build/google-astyle

fmt_lib: third_party/google-astyle/build/google-astyle
	third_party/google-astyle/build/google-astyle -n $(source_dirs)

fmt_osx:
	./third_party/Xcode-formatter/CodeFormatter/scripts/formatAllSources.sh src/ui/osx/

mkdir_build:
	mkdir -p build

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

build/json.o: src/json.cc
	$(cxx) $(cflags) -c src/json.cc -o build/json.o

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

build/context.o: src/context.cc
	$(cxx) $(cflags) -c src/context.cc -o build/context.o

build/kopsik_api_private.o: src/kopsik_api_private.cc
	$(cxx) $(cflags) -c src/kopsik_api_private.cc -o build/kopsik_api_private.o

build/kopsik_api.o: src/kopsik_api.cc
	$(cxx) $(cflags) -c src/kopsik_api.cc -o build/kopsik_api.o

build/test/test_data.o: src/test/test_data.cc
	$(cxx) $(cflags) -c src/test/test_data.cc -o build/test/test_data.o

build/test/kopsik_api_test.o: src/test/kopsik_api_test.cc
	$(cxx) $(cflags) -c src/test/kopsik_api_test.cc -o build/test/kopsik_api_test.o

build/test/toggl_api_client_test.o: src/test/toggl_api_client_test.cc
	$(cxx) $(cflags) -c src/test/toggl_api_client_test.cc -o build/test/toggl_api_client_test.o

build/get_focused_window_$(osname).o: src/get_focused_window_$(osname).cc
	$(cxx) $(cflags) -c src/get_focused_window_$(osname).cc -o build/get_focused_window_$(osname).o

build/timeline_uploader.o: src/timeline_uploader.cc
	$(cxx) $(cflags) -c src/timeline_uploader.cc -o build/timeline_uploader.o

build/window_change_recorder.o: src/window_change_recorder.cc
	$(cxx) $(cflags) -c src/window_change_recorder.cc -o build/window_change_recorder.o

build/test/gtest-all.o: $(GTEST_ROOT)/src/gtest-all.cc
	$(cxx) $(cflags) -c $(GTEST_ROOT)/src/gtest-all.cc -o build/test/gtest-all.o

build/test/gmock-all.o: ${GMOCK_DIR}/src/gmock-all.cc
	$(cxx) $(cflags) -c ${GMOCK_DIR}/src/gmock-all.cc -o build/test/gmock-all.o

objects: build/proxy.o \
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
	build/json.o \
	build/model_change.o \
	build/database.o \
	build/autocomplete_item.o \
	build/feedback.o \
	build/gui.o \
	build/context.o \
	build/kopsik_api_private.o \
	build/kopsik_api.o \
	build/get_focused_window_$(osname).o \
	build/timeline_uploader.o \
	build/window_change_recorder.o

test_objects: build/test/gtest-all.o \
	build/test/gmock-all.o \
	build/test/test_data.o \
	build/test/toggl_api_client_test.o \
	build/test/kopsik_api_test.o

toggl_test: objects test_objects
	rm -rf test
	mkdir -p test
	$(cxx) -o test/toggl_test build/*.o build/test/*.o $(libs)

test_lib: fmt_lib lint mkdir_build toggl_test
ifeq ($(uname), Linux)
	cp -r $(jsondir)/libjson.so* test/.
	cp -r $(pocodir)/lib/Linux/$(architecture)/*.so* test/.
	cd test && LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./toggl_test
else
	cd test && ./toggl_test
endif

test: test_lib
