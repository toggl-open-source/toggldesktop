
pwd=$(shell pwd)
builddir=build
pocodir=third_party/poco-1.4.6p1-all
openssldir=third_party/openssl-1.0.1e

uname=$(shell uname)
pocolib=$(pocodir)/lib/Darwin/x86_64/
ifeq ($(uname), Linux)
pocolib=$(pocodir)/lib/Linux/x86_64
endif

cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -O2 -DNDEBUG \
	-I$(openssldir)/include \
	-I$(pocodir)/Foundation/include \
	-I$(pocodir)/Util/include \
	-I$(pocodir)/Net/include \
	-I$(pocodir)/Crypto/include \
	-I$(pocodir)/NetSSL_OpenSSL/include
cxx=g++ $(cflags)
ldflags=-L$(builddir) -L$(pocolib) -L$(openssldir) -lPocoFoundation -lPocoUtil -lPocoNet -lPocoCrypto -lPocoNetSSL

default: builddir toggl_api_client.o libkopsik.a main.o kopsik

clean:
	rm -rf build

builddir:
	mkdir -p build

toggl_api_client.o:
	$(cxx) -c toggl_api_client.cc -o build/toggl_api_client.o

libkopsik.a: toggl_api_client.o
	rm -f $(builddir)/libkopsik.a && ar crs $(builddir)/libkopsik.a $(builddir)/toggl_api_client.o

main.o:
	$(cxx) -c main.cc -o build/main.o

kopsik:
	$(cxx) $(ldflags) -o kopsik -lkopsik $(builddir)/libkopsik.a $(builddir)/main.o

deps: openssl poco

openssl:
	cd $(openssldir) && ./config -fPIC && make

poco:
	cd $(pocodir) && \
	./configure --omit=Data/ODBC,Data/MySQL,Zip --no-tests --no-samples \
	--include-path=$(pwd)/$(openssldir)/include --library-path=$(pwd)/$(openssldir) && \
	make
