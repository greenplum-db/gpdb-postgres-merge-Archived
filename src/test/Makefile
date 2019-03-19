#-------------------------------------------------------------------------
#
# Makefile for src/test
#
# Copyright (c) 1994, Regents of the University of California
#
# src/test/Makefile
#
#-------------------------------------------------------------------------

subdir = src/test
top_builddir = ../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
SUBDIRS = perl regress isolation

SUBDIRS += fsync walrep heap_checksum isolation2 kerberos modules
=======
SUBDIRS = regress isolation modules
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8

# We don't build or execute examples/, locale/, or thread/ by default,
# but we do want "make clean" etc to recurse into them.  Likewise for ssl/,
# because the SSL test suite is not secure to run on a multi-user system.
ALWAYS_SUBDIRS = examples locale thread ssl

# We want to recurse to all subdirs for all standard targets, except that
# installcheck and install should not recurse into the subdirectory "modules".

recurse_alldirs_targets := $(filter-out installcheck install, $(standard_targets))
installable_dirs := $(filter-out modules, $(SUBDIRS))

$(call recurse,$(recurse_alldirs_targets))
$(call recurse,installcheck, $(installable_dirs))
$(call recurse,install, $(installable_dirs))

$(recurse_always)
