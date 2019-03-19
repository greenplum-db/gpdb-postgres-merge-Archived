use strict;
use warnings;
use TestLib;
<<<<<<< HEAD
use Test::More tests => 16;
=======
use Test::More tests => 20;
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8

program_help_ok('reindexdb');
program_version_ok('reindexdb');
program_options_handling_ok('reindexdb');

my $tempdir = tempdir;
start_test_server $tempdir;

$ENV{PGOPTIONS} = '-c gp_session_role=utility --client-min-messages=WARNING';

issues_sql_like(
	[ 'reindexdb', 'postgres' ],
	qr/statement: REINDEX DATABASE postgres;/,
	'SQL REINDEX run');

psql 'postgres',
  'CREATE TABLE test1 (a int); CREATE INDEX test1x ON test1 (a);';
issues_sql_like(
	[ 'reindexdb', '-t', 'test1', 'postgres' ],
<<<<<<< HEAD
	qr/statement: REINDEX TABLE public\.test1;/,
	'reindex specific table');
issues_sql_like(
	[ 'reindexdb', '-i', 'test1x', 'postgres' ],
	qr/statement: REINDEX INDEX public\.test1x;/,
	'reindex specific index');
issues_sql_like(
=======
	qr/statement: REINDEX TABLE test1;/,
	'reindex specific table');
issues_sql_like(
	[ 'reindexdb', '-i', 'test1x', 'postgres' ],
	qr/statement: REINDEX INDEX test1x;/,
	'reindex specific index');
issues_sql_like(
	[ 'reindexdb', '-S', 'pg_catalog', 'postgres' ],
	qr/statement: REINDEX SCHEMA pg_catalog;/,
	'reindex specific schema');
issues_sql_like(
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
	[ 'reindexdb', '-s', 'postgres' ],
	qr/statement: REINDEX SYSTEM postgres;/,
	'reindex system tables');
issues_sql_like(
	[ 'reindexdb', '-v', '-t', 'test1', 'postgres' ],
	qr/statement: REINDEX \(VERBOSE\) TABLE test1;/,
	'reindex with verbose output');
