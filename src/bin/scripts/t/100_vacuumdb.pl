use strict;
use warnings;
use TestLib;
<<<<<<< HEAD
use Test::More tests => 23;
=======
use Test::More tests => 18;
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8

program_help_ok('vacuumdb');
program_version_ok('vacuumdb');
program_options_handling_ok('vacuumdb');

my $tempdir = tempdir;
start_test_server $tempdir;

issues_sql_like(
	[ 'vacuumdb', 'postgres' ],
	qr/statement: VACUUM;/,
	'SQL VACUUM run');
issues_sql_like(
		[ 'vacuumdb', '-f', 'postgres' ],
		qr/statement: VACUUM \(FULL\);/,
		'vacuumdb -f');
issues_sql_like(
	[ 'vacuumdb', '-F', 'postgres' ],
	qr/statement: VACUUM \(FREEZE\);/,
	'vacuumdb -F');
issues_sql_like(
	[ 'vacuumdb', '-z', 'postgres' ],
	qr/statement: VACUUM \(ANALYZE\);/,
	'vacuumdb -z');
issues_sql_like(
	[ 'vacuumdb', '-Z', 'postgres' ],
	qr/statement: ANALYZE;/,
	'vacuumdb -Z');
<<<<<<< HEAD
command_ok([qw(vacuumdb -Z --table=pg_am dbname=template1)],
	'vacuumdb with connection string');

command_fails([qw(vacuumdb -Zt pg_am;ABORT postgres)],
	'trailing command in "-t", without COLUMNS');
# Unwanted; better if it failed.
command_ok([qw(vacuumdb -Zt pg_am(amname);ABORT postgres)],
	'trailing command in "-t", with COLUMNS');

psql('postgres', q|
  CREATE TABLE "need""q(uot" (")x" text);

  CREATE FUNCTION f0(int) RETURNS int LANGUAGE SQL AS 'SELECT $1 * $1';
  CREATE FUNCTION f1(int) RETURNS int LANGUAGE SQL AS 'SELECT f0($1)';
  CREATE TABLE funcidx (x int);
  INSERT INTO funcidx VALUES (0),(1),(2),(3);
  CREATE INDEX i0 ON funcidx ((f1(x)));
|);
command_ok([qw|vacuumdb -Z --table="need""q(uot"(")x") postgres|],
	'column list');
command_fails([qw|vacuumdb -Zt funcidx postgres|],
	'unqualifed name via functional index');
=======
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
