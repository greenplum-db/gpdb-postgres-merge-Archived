#-------------------------------------------------------------------------
#
# Makefile for src/tools/fsync
#
<<<<<<< HEAD
# Copyright (c) 2003-2009, PostgreSQL Global Development Group
#
# $PostgreSQL: pgsql/src/tools/fsync/Makefile,v 1.7 2009/01/01 17:24:05 momjian Exp $
=======
# Copyright (c) 2003-2008, PostgreSQL Global Development Group
#
# $PostgreSQL: pgsql/src/tools/fsync/Makefile,v 1.6 2008/01/01 19:46:01 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

PROGRAM = test_fsync
OBJS    = test_fsync.o

subdir = src/tools/fsync
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk

override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)
<<<<<<< HEAD
=======

OBJS= test_fsync.o
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

all: submake-libpq submake-libpgport test_fsync
