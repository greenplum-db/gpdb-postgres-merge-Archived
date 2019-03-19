use strict;
use warnings;
use TestLib;
use Test::More tests => 13;

my $tempdir = TestLib::tempdir;

program_help_ok('pg_controldata');
program_version_ok('pg_controldata');
program_options_handling_ok('pg_controldata');
command_fails(['pg_controldata'], 'pg_controldata without arguments fails');
command_fails([ 'pg_controldata', 'nonexistent' ],
	'pg_controldata with nonexistent directory fails');
<<<<<<< HEAD
system_or_bail 'initdb', '-D', "$tempdir/data", '-A', 'trust';
=======
system_or_bail "initdb -D '$tempdir'/data -A trust >/dev/null";
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
command_like([ 'pg_controldata', "$tempdir/data" ],
	qr/checkpoint/, 'pg_controldata produces output');
