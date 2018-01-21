-- first some tests of basic functionality
CREATE LANGUAGE plpython2u;

-- really stupid function just to get the module loaded
CREATE FUNCTION stupid() RETURNS text AS 'return "zarkon"' LANGUAGE plpythonu;

select stupid();

-- check 2/3 versioning
CREATE FUNCTION stupidn() RETURNS text AS 'return "zarkon"' LANGUAGE plpython2u;

select stupidn();

-- test multiple arguments
CREATE FUNCTION argument_test_one(u users, a1 text, a2 text) RETURNS text
	AS
'keys = list(u.keys())
keys.sort()
out = []
for key in keys:
    out.append("%s: %s" % (key, u[key]))
words = a1 + " " + a2 + " => {" + ", ".join(out) + "}"
return words'
	LANGUAGE plpythonu;

select argument_test_one(users, fname, lname) from users where lname = 'doe' order by 1;


<<<<<<< HEAD
-- spi and nested calls
--
select nested_call_one('pass this along');
select spi_prepared_plan_test_one('doe');
select spi_prepared_plan_test_one('smith');
select spi_prepared_plan_test_nested('smith');

-- quick peek at the table
--
SELECT * FROM users order by userid;

-- should fail
--
UPDATE users SET fname = 'william' WHERE fname = 'willem';

-- should modify william to willem and create username
--
INSERT INTO users (fname, lname) VALUES ('william', 'smith');
INSERT INTO users (fname, lname, username) VALUES ('charles', 'darwin', 'beagle');

SELECT * FROM users order by userid;

-- Greenplum doesn't support functions that execute SQL from segments
--
--  SELECT join_sequences(sequences) FROM sequences;
--  SELECT join_sequences(sequences) FROM sequences
--  	WHERE join_sequences(sequences) ~* '^A';
--  SELECT join_sequences(sequences) FROM sequences
--  	WHERE join_sequences(sequences) ~* '^B';

SELECT result_nrows_test();

-- test of exception handling
SELECT queryexec('SELECT 2'); 

SELECT queryexec('SELECT X'); 


select module_contents();

SELECT elog_test();


-- error in trigger
--

--
-- Check Universal Newline Support
--

SELECT newline_lf();
SELECT newline_cr();
SELECT newline_crlf();

-- Tests for functions returning void
SELECT test_void_func1(), test_void_func1() IS NULL AS "is null";
SELECT test_void_func2(); -- should fail
SELECT test_return_none(), test_return_none() IS NULL AS "is null";

-- Test for functions with named and nameless parameters
SELECT test_param_names0(2,7);
SELECT test_param_names1(1,'text');
SELECT test_param_names2(users) from users;
SELECT test_param_names3(1);

-- Test set returning functions
SELECT test_setof_as_list(0, 'list');
SELECT test_setof_as_list(1, 'list');
SELECT test_setof_as_list(2, 'list');
SELECT test_setof_as_list(2, null);

SELECT test_setof_as_tuple(0, 'tuple');
SELECT test_setof_as_tuple(1, 'tuple');
SELECT test_setof_as_tuple(2, 'tuple');
SELECT test_setof_as_tuple(2, null);

SELECT test_setof_as_iterator(0, 'list');
SELECT test_setof_as_iterator(1, 'list');
SELECT test_setof_as_iterator(2, 'list');
SELECT test_setof_as_iterator(2, null);

SELECT test_setof_spi_in_iterator();

-- Test tuple returning functions
SELECT * FROM test_table_record_as('dict', null, null, false);
SELECT * FROM test_table_record_as('dict', 'one', null, false);
SELECT * FROM test_table_record_as('dict', null, 2, false);
SELECT * FROM test_table_record_as('dict', 'three', 3, false);
SELECT * FROM test_table_record_as('dict', null, null, true);

SELECT * FROM test_table_record_as('tuple', null, null, false);
SELECT * FROM test_table_record_as('tuple', 'one', null, false);
SELECT * FROM test_table_record_as('tuple', null, 2, false);
SELECT * FROM test_table_record_as('tuple', 'three', 3, false);
SELECT * FROM test_table_record_as('tuple', null, null, true);

SELECT * FROM test_table_record_as('list', null, null, false);
SELECT * FROM test_table_record_as('list', 'one', null, false);
SELECT * FROM test_table_record_as('list', null, 2, false);
SELECT * FROM test_table_record_as('list', 'three', 3, false);
SELECT * FROM test_table_record_as('list', null, null, true);

SELECT * FROM test_table_record_as('obj', null, null, false);
SELECT * FROM test_table_record_as('obj', 'one', null, false);
SELECT * FROM test_table_record_as('obj', null, 2, false);
SELECT * FROM test_table_record_as('obj', 'three', 3, false);
SELECT * FROM test_table_record_as('obj', null, null, true);

SELECT * FROM test_type_record_as('dict', null, null, false);
SELECT * FROM test_type_record_as('dict', 'one', null, false);
SELECT * FROM test_type_record_as('dict', null, 2, false);
SELECT * FROM test_type_record_as('dict', 'three', 3, false);
SELECT * FROM test_type_record_as('dict', null, null, true);

SELECT * FROM test_type_record_as('tuple', null, null, false);
SELECT * FROM test_type_record_as('tuple', 'one', null, false);
SELECT * FROM test_type_record_as('tuple', null, 2, false);
SELECT * FROM test_type_record_as('tuple', 'three', 3, false);
SELECT * FROM test_type_record_as('tuple', null, null, true);

SELECT * FROM test_type_record_as('list', null, null, false);
SELECT * FROM test_type_record_as('list', 'one', null, false);
SELECT * FROM test_type_record_as('list', null, 2, false);
SELECT * FROM test_type_record_as('list', 'three', 3, false);
SELECT * FROM test_type_record_as('list', null, null, true);

SELECT * FROM test_type_record_as('obj', null, null, false);
SELECT * FROM test_type_record_as('obj', 'one', null, false);
SELECT * FROM test_type_record_as('obj', null, 2, false);
SELECT * FROM test_type_record_as('obj', 'three', 3, false);
SELECT * FROM test_type_record_as('obj', null, null, true);

SELECT * FROM test_in_out_params('test_in');
-- this doesn't work yet :-(
SELECT * FROM test_in_out_params_multi('test_in');
SELECT * FROM test_inout_params('test_in');

SELECT (split(10)).*; 

SELECT * FROM split(100); 

select unnamed_tuple_test(); 

select named_tuple_test(); 

select oneline() 
union all 
select oneline2()
union all 
select multiline() 
union all 
select multiline2() 
union all 
select multiline3()
=======
CREATE FUNCTION elog_test() RETURNS void
AS $$
plpy.debug('debug')
plpy.log('log')
plpy.info('info')
plpy.info(37)
plpy.info()
plpy.info('info', 37, [1, 2, 3])
plpy.notice('notice')
plpy.warning('warning')
plpy.error('error')
$$ LANGUAGE plpythonu;

SELECT elog_test();
>>>>>>> 78a09145e0
