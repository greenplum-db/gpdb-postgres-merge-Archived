MODULE_big = gpextprotocol
OBJS       = gpextprotocol.o
REGRESS = setup exttableext

<<<<<<< HEAD:contrib/extprotocol/Makefile
PG_CPPFLAGS = -I$(libpq_srcdir)
PG_LIBS = $(libpq_pgport)
=======
MODULE_big = adminpack
OBJS = adminpack.o $(WIN32RES)
PG_CPPFLAGS = -I$(libpq_srcdir)

EXTENSION = adminpack
DATA = adminpack--1.0.sql
PGFILEDESC = "adminpack - support functions for pgAdmin"
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8:contrib/adminpack/Makefile

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
