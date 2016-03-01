#-------------------------------------------------------------------------
#
# Makefile for src/bin/pgevent
#
<<<<<<< HEAD
# Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
# Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

PGFILEDESC = "Eventlog message formatter"
PGFILESHLIB = 1
subdir = src/bin/pgevent
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

ifeq ($(PORTNAME), win32)

OBJS=pgevent.o pgmsgevent.o
NAME=pgevent.dll

all: $(NAME)

install: all install-lib

<<<<<<< HEAD
pgevent.dll: pgevent.def $(OBJS)
	$(DLLWRAP) --def $< -o $(NAME) $(OBJS)
=======
pgevent.dll: $(OBJS) pgevent.def
	$(DLLWRAP) --def pgevent.def -o $(NAME) $(OBJS)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

pgmsgevent.o: pgmsgevent.rc win32ver.rc
	$(WINDRES) $< -o $@ --include-dir=$(top_builddir)/src/include --include-dir=$(top_srcdir)/src/include --include-dir=$(srcdir)

all-lib: $(NAME)

install-lib: $(NAME)
	$(INSTALL_STLIB) $< '$(DESTDIR)$(libdir)/$<'

uninstall-lib:
	rm -f '$(DESTDIR)$(libdir)/$(NAME)'

clean distclean:
	rm -f $(OBJS) $(NAME) win32ver.rc

clean-lib:
	rm -f $(NAME)

endif
