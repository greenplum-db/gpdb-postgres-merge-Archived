#-------------------------------------------------------------------------
#
# Makefile for src/test/perl
#
<<<<<<< HEAD
# Portions Copyright (c) 1996-2017, PostgreSQL Global Development Group
=======
# Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
>>>>>>> 8bc709b37411ba7ad0fd0f1f79c354714424af3d
# Portions Copyright (c) 1994, Regents of the University of California
#
# src/test/perl/Makefile
#
#-------------------------------------------------------------------------

subdir = src/test/perl
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

ifeq ($(enable_tap_tests),yes)
<<<<<<< HEAD
=======

>>>>>>> 8bc709b37411ba7ad0fd0f1f79c354714424af3d
installdirs:
	$(MKDIR_P) '$(DESTDIR)$(pgxsdir)/$(subdir)'

install: all installdirs
	$(INSTALL_DATA) $(srcdir)/TestLib.pm '$(DESTDIR)$(pgxsdir)/$(subdir)/TestLib.pm'
	$(INSTALL_DATA) $(srcdir)/SimpleTee.pm '$(DESTDIR)$(pgxsdir)/$(subdir)/SimpleTee.pm'
<<<<<<< HEAD
	$(INSTALL_DATA) $(srcdir)/RecursiveCopy.pm '$(DESTDIR)$(pgxsdir)/$(subdir)/RecursiveCopy.pm'
	$(INSTALL_DATA) $(srcdir)/PostgresNode.pm '$(DESTDIR)$(pgxsdir)/$(subdir)/PostgresNode.pm'
=======
>>>>>>> 8bc709b37411ba7ad0fd0f1f79c354714424af3d

uninstall:
	rm -f '$(DESTDIR)$(pgxsdir)/$(subdir)/TestLib.pm'
	rm -f '$(DESTDIR)$(pgxsdir)/$(subdir)/SimpleTee.pm'
<<<<<<< HEAD
	rm -f '$(DESTDIR)$(pgxsdir)/$(subdir)/RecursiveCopy.pm'
	rm -f '$(DESTDIR)$(pgxsdir)/$(subdir)/PostgresNode.pm'
=======

>>>>>>> 8bc709b37411ba7ad0fd0f1f79c354714424af3d
endif
