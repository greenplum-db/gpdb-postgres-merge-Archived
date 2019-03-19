use strict;
use warnings;
use TestLib;
use Test::More tests => 3;

<<<<<<< HEAD
my $tempdir = TestLib::tempdir;
my $tempdir_short = TestLib::tempdir_short;

standard_initdb "$tempdir/data";
=======
my $tempdir       = TestLib::tempdir;
my $tempdir_short = TestLib::tempdir_short;

command_exit_is([ 'pg_ctl', 'status', '-D', "$tempdir/nonexistent" ],
	4, 'pg_ctl status with nonexistent directory');

standard_initdb "$tempdir/data";
open CONF, ">>$tempdir/data/postgresql.conf";
print CONF "listen_addresses = ''\n";
print CONF "unix_socket_directories = '$tempdir_short'\n";
close CONF;
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8

command_exit_is([ 'pg_ctl', 'status', '-D', "$tempdir/data" ],
	3, 'pg_ctl status with server not running');

system_or_bail 'pg_ctl', '-s', '-l', "$tempdir/logfile", '-D',
  "$tempdir/data", '-w', 'start', '-o', '-c gp_role=utility --gp_dbid=-1 --gp_contentid=-1';

command_exit_is([ 'pg_ctl', 'status', '-D', "$tempdir/data" ],
	0, 'pg_ctl status with server running');

system_or_bail 'pg_ctl', 'stop', '-D', "$tempdir/data", '-m', 'fast';
