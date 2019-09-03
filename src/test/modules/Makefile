# src/test/modules/Makefile

subdir = src/test/modules
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

SUBDIRS = \
		  brin \
		  commit_ts \
		  dummy_seclabel \
		  snapshot_too_old \
		  test_ddl_deparse \
		  test_extensions \
		  test_parser \
		  test_pg_dump \
		  test_rls_hooks \
		  test_shm_mq \
		  worker_spi

<<<<<<< HEAD
# GPDB subdirs
SUBDIRS += test_planner

all: submake-errcodes

submake-errcodes:
	$(MAKE) -C $(top_builddir)/src/backend submake-errcodes
=======
all: submake-generated-headers
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

$(recurse)
