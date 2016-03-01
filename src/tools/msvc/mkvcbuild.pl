#
# Script that parses Unix style build environment and generates build files
# for building with Visual Studio.
#
<<<<<<< HEAD
# src/tools/msvc/mkvcbuild.pl
=======
# $PostgreSQL: pgsql/src/tools/msvc/mkvcbuild.pl,v 1.18 2007/03/17 14:01:01 mha Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
use strict;
use warnings;

use Mkvcbuild;

chdir('..\..\..') if (-d '..\msvc' && -d '..\..\..\src');
die 'Must run from root or msvc directory' unless (-d 'src\tools\msvc' && -d 'src');

<<<<<<< HEAD
die 'Could not find config_default.pl' unless (-f 'src/tools/msvc/config_default.pl');
print "Warning: no config.pl found, using default.\n" unless (-f 'src/tools/msvc/config.pl');

our $config;
require 'src/tools/msvc/config_default.pl';
require 'src/tools/msvc/config.pl' if (-f 'src/tools/msvc/config.pl');
=======
die 'Could not find config.pl' unless (-f 'src/tools/msvc/config.pl');

our $config;
require 'src/tools/msvc/config.pl';
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

Mkvcbuild::mkvcbuild($config);
