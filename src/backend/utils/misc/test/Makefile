subdir=src/backend/utils/misc
top_builddir=../../../../..
include $(top_builddir)/src/Makefile.global

TARGETS = ps_status bitstream bitmap_compression faultinjector_warnings pg_mkdir_p guc guc_gp

include $(top_builddir)/src/backend/mock.mk

faultinjector_warnings.t: ../faultinjector_warnings.o \
	faultinjector_warnings_test.o \
	$(MOCK_DIR)/backend/utils/fmgr/fmgr_mock.o \
	$(MOCK_DIR)/backend/access/hash/hash_mock.o \
