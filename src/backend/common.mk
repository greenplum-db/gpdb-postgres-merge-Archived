#
# Common make rules for backend
#
# src/backend/common.mk
#

# When including this file, set OBJS to the object files created in
# this directory and SUBDIRS to subdirectories containing more things
# to build.

subsysfilename = objfiles.txt

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

ifeq ($(with_llvm), yes)
objfiles.txt: $(patsubst %.o,%.bc, $(OBJS))
endif

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
<<<<<<< HEAD
	rm -f $(subsysfilename) $(OBJS)
	@if [ -d $(CURDIR)/test ]; then $(MAKE) -C $(CURDIR)/test clean; fi

$(call recurse,unittest-check)
unittest-check: unittest-check-local
unittest-check-local:
	@if [ -d $(CURDIR)/test ]; then $(MAKE) -C $(CURDIR)/test check; fi
=======
	rm -f $(subsysfilename) $(OBJS) $(patsubst %.o,%.bc, $(OBJS))
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

$(call recurse,coverage)
$(call recurse,install)
