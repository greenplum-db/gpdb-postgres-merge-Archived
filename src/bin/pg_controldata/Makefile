#-------------------------------------------------------------------------
#
# Makefile for src/bin/pg_controldata
#
<<<<<<< HEAD
# Copyright (c) 1998-2009, PostgreSQL Global Development Group
#
# $PostgreSQL: pgsql/src/bin/pg_controldata/Makefile,v 1.18 2009/01/01 17:23:54 momjian Exp $
=======
# Copyright (c) 1998-2008, PostgreSQL Global Development Group
#
# $PostgreSQL: pgsql/src/bin/pg_controldata/Makefile,v 1.17 2008/01/01 19:45:55 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

PGFILEDESC = "pg_controldata - reads the data from pg_control"
subdir = src/bin/pg_controldata
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
# The frontend doesn't need everything that's in LIBS, some are backend only
LIBS := $(filter-out -lresolv -lbz2, $(LIBS))
# This program isn't interactive, so doesn't need these
LIBS := $(filter-out -lreadline -ledit -ltermcap -lncurses -lcurses -lcurl -lssl -lcrypto -lz, $(LIBS))

override CPPFLAGS += -DFRONTEND

OBJS= pg_controldata.o $(WIN32RES)
=======
OBJS= pg_controldata.o pg_crc.o $(WIN32RES)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

all: submake-libpgport pg_controldata

pg_controldata: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@$(X)

install: all installdirs
	$(INSTALL_PROGRAM) pg_controldata$(X) '$(DESTDIR)$(bindir)/pg_controldata$(X)'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(bindir)'

uninstall:
	rm -f '$(DESTDIR)$(bindir)/pg_controldata$(X)'

clean distclean maintainer-clean:
	rm -f pg_controldata$(X) $(OBJS)
