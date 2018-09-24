<<<<<<< HEAD
# TestLib, low-level routines and actions regression tests.
#
# This module contains a set of routines dedicated to environment setup for
# a PostgreSQL regression test run and includes some low-level routines
# aimed at controlling command execution, logging and test functions. This
# module should never depend on any other PostgreSQL regression test modules.

=======
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
package TestLib;

use strict;
use warnings;

<<<<<<< HEAD
use Config;
use Exporter 'import';
use File::Basename;
use File::Spec;
use File::Temp ();
use IPC::Run;
use SimpleTee;

# specify a recent enough version of Test::More  to support the note() function
use Test::More 0.82;

our @EXPORT = qw(
  generate_ascii_string
  slurp_dir
  slurp_file
  append_to_file
  system_or_bail
  system_log
  run_log
=======
use Exporter 'import';
our @EXPORT = qw(
  tempdir
  start_test_server
  restart_test_server
  psql
  system_or_bail
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237

  command_ok
  command_fails
  command_exit_is
  program_help_ok
  program_version_ok
  program_options_handling_ok
  command_like
<<<<<<< HEAD
  command_fails_like

  $windows_os
);

our ($windows_os, $tmp_check, $log_path, $test_logfile);

BEGIN
{

	# Set to untranslated messages, to be able to compare program output
	# with expected strings.
	delete $ENV{LANGUAGE};
	delete $ENV{LC_ALL};
	$ENV{LC_MESSAGES} = 'C';

	$ENV{PGAPPNAME} = $0;

	# Must be set early
	$windows_os = $Config{osname} eq 'MSWin32' || $Config{osname} eq 'msys';
}

INIT
{

	# Determine output directories, and create them.  The base path is the
	# TESTDIR environment variable, which is normally set by the invoking
	# Makefile.
	$tmp_check = $ENV{TESTDIR} ? "$ENV{TESTDIR}/tmp_check" : "tmp_check";
	$log_path = "$tmp_check/log";

	mkdir $tmp_check;
	mkdir $log_path;

	# Open the test log file, whose name depends on the test name.
	$test_logfile = basename($0);
	$test_logfile =~ s/\.[^.]+$//;
	$test_logfile = "$log_path/regress_log_$test_logfile";
	open my $testlog, '>', $test_logfile
	  or die "could not open STDOUT to logfile \"$test_logfile\": $!";

	# Hijack STDOUT and STDERR to the log file
	open(my $orig_stdout, '>&', \*STDOUT);
	open(my $orig_stderr, '>&', \*STDERR);
	open(STDOUT,          '>&', $testlog);
	open(STDERR,          '>&', $testlog);

	# The test output (ok ...) needs to be printed to the original STDOUT so
	# that the 'prove' program can parse it, and display it to the user in
	# real time. But also copy it to the log file, to provide more context
	# in the log.
	my $builder = Test::More->builder;
	my $fh      = $builder->output;
	tie *$fh, "SimpleTee", $orig_stdout, $testlog;
	$fh = $builder->failure_output;
	tie *$fh, "SimpleTee", $orig_stderr, $testlog;

	# Enable auto-flushing for all the file handles. Stderr and stdout are
	# redirected to the same file, and buffering causes the lines to appear
	# in the log in confusing order.
	autoflush STDOUT 1;
	autoflush STDERR 1;
	autoflush $testlog 1;
}

END
{

	# Preserve temporary directory for this test on failure
	$File::Temp::KEEP_ALL = 1 unless all_tests_passing();
}

sub all_tests_passing
{
	my $fail_count = 0;
	foreach my $status (Test::More->builder->summary)
	{
		return 0 unless $status;
	}
	return 1;
}

#
# Helper functions
#
sub tempdir
{
	my ($prefix) = @_;
	$prefix = "tmp_test" unless defined $prefix;
	return File::Temp::tempdir(
		$prefix . '_XXXX',
		DIR     => $tmp_check,
		CLEANUP => 1);
}

sub tempdir_short
{

	# Use a separate temp dir outside the build tree for the
	# Unix-domain socket, to avoid file name length issues.
	return File::Temp::tempdir(CLEANUP => 1);
}

sub system_log
{
	print("# Running: " . join(" ", @_) . "\n");
	return system(@_);
=======
  issues_sql_like
);

use Cwd;
use File::Spec;
use File::Temp ();
use Test::More;

BEGIN
{
	eval {
		require IPC::Run;
		import IPC::Run qw(run start);
		1;
	} or do
	{
		plan skip_all => "IPC::Run not available";
	  }
}

delete $ENV{PGCONNECT_TIMEOUT};
delete $ENV{PGDATA};
delete $ENV{PGDATABASE};
delete $ENV{PGHOSTADDR};
delete $ENV{PGREQUIRESSL};
delete $ENV{PGSERVICE};
delete $ENV{PGSSLMODE};
delete $ENV{PGUSER};

if (!$ENV{PGPORT})
{
	$ENV{PGPORT} = 65432;
}

$ENV{PGPORT} = int($ENV{PGPORT}) % 65536;


#
# Helper functions
#


sub tempdir
{
	return File::Temp::tempdir('testXXXX', DIR => cwd(), CLEANUP => 1);
}

my ($test_server_datadir, $test_server_logfile);

sub start_test_server
{
	my ($tempdir) = @_;
	my $ret;

	system "initdb -D $tempdir/pgdata -A trust -N >/dev/null";
	$ret = system 'pg_ctl', '-D', "$tempdir/pgdata", '-s', '-w', '-l',
	  "$tempdir/logfile", '-o',
	  "--fsync=off -k $tempdir --listen-addresses='' --log-statement=all",
	  'start';

	if ($ret != 0)
	{
		system('cat', "$tempdir/logfile");
		BAIL_OUT("pg_ctl failed");
	}

	$ENV{PGHOST}         = $tempdir;
	$test_server_datadir = "$tempdir/pgdata";
	$test_server_logfile = "$tempdir/logfile";
}

sub restart_test_server
{
	system 'pg_ctl', '-s', '-D', $test_server_datadir, '-w', '-l',
	  $test_server_logfile, 'restart';
}

END
{
	if ($test_server_datadir)
	{
		system 'pg_ctl', '-D', $test_server_datadir, '-s', '-w', '-m',
		  'immediate', 'stop';
	}
}

sub psql
{
	my ($dbname, $sql) = @_;
	run [ 'psql', '-X', '-q', '-d', $dbname, '-f', '-' ], '<', \$sql or die;
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
}

sub system_or_bail
{
<<<<<<< HEAD
	if (system_log(@_) != 0)
	{
		BAIL_OUT("system $_[0] failed");
	}
}

sub run_log
{
	my $cmd = join(" ", @{ $_[0] });
	print("# Running: " . $cmd . "\n");
	return IPC::Run::run($cmd);
}

# Generate a string made of the given range of ASCII characters
sub generate_ascii_string
{
	my ($from_char, $to_char) = @_;
	my $res;

	for my $i ($from_char .. $to_char)
	{
		$res .= sprintf("%c", $i);
	}
	return $res;
}

sub slurp_dir
{
	my ($dir) = @_;
	opendir(my $dh, $dir)
	  or die "could not opendir \"$dir\": $!";
	my @direntries = readdir $dh;
	closedir $dh;
	return @direntries;
}

sub slurp_file
{
	my ($filename) = @_;
	local $/;
	open(my $in, '<', $filename)
	  or die "could not read \"$filename\": $!";
	my $contents = <$in>;
	close $in;
	$contents =~ s/\r//g if $Config{osname} eq 'msys';
	return $contents;
}

sub append_to_file
{
	my ($filename, $str) = @_;
	open my $fh, ">>", $filename
	  or die "could not write \"$filename\": $!";
	print $fh $str;
	close $fh;
}
=======
	system(@_) == 0 or BAIL_OUT("system @_ failed: $?");
}

