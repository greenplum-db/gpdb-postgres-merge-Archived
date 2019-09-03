package RewindTest;

# Test driver for pg_rewind. Each test consists of a cycle where a new cluster
# is first created with initdb, and a streaming replication standby is set up
# to follow the master. Then the master is shut down and the standby is
# promoted, and finally pg_rewind is used to rewind the old master, using the
# standby as the source.
#
# To run a test, the test script (in t/ subdirectory) calls the functions
# in this module. These functions should be called in this sequence:
#
# 1. setup_cluster - creates a PostgreSQL cluster that runs as the master
#
# 2. start_master - starts the master server
#
# 3. start_master - starts the master server
#
# 4. create_standby - runs pg_basebackup to initialize a standby server, and
#    sets it up to follow the master.
#
# 5. promote_standby - runs "pg_ctl promote" to promote the standby server.
# The old master keeps running.
#
# 6. run_pg_rewind - stops the old master (if it's still running) and runs
# pg_rewind to synchronize it with the now-promoted standby server.
#
# 7. clean_rewind_test - stops both servers used in the test, if they're
# still running.
#
# The test script can use the helper functions master_psql and standby_psql
# to run psql against the master and standby servers, respectively. The
# test script can also use the $connstr_master and $connstr_standby global
# variables, which contain libpq connection strings for connecting to the
# master and standby servers. The data directories are also available
# in paths $test_master_datadir and $test_standby_datadir

use strict;
use warnings;

use Config;
use Exporter 'import';
use File::Copy;
use File::Path qw(rmtree);
use IPC::Run qw(run);
use PostgresNode;
use TestLib;
use Test::More;

<<<<<<< HEAD
use Config;
use File::Copy;
use File::Path qw(rmtree);
use IPC::Run qw(run start);

use Exporter 'import';
=======
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
our @EXPORT = qw(
  $node_master
  $node_standby

  master_psql
  standby_psql
  check_query

  setup_cluster
  start_master
  create_standby
  promote_standby
  run_pg_rewind
  clean_rewind_test
);

<<<<<<< HEAD
our $test_master_datadir  = "$tmp_check/data_master";
our $test_standby_datadir = "$tmp_check/data_standby";

# Define non-conflicting ports for both nodes.
my $port_master  = $ENV{PGPORT};
my $port_standby = $port_master + 1;

my $connstr_master  = "port=$port_master";
my $connstr_standby = "port=$port_standby";

$ENV{PGDATABASE} = "postgres";
=======
# Our nodes.
our $node_master;
our $node_standby;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

sub master_psql
{
	my $cmd = shift;

<<<<<<< HEAD
	system_or_bail 'psql', '-q', '--no-psqlrc', '-d', $connstr_master,
	  '-c', "$cmd";
=======
	system_or_bail 'psql', '-q', '--no-psqlrc', '-d',
	  $node_master->connstr('postgres'), '-c', "$cmd";
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
}

sub standby_psql
{
	my $cmd = shift;

<<<<<<< HEAD
	system_or_bail 'psql', '-q', '--no-psqlrc', '-d', $connstr_standby,
	  '-c', "$cmd";
=======
	system_or_bail 'psql', '-q', '--no-psqlrc', '-d',
	  $node_standby->connstr('postgres'), '-c', "$cmd";
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
}

# Run a query against the master, and check that the output matches what's
# expected
sub check_query
{
	my ($query, $expected_stdout, $test_name) = @_;
	my ($stdout, $stderr);

	# we want just the output, no formatting
	my $result = run [
		'psql', '-q', '-A', '-t', '--no-psqlrc', '-d',
		$node_master->connstr('postgres'),
		'-c', $query ],
	  '>', \$stdout, '2>', \$stderr;

	# We don't use ok() for the exit code and stderr, because we want this
	# check to be just a single test.
	if (!$result)
	{
		fail("$test_name: psql exit code");
	}
	elsif ($stderr ne '')
	{
		diag $stderr;
		fail("$test_name: psql no stderr");
	}
	else
	{
		$stdout =~ s/\r//g if $Config{osname} eq 'msys';
		is($stdout, $expected_stdout, "$test_name: query result matches");
	}
}

