# contrib/citext/Makefile

MODULES = citext

EXTENSION = citext
<<<<<<< HEAD
DATA = citext--1.0.sql citext--1.1.sql citext--1.0--1.1.sql \
       citext--1.1--1.0.sql citext--unpackaged--1.0.sql
=======
DATA = citext--1.1.sql citext--1.0--1.1.sql citext--unpackaged--1.0.sql
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
PGFILEDESC = "citext - case-insensitive character string data type"

REGRESS = citext

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/citext
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
