builddir=build
pocolib=third_party/poco-1.4.6p1-all
cflags=-g -Wall -Wextra -Wno-deprecated -Wno-unused-parameter -O2 -DNDEBUG \
    -I$(pocolib)/Foundation/include \
    -I$(pocolib)/Util/include \
    -I$(pocolib)/Net/include \
    -I$(pocolib)/Crypto/include \
    -I$(pocolib)/NetSSL_OpenSSL/include
cxx=g++ $(cflags)
ldflags=-L$(builddir) -L$(pocolib)/lib/Darwin/x86_64/ -lPocoFoundation -lPocoUtil -lPocoNet -lPocoCrypto -lPocoNetSSL

default: clean builddir toggl_api_client.o libkopsik.a kopsik.o kopsik

clean:
	rm -rf build

builddir:
	mkdir build

toggl_api_client.o:
	$(cxx) -c toggl_api_client.cc -o build/toggl_api_client.o

libkopsik.a: toggl_api_client.o
	rm -f $(builddir)/libkopsik.a && ar crs $(builddir)/libkopsik.a $(builddir)/toggl_api_client.o

kopsik.o:
	$(cxx) -c kopsik.cc -o build/kopsik.o

kopsik:
	$(cxx) $(ldflags) -o $(builddir)/libkopsik.a $(builddir)/kopsik.o -lkopsik

deps:
	cd $(pocolib) && ./configure --omit=Data/ODBC,Data/MySQL && make
