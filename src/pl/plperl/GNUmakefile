# Makefile for PL/Perl
<<<<<<< HEAD
# src/pl/plperl/GNUmakefile
=======
# $PostgreSQL: pgsql/src/pl/plperl/GNUmakefile,v 1.44 2010/05/13 16:39:43 adunstan Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

subdir = src/pl/plperl
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

ifeq ($(perl_useshrplib),true)
shared_libperl = yes
endif
ifeq ($(perl_useshrplib),yes)
shared_libperl = yes
endif

# If we don't have a shared library and the platform doesn't allow it
# to work without, we have to skip it.
ifneq (,$(findstring yes, $(shared_libperl)$(allow_nonpic_in_shlib)))

ifeq ($(PORTNAME), win32)
perl_archlibexp := $(subst \,/,$(perl_archlibexp))
perl_privlibexp := $(subst \,/,$(perl_privlibexp))
perl_lib := $(basename $(notdir $(wildcard $(perl_archlibexp)/CORE/perl[5-9]*.lib)))
perl_embed_ldflags = -L$(perl_archlibexp)/CORE -l$(perl_lib)
override CPPFLAGS += -DPLPERL_HAVE_UID_GID
# Perl on win32 contains /* within comment all over the header file,
# so disable this warning.
override CFLAGS += -Wno-comment
endif

override CPPFLAGS := -I. -I$(srcdir) $(CPPFLAGS) -I$(perl_archlibexp)/CORE

rpathdir = $(perl_archlibexp)/CORE


NAME = plperl

OBJS = plperl.o SPI.o Util.o

<<<<<<< HEAD
DATA = plperl.control plperl--1.0.sql plperl--unpackaged--1.0.sql \
       plperlu.control plperlu--1.0.sql plperlu--unpackaged--1.0.sql

=======
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
PERLCHUNKS = plc_perlboot.pl plc_trusted.pl

SHLIB_LINK = $(perl_embed_ldflags)

<<<<<<< HEAD
REGRESS_OPTS = --dbname=$(PL_TESTDB) --load-language=plperl --load-language=plperlu
REGRESS = plperl plperl_lc plperl_trigger plperl_shared plperl_elog plperl_util plperl_init plperlu plperl_array
# if Perl can support two interpreters in one backend,
=======
REGRESS_OPTS = --dbname=$(PL_TESTDB) --load-language=plperl  --load-language=plperlu
REGRESS = plperl plperl_trigger plperl_shared plperl_elog plperl_util plperl_init plperlu
# if Perl can support two interpreters in one backend, 
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
# test plperl-and-plperlu cases
ifneq ($(PERL),)
ifeq ($(shell $(PERL) -V:usemultiplicity), usemultiplicity='define';)
	REGRESS += plperl_plperlu
endif
endif
<<<<<<< HEAD
# Add Greenplum specific tests
REGRESS += plperl_gp
=======
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
# where to find psql for running the tests
PSQLDIR = $(bindir)

# where to find xsubpp for building XS.
XSUBPPDIR = $(shell $(PERL) -e 'use List::Util qw(first); print first { -r "$$_/ExtUtils/xsubpp" } @INC')

include $(top_srcdir)/src/Makefile.shlib

<<<<<<< HEAD
plperl.o: perlchunks.h plperl_opmask.h plperl_helpers.h

plperl_opmask.h: plperl_opmask.pl
	@if [ x"$(perl_privlibexp)" = x"" ]; then echo "configure switch --with-perl was not specified."; exit 1; fi
	$(PERL) $< $@

perlchunks.h: $(PERLCHUNKS)
	@if [ x"$(perl_privlibexp)" = x"" ]; then echo "configure switch --with-perl was not specified."; exit 1; fi
=======
plperl.o: perlchunks.h plperl_opmask.h

plperl_opmask.h: plperl_opmask.pl
	$(PERL) $< $@

perlchunks.h: $(PERLCHUNKS) 
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
	$(PERL) $(srcdir)/text2macro.pl --strip='^(\#.*|\s*)$$' $^ > $@

all: all-lib

SPI.c: SPI.xs plperl_helpers.h
	@if [ x"$(perl_privlibexp)" = x"" ]; then echo "configure switch --with-perl was not specified."; exit 1; fi
	$(PERL) $(XSUBPPDIR)/ExtUtils/xsubpp -typemap $(perl_privlibexp)/ExtUtils/typemap $< >$@

<<<<<<< HEAD
Util.c: Util.xs plperl_helpers.h
	@if [ x"$(perl_privlibexp)" = x"" ]; then echo "configure switch --with-perl was not specified."; exit 1; fi
	$(PERL) $(XSUBPPDIR)/ExtUtils/xsubpp -typemap $(perl_privlibexp)/ExtUtils/typemap $< >$@


install: all install-lib install-data
=======
Util.c: Util.xs
	$(PERL) $(perl_privlibexp)/ExtUtils/xsubpp -typemap $(perl_privlibexp)/ExtUtils/typemap $< >$@

install: all installdirs install-lib
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

installdirs: installdirs-lib
	$(MKDIR_P) '$(DESTDIR)$(datadir)/extension'

uninstall: uninstall-lib uninstall-data

install-data: installdirs
	$(INSTALL_DATA) $(addprefix $(srcdir)/, $(DATA)) '$(DESTDIR)$(datadir)/extension/'

uninstall-data:
	rm -f $(addprefix '$(DESTDIR)$(datadir)/extension'/, $(notdir $(DATA)))

.PHONY: install-data uninstall-data


check: submake
	$(top_builddir)/src/test/regress/pg_regress --init-file=init_file $(REGRESS_OPTS) $(REGRESS)

installcheck: submake
	$(top_builddir)/src/test/regress/pg_regress --init-file=init_file $(REGRESS_OPTS) $(REGRESS)

.PHONY: submake
submake:
	$(MAKE) -C $(top_builddir)/src/test/regress pg_regress$(X)

clean distclean maintainer-clean: clean-lib
	rm -f SPI.c Util.c $(OBJS) perlchunks.h plperl_opmask.h
<<<<<<< HEAD
	rm -rf $(pg_regress_clean_files)
=======
	rm -rf results
	rm -f regression.diffs regression.out
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

else # can't build

all:
	@echo ""; \
	 echo "*** Cannot build PL/Perl because libperl is not a shared library."; \
	 echo "*** You might have to rebuild your Perl installation.  Refer to"; \
	 echo "*** the documentation for details."; \
	 echo ""

endif # can't build
