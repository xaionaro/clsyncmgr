
DESTDIR ?= 
PREFIX  ?= /usr
COMPRESS_MAN ?= yes
STRIP_BINARY ?= yes
EXAMPLES ?= yes

CSECFLAGS ?= -fstack-protector-all -Wall --param ssp-buffer-size=4 -D_FORTIFY_SOURCE=2 -fstack-check -DPARANOID -std=gnu99
CFLAGS ?= -pipe -O2
CFLAGS += $(CSECFLAGS)
DEBUGCFLAGS ?= -pipe -Wall -Werror -ggdb3 -export-dynamic -Wno-error=unused-variable $(CSECFLAGS)

CARCHFLAGS ?= -march=native

LIBS := $(shell pkg-config --libs clsync) $(shell pkg-config --libs glib-2.0) -lpthread
LDSECFLAGS ?= -Xlinker -zrelro
LDFLAGS += $(LDSECFLAGS)
INC := $(shell pkg-config --cflags clsync) $(shell pkg-config --cflags glib-2.0) $(INC)

INSTDIR = $(DESTDIR)$(PREFIX)

objs=\
error.o\
malloc.o\
clsyncmgr.o\
main.o\

binary=clsyncmgr

.PHONY: doc

all: $(objs)
	$(CC) $(CARCHFLAGS) $(CFLAGS) $(LDFLAGS) $(objs) $(LIBS) -o $(binary)

%.o: %.c
	$(CC) $(CARCHFLAGS) $(CFLAGS) $(INC) $< -c -o $@

debug:
	$(CC) $(CARCHFLAGS) -DFANOTIFY_SUPPORT $(DEBUGCFLAGS) $(INC) $(LDFLAGS) *.c $(LIBS) -o $(binary)


clean:
	rm -f $(binary) *.o

distclean: clean

doc:
	doxygen .doxygen

install:
	install -d "$(INSTDIR)/bin" "$(INSTDIR)/share/man/man1"
ifeq ($(STRIP_BINARY),yes)
	strip --strip-unneeded -R .comment -R .GCC.command.line -R .note.gnu.gold-version $(binary)
endif
	install -m 755 $(binary) "$(INSTDIR)"/bin/
	install -m 644 man/man1/clsyncmgr.1 "$(INSTDIR)"/share/man/man1/
ifeq ($(COMPRESS_MAN),yes)
	rm -f "$(INSTDIR)"/share/man/man1/clsyncmgr.1.gz
	gzip "$(INSTDIR)"/share/man/man1/clsyncmgr.1
endif

deinstall:
	rm -f "$(INSTDIR)"/bin/$(binary) "$(INSTDIR)"/share/man/man1/clsync.1.gz "$(INSTDIR)"/share/man/man1/clsync.1

