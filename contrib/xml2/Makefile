<<<<<<< HEAD
# $PostgreSQL: pgsql/contrib/xml2/Makefile,v 1.12 2008/05/08 16:49:37 tgl Exp $

MODULE_big = pgxml

OBJS = $(if $(filter -lxslt, $(LIBS)), xpath.o xslt_proc.o, xpath.o)
=======
# $PostgreSQL: pgsql/contrib/xml2/Makefile,v 1.11.2.3 2010/03/01 18:08:16 tgl Exp $

MODULE_big = pgxml

OBJS = xpath.o xslt_proc.o
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

SHLIB_LINK += $(filter -lxslt, $(LIBS)) $(filter -lxml2, $(LIBS))

DATA_built = pgxml.sql
DATA = uninstall_pgxml.sql
<<<<<<< HEAD
=======
REGRESS = xml2
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/xml2
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
