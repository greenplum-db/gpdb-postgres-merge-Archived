<<<<<<< HEAD
# contrib/fuzzystrmatch/Makefile
=======
# $PostgreSQL: pgsql/contrib/fuzzystrmatch/Makefile,v 1.10 2007/11/10 23:59:50 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

MODULE_big = fuzzystrmatch
OBJS = fuzzystrmatch.o dmetaphone.o

DATA_built = fuzzystrmatch.sql
<<<<<<< HEAD
DATA =  uninstall_fuzzystrmatch.sql
=======
DATA = uninstall_fuzzystrmatch.sql
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/fuzzystrmatch
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

# levenshtein.c is #included by fuzzystrmatch.c
fuzzystrmatch.o: fuzzystrmatch.c levenshtein.c
