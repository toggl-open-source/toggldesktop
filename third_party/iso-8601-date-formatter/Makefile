CLANG=/usr/bin/clang
CC=$(CLANG)
CFLAGS+=-std=c99 -g -Werror -Wmissing-field-initializers -Wreturn-type -Wmissing-braces -Wparentheses -Wswitch -Wunused-function -Wunused-label -Wunused-variable -Wunused-value -Wshadow -Wsign-compare -Wnewline-eof -Wshorten-64-to-32 -Wundeclared-selector -Wmissing-prototypes -Wformat -Wunknown-pragmas
LDFLAGS+=-framework Foundation

all: testparser unparse-weekdate unparse-ordinaldate unparse-date
test: all parser-test unparser-test
analysis: ISO8601DateFormatter-analysis.plist

parser-test: testparser testparser.sh
	./testparser.sh
unparser-test: testunparser.sh unparse-weekdate unparse-ordinaldate unparse-date
	./testunparser.sh > testunparser.out
	diff -qs test_files/testunparser-expected.out testunparser.out

.PHONY: all test analysis parser-test unparser-test

testparser: testparser.o ISO8601DateFormatter.o

testparser.sh: testparser.sh.in
	python testparser.sh.py

unparse-weekdate: unparse-weekdate.o ISO8601DateFormatter.o ISO8601DateFormatter.o
unparse-ordinaldate: unparse-ordinaldate.o ISO8601DateFormatter.o ISO8601DateFormatter.o
unparse-date: unparse-date.o ISO8601DateFormatter.o ISO8601DateFormatter.o

testunparsewithtime: testunparsewithtime.o ISO8601DateFormatter.o

ISO8601DateFormatter-analysis.plist: ISO8601DateFormatter.m
	$(CLANG) $^ --analyze -o /dev/null

timetrial: timetrial.o ISO8601DateFormatter.o
