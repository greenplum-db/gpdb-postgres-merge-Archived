<<<<<<< HEAD
# $PostgreSQL: pgsql/contrib/tsearch2/Makefile,v 1.19 2007/06/26 22:05:03 tgl Exp $
=======
# $PostgreSQL: pgsql/contrib/tsearch2/Makefile,v 1.20 2007/11/13 21:02:28 tgl Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

MODULES = tsearch2
DATA_built = tsearch2.sql
DATA = uninstall_tsearch2.sql
REGRESS = tsearch2

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/tsearch2
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
