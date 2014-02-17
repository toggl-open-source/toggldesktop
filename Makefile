
pwd=$(shell pwd)
uname=$(shell uname)
timestamp=$(shell date "+%Y-%m-%d-%H-%M-%S")

pocodir=third_party/poco-1.4.6p2-all
openssldir=third_party/openssl-1.0.1e
jsondir=third_party/libjson

GTEST_ROOT=third_party/googletest-read-only
GMOCK_DIR=third_party/gmock-1.7.0

osx_executable=./src/ui/osx/test2.project/build/Release/TogglDesktop.app/Contents/MacOS/TogglDesktop

oclintbin=./third_party/oclint-0.7-x86_64-apple-darwin-10/bin/oclint
oclintflags=-fatal-assembler-warnings -max-priority-3 0 -max-priority-2 0 -max-priority-1 0 \
	-- -c \
	-Isrc/ui/osx/test2.project \
	-Isrc \
	-Ithird_party/bugsnag-cocoa/bugsnag \
	-Isrc/ui/osx/test2.project/test2 \
	-Ithird_party/PLCrashReporter/Mac\ OS\ X\ Framework/CrashReporter.framework/Versions/Current/Headers \
	-I$(pocodir)/Foundation/include \
	-I$(pocodir)/Util/include \
	-I$(pocodir)/Data/include \
	-I$(pocodir)/Data/SQLite/include \
	-I$(pocodir)/Crypto/include \
	-I$(pocodir)/Net/include \
	-I$(pocodir)/NetSSL_OpenSSL/include \
	-I$(jsondir) \
	-DNDEBUG \
	-Werror

main=toggl

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
	-lrt \
	-ldl
endif

cxx=g++

cmdline: clean lint
	mkdir -p build
	$(cxx) $(cflags) -O2 -c src/version.cc -o build/version.o
	$(cxx) $(cflags) -O2 -c src/https_client.cc -o build/https_client.o
	$(cxx) $(cflags) -O2 -c src/websocket_client.cc -o build/websocket_client.o
	$(cxx) $(cflags) -O2 -c src/toggl_api_client.cc -o build/toggl_api_client.o
	$(cxx) $(cflags) -O2 -c src/database.cc -o build/database.o
	$(cxx) $(cflags) -O2 -c src/context.cc -o build/context.o
	$(cxx) $(cflags) -O2 -c src/kopsik_api_private.cc -o build/kopsik_api_private.o
	$(cxx) $(cflags) -O2 -c src/kopsik_api.cc -o build/kopsik_api.o
	$(cxx) $(cflags) -O2 -c src/tasks.cc -o build/tasks.o
	$(cxx) $(cflags) -O2 -c src/get_focused_window_$(osname).cc -o build/get_focused_window_$(osname).o
	$(cxx) $(cflags) -O2 -c src/timeline_uploader.cc -o build/timeline_uploader.o
	$(cxx) $(cflags) -O2 -c src/window_change_recorder.cc -o build/window_change_recorder.o
	$(cxx) $(cflags) -O2 -c src/ui/cmdline/main.cc -o build/main.o
	$(cxx) -o $(main) -o $(main) build/*.o $(libs)
	strip $(main)

clean:
	rm -rf build && \
	rm -f $(main) && \
	rm -f $(main)_test && \
	rm -rf src/ui/osx/test2.project/build && \
	rm -rf src/libkopsik/Kopsik/build && \
	rm -f TogglDesktop.dmg

osx:
	xcodebuild -project src/ui/osx/test2.project/kopsik_ui_osx.xcodeproj && \
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

test: clean lint
	mkdir -p build
	$(cxx) $(cflags) -c src/version.cc -o build/version.o
	$(cxx) $(cflags) -c src/https_client.cc -o build/https_client.o
	$(cxx) $(cflags) -c src/websocket_client.cc -o build/websocket_client.o
	$(cxx) $(cflags) -c src/toggl_api_client.cc -o build/toggl_api_client.o
	$(cxx) $(cflags) -c src/database.cc -o build/database.o
	$(cxx) $(cflags) -c src/context.cc -o build/context.o
	$(cxx) $(cflags) -c src/kopsik_api_private.cc -o build/kopsik_api_private.o
	$(cxx) $(cflags) -c src/kopsik_api.cc -o build/kopsik_api.o
	$(cxx) $(cflags) -c src/tasks.cc -o build/tasks.o
	$(cxx) $(cflags) -c src/test_data.cc -o build/test_data.o
	$(cxx) $(cflags) -c src/kopsik_api_test.cc -o build/kopsik_api_test.o
	$(cxx) $(cflags) -c src/toggl_api_client_test.cc -o build/toggl_api_client_test.o
	$(cxx) $(cflags) -c src/get_focused_window_$(osname).cc -o build/get_focused_window_$(osname).o
	$(cxx) $(cflags) -c src/timeline_uploader.cc -o build/timeline_uploader.o
	$(cxx) $(cflags) -c src/window_change_recorder.cc -o build/window_change_recorder.o
	$(cxx) $(cflags) -c $(GTEST_ROOT)/src/gtest-all.cc -o build/gtest-all.o
	$(cxx) $(cflags) -c ${GMOCK_DIR}/src/gmock-all.cc -o build/gmock-all.o
	$(cxx) -o $(main) -o $(main)_test build/*.o $(libs)
	./$(main)_test

covflags=-fprofile-arcs -ftest-coverage

coverage: clean
	mkdir -p build
	$(cxx) $(cflags) $(covflags) -c src/version.cc -o build/version.o
	$(cxx) $(cflags) $(covflags) -c src/https_client.cc -o build/https_client.o
	$(cxx) $(cflags) $(covflags) -c src/websocket_client.cc -o build/websocket_client.o
	$(cxx) $(cflags) $(coverage) -c src/toggl_api_client.cc -o build/toggl_api_client.o
	$(cxx) $(cflags) $(covflags) -c src/database.cc -o build/database.o
	$(cxx) $(cflags) $(covflags) -c src/context.cc -o build/context.o
	$(cxx) $(cflags) $(covflags) -c src/kopsik_api_private.cc -o build/kopsik_api_private.o
	$(cxx) $(cflags) $(covflags) -c src/kopsik_api.cc -o build/kopsik_api.o
	$(cxx) $(cflags) $(covflags) -c src/tasks.cc -o build/tasks.o
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
	mkdir -p coverage && genhtml build/coverage.info --output-directory coverage

pull:
	./$(main) pull

push:
	./$(main) push

lint:
	./third_party/cpplint/cpplint.py src/*.cc src/*.h src/ui/cmdline/*

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

oclint:
	$(oclintbin) src/ui/osx/test2.project/test2/*.m src/kopsik_api.cc \
	src/toggl_api_client.cc src/database.cc $(oclintflags)
