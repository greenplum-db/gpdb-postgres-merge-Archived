-- This file contains extra Greenplum tests for partitioning. These tests are
-- focused on the upstream partitioning commands like CREATE TABLE PARTITION
-- OF and ATTACH/DETACH, in corner cases that not covered by upstream
-- tests.


-- Test with different distribution policies
CREATE TABLE tpart (a int, b int) PARTITION BY RANGE (a) DISTRIBUTED BY (b);
CREATE TABLE tpart_ten PARTITION OF tpart FOR VALUES FROM (0) TO (10);

CREATE TABLE tpart_twenty (b int, a int) DISTRIBUTED BY (b);
ALTER TABLE tpart ATTACH PARTITION tpart_twenty FOR VALUES FROM (10) TO (20);

-- fail: distribution keys must match between parent and partition.
CREATE TABLE tpart_thirty (b int, a int) DISTRIBUTED BY (a);
ALTER TABLE tpart ATTACH PARTITION tpart_thirty FOR VALUES FROM (20) TO (30);

ALTER TABLE tpart_thirty SET DISTRIBUTED RANDOMLY;
ALTER TABLE tpart ATTACH PARTITION tpart_thirty FOR VALUES FROM (20) TO (30);

ALTER TABLE tpart_thirty SET DISTRIBUTED REPLICATED;
ALTER TABLE tpart ATTACH PARTITION tpart_thirty FOR VALUES FROM (20) TO (30);

ALTER TABLE tpart_thirty SET DISTRIBUTED BY (b, a);
ALTER TABLE tpart ATTACH PARTITION tpart_thirty FOR VALUES FROM (20) TO (30);

ALTER TABLE tpart_thirty SET DISTRIBUTED BY (b);
-- now it works
ALTER TABLE tpart ATTACH PARTITION tpart_thirty FOR VALUES FROM (20) TO (30);

-- Also try with different replication policies in the parent.

CREATE TABLE tpart_rand (a int, b int) PARTITION BY RANGE (a) DISTRIBUTED RANDOMLY;
CREATE TABLE tpart_rand_ten PARTITION OF tpart_rand FOR VALUES FROM (00) TO (10);

CREATE TABLE tpart_rand_twenty (b int, a int) DISTRIBUTED RANDOMLY;
ALTER TABLE tpart_rand ATTACH PARTITION tpart_rand_twenty FOR VALUES FROM (10) TO (20);

CREATE TABLE tpart_rpt (a int, b int) PARTITION BY RANGE (a) DISTRIBUTED REPLICATED;
CREATE TABLE tpart_rpt_ten PARTITION OF tpart_rpt FOR VALUES FROM (0) TO (10);

CREATE TABLE tpart_rpt_twenty (b int, a int) DISTRIBUTED REPLICATED;
ALTER TABLE tpart_rpt ATTACH PARTITION tpart_rpt_twenty FOR VALUES FROM (10) TO (20);

insert into tpart_rpt select g, g from generate_series(5,14) g;
select * from tpart_rpt_ten;
select * from tpart_rpt_twenty;

delete from tpart_rpt where a = 7 or a = 12;

-- Test row movement across partitions in replicated table
update tpart_rpt set a = 20-a ;
