#-------------------------------------------------------------------------
#
# Makefile for src/tools/fsync
#
# Copyright (c) 2003-2010, PostgreSQL Global Development Group
#
# $PostgreSQL: pgsql/src/tools/fsync/Makefile,v 1.9 2010/07/05 18:54:38 tgl Exp $
#
#-------------------------------------------------------------------------

PROGRAM = test_fsync
OBJS    = test_fsync.o

subdir = src/tools/fsync
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk

override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)

all: submake-libpq submake-libpgport test_fsync
<<<<<<< HEAD
=======

test_fsync: test_fsync.o $(libpq_builddir)/libpq.a
	$(CC) $(CFLAGS) test_fsync.o $(libpq_pgport) $(LDFLAGS) $(LDFLAGS_EX) $(LIBS) -o $@$(X)

clean distclean maintainer-clean:
	rm -f test_fsync$(X) $(OBJS)
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
