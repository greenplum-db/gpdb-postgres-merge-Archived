#-------------------------------------------------------------------------
#
# Makefile for ecpg library
#
# Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# src/interfaces/ecpg/ecpglib/Makefile
#
#-------------------------------------------------------------------------

subdir = src/interfaces/ecpg/ecpglib
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

PGFILEDESC = "ECPG - embedded SQL in C"
NAME= ecpg
SO_MAJOR_VERSION= 7
SO_MINOR_VERSION= $(MAJORVERSION)

override CPPFLAGS := -I../include -I$(top_srcdir)/src/interfaces/ecpg/include \
	-I$(libpq_srcdir) -I$(top_builddir)/src/port -DFRONTEND $(CPPFLAGS)


ifeq ($(GCC), yes)
override CFLAGS += -Wno-error
endif
override CFLAGS += $(PTHREAD_CFLAGS)

OBJS= execute.o typename.o descriptor.o sqlda.o data.o error.o prepare.o \
	memory.o connect.o misc.o cursor.o $(WIN32RES)

<<<<<<< HEAD
OBJS= execute.o typename.o descriptor.o sqlda.o data.o error.o prepare.o memory.o \
	connect.o misc.o path.o pgstrcasecmp.o \
	$(filter snprintf.o strlcpy.o win32setlocale.o isinf.o, $(LIBOBJS)) $(WIN32RES)

# thread.c is needed only for non-WIN32 implementation of path.c
ifneq ($(PORTNAME), win32)
OBJS += thread.o
else
PTHREAD_LIBS=-lpthread
endif

SHLIB_LINK_INTERNAL = -L../pgtypeslib -lpgtypes $(libpq)
=======
SHLIB_LINK_INTERNAL = -L../pgtypeslib -lpgtypes $(libpq_pgport_shlib)
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
SHLIB_LINK = $(filter -lintl -lm, $(LIBS)) $(PTHREAD_LIBS)
SHLIB_PREREQS = submake-libpq submake-pgtypeslib

SHLIB_EXPORTS = exports.txt

PKG_CONFIG_REQUIRES_PRIVATE = libpq libpgtypes

all: all-lib

.PHONY: submake-pgtypeslib
submake-pgtypeslib:
	$(MAKE) -C $(top_builddir)/src/interfaces/ecpg/pgtypeslib all

# Shared library stuff
include $(top_srcdir)/src/Makefile.shlib

# Make dependency on pg_config_paths.h visible.
misc.o: misc.c $(top_builddir)/src/port/pg_config_paths.h

$(top_builddir)/src/port/pg_config_paths.h:
	$(MAKE) -C $(top_builddir)/src/port pg_config_paths.h

install: all installdirs install-lib

installdirs: installdirs-lib

uninstall: uninstall-lib

clean distclean: clean-lib
	rm -f $(OBJS)

maintainer-clean: distclean maintainer-clean-lib
