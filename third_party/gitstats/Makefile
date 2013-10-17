PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
RESOURCEDIR=$(PREFIX)/share/gitstats
RESOURCES=gitstats.css sortable.js *.gif
BINARIES=gitstats
VERSION=$(shell git describe 2>/dev/null || git rev-parse --short HEAD)
SEDVERSION=perl -pi -e 's/VERSION = 0/VERSION = "$(VERSION)"/' --

all: help

help:
	@echo "Usage:"
	@echo
	@echo "make install                   # install to ${PREFIX}"
	@echo "make install PREFIX=~          # install to ~"
	@echo "make release [VERSION=foo]     # make a release tarball"
	@echo

install:
	install -d $(BINDIR) $(RESOURCEDIR)
	install -v $(BINARIES) $(BINDIR)
	install -v -m 644 $(RESOURCES) $(RESOURCEDIR)
	$(SEDVERSION) $(BINDIR)/gitstats

release:
	@cp gitstats gitstats.tmp
	@$(SEDVERSION) gitstats.tmp
	@tar --owner=0 --group=0 --transform 's!^!gitstats/!' --transform 's!gitstats.tmp!gitstats!' -zcf gitstats-$(VERSION).tar.gz gitstats.tmp $(RESOURCES) doc/ Makefile
	@$(RM) gitstats.tmp

man:
	pod2man --center "User Commands" -r $(shell git rev-parse --short HEAD) doc/gitstats.pod > doc/gitstats.1

.PHONY: all help install release
