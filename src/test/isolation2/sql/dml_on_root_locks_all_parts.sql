-- For partitioned tables if a DML is run on the root table, we should
-- take ExclusiveLock on the root and RowExclusive the child partitions,
-- which same as upstream strategy. Since even a concurrent DML on the 
-- leaf partition, the second transaction requires ExclusiveLock for leaf
-- partition.
--      Note: Same applies to UPDATE as well.

DROP TABLE IF EXISTS part_tbl;
CREATE TABLE part_tbl (a int, b int, c int) PARTITION BY RANGE (b) (start(1) end(2) every(1));

INSERT INTO part_tbl SELECT i, 1, i FROM generate_series(1,12)i;

1: BEGIN;
-- DELETE will acquire Exclusive lock on root and leaf partition on QD.
1: DELETE FROM part_tbl;

-- Delete must hold an exclusive lock on the leaf partition on QD.
SELECT GRANTED FROM pg_locks WHERE relation = 'part_tbl_1_prt_1'::regclass::oid AND mode='RowExclusiveLock' AND gp_segment_id=-1 AND locktype='relation';

-- DELETE on the leaf partition must wait on the QD as Session 1 already holds ExclusiveLock.
2&: DELETE FROM part_tbl_1_prt_1 WHERE b = 10;
SELECT relation::regclass, mode, granted FROM pg_locks WHERE gp_segment_id=-1 AND granted='f';

1: COMMIT;
2<:

1: BEGIN;
-- UPDATE will acquire Exclusive lock on root and leaf partition on QD.
1: UPDATE part_tbl SET c = 1; 
SELECT GRANTED FROM pg_locks WHERE relation = 'part_tbl_1_prt_1'::regclass::oid AND mode='RowExclusiveLock' AND gp_segment_id=-1 AND locktype='relation';

-- UPDATE on leaf must be blocked on QD as previous UPDATE acquires Exclusive lock on the root and the leaf partitions
2&: UPDATE part_tbl_1_prt_1 set c = 10;
SELECT relation::regclass, mode, granted FROM pg_locks WHERE gp_segment_id=-1 AND granted='f';

1: COMMIT;
2<:
