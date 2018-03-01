
# Prelude.

SHELL = /bin/sh

# Make paths.

VPATH = ../obj

BELLE_DIR = ../libraries/belle

BIN_DIR = ../bin
TCL_DIR = ../tcl

HELP_DIR = ../resources/help

# Install paths.

# /usr/local/bin/spaghettis
# /usr/local/bin/spaghettissnd
# /usr/local/bin/spaghettisrcv
# /usr/local/include/m_spaghettis.h
# /usr/local/lib/spaghettis/tcl/*.tcl
# /usr/local/lib/spaghettis/help/*.pdhelp

# prefix          = /usr/local
# exec_prefix     = $(prefix)
# bindir          = $(exec_prefix)/bin
# includedir      = $(prefix)/include
# libdir          = $(exec_prefix)/lib
# libpddir        = $(libdir)/spaghettis
# libpdtcldir     = $(libpddir)/tcl
# libpdhelpdir    = $(libpddir)/help

# Warnings

WARNINGS = -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-strict-overflow

# Linker flags.

LIB = -ldl -lpthread -lm -lasound -ljack

LDFLAGS = -rdynamic

# Preprocessor and compiler flags.

CPPFLAGS = -DNDEBUG -DPD_BUILDING_APPLICATION -I$(BELLE_DIR)/Source

CFLAGS   = -O3 -ffast-math $(MARCH) -fvisibility=hidden $(WARNINGS)
CXXFLAGS = $(CXXSTD) $(CFLAGS)

# Expr with TinyExpr.

EXPR_SRC = tinyexpr.c

# Sources amalgamated.

SRC = amalgam.cpp

# Objects.

OBJ_CPP = $(SRC:.cpp=.o)
OBJ_LIB = $(EXPR_SRC:.c=.o)

# Targets.

.PHONY: all

all: $(BIN_DIR)/spaghettis $(BIN_DIR)/spaghettissnd $(BIN_DIR)/spaghettisrcv

$(BIN_DIR):
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)

$(OBJ_CPP): %.o : %.cpp
	@echo "Build CPP $@ ..."
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_LIB): %.o : %.c
	@echo "Build $@ ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/spaghettis: $(OBJ_CPP) $(OBJ_LIB) | $(BIN_DIR)
	@echo "Build spaghettis ..."
	@$(CXX) $(LDFLAGS) -o $(BIN_DIR)/spaghettis $(OBJ_CPP) $(OBJ_LIB) $(LIB)

$(BIN_DIR)/spaghettissnd: u_pdsend.c | $(BIN_DIR)
	@echo "Build spaghettissnd ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) -o $(BIN_DIR)/spaghettissnd u_pdsend.c

$(BIN_DIR)/spaghettisrcv: u_pdreceive.c | $(BIN_DIR)
	@echo "Build spaghettisrcv ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) -o $(BIN_DIR)/spaghettisrcv u_pdreceive.c

depend: makefile.dependencies

makefile.dependencies:
	@echo "Build makefile.dependencies ..."
	@$(CC) $(CPPFLAGS) -M $(SRC) > makefile.dependencies

clean:
	@echo "Remove makefile.dependencies ..."
	@-rm -f makefile.dependencies
	@echo "Remove objects ..."
	@-rm -f $(OBJ_CPP)
	@-rm -f $(OBJ_LIB)
	@echo "Remove binaries ..."
	@-rm -f $(BIN_DIR)/spaghettis $(BIN_DIR)/spaghettissnd $(BIN_DIR)/spaghettisrcv
	@echo "Remove bin directory ..."
	@-rmdir $(BIN_DIR)

# install: all
#	@echo "Install binaries ..."
#	install -d $(DESTDIR)$(bindir)
#	install -m755 $(BIN_DIR)/spaghettis $(DESTDIR)$(bindir)/spaghettis
#	install -m755 $(BIN_DIR)/spaghettissnd $(DESTDIR)$(bindir)/spaghettissnd
#	install -m755 $(BIN_DIR)/spaghettisrcv $(DESTDIR)$(bindir)/spaghettisrcv
#	@echo "Install scripts ..."
#	install -d $(DESTDIR)$(libpdtcldir)
#	install $(TCL_DIR)/*.tcl $(DESTDIR)$(libpdtcldir)
#	@echo "Install help ..."
#	install -d $(DESTDIR)$(libpdhelpdir)
#	install $(HELP_DIR)/*.pdhelp $(DESTDIR)$(libpdhelpdir)
#	install $(HELP_DIR)/*.pd $(DESTDIR)$(libpdhelpdir)
#	install $(HELP_DIR)/*.txt $(DESTDIR)$(libpdhelpdir)
#	install $(HELP_DIR)/*.aiff $(DESTDIR)$(libpdhelpdir)
#	install $(HELP_DIR)/*.wav $(DESTDIR)$(libpdhelpdir)
#	@echo "Install headers ..."
#	install -d $(DESTDIR)$(includedir)
#	install -m644 m_spaghettis.h $(DESTDIR)$(includedir)/m_spaghettis.h

# uninstall:
#	@echo "Uninstall binaries ..."
#	rm -f $(DESTDIR)$(bindir)/spaghettis
#	rm -f $(DESTDIR)$(bindir)/spaghettissnd
#	rm -f $(DESTDIR)$(bindir)/spaghettisrcv
#	@echo "Uninstall scripts ..."
#	@echo "Uninstall help ..."
#	rm -f -r $(DESTDIR)$(libpddir)
#	@echo "Uninstall headers ..."
#	rm -f $(DESTDIR)$(includedir)/m_spaghettis.h

include makefile.dependencies
