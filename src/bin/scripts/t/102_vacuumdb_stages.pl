use strict;
use warnings;
use TestLib;
use Test::More tests => 4;

my $tempdir = tempdir;
start_test_server $tempdir;

issues_sql_like(
	[ 'vacuumdb', '--analyze-in-stages', 'postgres' ],
qr/statement:\ SET\ default_statistics_target=1;\ SET\ vacuum_cost_delay=0;
                   .*statement:\ ANALYZE
                   .*statement:\ SET\ default_statistics_target=10;\ RESET\ vacuum_cost_delay;
                   .*statement:\ ANALYZE
                   .*statement:\ RESET\ default_statistics_target;
                   .*statement:\ ANALYZE/sx,
	'analyze three times');


issues_sql_like(
	[ 'vacuumdb', '--analyze-in-stages', '--all' ],
<<<<<<< HEAD
                qr/statement:\ SET\ default_statistics_target=1;\ SET\ vacuum_cost_delay=0;
                   .*statement:\ ANALYZE
                   .*statement:\ SET\ default_statistics_target=1;\ SET\ vacuum_cost_delay=0;
                   .*statement:\ ANALYZE
                   .*statement:\ SET\ default_statistics_target=10;\ RESET\ vacuum_cost_delay;
                   .*statement:\ ANALYZE
                   .*statement:\ SET\ default_statistics_target=10;\ RESET\ vacuum_cost_delay;
                   .*statement:\ ANALYZE
                   .*statement:\ RESET\ default_statistics_target;
                   .*statement:\ ANALYZE
=======
qr/.*statement:\ SET\ default_statistics_target=1;\ SET\ vacuum_cost_delay=0;
                   .*statement:\ ANALYZE.*
                   .*statement:\ SET\ default_statistics_target=1;\ SET\ vacuum_cost_delay=0;
                   .*statement:\ ANALYZE.*
                   .*statement:\ SET\ default_statistics_target=10;\ RESET\ vacuum_cost_delay;
                   .*statement:\ ANALYZE.*
                   .*statement:\ SET\ default_statistics_target=10;\ RESET\ vacuum_cost_delay;
                   .*statement:\ ANALYZE.*
                   .*statement:\ RESET\ default_statistics_target;
                   .*statement:\ ANALYZE.*
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
                   .*statement:\ RESET\ default_statistics_target;
                   .*statement:\ ANALYZE/sx,
	'analyze more than one database in stages');
