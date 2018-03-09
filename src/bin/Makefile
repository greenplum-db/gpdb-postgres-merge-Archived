#-------------------------------------------------------------------------
#
# Makefile for src/bin (client programs)
#
# Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# src/bin/Makefile
#
#-------------------------------------------------------------------------

subdir = src/bin
top_builddir = ../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
unittest-check:
	$(MAKE) -C pg_dump/test check

SUBDIRS = initdb pg_basebackup pg_ctl pg_dump \
	psql scripts pg_config pg_controldata pg_resetxlog

=======
SUBDIRS = initdb pg_ctl pg_dump \
	psql scripts pg_config pg_controldata pg_resetxlog pg_basebackup
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
ifeq ($(PORTNAME), win32)
SUBDIRS+=pgevent
endif

<<<<<<< HEAD
ifeq ($(enable_gpfdist), yes)
SUBDIRS+=gpfdist
endif

all install installdirs uninstall distprep clean distclean maintainer-clean:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir $@ || exit; done
=======
$(recurse)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
