#-------------------------------------------------------------------------
#
# Makefile for ecpg compatibility library
#
<<<<<<< HEAD
# Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/src/interfaces/ecpg/compatlib/Makefile,v 1.42 2009/01/01 17:24:02 momjian Exp $
=======
# Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/src/interfaces/ecpg/compatlib/Makefile,v 1.37 2008/01/01 19:45:59 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

subdir = src/interfaces/ecpg/compatlib
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

# The frontend doesn't need everything that's in LIBS, some are backend only
LIBS := $(filter-out -lresolv -lbz2, $(LIBS))
# This program isn't interactive, so doesn't need these
LIBS := $(filter-out -lreadline -ledit -ltermcap -lncurses -lcurses -lcurl -lssl -lcrypto -lz, $(LIBS))

NAME= ecpg_compat
SO_MAJOR_VERSION= 3
<<<<<<< HEAD
SO_MINOR_VERSION= 1
=======
SO_MINOR_VERSION= 0
DLTYPE= library
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

override CPPFLAGS := -I../include -I$(top_srcdir)/src/interfaces/ecpg/include \
	-I$(libpq_srcdir) -I$(top_srcdir)/src/include/utils $(CPPFLAGS)
override CFLAGS += $(PTHREAD_CFLAGS)
<<<<<<< HEAD

SHLIB_LINK = -L../ecpglib -lecpg -L../pgtypeslib -lpgtypes $(libpq) \
	$(filter -lintl -lm, $(LIBS)) $(PTHREAD_LIBS)

# For a bug in gcc on centos 32-bit
ifeq ($(GCC), yes)
 CFLAGS += -Wno-error
endif # GCC

SHLIB_EXPORTS = exports.txt

=======

SHLIB_LINK = -L../ecpglib -lecpg -L../pgtypeslib -lpgtypes $(libpq) -lm \
	$(PTHREAD_LIBS)

>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
# Need to recompile any libpgport object files
LIBS := $(filter-out -lpgport, $(LIBS))

OBJS= informix.o $(filter snprintf.o, $(LIBOBJS))
<<<<<<< HEAD
=======

ifeq ($(PORTNAME), win32)
DLL_DEFFILE=libecpg_compatdll.def
endif
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

all: def-files all-lib

# Shared library stuff
include $(top_srcdir)/src/Makefile.shlib

snprintf.c: % : $(top_srcdir)/src/port/%
	rm -f $@ && $(LN_S) $< .

<<<<<<< HEAD
=======
# We need several not-quite-identical variants of .DEF files to build libecpg
# DLLs for Windows.  These are made from the single source file exports.txt.
# Since we can't assume that Windows boxes will have sed, the .DEF files are
# always built and included in distribution tarballs.

distprep: def-files

.PHONY: def-files

def-files: $(srcdir)/libecpg_compatdll.def $(srcdir)/blibecpg_compatdll.def

$(srcdir)/libecpg_compatdll.def: exports.txt
	echo '; DEF file for MS VC++' > $@
	echo 'LIBRARY LIBECPG_COMPAT' >> $@
	echo 'EXPORTS' >> $@
	sed -e '/^#/d' -e 's/^\(.* \)\([0-9][0-9]*\)/    \1@ \2/' < $< >> $@

$(srcdir)/blibecpg_compatdll.def: exports.txt
	echo '; DEF file for Borland C++ Builder' > $@
	echo 'LIBRARY BLIBECPG_COMPAT' >> $@
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

>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
install: all installdirs install-lib

installdirs: installdirs-lib

uninstall: uninstall-lib

clean distclean: clean-lib
<<<<<<< HEAD
	rm -f $(OBJS) snprintf.c

maintainer-clean: distclean maintainer-clean-lib
=======
	rm -f $(OBJS) snprintf.c exports.list

maintainer-clean: distclean
	rm -f $(srcdir)/libecpg_compatdll.def $(srcdir)/blibecpg_compatdll.def
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
