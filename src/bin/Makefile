#-------------------------------------------------------------------------
#
# Makefile for src/bin (client programs)
#
# Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/src/bin/Makefile,v 1.55 2009/08/07 20:50:22 petere Exp $
#
#-------------------------------------------------------------------------

subdir = src/bin
top_builddir = ../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
unittest-check:
	$(MAKE) -C pg_dump/test check

DIRS = initdb pg_basebackup pg_ctl pg_dump \
	psql scripts pg_config pg_controldata pg_resetxlog \
	gpmirrortransition

=======
SUBDIRS = initdb pg_ctl pg_dump \
	psql scripts pg_config pg_controldata pg_resetxlog
>>>>>>> 78a09145e0
ifeq ($(PORTNAME), win32)
SUBDIRS+=pgevent
endif

ifeq ($(enable_gpfdist), yes)
DIRS+=gpfdist
endif

all install installdirs uninstall distprep clean distclean maintainer-clean:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir $@ || exit; done
