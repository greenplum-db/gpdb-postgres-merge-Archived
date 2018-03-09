#
# Common make rules for backend
#
# src/backend/common.mk
#

# When including this file, set OBJS to the object files created in
# this directory and SUBDIRS to subdirectories containing more things
# to build.

ifdef PARTIAL_LINKING
# old style: linking using SUBSYS.o
subsysfilename = SUBSYS.o
else
# new style: linking all object files at once
subsysfilename = objfiles.txt
endif

SUBDIROBJS = $(SUBDIRS:%=%/$(subsysfilename))

# top-level backend directory obviously has its own "all" target
ifneq ($(subdir), src/backend)
all: $(subsysfilename)
endif

SUBSYS.o: $(SUBDIROBJS) $(OBJS)
	$(LD) $(LDREL) $(LDOUT) $@ $^ $(LDOPTS)

objfiles.txt: Makefile $(SUBDIROBJS) $(OBJS)
# Don't rebuild the list if only the OBJS have changed.
	$(if $(filter-out $(OBJS),$?),( $(if $(SUBDIROBJS),cat $(SUBDIROBJS); )echo $(addprefix $(subdir)/,$(OBJS)) ) >$@,touch $@)

# make function to expand objfiles.txt contents
expand_subsys = $(foreach file,$(1),$(if $(filter %/objfiles.txt,$(file)),$(patsubst ../../src/backend/%,%,$(addprefix $(top_builddir)/,$(shell cat $(file)))),$(file)))

# Parallel make trickery
$(SUBDIROBJS): $(SUBDIRS:%=%-recursive) ;

.PHONY: $(SUBDIRS:%=%-recursive)
$(SUBDIRS:%=%-recursive):
	$(MAKE) -C $(subst -recursive,,$@) all

$(call recurse,clean)
clean: clean-local
clean-local:
	rm -f $(subsysfilename) $(OBJS)
<<<<<<< HEAD
	@if [ -d $(CURDIR)/test ]; then $(MAKE) -C $(CURDIR)/test clean; fi

.PHONY : unittest-check
unittest-check:
	@if [ -d $(CURDIR)/test ]; then $(MAKE) -C $(CURDIR)/test check; fi
ifdef SUBDIRS
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir unittest-check || exit; done
endif
=======

$(call recurse,coverage)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
