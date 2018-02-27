-- gp_dump_query_oids() doesn't list built-in functions, so we need a UDF to test with.
CREATE FUNCTION myfunc(t text) RETURNS integer AS $$ SELECT 123 $$ LANGUAGE SQL;
CREATE FUNCTION myfunc2(t text) RETURNS integer AS $$ SELECT 123 $$ LANGUAGE SQL;

-- The function returns OIDs. We need to replace them with something reproducable.
CREATE FUNCTION sanitize_output(t text) RETURNS text AS $$
declare
  myfunc_oid oid;
  myfunc2_oid oid;
begin
    myfunc_oid = 'myfunc'::regproc::oid;
    myfunc2_oid = 'myfunc2'::regproc::oid;

    t := replace(t, myfunc_oid::text, '<myfunc>');
    t := replace(t, myfunc2_oid::text, '<myfunc2>');

    RETURN t;
end;
$$ LANGUAGE plpgsql;

-- Test the built-in gp_dump_query function.
SELECT sanitize_output(gp_dump_query_oids('SELECT 123'));
SELECT sanitize_output(gp_dump_query_oids('SELECT * FROM pg_proc'));
SELECT sanitize_output(gp_dump_query_oids('SELECT length(proname) FROM pg_proc'));
SELECT sanitize_output(gp_dump_query_oids('SELECT myfunc(proname) FROM pg_proc'));

-- with EXPLAIN
SELECT sanitize_output(gp_dump_query_oids('explain SELECT myfunc(proname) FROM pg_proc'));

-- with a multi-query statement
SELECT sanitize_output(gp_dump_query_oids('SELECT myfunc(proname) FROM pg_proc; SELECT myfunc2(relname) FROM pg_class'));

-- Test error reporting on an invalid query.
SELECT gp_dump_query_oids('SELECT * FROM nonexistent_table');
SELECT gp_dump_query_oids('SELECT with syntax error');

-- Test partition and inherited tables
CREATE TABLE minirepro_partition_test (id int, info json);
CREATE TABLE foo (id int, year int, a int, b int, c int, d int, region text)
 DISTRIBUTED BY (id)
 PARTITION BY RANGE (year)
     SUBPARTITION BY RANGE (a)
        SUBPARTITION TEMPLATE (
         START (1) END (2) EVERY (1),
         DEFAULT SUBPARTITION other_a )
            SUBPARTITION BY RANGE (b)
               SUBPARTITION TEMPLATE (
               START (1) END (2) EVERY (1),
               DEFAULT SUBPARTITION other_b )
( START (2002) END (2003) EVERY (1),
   DEFAULT PARTITION outlying_years );
CREATE TABLE ptable (c1 text, c2 float);
CREATE TABLE ctable (c3 char(2)) INHERITS (ptable);
CREATE TABLE cctable (c4 char(2)) INHERITS (ctable);
INSERT INTO minirepro_partition_test VALUES (1, (select gp_dump_query_oids('SELECT * FROM foo') ) :: json);
INSERT INTO minirepro_partition_test VALUES (2, (select gp_dump_query_oids('SELECT * FROM ptable') ) :: json);
INSERT INTO minirepro_partition_test VALUES (3, (select gp_dump_query_oids('SELECT * FROM pg_class') ) :: json);
SELECT array['foo'::regclass::oid::text,
             'foo_1_prt_outlying_years'::regclass::oid::text,
             'foo_1_prt_outlying_years_2_prt_other_a'::regclass::oid::text,
             'foo_1_prt_outlying_years_2_prt_other_a_3_prt_other_b'::regclass::oid::text,
             'foo_1_prt_outlying_years_2_prt_other_a_3_prt_2'::regclass::oid::text,
             'foo_1_prt_outlying_years_2_prt_2'::regclass::oid::text,
             'foo_1_prt_outlying_years_2_prt_2_3_prt_other_b'::regclass::oid::text,
             'foo_1_prt_outlying_years_2_prt_2_3_prt_2'::regclass::oid::text,
             'foo_1_prt_2'::regclass::oid::text,
             'foo_1_prt_2_2_prt_other_a'::regclass::oid::text,
             'foo_1_prt_2_2_prt_other_a_3_prt_other_b'::regclass::oid::text,
             'foo_1_prt_2_2_prt_other_a_3_prt_2'::regclass::oid::text,
             'foo_1_prt_2_2_prt_2'::regclass::oid::text,
             'foo_1_prt_2_2_prt_2_3_prt_other_b'::regclass::oid::text,
             'foo_1_prt_2_2_prt_2_3_prt_2'::regclass::oid::text] <@  (string_to_array((SELECT info->>'relids' FROM minirepro_partition_test WHERE id = 1),','));
SELECT array['ptable'::regclass::oid::text,
             'ctable'::regclass::oid::text,
             'cctable'::regclass::oid::text] <@  (string_to_array((SELECT info->>'relids' FROM minirepro_partition_test WHERE id = 2),','));
SELECT array['pg_class'::regclass::oid::text] <@  (string_to_array((SELECT info->>'relids' FROM minirepro_partition_test WHERE id = 3),','));
DROP TABLE foo;
DROP TABLE cctable;
DROP TABLE ctable;
DROP TABLE ptable;
DROP TABLE minirepro_partition_test;
