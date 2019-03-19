<<<<<<< HEAD:contrib/indexscan/Makefile
MODULES = indexscan
DATA_built = indexscan.sql
REGRESS = create_function read_heap_index read_ao_index read_co_index drop_function
=======
# contrib/chkpass/Makefile

MODULE_big = chkpass
OBJS = chkpass.o $(WIN32RES)

EXTENSION = chkpass
DATA = chkpass--1.0.sql chkpass--unpackaged--1.0.sql
PGFILEDESC = "chkpass - encrypted password data type"

SHLIB_LINK = $(filter -lcrypt, $(LIBS))
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8:contrib/chkpass/Makefile

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/indexscan
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

