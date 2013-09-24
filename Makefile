
pwd=$(shell pwd)
uname=$(shell uname)

pocodir=third_party/poco-1.4.6p1-all
openssldir=third_party/openssl-1.0.1e
GTEST_ROOT=third_party/googletest-read-only
jsondir=third_party/libjson

main=toggl

ifeq ($(uname), Darwin)
pocolib=$(pocodir)/lib/Darwin/x86_64/
endif

ifeq ($(uname), Linux)
pocolib=$(pocodir)/lib/Linux/x86_64
endif

ifeq ($(uname), Darwin)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter \
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
	-I$(jsondir)
endif

ifeq ($(uname), Linux)
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -static \
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
	-I$(jsondir)
endif

ifeq ($(uname), Darwin)
libs=-L$(pocolib) \
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
libs=-L$(pocolib) \
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

srcs=src/toggl_api_client.h src/toggl_api_client.cc src/database.h src/database.cc src/kopsik_api.h src/kopsik_api.cc src/main.h src/main.cc
objs=$(srcs:.c=.o)

test_srcs=src/toggl_api_client.h src/toggl_api_client.cc src/database.h src/database.cc $(GTEST_ROOT)/src/gtest-all.cc src/kopsik_test.h src/kopsik_test.cc
test_objs=$(test_srcs:.c=.o)

default: lint command_line_client

clean:
	rm -f $(main)
	rm -f $(main)_test

command_line_client:
	$(cxx) $(cflags) -O2 -DNDEBUG -o $(main) $(objs) $(libs) && strip $(main)

test: lint
	$(cxx) $(cflags) -DNDEBUG -o $(main)_test $(test_objs) $(libs)
	./$(main)_test

pull:
	./$(main) pull

push:
	./$(main) push

lint:
	./third_party/cpplint/cpplint.py src/*

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

.phony:
	kopsik