<<<<<<< HEAD
# Run a query once a second, until it returns 't' (i.e. SQL boolean true).
sub poll_query_until
{
	my ($query, $connstr) = @_;

	my $max_attempts = 90;
	my $attempts     = 0;
	my ($stdout, $stderr);

	while ($attempts < $max_attempts)
	{
		my $cmd = [ 'psql', '-At', '-c', "$query", '-d', "$connstr" ];
		my $result = run $cmd, '>', \$stdout, '2>', \$stderr;

		chomp($stdout);
		$stdout =~ s/\r//g if $Config{osname} eq 'msys';
		if ($stdout eq "t")
		{
			return 1;
		}

		# Wait a second before retrying.
		sleep 1;
		$attempts++;
	}

	# The query result didn't change in 90 seconds. Give up. Print the
	# output from the last attempt, hopefully that's useful for debugging.
	chomp($stderr);
	$stderr =~ s/\r//g if $Config{osname} eq 'msys';
	diag qq(poll_query_until timed out executing this query:
$query
expecting this output:
t
last actual query output:
$stdout
with stderr:
$stderr);
	return 0;
}

sub append_to_file
{
	my ($filename, $str) = @_;

	open my $fh, ">>", $filename or die "could not open file $filename";
	print $fh $str;
	close $fh;
}

sub setup_cluster
{
	# Initialize master, data checksums are mandatory
	rmtree($test_master_datadir);
	standard_initdb($test_master_datadir);

	# Custom parameters for master's postgresql.conf
	append_to_file(
		"$test_master_datadir/postgresql.conf", qq(
wal_level = hot_standby
max_wal_senders = 2
wal_keep_segments = 20
max_wal_size = 200MB
shared_buffers = 1MB
wal_log_hints = on
hot_standby = on
autovacuum = off
max_connections = 10
));

	# Accept replication connections on master
	configure_hba_for_replication $test_master_datadir;
}

