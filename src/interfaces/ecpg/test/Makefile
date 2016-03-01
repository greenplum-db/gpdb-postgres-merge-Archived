<<<<<<< HEAD
# $PostgreSQL: pgsql/src/interfaces/ecpg/test/Makefile,v 1.77 2009/01/15 09:30:20 petere Exp $
=======
# $PostgreSQL: pgsql/src/interfaces/ecpg/test/Makefile,v 1.72 2007/08/14 10:01:53 meskes Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

subdir = src/interfaces/ecpg/test
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
override CPPFLAGS := \
	-I$(top_builddir)/src/port \
	-I$(top_srcdir)/src/test/regress \
	-Wno-error \
	'-DHOST_TUPLE="$(host_tuple)"' \
	'-DMAKEPROG="$(MAKE)"' \
	'-DSHELLPROG="$(SHELL)"' \
	'-DDLSUFFIX="$(DLSUFFIX)"' \
	$(CPPFLAGS)
=======
# port number for temp-installation test postmaster
# this is also defined in test/connect/Makefile
TEMP_PORT = 4$(DEF_PGPORT)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

# where to find psql for testing an existing installation
PSQLDIR = $(bindir)

# default encoding
MULTIBYTE = SQL_ASCII

# locale
NOLOCALE =
ifdef NO_LOCALE
NOLOCALE += --no-locale
endif

ifneq ($(build_os),mingw32)
abs_builddir := $(shell pwd)
else
abs_builddir := $(shell pwd -W)
endif

# stuff to pass into build of pg_regress
EXTRADEFS = '-DHOST_TUPLE="$(host_tuple)"' \
	'-DMAKEPROG="$(MAKE)"' \
	'-DSHELLPROG="$(SHELL)"' \
	'-DDLSUFFIX="$(DLSUFFIX)"'

REGRESSINCLUDES = "-I$(top_srcdir)/src/test/regress"
REGRESSDRIVER = "$(top_builddir)/src/test/regress/pg_regress.o"

all install installdirs uninstall distprep:
	$(MAKE) -C connect $@
	$(MAKE) -C sql $@
	$(MAKE) -C pgtypeslib $@
	$(MAKE) -C preproc $@
	$(MAKE) -C compat_informix $@
	$(MAKE) -C thread $@

clean distclean maintainer-clean:
	$(MAKE) -C connect $@
	$(MAKE) -C sql $@
	$(MAKE) -C pgtypeslib $@
	$(MAKE) -C preproc $@
	$(MAKE) -C compat_informix $@
	$(MAKE) -C thread $@
	rm -rf tmp_check results log
<<<<<<< HEAD
	rm -f pg_regress regression.diffs regression.out pg_regress_ecpg.o
=======
	rm -f pg_regress regression.diffs pg_regress_ecpg.o
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

# Build regression test driver

all: pg_regress$(X)

<<<<<<< HEAD
pg_regress$(X): pg_regress_ecpg.o $(top_builddir)/src/test/regress/pg_regress.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

$(top_builddir)/src/test/regress/pg_regress.o:
	$(MAKE) -C $(dir $@) $(notdir $@)

# dependencies ensure that path changes propagate
pg_regress_ecpg.o: pg_regress_ecpg.c $(top_builddir)/src/port/pg_config_paths.h
=======
pg_regress$(X): pg_regress_ecpg.o
	$(CC) $(CFLAGS) $^ $(REGRESSDRIVER) $(REGRESSINCLUDES) $(LDFLAGS) $(LIBS) -o $@

# dependencies ensure that path changes propagate
pg_regress_ecpg.o: pg_regress_ecpg.c $(top_builddir)/src/port/pg_config_paths.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(top_builddir)/src/port $(REGRESSINCLUDES) $(EXTRADEFS) -c -o $@ $<
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

$(top_builddir)/src/port/pg_config_paths.h: $(top_builddir)/src/Makefile.global
	$(MAKE) -C $(top_builddir)/src/port pg_config_paths.h

# When doing a VPATH build, copy over the .pgc, .stdout and .stderr
# files so that the driver script can find them.  We have to use an
# absolute path for the targets, because otherwise make will try to
# locate the missing files using VPATH, and will find them in
# $(srcdir), but the point here is that we want to copy them from
# $(srcdir) to the build directory.

ifdef VPATH
remaining_files_src := $(wildcard $(srcdir)/*/*.pgc) $(wildcard $(srcdir)/expected/*.c) $(wildcard $(srcdir)/expected/*.stdout) $(wildcard $(srcdir)/expected/*.stderr)
remaining_files_build := $(patsubst $(srcdir)/%, $(abs_builddir)/%, $(remaining_files_src))

all: $(remaining_files_build)
$(remaining_files_build): $(abs_builddir)/%: $(srcdir)/%
	ln -s $< $@
endif


check: all
<<<<<<< HEAD
	./pg_regress  --dbname=regress1,connectdb --top-builddir=$(top_builddir) --temp-install=./tmp_check --multibyte=$(MULTIBYTE) --load-language=plpgsql $(NOLOCALE) $(THREAD) --schedule=$(srcdir)/ecpg_schedule --create-role=connectuser,connectdb

# the same options, but with --listen-on-tcp
checktcp: all
	./pg_regress  --dbname=regress1,connectdb --top-builddir=$(top_builddir) --temp-install=./tmp_check --multibyte=$(MULTIBYTE) --load-language=plpgsql $(NOLOCALE) $(THREAD) --schedule=$(srcdir)/ecpg_schedule_tcp --create-role=connectuser,connectdb --host=localhost

installcheck: all
	./pg_regress  --psqldir=$(PSQLDIR) --dbname=regress1,connectdb --top-builddir=$(top_builddir) --multibyte=$(MULTIBYTE) --load-language=plpgsql $(NOLOCALE) $(THREAD) --schedule=$(srcdir)/ecpg_schedule --create-role=connectuser,connectdb
=======
	./pg_regress  --dbname=regress1,connectdb --top-builddir=$(top_builddir) --temp-install=./tmp_check --temp-port=$(TEMP_PORT) --multibyte=$(MULTIBYTE) --load-language=plpgsql $(NOLOCALE) $(THREAD) --schedule=$(srcdir)/ecpg_schedule --create-role=connectuser,connectdb

# the same options, but with --listen-on-tcp
checktcp: all
	./pg_regress  --dbname=regress1,connectdb --top-builddir=$(top_builddir) --temp-install=./tmp_check --temp-port=$(TEMP_PORT) --multibyte=$(MULTIBYTE) --load-language=plpgsql $(NOLOCALE) $(THREAD) --schedule=$(srcdir)/ecpg_schedule_tcp --create-role=connectuser,connectdb --host=localhost

installcheck: all
	./pg_regress  --psqldir="$(PSQLDIR)" --dbname=regress1,connectdb --top-builddir=$(top_builddir) --multibyte=$(MULTIBYTE) --load-language=plpgsql $(NOLOCALE) $(THREAD) --schedule=$(srcdir)/ecpg_schedule --create-role=connectuser,connectdb
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
