<<<<<<< HEAD:contrib/gp_internal_tools/Makefile
MODULE_big = gp_ao_co_diagnostics
OBJS       = gp_ao_co_diagnostics.o

PG_CPPFLAGS = -I$(libpq_srcdir)
=======
# $PostgreSQL: pgsql/contrib/lo/Makefile,v 1.17 2007/11/10 23:59:51 momjian Exp $

MODULES = lo
DATA_built = lo.sql
DATA = uninstall_lo.sql
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588:contrib/lo/Makefile

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/gp_diagnostic_libraries
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
