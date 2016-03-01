#-------------------------------------------------------------------------
#
# pgrowlocks Makefile
#
<<<<<<< HEAD
# $PostgreSQL: pgsql/contrib/pgrowlocks/Makefile,v 1.6 2009/04/28 17:07:50 momjian Exp $
=======
# $PostgreSQL: pgsql/contrib/pgrowlocks/Makefile,v 1.5 2007/11/10 23:59:51 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

MODULE_big	= pgrowlocks
OBJS		= pgrowlocks.o
DATA_built	= pgrowlocks.sql
DATA		= uninstall_pgrowlocks.sql

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pgrowlocks
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

