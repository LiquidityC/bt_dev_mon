VERSION = \"1.0.0\"

CC			= gcc
LDFLAGS		= -lbluetooth
CFLAGS		= -Iinc -DVERSION=$(VERSION)
RM			= rm
MODULE		= bt_mon
FORMAT		= clang-format
CHECK		= cppcheck
CP			= cp
ECHO		= echo
MKDIR		= mkdir
PANDOC		= pandoc

PREFIX ?= out
SRCDIR	= src
OBJDIR	= .obj
DOCDIR	= doc
MANDIR	= doc/man

ifndef RELEASE_BUILD
	CFLAGS += -DDEBUG_BUILD -Wall -g -fsanitize=address -fno-omit-frame-pointer
else
	CFLAGS += -O2 -DRELEASE_BUILD
	OBJDIR = .objrls
endif

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(subst $(SRCDIR)/,$(OBJDIR)/,$(patsubst %.c,%.o,$(SRC)))

MDFILES = $(wildcard $(DOCDIR)/mon.*.md)
MANFILES = $(subst $(DOCDIR)/,$(MANDIR)/,$(patsubst $(DOCDIR)/%.md,$(DOCDIR)/%, $(MDFILES)))

default: $(MODULE)

all: $(MODULE) $(MANFILES)

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(MKDIR) -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(MODULE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(MODULE)
	@./$(MODULE)

clean:
	$(RM) -rf $(OBJ) $(MODULE) $(MANFILES) $(PREFIX) $(OBJDIR) $(MANDIR)

fmt:
	@$(FORMAT) -i src/*.c inc/*.h

check:
	@$(ECHO) "Checking formatting"
	@$(FORMAT) --dry-run -Werror src/*.[ch]
	@$(CHECK) -x c --std=c11 -Iinc -i/usr/include --enable=all --suppress=missingIncludeSystem .

$(MANFILES): $(MDFILES)
	@mkdir -p $(MANDIR)
	$(PANDOC) $< -s -t man -o $@

man: $(MANFILES)

install: $(MODULE) $(MANFILES)
	@$(MKDIR) -p $(PREFIX)/bin
	@$(MKDIR) -p $(PREFIX)/share/man/man1
	@$(ECHO) "Installing binary : $(PREFIX)/bin/$(MODULE)"
	@$(CP) -v $(MODULE) $(PREFIX)/bin/$(MODULE)
	@$(ECHO) "Installing man files"
	@$(CP) -v $(MANDIR)/*.1 $(PREFIX)/share/man/man1/

.PHONY: $(MODULE) clean all fmt run check install
