# contrib/intarray/Makefile

MODULE_big = _int
<<<<<<< HEAD
OBJS = _int_bool.o _int_gist.o _int_op.o _int_tool.o _intbig_gist.o
DATA_built = _int.sql
DATA = uninstall__int.sql
=======
OBJS = _int_bool.o _int_gist.o _int_op.o _int_tool.o _intbig_gist.o _int_gin.o

EXTENSION = intarray
DATA = intarray--1.0.sql intarray--unpackaged--1.0.sql

>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
REGRESS = _int

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/intarray
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
