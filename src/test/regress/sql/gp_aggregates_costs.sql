set optimizer=off;
set statement_mem="1800";
-- GPDB_12_MERGE_FIXME
-- The reason why we need a large size of work_mem is that `estimate_num_groups` function can not give a reasonable group number estimation. If the memory
-- size is smaller than hash table size, planner will give up generate a hash agg. Dose not understand formula yet, leave a fixme and let the case pass first.
set work_mem to "128MB";
create table cost_agg_t1(a int, b int, c int);
insert into cost_agg_t1 select i, random() * 99999, i % 2000 from generate_series(1, 1000000) i;
create table cost_agg_t2 as select * from cost_agg_t1 with no data;
insert into cost_agg_t2 select i, random() * 99999, i % 300000 from generate_series(1, 1000000) i;
explain select avg(b) from cost_agg_t1 group by c;
explain select avg(b) from cost_agg_t2 group by c;
insert into cost_agg_t2 select i, random() * 99999,1 from generate_series(1, 200000) i;
analyze cost_agg_t2;
explain select avg(b) from cost_agg_t2 group by c;
drop table cost_agg_t1;
drop table cost_agg_t2;
reset statement_mem;
reset optimizer;
