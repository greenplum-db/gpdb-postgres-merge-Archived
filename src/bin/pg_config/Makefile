#-------------------------------------------------------------------------
#
# Makefile for src/bin/pg_config
#
# Copyright (c) 1998-2016, PostgreSQL Global Development Group
#
# src/bin/pg_config/Makefile
#
#-------------------------------------------------------------------------

PGFILEDESC = "pg_config - report configuration information"
PGAPPICON=win32

subdir = src/bin/pg_config
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

OBJS=   pg_config.o $(WIN32RES)

# GPDB_96_MERGE_FIXME: do we still need these lines?
# The frontend doesn't need everything that's in LIBS, some are backend only
LIBS := $(filter-out -lresolv -lbz2, $(LIBS))
# This program isn't interactive, so doesn't need these
LIBS := $(filter-out -lreadline -ledit -ltermcap -lncurses -lcurses -lcurl -lssl -lcrypto -lz, $(LIBS))

all: pg_config

pg_config: $(OBJS) | submake-libpgport
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LDFLAGS_EX) $(LIBS) -o $@$(X)

install: all installdirs
	$(INSTALL_SCRIPT) pg_config$(X) '$(DESTDIR)$(bindir)/pg_config$(X)'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(bindir)'

uninstall:
	rm -f '$(DESTDIR)$(bindir)/pg_config$(X)'

clean distclean maintainer-clean:
	rm -f pg_config$(X) $(OBJS)
	rm -rf tmp_check

check:
	$(prove_check)

installcheck:
	$(prove_installcheck)
