#-------------------------------------------------------------------------
#
# Makefile for ecpg library
#
<<<<<<< HEAD
# Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/src/interfaces/ecpg/ecpglib/Makefile,v 1.63 2009/02/07 17:17:34 momjian Exp $
=======
# Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/src/interfaces/ecpg/ecpglib/Makefile,v 1.54.2.2 2008/03/21 16:10:10 meskes Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

subdir = src/interfaces/ecpg/ecpglib
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

NAME= ecpg
SO_MAJOR_VERSION= 6
<<<<<<< HEAD
SO_MINOR_VERSION= 1


# The frontend doesn't need everything that's in LIBS, some are backend only
LIBS := $(filter-out -lresolv -lbz2, $(LIBS))
# This program isn't interactive, so doesn't need these
LIBS := $(filter-out -lreadline -ledit -ltermcap -lncurses -lcurses -lcurl -lssl -lcrypto -lz, $(LIBS))
=======
SO_MINOR_VERSION= 0
DLTYPE= library
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

override CPPFLAGS := -I../include -I$(top_srcdir)/src/interfaces/ecpg/include \
	-I$(libpq_srcdir) -I$(top_builddir)/src/port $(CPPFLAGS)


ifeq ($(GCC), yes)
override CFLAGS += -Wno-error
endif
override CFLAGS += $(PTHREAD_CFLAGS)

# Need to recompile any libpgport object files
LIBS := $(filter-out -lpgport, $(LIBS))

OBJS= execute.o typename.o descriptor.o data.o error.o prepare.o memory.o \
	connect.o misc.o path.o pgstrcasecmp.o \
	$(filter snprintf.o strlcpy.o, $(LIBOBJS))

# thread.c is needed only for non-WIN32 implementation of path.c
ifneq ($(PORTNAME), win32)
OBJS += thread.o
else
PTHREAD_LIBS=-lpthread
endif

SHLIB_LINK = -L../pgtypeslib -lpgtypes $(libpq) $(filter -lintl -lm, $(LIBS)) $(PTHREAD_LIBS)

SHLIB_EXPORTS = exports.txt

ifeq ($(PORTNAME), win32)
# Link to shfolder.dll instead of shell32.dll
SHLIB_LINK += -lshfolder
DLL_DEFFILE=libecpgdll.def
endif

<<<<<<< HEAD
all: libpq pgtypeslib all-lib

libpq:
	$(MAKE) -C $(top_builddir)/src/interfaces/libpq all

pgtypeslib:
	$(MAKE) -C $(top_builddir)/src/interfaces/ecpg/pgtypeslib all
=======
all: def-files all-lib
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

# Shared library stuff
include $(top_srcdir)/src/Makefile.shlib

# We use some port modules verbatim, but since we need to
# compile with appropriate options to build a shared lib, we can't
# necessarily use the same object files as the backend uses. Instead,
# symlink the source files in here and build our own object file.

path.c pgstrcasecmp.c snprintf.c strlcpy.c thread.c: % : $(top_srcdir)/src/port/%
	rm -f $@ && $(LN_S) $< .

misc.o: misc.c $(top_builddir)/src/port/pg_config_paths.h
path.o: path.c $(top_builddir)/src/port/pg_config_paths.h

$(top_builddir)/src/port/pg_config_paths.h:
	$(MAKE) -C $(top_builddir)/src/port pg_config_paths.h

# We need several not-quite-identical variants of .DEF files to build libecpg
# DLLs for Windows.  These are made from the single source file exports.txt.
# Since we can't assume that Windows boxes will have sed, the .DEF files are
# always built and included in distribution tarballs.

distprep: def-files

.PHONY: def-files

def-files: $(srcdir)/libecpgdll.def $(srcdir)/blibecpgdll.def

$(srcdir)/libecpgdll.def: exports.txt
	echo '; DEF file for MS VC++' > $@
	echo 'LIBRARY LIBECPG' >> $@
	echo 'EXPORTS' >> $@
	sed -e '/^#/d' -e 's/^\(.* \)\([0-9][0-9]*\)/    \1@ \2/' < $< >> $@

$(srcdir)/blibecpgdll.def: exports.txt
	echo '; DEF file for Borland C++ Builder' > $@
	echo 'LIBRARY BLIBECPG' >> $@
	echo 'EXPORTS' >> $@
	sed -e '/^#/d' -e 's/^\(.* \)\([0-9][0-9]*\)/    _\1@ \2/' < $< >> $@
	echo '' >> $@
	echo '; Aliases for MS compatible names' >> $@
	sed -e '/^#/d' -e 's/^\(.* \)\([0-9][0-9]*\)/    \1= _\1/' < $< | sed 's/ *$$//' >> $@

# Where possible, restrict the symbols exported by the library to just the
# official list, so as to avoid unintentional ABI changes.

ifeq ($(PORTNAME), darwin)
$(shlib): exports.list

exports.list: exports.txt
	$(AWK) '/^[^#]/ {printf "_%s\n",$$1}' $< >$@

exported_symbols_list = -exported_symbols_list exports.list
endif

ifeq ($(PORTNAME), linux)
$(shlib): exports.list

exports.list: exports.txt
	echo '{ global:' >$@
	$(AWK) '/^[^#]/ {printf "%s;\n",$$1}' $< >>$@
	echo ' local: *; };' >>$@

exported_symbols_list = -Wl,--version-script=exports.list
endif

install: all installdirs install-lib

installdirs: installdirs-lib

uninstall: uninstall-lib

clean distclean: clean-lib
	rm -f $(OBJS)
<<<<<<< HEAD
	rm -f path.c pgstrcasecmp.c snprintf.c strlcpy.c thread.c

maintainer-clean: distclean maintainer-clean-lib
=======
	rm -f path.c pgstrcasecmp.c snprintf.c strlcpy.c thread.c exports.list

maintainer-clean: distclean
	rm -f $(srcdir)/libecpgdll.def $(srcdir)/blibecpgdll.def
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
