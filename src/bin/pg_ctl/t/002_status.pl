use strict;
use warnings;

use PostgresNode;
use TestLib;
use Test::More tests => 3;

my $tempdir       = TestLib::tempdir;
my $tempdir_short = TestLib::tempdir_short;

command_exit_is([ 'pg_ctl', 'status', '-D', "$tempdir/nonexistent" ],
	4, 'pg_ctl status with nonexistent directory');

<<<<<<< HEAD
standard_initdb "$tempdir/data";
=======
my $node = get_new_node('main');
$node->init;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

command_exit_is([ 'pg_ctl', 'status', '-D', $node->data_dir ],
	3, 'pg_ctl status with server not running');

<<<<<<< HEAD
system_or_bail 'pg_ctl', '-s', '-l', "$tempdir/logfile", '-D',
  "$tempdir/data", '-w', 'start', '-o', '-c gp_role=utility --gp_dbid=-1 --gp_contentid=-1';

command_exit_is([ 'pg_ctl', 'status', '-D', "$tempdir/data" ],
	0, 'pg_ctl status with server running');

system_or_bail 'pg_ctl', 'stop', '-D', "$tempdir/data", '-m', 'fast';
=======
system_or_bail 'pg_ctl', '-l', "$tempdir/logfile", '-D',
  $node->data_dir, '-w', 'start';
command_exit_is([ 'pg_ctl', 'status', '-D', $node->data_dir ],
	0, 'pg_ctl status with server running');

system_or_bail 'pg_ctl', 'stop', '-D', $node->data_dir, '-m', 'fast';
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
