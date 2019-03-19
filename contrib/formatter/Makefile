MODULE_big = gpformatter
OBJS       = gpformatter.o

<<<<<<< HEAD:contrib/formatter/Makefile
PG_CPPFLAGS = -I$(libpq_srcdir)
PG_LIBS = $(libpq_pgport)
=======
MODULES = lo

EXTENSION = lo
DATA = lo--1.0.sql lo--unpackaged--1.0.sql
PGFILEDESC = "lo - management for large objects"
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8:contrib/lo/Makefile

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
