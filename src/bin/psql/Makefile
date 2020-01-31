#-------------------------------------------------------------------------
#
# Makefile for src/bin/psql
#
# Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
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
override CPPFLAGS := -DFRONTEND -I. -I$(srcdir) -I$(libpq_srcdir) $(CPPFLAGS)
LDFLAGS += -L$(top_builddir)/src/fe_utils -lpgfeutils -lpq

# necessary to get flex to play nice with large files on 32bit solaris sparc
ifeq ($(PORTNAME), solaris)
#ifeq ($(host_cpu), sparc)
ifneq ($(enable_largefile), no)
override CFLAGS := -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1
endif
#endif
endif

OBJS=	command.o common.o help.o input.o stringutils.o mainloop.o copy.o \
	startup.o prompt.o variables.o large_obj.o describe.o \
	crosstabview.o tab-complete.o \
	sql_help.o psqlscanslash.o \
=======
override CPPFLAGS := -I. -I$(srcdir) -I$(libpq_srcdir) $(CPPFLAGS)
LDFLAGS_INTERNAL += -L$(top_builddir)/src/fe_utils -lpgfeutils $(libpq_pgport)

OBJS=	command.o common.o copy.o crosstabview.o \
	describe.o help.o input.o large_obj.o mainloop.o \
	prompt.o psqlscanslash.o sql_help.o startup.o stringutils.o \
	tab-complete.o variables.o \
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
	$(WIN32RES)


all: psql

psql: $(OBJS) | submake-libpq submake-libpgport submake-libpgfeutils
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LDFLAGS_EX) $(LIBS) -o $@$(X)

help.o: sql_help.h

# See notes in src/backend/parser/Makefile about the following two rules
sql_help.c: sql_help.h
	touch $@

sql_help.h: create_help.pl $(wildcard $(REFDOCDIR)/*.sgml)
	$(PERL) $< $(REFDOCDIR) $*

psqlscanslash.c: FLEXFLAGS = -Cfe -p -p
psqlscanslash.c: FLEX_NO_BACKUP=yes
psqlscanslash.c: FLEX_FIX_WARNING=yes

<<<<<<< HEAD
# Latest flex causes warnings in this file.
ifeq ($(GCC),yes)
psqlscanslash.o: CFLAGS += -Wno-error
endif

=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
distprep: sql_help.h sql_help.c psqlscanslash.c

install: all installdirs
	$(INSTALL_PROGRAM) psql$(X) '$(DESTDIR)$(bindir)/psql$(X)'
	$(INSTALL_DATA) $(srcdir)/psqlrc.sample '$(DESTDIR)$(datadir)/psqlrc.sample'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(bindir)' '$(DESTDIR)$(datadir)'

uninstall:
	rm -f '$(DESTDIR)$(bindir)/psql$(X)' '$(DESTDIR)$(datadir)/psqlrc.sample'

clean distclean:
	rm -f psql$(X) $(OBJS) lex.backup

# files removed here are supposed to be in the distribution tarball,
# so do not clean them in the clean/distclean rules
maintainer-clean: distclean
	rm -f sql_help.h sql_help.c psqlscanslash.c
