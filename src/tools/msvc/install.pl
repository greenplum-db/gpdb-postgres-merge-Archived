#
# Script that provides 'make install' functionality for msvc builds
#
# src/tools/msvc/install.pl
#
use strict;
use warnings;

use File::Basename;
use File::Spec;
<<<<<<< HEAD
BEGIN  { use lib File::Spec->rel2abs(dirname(__FILE__)); }
=======
BEGIN { use lib File::Spec->rel2abs(dirname(__FILE__)); }
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

use Install qw(Install);

# buildenv.pl is for specifying the build environment settings
# it should contain lines like:
# $ENV{PATH} = "c:/path/to/bison/bin;$ENV{PATH}";

if (-e "src/tools/msvc/buildenv.pl")
{
	do "./src/tools/msvc/buildenv.pl";
}
elsif (-e "./buildenv.pl")
{
	do "./buildenv.pl";
}

my $target = shift || Usage();
my $insttype = shift;
Install($target, $insttype);

sub Usage
{
	print "Usage: install.pl <targetdir> [installtype]\n";
	print "installtype: client\n";
	exit(1);
}
