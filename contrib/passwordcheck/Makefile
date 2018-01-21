<<<<<<< HEAD
#stgreSQL: pgsql/contrib/passwordcheck/Makefile,v 1.1 2009/11/18 21:57:56 tgl Exp $
=======
# $PostgreSQL: pgsql/contrib/passwordcheck/Makefile,v 1.1 2009/11/18 21:57:56 tgl Exp $
>>>>>>> 78a09145e0

MODULE_big = passwordcheck
OBJS = passwordcheck.o

<<<<<<< HEAD
#uncomment the following two lines to enable cracklib support
=======
# uncomment the following two lines to enable cracklib support
>>>>>>> 78a09145e0
# PG_CPPFLAGS = -DUSE_CRACKLIB '-DCRACKLIB_DICTPATH="/usr/lib/cracklib_dict"'
# SHLIB_LINK = -lcrack

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/passwordcheck
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
