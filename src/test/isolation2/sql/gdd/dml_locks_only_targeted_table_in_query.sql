DROP TABLE IF EXISTS part_tbl_upd_del;

-- check gdd is on
show gp_enable_global_deadlock_detector;

CREATE TABLE part_tbl_upd_del (a int, b int, c int) PARTITION BY RANGE(b) (START(1) END(2) EVERY(1));
INSERT INTO part_tbl_upd_del SELECT i, 1, i FROM generate_series(1,10)i;

1: BEGIN;
1: DELETE FROM part_tbl_upd_del;
-- on QD, there's a lock on the root and the target partition
1: SELECT locktype, mode, granted FROM pg_locks WHERE relation='part_tbl_upd_del_1_prt_1'::regclass::oid AND gp_segment_id = -1;
1: SELECT locktype, mode, granted FROM pg_locks WHERE relation='part_tbl_upd_del'::regclass::oid AND gp_segment_id = -1;
1: ROLLBACK;

--
-- The session cannot be reused.
--
-- The macro RELCACHE_FORCE_RELEASE is defined iff USE_ASSERT_CHECKING is
-- defined, and when RELCACHE_FORCE_RELEASE is defined the relcache is
-- forcefully released when closing the relation.
--
-- The function generate_partition_qual() will behave differently depends on
-- the existence of the relcache.
--
-- - if the relation is not cached, it will open it in AccessShareLock mode,
--   and save the relpartbound in the relcache;
-- - if the relation is already cached, it will load the relpartbound from the
--   cache directly without opening the relation;
--
-- So as a result, in the following transactions we will see an extra
-- AccessShareLock lock in a --enable-cassert build compared to a
-- --disable-cassert build.
--
-- To make the test results stable, we do not reuse the sessions in the test,
-- all the tests are performed without the relcache.
--
1q:

1: BEGIN;
1: UPDATE part_tbl_upd_del SET c = 1 WHERE c = 1;
-- on QD, there's a lock on the root and the target partition
1: SELECT locktype, mode, granted FROM pg_locks WHERE relation='part_tbl_upd_del_1_prt_1'::regclass::oid AND gp_segment_id = -1;
1: SELECT locktype, mode, granted FROM pg_locks WHERE relation='part_tbl_upd_del'::regclass::oid AND gp_segment_id = -1;
1: ROLLBACK;
1q:

1: BEGIN;
1: DELETE FROM part_tbl_upd_del_1_prt_1;
-- since the delete operation is on leaf part, there will be a lock on QD
1: SELECT locktype, mode, granted FROM pg_locks WHERE relation='part_tbl_upd_del_1_prt_1'::regclass::oid AND gp_segment_id = -1 AND mode='RowExclusiveLock';
1: ROLLBACK;
1q:

1: BEGIN;
1: UPDATE part_tbl_upd_del_1_prt_1 SET c = 1 WHERE c = 1;
-- since the update operation is on leaf part, there will be a lock on QD
1: SELECT locktype, mode, granted FROM pg_locks WHERE relation='part_tbl_upd_del_1_prt_1'::regclass::oid AND gp_segment_id = -1 AND mode='RowExclusiveLock';
1: ROLLBACK;
1q:
