create table ao_basic_t1 (a int, b varchar) using appendoptimized distributed by (a);

insert into ao_basic_t1 values (1, 'abc'), (2, 'pqr'), (3, 'lmn');

insert into ao_basic_t1 select i, i from generate_series(1,12)i;

select * from ao_basic_t1;

select gp_segment_id, * from ao_basic_t1;

select gp_segment_id, count(*) from ao_basic_t1 group by gp_segment_id;

insert into ao_basic_t1 values (1, repeat('abc', 100000));

select a, length(b) from ao_basic_t1;
