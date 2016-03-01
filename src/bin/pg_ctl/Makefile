#-------------------------------------------------------------------------
#
# Makefile for src/bin/pg_ctl
#
<<<<<<< HEAD
# Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/src/bin/pg_ctl/Makefile,v 1.26 2009/01/01 17:23:54 momjian Exp $
=======
# Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/src/bin/pg_ctl/Makefile,v 1.25 2008/01/01 19:45:55 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

PGFILEDESC = "pg_ctl - starts/stops/restarts the PostgreSQL server"
subdir = src/bin/pg_ctl
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
ifneq ($(PORTNAME), win32)
override CFLAGS += $(PTHREAD_CFLAGS) -pthread
endif

# The frontend doesn't need everything that's in LIBS, some are backend only
LIBS := $(filter-out -lresolv -lbz2, $(LIBS))
# This program isn't interactive, so doesn't need these
LIBS := $(filter-out -lreadline -ledit -ltermcap -lncurses -lcurses -lcurl -lssl -lcrypto -lz, $(LIBS))



override CPPFLAGS := -DDEF_PGPORT=$(DEF_PGPORT) -I$(libpq_srcdir) $(CPPFLAGS)

OBJS=	pg_ctl.o $(WIN32RES)

all: submake-libpq submake-libpgport pg_ctl

pg_ctl: $(OBJS) $(libpq_builddir)/libpq.a
	$(CC) $(CFLAGS) $(OBJS) $(libpq_pgport) $(LDFLAGS) $(LIBS) -o $@$(X)

install: all installdirs
	$(INSTALL_PROGRAM) pg_ctl$(X) '$(DESTDIR)$(bindir)/pg_ctl$(X)'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(bindir)'

uninstall:
	rm -f '$(DESTDIR)$(bindir)/pg_ctl$(X)'

clean distclean maintainer-clean:
	rm -f pg_ctl$(X) $(OBJS)


# ensure that changes in DEF_PGPORT propagate into object file
pg_ctl.o: pg_ctl.c $(top_builddir)/src/Makefile.global
