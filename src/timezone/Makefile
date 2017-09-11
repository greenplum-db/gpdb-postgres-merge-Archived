#-------------------------------------------------------------------------
#
# Makefile
#    Makefile for the timezone library

# IDENTIFICATION
<<<<<<< HEAD
#    $PostgreSQL: pgsql/src/timezone/Makefile,v 1.31 2009/01/06 02:25:29 momjian Exp $
=======
#    $PostgreSQL: pgsql/src/timezone/Makefile,v 1.29 2008/02/19 15:29:58 petere Exp $
>>>>>>> 0f855d621b
#
#-------------------------------------------------------------------------

subdir = src/timezone
top_builddir = ../..
include $(top_builddir)/src/Makefile.global

override CPPFLAGS := $(CPPFLAGS)
override CFLAGS := $(CFLAGS) -O1

# files to build into backend
OBJS= localtime.o strftime.o pgtz.o gptime.o

# files needed to build zic utility program
ZICOBJS= zic.o ialloc.o scheck.o localtime.o

# timezone data files
TZDATA = africa antarctica asia australasia europe northamerica southamerica \
	pacificnew etcetera factory backward systemv solar87 solar88 solar89
TZDATAFILES = $(TZDATA:%=$(srcdir)/data/%)

# which zone should determine the DST rules (not the specific UTC offset!)
# for POSIX-style timezone specs
POSIXRULES = US/Eastern

# use system timezone data?
ifneq (,$(with_system_tzdata))
override CPPFLAGS += '-DSYSTEMTZDIR="$(with_system_tzdata)"'
endif

include $(top_srcdir)/src/backend/common.mk

ifeq (,$(with_system_tzdata))
all: submake-libpgport zic
endif

<<<<<<< HEAD
# We could do this test in the action section:
#	$(if $(ZIC),$(ZIC),./zic)
# but gmake versions <= 3.78.1 or perhaps later have a bug
# that causes a segfault;  gmake 3.81 or later fixes this.
ifeq (,$(ZIC))
ZIC= ./zic
endif

=======
>>>>>>> 0f855d621b
zic: $(ZICOBJS)
	$(CC) $(CFLAGS) $(ZICOBJS) $(LDFLAGS) $(LDFLAGS_EX) $(LIBS) -o $@$(X)

install: all installdirs
ifeq (,$(with_system_tzdata))
	$(ZIC) -d '$(DESTDIR)$(datadir)/timezone' -p '$(POSIXRULES)' $(TZDATAFILES)
endif
	$(MAKE) -C tznames $@

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(datadir)'

uninstall:
ifeq (,$(with_system_tzdata))
	rm -rf '$(DESTDIR)$(datadir)/timezone'
endif
	$(MAKE) -C tznames $@

clean distclean maintainer-clean:
<<<<<<< HEAD
	rm -f zic$(X) $(ZICOBJS) gmon.out
=======
	rm -f zic$(X) $(ZICOBJS)
>>>>>>> 0f855d621b
