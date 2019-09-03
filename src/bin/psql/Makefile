#-------------------------------------------------------------------------
#
# Makefile for src/bin/psql
#
# Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# src/bin/psql/Makefile
#
#-------------------------------------------------------------------------

PGFILEDESC = "psql - the PostgreSQL interactive terminal"
PGAPPICON=win32

subdir = src/bin/psql
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

REFDOCDIR= $(top_srcdir)/doc/src/sgml/ref

<<<<<<< HEAD
override CPPFLAGS := -DFRONTEND -I. -I$(srcdir) -I$(libpq_srcdir) -I$(top_srcdir)/src/bin/pg_dump $(CPPFLAGS)

# necessary to get flex to play nice with large files on 32bit solaris sparc
ifeq ($(PORTNAME), solaris)
#ifeq ($(host_cpu), sparc)
ifneq ($(enable_largefile), no)
override CFLAGS := -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1
endif
#endif
endif
=======
override CPPFLAGS := -I. -I$(srcdir) -I$(libpq_srcdir) $(CPPFLAGS)
LDFLAGS += -L$(top_builddir)/src/fe_utils -lpgfeutils -lpq
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

OBJS=	command.o common.o help.o input.o stringutils.o mainloop.o copy.o \
	startup.o prompt.o variables.o large_obj.o describe.o \
	crosstabview.o tab-complete.o \
	sql_help.o psqlscanslash.o \
	$(WIN32RES)

# The frontend doesn't need everything that's in LIBS, some are backend only
LIBS := $(filter-out -lresolv -lbz2, $(LIBS))


all: psql

psql: $(OBJS) | submake-libpq submake-libpgport submake-libpgfeutils
	$(CC) $(CFLAGS) $(OBJS) $(libpq_pgport) $(LDFLAGS) $(LDFLAGS_EX) $(LIBS) -o $@$(X)

help.o: sql_help.h

<<<<<<< HEAD
dumputils.c keywords.c: % : $(top_srcdir)/src/bin/pg_dump/%
	rm -f $@ && $(LN_S) $< .

kwlookup.c: % : $(top_srcdir)/src/backend/parser/%
	rm -f $@ && $(LN_S) $< .

# See notes in src/backend/parser/Makefile about the following two rules
sql_help.c: sql_help.h
	touch $@

=======
sql_help.c: sql_help.h ;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
sql_help.h: create_help.pl $(wildcard $(REFDOCDIR)/*.sgml)
	$(PERL) $< $(REFDOCDIR) $*

psqlscanslash.c: FLEXFLAGS = -Cfe -p -p
psqlscanslash.c: FLEX_NO_BACKUP=yes

# Latest flex causes warnings in this file.
ifeq ($(GCC),yes)
psqlscanslash.o: CFLAGS += -Wno-error
endif

<<<<<<< HEAD
distprep: sql_help.h sql_help.c psqlscan.c
=======
distprep: sql_help.h psqlscanslash.c
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

install: all installdirs
	$(INSTALL_PROGRAM) psql$(X) '$(DESTDIR)$(bindir)/psql$(X)'
	$(INSTALL_DATA) $(srcdir)/psqlrc.sample '$(DESTDIR)$(datadir)/psqlrc.sample'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(bindir)' '$(DESTDIR)$(datadir)'

uninstall:
	rm -f '$(DESTDIR)$(bindir)/psql$(X)' '$(DESTDIR)$(datadir)/psqlrc.sample'

clean distclean:
<<<<<<< HEAD
	rm -f psql$(X) $(OBJS) dumputils.c keywords.c kwlookup.c psqlscan.o
=======
	rm -f psql$(X) $(OBJS) lex.backup
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

# files removed here are supposed to be in the distribution tarball,
# so do not clean them in the clean/distclean rules
maintainer-clean: distclean
	rm -f sql_help.h sql_help.c psqlscanslash.c
