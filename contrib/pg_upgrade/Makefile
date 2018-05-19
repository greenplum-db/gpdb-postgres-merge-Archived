# contrib/pg_upgrade/Makefile

top_builddir = ../..

PGFILEDESC = "pg_upgrade - an in-place binary upgrade utility"
PGAPPICON = win32

PROGRAM  = pg_upgrade
OBJS = check.o controldata.o dump.o exec.o file.o function.o info.o \
       option.o page.o pg_upgrade.o relfilenode.o server.o \
       tablespace.o util.o version.o version_old_8_3.o $(WIN32RES)

OBJS += aotable.o gpdb4_heap_convert.o version_old_gpdb4.o oid_dump.o

PG_CPPFLAGS  = -DFRONTEND -DDLSUFFIX=\"$(DLSUFFIX)\" -I$(srcdir) -I$(libpq_srcdir)
PG_LIBS = $(libpq_pgport)

<<<<<<< HEAD
# We don't need to add tmp_check to the EXTRA_CLEAN since it's a default name
# and thus already handled by PGXS "make clean"
EXTRA_CLEAN = clusterConfigPostgresAddonsFile clusterConfigFile gpdemo-env.sh \
              hostfile
=======
EXTRA_CLEAN = delete_old_cluster.sh log/ tmp_check/
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_upgrade
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

<<<<<<< HEAD
installcheck: test_gpdb.sh all
	bash $< -o $(top_builddir)/gpAux/gpdemo/datadirs/ -b $(DESTDIR)$(bindir)

check: test_gpdb.sh all
	bash $< -C -s -o $(top_builddir)/gpAux/gpdemo/datadirs/ -b $(DESTDIR)$(bindir)
=======
check: test.sh all
	MAKE=$(MAKE) bindir=$(bindir) libdir=$(libdir) $(SHELL) $< --install

# disabled because it upsets the build farm
#installcheck: test.sh
#	MAKE=$(MAKE) bindir=$(bindir) libdir=$(libdir) $(SHELL) $<
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
