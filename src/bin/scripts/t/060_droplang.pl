use strict;
use warnings;

use PostgresNode;
use TestLib;
use Test::More tests => 11;

program_help_ok('droplang');
program_version_ok('droplang');
program_options_handling_ok('droplang');

my $node = get_new_node('main');
$node->init;
$node->start;

<<<<<<< HEAD
# GPDB: drop gp_toolkit before trying to drop plpgsql in tests; otherwise they
# fail because of the dependency.
psql 'postgres', 'DROP SCHEMA gp_toolkit CASCADE';

issues_sql_like(
=======
$node->issues_sql_like(
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
	[ 'droplang', 'plpgsql', 'postgres' ],
	qr/statement: DROP EXTENSION "plpgsql"/,
	'SQL DROP EXTENSION run');

$node->command_fails(
	[ 'droplang', 'nonexistent', 'postgres' ],
	'fails with nonexistent language');
