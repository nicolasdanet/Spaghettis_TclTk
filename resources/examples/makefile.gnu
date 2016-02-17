
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

all: $(EXTERNALS)/hello

$(EXTERNALS):
	@test -d $(EXTERNALS) || mkdir -p $(EXTERNALS)

$(EXTERNALS)/hello: hello.c | $(EXTERNALS)
	@echo "Build hello ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(EXTERNALS)/hello$(EXTENSION) hello.c