>>>>>>> ab76208e3df6841b3770edeece57d0f048392237

#
# Test functions
#
<<<<<<< HEAD
sub command_ok
{
	my ($cmd, $test_name) = @_;
	my $result = run_log($cmd);
=======


sub command_ok
{
	my ($cmd, $test_name) = @_;
	my $result = run $cmd, '>', File::Spec->devnull(), '2>',
	  File::Spec->devnull();
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
	ok($result, $test_name);
}

sub command_fails
{
	my ($cmd, $test_name) = @_;
<<<<<<< HEAD
	my $result = run_log($cmd);
=======
	my $result = run $cmd, '>', File::Spec->devnull(), '2>',
	  File::Spec->devnull();
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
	ok(!$result, $test_name);
}

sub command_exit_is
{
	my ($cmd, $expected, $test_name) = @_;
<<<<<<< HEAD
	print("# Running: " . join(" ", @{$cmd}) . "\n");
	my $h = IPC::Run::start $cmd;
	$h->finish();

	# On Windows, the exit status of the process is returned directly as the
	# process's exit code, while on Unix, it's returned in the high bits
	# of the exit code (see WEXITSTATUS macro in the standard <sys/wait.h>
	# header file). IPC::Run's result function always returns exit code >> 8,
	# assuming the Unix convention, which will always return 0 on Windows as
	# long as the process was not terminated by an exception. To work around
	# that, use $h->full_result on Windows instead.
	my $result =
	    ($Config{osname} eq "MSWin32")
	  ? ($h->full_results)[0]
	  : $h->result(0);
	is($result, $expected, $test_name);
=======
	my $h = start $cmd, '>', File::Spec->devnull(), '2>',
	  File::Spec->devnull();
	$h->finish();
	is($h->result(0), $expected, $test_name);
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
}

sub program_help_ok
{
	my ($cmd) = @_;
<<<<<<< HEAD
	my ($stdout, $stderr);
	print("# Running: $cmd --help\n");
	my $result = IPC::Run::run [ $cmd, '--help' ], '>', \$stdout, '2>',
	  \$stderr;
	ok($result, "$cmd --help exit code 0");
	isnt($stdout, '', "$cmd --help goes to stdout");
	is($stderr, '', "$cmd --help nothing to stderr");
=======
	subtest "$cmd --help" => sub {
		plan tests => 3;
		my ($stdout, $stderr);
		my $result = run [ $cmd, '--help' ], '>', \$stdout, '2>', \$stderr;
		ok($result, "$cmd --help exit code 0");
		isnt($stdout, '', "$cmd --help goes to stdout");
		is($stderr, '', "$cmd --help nothing to stderr");
	};
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
}

sub program_version_ok
{
	my ($cmd) = @_;
<<<<<<< HEAD
	my ($stdout, $stderr);
	print("# Running: $cmd --version\n");
	my $result = IPC::Run::run [ $cmd, '--version' ], '>', \$stdout, '2>',
	  \$stderr;
	ok($result, "$cmd --version exit code 0");
	isnt($stdout, '', "$cmd --version goes to stdout");
	is($stderr, '', "$cmd --version nothing to stderr");
=======
	subtest "$cmd --version" => sub {
		plan tests => 3;
		my ($stdout, $stderr);
		my $result = run [ $cmd, '--version' ], '>', \$stdout, '2>', \$stderr;
		ok($result, "$cmd --version exit code 0");
		isnt($stdout, '', "$cmd --version goes to stdout");
		is($stderr, '', "$cmd --version nothing to stderr");
	};
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
}

sub program_options_handling_ok
{
	my ($cmd) = @_;
<<<<<<< HEAD
	my ($stdout, $stderr);
	print("# Running: $cmd --not-a-valid-option\n");
	my $result = IPC::Run::run [ $cmd, '--not-a-valid-option' ], '>',
	  \$stdout,
	  '2>', \$stderr;
	ok(!$result, "$cmd with invalid option nonzero exit code");
	isnt($stderr, '', "$cmd with invalid option prints error message");
=======
	subtest "$cmd options handling" => sub {
		plan tests => 2;
		my ($stdout, $stderr);
		my $result = run [ $cmd, '--not-a-valid-option' ], '>', \$stdout,
		  '2>', \$stderr;
		ok(!$result, "$cmd with invalid option nonzero exit code");
		isnt($stderr, '', "$cmd with invalid option prints error message");
	};
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
}

sub command_like
{
	my ($cmd, $expected_stdout, $test_name) = @_;
<<<<<<< HEAD
	my ($stdout, $stderr);
	print("# Running: " . join(" ", @{$cmd}) . "\n");
	my $result = IPC::Run::run $cmd, '>', \$stdout, '2>', \$stderr;
	ok($result, "$test_name: exit code 0");
	is($stderr, '', "$test_name: no stderr");
	like($stdout, $expected_stdout, "$test_name: matches");
}

sub command_fails_like
{
	my ($cmd, $expected_stderr, $test_name) = @_;
	my ($stdout, $stderr);
	print("# Running: " . join(" ", @{$cmd}) . "\n");
	my $result = IPC::Run::run $cmd, '>', \$stdout, '2>', \$stderr;
	ok(!$result, "$test_name: exit code not 0");
	like($stderr, $expected_stderr, "$test_name: matches");
=======
	subtest $test_name => sub {
		plan tests => 3;
		my ($stdout, $stderr);
		my $result = run $cmd, '>', \$stdout, '2>', \$stderr;
		ok($result, "@$cmd exit code 0");
		is($stderr, '', "@$cmd no stderr");
		like($stdout, $expected_stdout, "$test_name: matches");
	};
}

sub issues_sql_like
{
	my ($cmd, $expected_sql, $test_name) = @_;
	subtest $test_name => sub {
		plan tests => 2;
		my ($stdout, $stderr);
		truncate $test_server_logfile, 0;
		my $result = run $cmd, '>', \$stdout, '2>', \$stderr;
		ok($result, "@$cmd exit code 0");
		my $log = `cat $test_server_logfile`;
		like($log, $expected_sql, "$test_name: SQL found in server log");
	};
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
}

1;
