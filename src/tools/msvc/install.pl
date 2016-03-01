#
# Script that provides 'make install' functionality for msvc builds
#
<<<<<<< HEAD
# src/tools/msvc/install.pl
=======
# $PostgreSQL: pgsql/src/tools/msvc/install.pl,v 1.7 2007/03/17 14:01:01 mha Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
use strict;
use warnings;

use Install qw(Install);

my $target = shift || Usage();
Install($target);

sub Usage
{
    print "Usage: install.pl <targetdir>\n";
    exit(1);
}