sub start_master
{
	system_or_bail('pg_ctl' , '-w',
				   '-D' , $test_master_datadir,
				   '-l',  "$log_path/master.log",
				   "-o", "-p $port_master", 'start');
=======
sub setup_cluster
{

	# Initialize master, data checksums are mandatory
	$node_master = get_new_node('master');
	$node_master->init(allows_streaming => 1);
}

sub start_master
{
	$node_master->start;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

	#### Now run the test-specific parts to initialize the master before setting
	# up standby
}

sub create_standby
{
	$node_standby = get_new_node('standby');
	$node_master->backup('my_backup');
	$node_standby->init_from_backup($node_master, 'my_backup');
	my $connstr_master = $node_master->connstr('postgres');

<<<<<<< HEAD
	# Set up standby with necessary parameter
	rmtree $test_standby_datadir;

	# Base backup is taken with xlog files included
	system_or_bail('pg_basebackup', '-D', $test_standby_datadir,
				   '-p', $port_master, '-x');
	append_to_file(
		"$test_standby_datadir/recovery.conf", qq(
=======
	$node_standby->append_conf(
		"recovery.conf", qq(
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
primary_conninfo='$connstr_master application_name=rewind_standby'
standby_mode=on
recovery_target_timeline='latest'
));

	# Start standby
<<<<<<< HEAD
	system_or_bail('pg_ctl', '-w', '-D', $test_standby_datadir,
				   '-l', "$log_path/standby.log",
				   '-o', "-p $port_standby", 'start');
=======
	$node_standby->start;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

	# The standby may have WAL to apply before it matches the primary.  That
	# is fine, because no test examines the standby before promotion.
}

sub promote_standby
{
	#### Now run the test-specific parts to run after standby has been started
	# up standby

	# Wait for the standby to receive and write all WAL.
	my $wal_received_query =
"SELECT pg_current_xlog_location() = write_location FROM pg_stat_replication WHERE application_name = 'rewind_standby';";
<<<<<<< HEAD
	poll_query_until($wal_received_query, $connstr_master)
=======
	$node_master->poll_query_until('postgres', $wal_received_query)
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
	  or die "Timed out while waiting for standby to receive and write WAL";

	# Now promote slave and insert some new data on master, this will put
	# the master out-of-sync with the standby. Wait until the standby is
	# out of recovery mode, and is ready to accept read-write connections.
<<<<<<< HEAD
	system_or_bail('pg_ctl', '-w', '-D', $test_standby_datadir, 'promote');
	poll_query_until("SELECT NOT pg_is_in_recovery()", $connstr_standby)
=======
	$node_standby->promote;
	$node_standby->poll_query_until('postgres',
		"SELECT NOT pg_is_in_recovery()")
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
	  or die "Timed out while waiting for promotion of standby";

	# Force a checkpoint after the promotion. pg_rewind looks at the control
	# file to determine what timeline the server is on, and that isn't updated
	# immediately at promotion, but only at the next checkpoint. When running
	# pg_rewind in remote mode, it's possible that we complete the test steps
	# after promotion so quickly that when pg_rewind runs, the standby has not
	# performed a checkpoint after promotion yet.
	standby_psql("checkpoint");
}

sub run_pg_rewind
{
	my $test_mode       = shift;
	my $master_pgdata   = $node_master->data_dir;
	my $standby_pgdata  = $node_standby->data_dir;
	my $standby_connstr = $node_standby->connstr('postgres');
	my $tmp_folder      = TestLib::tempdir;

	# Stop the master and be ready to perform the rewind
<<<<<<< HEAD
	system_or_bail('pg_ctl', '-D', $test_master_datadir, '-m', 'fast', 'stop');
=======
	$node_master->stop;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

	# At this point, the rewind processing is ready to run.
	# We now have a very simple scenario with a few diverged WAL record.
	# The real testing begins really now with a bifurcation of the possible
	# scenarios that pg_rewind supports.

	# Keep a temporary postgresql.conf for master node or it would be
	# overwritten during the rewind.
<<<<<<< HEAD
	copy("$test_master_datadir/postgresql.conf",
		 "$tmp_check/master-postgresql.conf.tmp");
=======
	copy(
		"$master_pgdata/postgresql.conf",
		"$tmp_folder/master-postgresql.conf.tmp");
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

	# Now run pg_rewind
	if ($test_mode eq "local")
	{
		# Do rewind using a local pgdata as source
		# Stop the master and be ready to perform the rewind
<<<<<<< HEAD
		system_or_bail('pg_ctl', '-D', $test_standby_datadir,
					   '-m', 'fast', 'stop');
		command_ok(['pg_rewind',
					"--debug",
					"--source-pgdata=$test_standby_datadir",
					"--target-pgdata=$test_master_datadir"],
				   'pg_rewind local');
=======
		$node_standby->stop;
		command_ok(
			[   'pg_rewind',
				"--debug",
				"--source-pgdata=$standby_pgdata",
				"--target-pgdata=$master_pgdata" ],
			'pg_rewind local');
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
	}
	elsif ($test_mode eq "remote")
	{
		# Do rewind using a remote connection as source
<<<<<<< HEAD
		command_ok(['pg_rewind',
					"--debug",
					"--source-server",
					"port=$port_standby dbname=postgres",
					"--target-pgdata=$test_master_datadir"],
				   'pg_rewind remote');
=======
		command_ok(
			[   'pg_rewind',       "--debug",
				"--source-server", $standby_connstr,
				"--target-pgdata=$master_pgdata" ],
			'pg_rewind remote');
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
	}
	else
	{

		# Cannot come here normally
		die("Incorrect test mode specified");
	}

	# Now move back postgresql.conf with old settings
<<<<<<< HEAD
	move("$tmp_check/master-postgresql.conf.tmp",
		 "$test_master_datadir/postgresql.conf");
=======
	move(
		"$tmp_folder/master-postgresql.conf.tmp",
		"$master_pgdata/postgresql.conf");
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

	# Plug-in rewound node to the now-promoted standby node
	my $port_standby = $node_standby->port;
	$node_master->append_conf(
		'recovery.conf', qq(
primary_conninfo='port=$port_standby'
standby_mode=on
recovery_target_timeline='latest'
));

	# Restart the master to check that rewind went correctly
<<<<<<< HEAD
	system_or_bail('pg_ctl', '-w', '-D', $test_master_datadir,
				   '-l', "$log_path/master.log",
				   '-o', "-p $port_master", 'start');
=======
	$node_master->start;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

	#### Now run the test-specific parts to check the result
}

# Clean up after the test. Stop both servers, if they're still running.
sub clean_rewind_test
{
<<<<<<< HEAD
	if ($test_master_datadir)
	{
		system
		  'pg_ctl', '-D', $test_master_datadir, '-m', 'immediate', 'stop';
	}
	if ($test_standby_datadir)
	{
		system
		  'pg_ctl', '-D', $test_standby_datadir, '-m', 'immediate', 'stop';
	}
=======
	$node_master->teardown_node  if defined $node_master;
	$node_standby->teardown_node if defined $node_standby;
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
}

1;
