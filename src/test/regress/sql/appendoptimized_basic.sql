create table ao_basic_t1 (a int, b varchar) using appendoptimized distributed by (a);

-- Validate that the appendoptimized table access method will be used
-- for this table.
select amhandler from pg_class c, pg_am a where c.relname = 'ao_basic_t1' and c.relam = a.oid;
select count(*) = 1 from pg_appendonly where relid = 'ao_basic_t1'::regclass;

insert into ao_basic_t1 values (1, 'abc'), (2, 'pqr'), (3, 'lmn');

insert into ao_basic_t1 select i, i from generate_series(1,12)i;

select * from ao_basic_t1;

select gp_segment_id, * from ao_basic_t1;

select gp_segment_id, count(*) from ao_basic_t1 group by gp_segment_id;

insert into ao_basic_t1 values (1, repeat('abc', 100000));

select a, length(b) from ao_basic_t1;

create index i_ao_basic_t1 on ao_basic_t1 using btree(a);
select * from ao_basic_t1 where a = 2;

create table heap_t2(a int, b varchar) distributed by (a);
insert into heap_t2 select i, i from generate_series(1, 20)i;

select * from ao_basic_t1 t1 join heap_t2 t2 on t1.a=t2.a where t1.a != 1;


