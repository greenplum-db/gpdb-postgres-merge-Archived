<<<<<<< HEAD:contrib/extprotocol/Makefile
MODULE_big = gpextprotocol
OBJS       = gpextprotocol.o
=======
# $PostgreSQL: pgsql/contrib/oid2name/Makefile,v 1.10 2007/11/10 23:59:51 momjian Exp $

PROGRAM = oid2name
OBJS	= oid2name.o
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588:contrib/oid2name/Makefile

PG_CPPFLAGS = -I$(libpq_srcdir)
PG_LIBS = $(libpq_pgport)

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/extprotocol
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
