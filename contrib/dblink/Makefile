# contrib/dblink/Makefile

MODULE_big = dblink
OBJS	= dblink.o
<<<<<<< HEAD
=======
PG_CPPFLAGS = -I$(libpq_srcdir)
SHLIB_LINK = $(libpq)
SHLIB_PREREQS = submake-libpq

EXTENSION = dblink
DATA = dblink--1.0.sql dblink--unpackaged--1.0.sql
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

REGRESS = dblink
<<<<<<< HEAD
REGRESS_OPTS = --init-file=$(top_builddir)/src/test/regress/init_file --dbname=contrib_regression
=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/dblink
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

installcheck: prepare_dblink_sql

prepare_dblink_sql:
	cp $(GPHOME)/share/postgresql/contrib/dblink.sql ./dblink.sql

.PHONY: prepare_dblink_sql
