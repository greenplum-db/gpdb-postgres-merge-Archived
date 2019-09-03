use strict;
use warnings;

use PostgresNode;
use TestLib;
use Test::More tests => 10;

program_help_ok('pg_isready');
program_version_ok('pg_isready');
program_options_handling_ok('pg_isready');

command_fails(['pg_isready'], 'fails with no server running');

my $node = get_new_node('main');
$node->init;
$node->start;

<<<<<<< HEAD
# use a long timeout for the benefit of very slow buildfarm machines
command_ok([qw(pg_isready --timeout=60)], 'succeeds with server running');
=======
$node->command_ok(['pg_isready'], 'succeeds with server running');
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
