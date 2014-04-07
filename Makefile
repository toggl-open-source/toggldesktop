$(shell mkdir -p build/ui/cmdline build/test coverage build/test)

pwd=$(shell pwd)
uname=$(shell uname)
timestamp=$(shell date "+%Y-%m-%d-%H-%M-%S")

pocodir=third_party/poco-1.4.6p2-all
openssldir=third_party/openssl-1.0.1e
jsondir=third_party/libjson

GTEST_ROOT=third_party/googletest-read-only
GMOCK_DIR=third_party/gmock-1.7.0

osx_executable=./src/ui/osx/test2.project/build/Release/TogglDesktop.app/Contents/MacOS/TogglDesktop

source_dirs=src/*.cc src/*.h src/test/* src/ui/cmdline/* 

ifeq ($(uname), Darwin)
pocolib=$(pocodir)/lib/Darwin/x86_64/
osname=mac
endif

ifeq ($(uname), Linux)
pocolib=$(pocodir)/lib/Linux/x86_64
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
	-ldl
endif

cxx=g++

default: cmdline

build/ui/cmdline/main.o: src/ui/cmdline/main.cc
	$(cxx) $(cflags) -O2 -c src/ui/cmdline/main.cc -o build/ui/cmdline/main.o

cmdline: fmt lint objects build/ui/cmdline/main.o toggl

toggl:
	$(cxx) -o toggl build/*.o build/ui/cmdline/main.o $(libs)

clean:
	rm -rf build gitstats && \
	rm -rf src/ui/osx/test2.project/build && \
	rm -rf src/libkopsik/Kopsik/build && \
	rm -rf third_party/TFDatePicker/TFDatePicker/build && \
	rm -f toggl toggl_test TogglDesktop*.dmg TogglDesktop*.tar.gz

osx:
	xcodebuild -project src/ui/osx/test2.project/TogglDesktop.xcodeproj && \
	!(otool -L $(osx_executable) | grep "Users" && echo "Executable should not contain hardcoded paths!")

run: osx
	$(osx_executable)

sikuli: osx
	(pkill TogglDesktop) || true
	rm -rf kopsik_sikuli.db
	rm -rf kopsik_sikuli.log
	$(osx_executable) \
	--api_url http://0.0.0.0:8080 \
	--websocket_url http://0.0.0.0:8088 \
	--db_path kopsik_sikuli.db \
	--log_path kopsik_sikuli.log 

covflags=-fprofile-arcs -ftest-coverage

coverage: clean
	$(cxx) $(cflags) $(covflags) -c src/version.cc -o build/version.o
	$(cxx) $(cflags) $(covflags) -c src/proxy.cc -o build/proxy.o
	$(cxx) $(cflags) $(covflags) -c src/https_client.cc -o build/https_client.o
	$(cxx) $(cflags) $(covflags) -c src/websocket_client.cc -o build/websocket_client.o
	$(cxx) $(cflags) $(covflags) -c src/base_model.cc -o build/base_model.o
	$(cxx) $(cflags) $(covflags) -c src/user.cc -o build/user.o
	$(cxx) $(cflags) $(covflags) -c src/workspace.cc -o build/workspace.o
	$(cxx) $(cflags) $(covflags) -c src/client.cc -o build/client.o
	$(cxx) $(cflags) $(covflags) -c src/project.cc -o build/project.o
	$(cxx) $(cflags) $(covflags) -c src/task.cc -o build/task.o
	$(cxx) $(cflags) $(covflags) -c src/time_entry.cc -o build/time_entry.o
	$(cxx) $(cflags) $(covflags) -c src/tag.cc -o build/tag.o
	$(cxx) $(cflags) $(covflags) -c src/related_data.cc -o build/related_data.o
	$(cxx) $(cflags) $(covflags) -c src/batch_update_result.cc -o build/batch_update_result.o
	$(cxx) $(cflags) $(covflags) -c src/formatter.cc -o build/formatter.o
	$(cxx) $(cflags) $(covflags) -c src/json.cc -o build/json.o
	$(cxx) $(cflags) $(covflags) -c src/model_change.cc -o build/model_change.o
	$(cxx) $(cflags) $(covflags) -c src/database.cc -o build/database.o
	$(cxx) $(cflags) $(covflags) -c src/autocomplete_item.cc -o build/autocomplete_item.o
	$(cxx) $(cflags) $(covflags) -c src/feedback.cc -o build/feedback.o
	$(cxx) $(cflags) $(covflags) -c src/context.cc -o build/context.o
	$(cxx) $(cflags) $(covflags) -c src/kopsik_api_private.cc -o build/kopsik_api_private.o
	$(cxx) $(cflags) $(covflags) -c src/kopsik_api.cc -o build/kopsik_api.o
	$(cxx) $(cflags) $(covflags) -c src/test_data.cc -o build/test_data.o
	$(cxx) $(cflags) $(covflags) -c src/kopsik_api_test.cc -o build/kopsik_api_test.o
	$(cxx) $(cflags) $(covflags) -c src/toggl_api_client_test.cc -o build/toggl_api_client_test.o
	$(cxx) $(cflags) $(covflags) -c src/get_focused_window_$(osname).cc -o build/get_focused_window_$(osname).o
	$(cxx) $(cflags) $(covflags) -c src/timeline_uploader.cc -o build/timeline_uploader.o
	$(cxx) $(cflags) $(covflags) -c src/window_change_recorder.cc -o build/window_change_recorder.o
	$(cxx) $(cflags) $(covflags) -c $(GTEST_ROOT)/src/gtest-all.cc -o build/gtest-all.o
	$(cxx) $(cflags) $(covflags) -c ${GMOCK_DIR}/src/gmock-all.cc -o build/gmock-all.o
	$(cxx) -o $(main) -o $(main)_test build/*.o $(libs) $(covflags)
	./$(main)_test && lcov --capture --directory build --output-file build/coverage.info && \
	genhtml build/coverage.info --output-directory coverage

lint:
	./third_party/cpplint/cpplint.py $(source_dirs)

clean_deps:
	cd third_party/libjson && make clean

deps: clean_deps openssl poco json

json:
	cd $(jsondir) && make

openssl:
ifeq ($(uname), Darwin)
	cd $(openssldir) && ./config -fPIC no-shared no-dso && ./Configure darwin64-x86_64-cc && make
endif
ifeq ($(uname), Linux)
	cd $(openssldir) && ./config -fPIC no-shared no-dso && make
endif

poco:
	cd $(pocodir) && \
	./configure --omit=Data/ODBC,Data/MySQL,Zip --no-tests --no-samples --static \
	--include-path=$(pwd)/$(openssldir)/include --library-path=$(pwd)/$(openssldir) && \
	make

stats:
	rm -rf gitstats
	./third_party/gitstats/gitstats -c merge_authors="Tanel","Tanel Lebedev" . gitstats

simian:
	java -jar third_party/simian/bin/simian-2.3.35.jar src/*

third_party/google-astyle/build/google-astyle:
	cd third_party/google-astyle && mkdir -p build && g++ *.cpp -o build/google-astyle

fmt: third_party/google-astyle/build/google-astyle
	third_party/google-astyle/build/google-astyle -n $(source_dirs)


mkdir_build:
	mkdir -p build

build/version.o: src/version.cc
	$(cxx) $(cflags) -c src/version.cc -o build/version.o

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

objects: build/version.o \
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
	build/json.o \
	build/model_change.o \
	build/database.o \
	build/autocomplete_item.o \
	build/feedback.o \
	build/context.o \
	build/kopsik_api_private.o \
	build/kopsik_api.o \
	build/get_focused_window_$(osname).o \
	build/timeline_uploader.o \
	build/window_change_recorder.o

toggl_test: objects \
	build/test/gtest-all.o \
	build/test/gmock-all.o \
	build/test/test_data.o \
	build/test/toggl_api_client_test.o \
	build/test/kopsik_api_test.o
	$(cxx) -o toggl_test build/*.o build/test/*.o $(libs)

test: fmt lint mkdir_build toggl_test
	./toggl_test

