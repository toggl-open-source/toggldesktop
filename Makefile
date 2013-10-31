
pwd=$(shell pwd)
uname=$(shell uname)
timestamp=$(shell date "+%Y-%m-%d-%H-%M-%S")

pocodir=third_party/poco-1.4.6p1-all
openssldir=third_party/openssl-1.0.1e
GTEST_ROOT=third_party/googletest-read-only
GMOCK_DIR=third_party/gmock-1.7.0
jsondir=third_party/libjson

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
	-I$(jsondir)
endif

ifeq ($(uname), Linux)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -static \
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
	-I$(jsondir)
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

default: cmdline

clean:
	rm -rf build
	rm -f $(main)
	rm -f $(main)_test

osx:
	xcodebuild -project src/ui/osx/test2.project/kopsik_ui_osx.xcodeproj

osx_test: osx
	python src/ui/osx/test/guitest.py

run: osx
	./src/ui/osx/test2.project/build/Release/TogglDesktop.app/Contents/MacOS/TogglDesktop

cmdline: clean lint
	mkdir -p build
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/https_client.cc -o build/https_client.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/websocket_client.cc -o build/websocket_client.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/toggl_api_client.cc -o build/toggl_api_client.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/database.cc -o build/database.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/kopsik_api.cc -o build/kopsik_api.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/get_focused_window_$(osname).cc -o build/get_focused_window_$(osname).o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/timeline_uploader.cc -o build/timeline_uploader.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/window_change_recorder.cc -o build/window_change_recorder.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/ui/cmdline/main.cc -o build/main.o
	$(cxx) -o $(main) -o $(main) build/*.o $(libs)
	strip $(main)

test: clean lint
	mkdir -p build
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/https_client.cc -o build/https_client.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/websocket_client.cc -o build/websocket_client.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/toggl_api_client.cc -o build/toggl_api_client.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/database.cc -o build/database.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/kopsik_api.cc -o build/kopsik_api.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/kopsik_api_test.cc -o build/kopsik_api_test.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/kopsik_test.cc -o build/kopsik_test.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/get_focused_window_$(osname).cc -o build/get_focused_window_$(osname).o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/timeline_uploader.cc -o build/timeline_uploader.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c src/window_change_recorder.cc -o build/window_change_recorder.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c $(GTEST_ROOT)/src/gtest-all.cc -o build/gtest-all.o
	$(cxx) $(cflags) -O2 -DNDEBUG -c ${GMOCK_DIR}/src/gmock-all.cc -o build/gmock-all.o
	$(cxx) -o $(main) -o $(main)_test build/*.o $(libs)
	./$(main)_test

pull:
	./$(main) pull

push:
	./$(main) push

lint:
	./third_party/cpplint/cpplint.py src/*.cc src/*.h src/ui/cmdline/*

deps: openssl poco json

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

.phony:
	command_line_client
