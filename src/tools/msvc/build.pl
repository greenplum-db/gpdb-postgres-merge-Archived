# -*-perl-*- hey - emacs - this is a perl file

<<<<<<< HEAD
# src/tools/msvc/build.pl
=======
# $PostgreSQL: pgsql/src/tools/msvc/build.pl,v 1.3 2010/04/09 13:05:58 mha Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

BEGIN
{

    chdir("../../..") if  (-d "../msvc" && -d "../../../src");

}

use lib "src/tools/msvc";

use Cwd;

use Mkvcbuild;

# buildenv.pl is for specifying the build environment settings
# it should contain lines like:
# $ENV{PATH} = "c:/path/to/bison/bin;$ENV{PATH}";

if ( -e "src/tools/msvc/buildenv.pl")
{
    require "src/tools/msvc/buildenv.pl";
}
elsif (-e "./buildenv.pl" )
{
    require "./buildenv.pl";
}

<<<<<<< HEAD
=======
# set up the project
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
our $config;
require "config_default.pl";
require "config.pl" if (-f "src/tools/msvc/config.pl");

Mkvcbuild::mkvcbuild($config);

# check what sort of build we are doing

my $bconf = $ENV{CONFIG} || "Release";
my $buildwhat = $ARGV[1] || "";
if ($ARGV[0] eq 'DEBUG')
{
    $bconf = "Debug";
}
elsif ($ARGV[0] ne "RELEASE")
{
    $buildwhat = $ARGV[0] || "";
}

# ... and do it

if ($buildwhat)
{
    system("vcbuild $buildwhat.vcproj $bconf");
}
else
{
    system("msbuild pgsql.sln /verbosity:detailed /p:Configuration=$bconf");
}

# report status

$status = $? >> 8;

exit $status;
