use strict;
use warnings;
use PostgresNode;
use TestLib;
use Test::More tests => 13;

program_help_ok('pg_controldata');
program_version_ok('pg_controldata');
program_options_handling_ok('pg_controldata');
command_fails(['pg_controldata'], 'pg_controldata without arguments fails');
command_fails([ 'pg_controldata', 'nonexistent' ],
	'pg_controldata with nonexistent directory fails');
<<<<<<< HEAD
system_or_bail 'initdb', '-D', "$tempdir/data", '-A', 'trust';
command_like([ 'pg_controldata', "$tempdir/data" ],
=======

my $node = get_new_node('main');
$node->init;

command_like([ 'pg_controldata', $node->data_dir ],
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
	qr/checkpoint/, 'pg_controldata produces output');
