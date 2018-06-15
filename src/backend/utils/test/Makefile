subdir=src/backend/utils
top_builddir=../../../..
include $(top_builddir)/src/Makefile.global

# GPDB_92_MERGE_FIXME : IF use -O3, there will be some confusing
# errors cause by link

CFLAGS += -O0

TARGETS=session_state

include $(top_builddir)/src/backend/mock.mk

session_state.t: \
	$(MOCK_DIR)/backend/storage/ipc/shmem_mock.o \
	$(MOCK_DIR)/backend/storage/lmgr/lwlock_mock.o \
	$(MOCK_DIR)/backend/utils/error/assert_mock.o \
	$(MOCK_DIR)/backend/utils/error/elog_mock.o \
	$(MOCK_DIR)/backend/utils/mmgr/runaway_cleaner_mock.o
