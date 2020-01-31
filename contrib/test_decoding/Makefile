# contrib/test_decoding/Makefile

MODULES = test_decoding
PGFILEDESC = "test_decoding - example of a logical decoding output plugin"

REGRESS = ddl xact rewrite toast permissions decoding_in_xact \
	decoding_into_rel binary prepared replorigin time messages \
	spill slot truncate
ISOLATION = mxact delayed_startup ondisk_startup concurrent_ddl_dml \
	oldest_xmin snapshot_transfer

REGRESS_OPTS = --temp-config $(top_srcdir)/contrib/test_decoding/logical.conf
ISOLATION_OPTS = --temp-config $(top_srcdir)/contrib/test_decoding/logical.conf

# Disabled because these tests require "wal_level=logical", which
# typical installcheck users do not have (e.g. buildfarm clients).
NO_INSTALLCHECK = 1

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/test_decoding
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

# But it can nonetheless be very helpful to run tests on preexisting
# installation, allow to do so, but only if requested explicitly.
<<<<<<< HEAD
installcheck-force: regresscheck-install-force isolationcheck-install-force

check: regresscheck isolationcheck

submake-regress:
	$(MAKE) -C $(top_builddir)/src/test/regress all

submake-isolation:
	$(MAKE) -C $(top_builddir)/src/test/isolation all

submake-test_decoding:
	$(MAKE) -C $(top_builddir)/contrib/test_decoding

REGRESSCHECKS=ddl xact rewrite toast permissions decoding_in_xact \
	decoding_into_rel binary prepared replorigin time messages \
	spill

regresscheck: | submake-regress submake-test_decoding temp-install
	$(MKDIR_P) regression_output
	$(pg_regress_check) \
	    --temp-config $(top_srcdir)/contrib/test_decoding/logical.conf \
	    --temp-instance=./tmp_check \
	    --outputdir=./regression_output \
	    $(REGRESSCHECKS)

regresscheck-install-force: | submake-regress submake-test_decoding temp-install
	$(pg_regress_installcheck) \
	    $(REGRESSCHECKS)

ISOLATIONCHECKS=mxact delayed_startup ondisk_startup concurrent_ddl_dml \
	oldest_xmin snapshot_transfer

isolationcheck: | submake-isolation submake-test_decoding temp-install
	$(MKDIR_P) isolation_output
	$(pg_isolation_regress_check) \
	    --temp-config $(top_srcdir)/contrib/test_decoding/logical.conf \
	    --outputdir=./isolation_output \
	    $(ISOLATIONCHECKS)

isolationcheck-install-force: all | submake-isolation submake-test_decoding temp-install
	$(pg_isolation_regress_installcheck) \
	    $(ISOLATIONCHECKS)

.PHONY: submake-test_decoding submake-regress check \
	regresscheck regresscheck-install-force \
	isolationcheck isolationcheck-install-force

temp-install: EXTRA_INSTALL=contrib/test_decoding
=======
installcheck-force:
	$(pg_regress_installcheck) $(REGRESS)
	$(pg_isolation_regress_installcheck) $(ISOLATION)
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
