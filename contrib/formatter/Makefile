<<<<<<< HEAD:contrib/formatter/Makefile
MODULE_big = gpformatter
OBJS       = gpformatter.o
=======
# $PostgreSQL: pgsql/contrib/vacuumlo/Makefile,v 1.17 2007/11/10 23:59:52 momjian Exp $

PROGRAM = vacuumlo
OBJS	= vacuumlo.o
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588:contrib/vacuumlo/Makefile

PG_CPPFLAGS = -I$(libpq_srcdir)
PG_LIBS = $(libpq_pgport)

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/formatter
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
