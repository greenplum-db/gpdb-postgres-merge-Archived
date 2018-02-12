#
# Makefile for pg_upgrade
#
# $PostgreSQL: pgsql/contrib/pg_upgrade/Makefile,v 1.4 2010/07/03 14:23:13 momjian Exp $

<<<<<<< HEAD
top_builddir = ../..

=======
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
PGFILEDESC = "pg_upgrade - an in-place binary upgrade utility"
PGAPPICON = win32

PROGRAM  = pg_upgrade
OBJS = check.o controldata.o dump.o exec.o file.o function.o info.o \
       option.o page.o pg_upgrade.o relfilenode.o server.o \
       tablespace.o util.o version.o version_old_8_3.o $(WIN32RES)

<<<<<<< HEAD
OBJS += aotable.o gpdb4_heap_convert.o version_old_gpdb4.o oid_dump.o

=======
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
PG_CPPFLAGS  = -DFRONTEND -DDLSUFFIX=\"$(DLSUFFIX)\" -I$(srcdir) -I$(libpq_srcdir)
PG_LIBS = $(libpq_pgport)

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_upgrade
<<<<<<< HEAD
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

installcheck: test_gpdb.sh all
	bash $< -o $(top_builddir)/gpAux/gpdemo/datadirs/ -b $(DESTDIR)$(bindir)

check: test_gpdb.sh all
	bash $< -C -s -o $(top_builddir)/gpAux/gpdemo/datadirs/ -b $(DESTDIR)$(bindir)
=======
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
