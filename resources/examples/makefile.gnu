
# Prelude.

SHELL = /bin/sh

ifndef EXTENSION
    $(error EXTENSION must be provided)
endif

# Make paths.

VPATH = ../obj

PUREDATA = ../../src
EXTERNALS = ./externals

# Warnings

WARNINGS = -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unknown-pragmas

# Flags.

CPPFLAGS = -I$(PUREDATA)
CFLAGS = -Os -fvisibility=hidden -shared -fpic $(WARNINGS) $(ARCH)
LDFLAGS = -lm

# Targets.

.PHONY: all

all:    $(EXTERNALS)/hello \
		$(EXTERNALS)/helloRoot \
		$(EXTERNALS)/helloRelease \
		$(EXTERNALS)/helloBad

$(EXTERNALS):
	@test -d $(EXTERNALS) || mkdir -p $(EXTERNALS)

$(EXTERNALS)/hello: hello.c | $(EXTERNALS)
	@echo "Build hello ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(EXTERNALS)/hello$(EXTENSION) hello.c

$(EXTERNALS)/helloRoot: helloRoot.c | $(EXTERNALS)
	@echo "Build helloRoot ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(EXTERNALS)/helloRoot$(EXTENSION) helloRoot.c

$(EXTERNALS)/helloRelease: helloRelease.c | $(EXTERNALS)
	@echo "Build helloRelease ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(EXTERNALS)/helloRelease$(EXTENSION) helloRelease.c

$(EXTERNALS)/helloBad: helloBad.c | $(EXTERNALS)
	@echo "Build helloBad ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(EXTERNALS)/helloBad$(EXTENSION) helloBad.c