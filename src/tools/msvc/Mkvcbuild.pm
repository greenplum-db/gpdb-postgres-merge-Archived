package Mkvcbuild;

#
# Package that generates build files for msvc build
#
<<<<<<< HEAD
# src/tools/msvc/Mkvcbuild.pm
=======
# $PostgreSQL: pgsql/src/tools/msvc/Mkvcbuild.pm,v 1.25.2.5 2010/05/13 21:34:30 adunstan Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
use Carp;
use Win32;
use strict;
use warnings;
use Project;
use Solution;
use Cwd;
<<<<<<< HEAD
use File::Copy;
=======
use Config;
use List::Util qw(first);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

use Exporter;
our (@ISA, @EXPORT_OK);
@ISA = qw(Exporter);
@EXPORT_OK = qw(Mkvcbuild);

my $solution;
my $libpgport;
my $postgres;
my $libpq;

my $contrib_defines = {'refint' => 'REFINT_VERBOSE'};
<<<<<<< HEAD
# for GPDB, I've added xlogdump
my @contrib_uselibpq = ('dblink', 'oid2name', 'pgbench', 'vacuumlo', 'xlogdump');
my @contrib_uselibpgport = ('oid2name', 'pgbench', 'pg_standby', 'vacuumlo', 'xlogdump');
my $contrib_extralibs = {'pgbench' => ['wsock32.lib']};
my $contrib_extraincludes = {'tsearch2' => ['contrib/tsearch2'], 'dblink' => ['src/backend']};
=======
my @contrib_uselibpq = ('dblink', 'oid2name', 'pgbench', 'vacuumlo');
my @contrib_uselibpgport = ('oid2name', 'pgbench', 'pg_standby', 'vacuumlo');
my $contrib_extralibs = {'pgbench' => ['wsock32.lib']};
my $contrib_extraincludes = {'tsearch2' => ['contrib/tsearch2']};
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
my $contrib_extrasource = {
    'cube' => ['cubescan.l','cubeparse.y'],
    'seg' => ['segscan.l','segparse.y']
};
<<<<<<< HEAD
my @contrib_excludes = ('pgcrypto','intagg','uuid-ossp','bufferedtest', 'varblocktest', 'hstore', 'gp_filedump', 'tsearch2', 'dblink', 'xlogdump','xlogviewer', 'changetrackingdump','gp_sparse_vector','xml2','adminpack');
=======
my @contrib_excludes = ('pgcrypto');
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

sub mkvcbuild
{
    our $config = shift;

    chdir('..\..\..') if (-d '..\msvc' && -d '..\..\..\src');
    die 'Must run from root or msvc directory' unless (-d 'src\tools\msvc' && -d 'src');

    $solution = new Solution($config);

    our @pgportfiles = qw(
      chklocale.c crypt.c fseeko.c getrusage.c inet_aton.c random.c srandom.c
<<<<<<< HEAD
      getaddrinfo.c gettimeofday.c kill.c open.c rand.c
      snprintf.c strlcat.c strlcpy.c copydir.c dirmod.c exec.c noblock.c path.c pipe.c
      pgsleep.c pgstrcasecmp.c qsort.c qsort_arg.c sprompt.c thread.c
      getopt.c getopt_long.c dirent.c rint.c win32env.c win32error.c glob.c);
=======
      unsetenv.c getaddrinfo.c gettimeofday.c kill.c open.c rand.c
      snprintf.c strlcat.c strlcpy.c copydir.c dirmod.c exec.c noblock.c path.c pipe.c
      pgsleep.c pgstrcasecmp.c qsort.c qsort_arg.c sprompt.c thread.c
      getopt.c getopt_long.c dirent.c rint.c win32error.c);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

    $libpgport = $solution->AddProject('libpgport','lib','misc');
    $libpgport->AddDefine('FRONTEND');
    $libpgport->AddFiles('src\port',@pgportfiles);

    $postgres = $solution->AddProject('postgres','exe','','src\backend');
    $postgres->AddIncludeDir('src\backend');
<<<<<<< HEAD
    $postgres->AddIncludeDir('src\backend\gp_libpq_fe');
    $postgres->AddIncludeDir('src\port');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    $postgres->AddDir('src\backend\port\win32');
    $postgres->AddFile('src\backend\utils\fmgrtab.c');
    $postgres->ReplaceFile('src\backend\port\dynloader.c','src\backend\port\dynloader\win32.c');
    $postgres->ReplaceFile('src\backend\port\pg_sema.c','src\backend\port\win32_sema.c');
    $postgres->ReplaceFile('src\backend\port\pg_shmem.c','src\backend\port\win32_shmem.c');
    $postgres->AddFiles('src\port',@pgportfiles);
    $postgres->AddDir('src\timezone');
    $postgres->AddFiles('src\backend\parser','scan.l','gram.y');
    $postgres->AddFiles('src\backend\bootstrap','bootscanner.l','bootparse.y');
    $postgres->AddFiles('src\backend\utils\misc','guc-file.l');
    $postgres->AddDefine('BUILDING_DLL');
    $postgres->AddLibrary('wsock32.lib');
    $postgres->AddLibrary('ws2_32.lib');
    $postgres->AddLibrary('secur32.lib');
    $postgres->AddLibrary('wldap32.lib') if ($solution->{options}->{ldap});
<<<<<<< HEAD
    
    # GPDB actually requires pthreads, can't run without.
    $postgres->AddLibrary($config->{'pthread'} . '\pthreadVC2.lib');
    
    $postgres->FullExportDLL('postgres.lib');

    my $plpgsql = $solution->AddProject('plpgsql','dll','PLs','src\pl\plpgsql\src');
    $plpgsql->AddFiles('src\pl\plpgsql\src','scan.l','gram.y');
    $plpgsql->AddReference($postgres);
    $plpgsql->AddLibrary('wsock32.lib');
    $plpgsql->AddLibrary('ws2_32.lib');

    if ($solution->{options}->{perl})
    {
        my $plperlsrc = "src\\pl\\plperl\\";
        my $plperl = $solution->AddProject('plperl','dll','PLs','src\pl\plperl');
        $plperl->AddIncludeDir($solution->{options}->{perl} . '/lib/CORE');
        $plperl->AddDefine('PLPERL_HAVE_UID_GID');
        foreach my $xs ('SPI.xs', 'Util.xs')
        {
            (my $xsc = $xs) =~ s/\.xs/.c/;
            if (Solution::IsNewer("$plperlsrc$xsc","$plperlsrc$xs"))
            {
                print "Building $plperlsrc$xsc...\n";
            system( $solution->{options}->{perl}
                  . '/bin/perl '
                  . $solution->{options}->{perl}
                  . '/lib/ExtUtils/xsubpp -typemap '
                  . $solution->{options}->{perl}
                      . '/lib/ExtUtils/typemap '
                      . "$plperlsrc$xs "
                      . ">$plperlsrc$xsc");
                if ((!(-f "$plperlsrc$xsc")) || -z "$plperlsrc$xsc")
            {
                    unlink("$plperlsrc$xsc"); # if zero size
                    die "Failed to create $xsc.\n";
                }
            }
        }
        if (  Solution::IsNewer('src\pl\plperl\perlchunks.h','src\pl\plperl\plc_perlboot.pl')
            ||Solution::IsNewer('src\pl\plperl\perlchunks.h','src\pl\plperl\plc_safe_bad.pl')
            ||Solution::IsNewer('src\pl\plperl\perlchunks.h','src\pl\plperl\plc_safe_ok.pl'))
        {
            print 'Building src\pl\plperl\perlchunks.h ...' . "\n";
=======
    $postgres->FullExportDLL('postgres.lib');

    my $snowball = $solution->AddProject('dict_snowball','dll','','src\backend\snowball');
    $snowball->RelocateFiles('src\backend\snowball\libstemmer', sub {
        return shift !~ /dict_snowball.c$/;
    });
    $snowball->AddIncludeDir('src\include\snowball');
    $snowball->AddReference($postgres);

    my $plpgsql = $solution->AddProject('plpgsql','dll','PLs','src\pl\plpgsql\src');
    $plpgsql->AddFiles('src\pl\plpgsql\src','scan.l','gram.y');
    $plpgsql->AddReference($postgres);

    if ($solution->{options}->{perl})
    {
        my $plperl = $solution->AddProject('plperl','dll','PLs','src\pl\plperl');
        $plperl->AddIncludeDir($solution->{options}->{perl} . '/lib/CORE');
        $plperl->AddDefine('PLPERL_HAVE_UID_GID');
        if (Solution::IsNewer('src\pl\plperl\SPI.c','src\pl\plperl\SPI.xs'))
        {
			my $xsubppdir = first { -e "$_\\ExtUtils\\xsubpp" } @INC;
            print 'Building src\pl\plperl\SPI.c...' . "\n";
            system( $solution->{options}->{perl}
                  . '/bin/perl '
				  . "$xsubppdir/ExtUtils/xsubpp -typemap "
                  . $solution->{options}->{perl}
                  . '/lib/ExtUtils/typemap src\pl\plperl\SPI.xs >src\pl\plperl\SPI.c');
            if ((!(-f 'src\pl\plperl\SPI.c')) || -z 'src\pl\plperl\SPI.c')
            {
                unlink('src\pl\plperl\SPI.c'); # if zero size
                die 'Failed to create SPI.c' . "\n";
            }
        }
        if (  Solution::IsNewer('src\pl\plperl\plperl_opmask.h','src\pl\plperl\plperl_opmask.pl'))
        {
            print 'Building src\pl\plperl\plperl_opmask.h ...' . "\n";
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
            my $basedir = getcwd;
            chdir 'src\pl\plperl';
            system( $solution->{options}->{perl}
                  . '/bin/perl '
<<<<<<< HEAD
                  . 'text2macro.pl '
                  . '--strip="^(\#.*|\s*)$$" '
                  . 'plc_perlboot.pl plc_safe_bad.pl plc_safe_ok.pl '
                  .	'>perlchunks.h');
            chdir $basedir;
            if ((!(-f 'src\pl\plperl\perlchunks.h')) || -z 'src\pl\plperl\perlchunks.h')
            {
                unlink('src\pl\plperl\perlchunks.h'); # if zero size
                die 'Failed to create perlchunks.h' . "\n";
            }
        }
        $plperl->AddReference($postgres);
        my @perl_libs =
          grep {/perl\d+.lib$/ }glob($solution->{options}->{perl} . '\lib\CORE\perl*.lib');
        if (@perl_libs == 1)
        {
            $plperl->AddLibrary($perl_libs[0]);
        }
        else
        {
			die "could not identify perl library version";
        }
=======
                  . 'plperl_opmask.pl '
                  .	'plperl_opmask.h');
            chdir $basedir;
            if ((!(-f 'src\pl\plperl\plperl_opmask.h')) || -z 'src\pl\plperl\plperl_opmask.h')
            {
                unlink('src\pl\plperl\plperl_opmask.h'); # if zero size
                die 'Failed to create plperl_opmask.h' . "\n";
            }
        }
        $plperl->AddReference($postgres);
		my @perl_libs = grep {/perl\d+.lib$/ }
		   glob($solution->{options}->{perl} . '\lib\CORE\perl*.lib');
		if (@perl_libs == 1)
		{
			$plperl->AddLibrary($perl_libs[0]);
		}
		else
		{
			die "could not identify perl library version";
		}
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    }

    if ($solution->{options}->{python})
    {
<<<<<<< HEAD

        # Attempt to get python version and location. Assume python.exe in specified dir.
        open(P,
            $solution->{options}->{python}
              . "\\python -c \"import sys;print(sys.prefix);print(str(sys.version_info[0])+str(sys.version_info[1]))\" |"
        ) || die "Could not query for python version!\n";
        my $pyprefix = <P>;
        chomp($pyprefix);
        my $pyver = <P>;
        chomp($pyver);
        close(P);

        # Sometimes (always?) if python is not present, the execution
        # appears to work, but gives no data...
        die "Failed to query python for version information\n"
          if (!(defined($pyprefix) && defined($pyver)));

        my $plpython = $solution->AddProject('plpython','dll','PLs','src\pl\plpython');
        $plpython->AddIncludeDir($pyprefix . '\include');
        $plpython->AddLibrary($pyprefix . "\\Libs\\python$pyver.lib");
=======
        my $plpython = $solution->AddProject('plpython','dll','PLs','src\pl\plpython');
        $plpython->AddIncludeDir($solution->{options}->{python} . '\include');
        $solution->{options}->{python} =~ /\\Python(\d{2})/i
          || croak "Could not determine python version from path";
        $plpython->AddLibrary($solution->{options}->{python} . "\\Libs\\python$1.lib");
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
        $plpython->AddReference($postgres);
    }

    if ($solution->{options}->{tcl})
    {
        my $pltcl = $solution->AddProject('pltcl','dll','PLs','src\pl\tcl');
        $pltcl->AddIncludeDir($solution->{options}->{tcl} . '\include');
        $pltcl->AddReference($postgres);
<<<<<<< HEAD
        if (-e $solution->{options}->{tcl} . '\lib\tcl85.lib')
        {
            $pltcl->AddLibrary($solution->{options}->{tcl} . '\lib\tcl85.lib');
        }
        else
        {
        $pltcl->AddLibrary($solution->{options}->{tcl} . '\lib\tcl84.lib');
        }
=======
        $pltcl->AddLibrary($solution->{options}->{tcl} . '\lib\tcl84.lib');
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    }

    $libpq = $solution->AddProject('libpq','dll','interfaces','src\interfaces\libpq');
    $libpq->AddDefine('FRONTEND');
<<<<<<< HEAD
    $libpq->AddDefine('UNSAFE_STAT_OK');
    $libpq->AddIncludeDir('src\port');
    $libpq->AddLibrary('wsock32.lib');
    $libpq->AddLibrary('secur32.lib');
    $libpq->AddLibrary('ws2_32.lib');
=======
	$libpq->AddDefine('UNSAFE_STAT_OK');
    $libpq->AddIncludeDir('src\port');
    $libpq->AddLibrary('wsock32.lib');
    $libpq->AddLibrary('secur32.lib');
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    $libpq->AddLibrary('wldap32.lib') if ($solution->{options}->{ldap});
    $libpq->UseDef('src\interfaces\libpq\libpqdll.def');
    $libpq->ReplaceFile('src\interfaces\libpq\libpqrc.c','src\interfaces\libpq\libpq.rc');
    $libpq->AddReference($libpgport);

    my $pgtypes =
      $solution->AddProject('libpgtypes','dll','interfaces','src\interfaces\ecpg\pgtypeslib');
    $pgtypes->AddDefine('FRONTEND');
    $pgtypes->AddReference($libpgport);
    $pgtypes->UseDef('src\interfaces\ecpg\pgtypeslib\pgtypeslib.def');
    $pgtypes->AddIncludeDir('src\interfaces\ecpg\include');

    my $libecpg =$solution->AddProject('libecpg','dll','interfaces','src\interfaces\ecpg\ecpglib');
    $libecpg->AddDefine('FRONTEND');
    $libecpg->AddIncludeDir('src\interfaces\ecpg\include');
    $libecpg->AddIncludeDir('src\interfaces\libpq');
    $libecpg->AddIncludeDir('src\port');
    $libecpg->UseDef('src\interfaces\ecpg\ecpglib\ecpglib.def');
    $libecpg->AddLibrary('wsock32.lib');
<<<<<<< HEAD
    $libecpg->AddLibrary($config->{'pthread'} . '\pthreadVC2.lib');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    $libecpg->AddReference($libpq,$pgtypes,$libpgport);

    my $libecpgcompat =
      $solution->AddProject('libecpg_compat','dll','interfaces','src\interfaces\ecpg\compatlib');
    $libecpgcompat->AddIncludeDir('src\interfaces\ecpg\include');
    $libecpgcompat->AddIncludeDir('src\interfaces\libpq');
    $libecpgcompat->UseDef('src\interfaces\ecpg\compatlib\compatlib.def');
    $libecpgcompat->AddReference($pgtypes,$libecpg,$libpgport);

    my $ecpg = $solution->AddProject('ecpg','exe','interfaces','src\interfaces\ecpg\preproc');
    $ecpg->AddIncludeDir('src\interfaces\ecpg\include');
    $ecpg->AddIncludeDir('src\interfaces\libpq');
    $ecpg->AddPrefixInclude('src\interfaces\ecpg\preproc');
    $ecpg->AddFiles('src\interfaces\ecpg\preproc','pgc.l','preproc.y');
    $ecpg->AddDefine('MAJOR_VERSION=4');
    $ecpg->AddDefine('MINOR_VERSION=2');
    $ecpg->AddDefine('PATCHLEVEL=1');
<<<<<<< HEAD
    $ecpg->AddDefine('ECPG_COMPILE');
    $ecpg->AddReference($libpgport);

    #my $pgregress_ecpg = $solution->AddProject('pg_regress_ecpg','exe','misc');
    #$pgregress_ecpg->AddFile('src\interfaces\ecpg\test\pg_regress_ecpg.c');
    #$pgregress_ecpg->AddFile('src\test\regress\pg_regress.c');
    #$pgregress_ecpg->AddIncludeDir('src\port');
    #$pgregress_ecpg->AddIncludeDir('src\test\regress');
    #$pgregress_ecpg->AddDefine('HOST_TUPLE="i686-pc-win32vc"');
    #$pgregress_ecpg->AddDefine('FRONTEND');
    #$pgregress_ecpg->AddReference($libpgport);
=======
    $ecpg->AddReference($libpgport);

    my $pgregress_ecpg = $solution->AddProject('pg_regress_ecpg','exe','misc');
    $pgregress_ecpg->AddFile('src\interfaces\ecpg\test\pg_regress_ecpg.c');
    $pgregress_ecpg->AddFile('src\test\regress\pg_regress.c');
    $pgregress_ecpg->AddIncludeDir('src\port');
    $pgregress_ecpg->AddIncludeDir('src\test\regress');
    $pgregress_ecpg->AddDefine('HOST_TUPLE="i686-pc-win32vc"');
    $pgregress_ecpg->AddDefine('FRONTEND');
    $pgregress_ecpg->AddReference($libpgport);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

    # src/bin
    my $initdb = AddSimpleFrontend('initdb');
    $initdb->AddIncludeDir('src\interfaces\libpq');
    $initdb->AddDefine('FRONTEND');
    $initdb->AddLibrary('wsock32.lib');
    $initdb->AddLibrary('ws2_32.lib');

    my $pgconfig = AddSimpleFrontend('pg_config');

    my $pgcontrol = AddSimpleFrontend('pg_controldata');

    my $pgctl = AddSimpleFrontend('pg_ctl', 1);

    my $pgreset = AddSimpleFrontend('pg_resetxlog');

    my $pgevent = $solution->AddProject('pgevent','dll','bin');
    $pgevent->AddFiles('src\bin\pgevent','pgevent.c','pgmsgevent.rc');
    $pgevent->AddResourceFile('src\bin\pgevent','Eventlog message formatter');
    $pgevent->RemoveFile('src\bin\pgevent\win32ver.rc');
    $pgevent->UseDef('src\bin\pgevent\pgevent.def');
    $pgevent->DisableLinkerWarnings('4104');

    my $psql = AddSimpleFrontend('psql', 1);
    $psql->AddIncludeDir('src\bin\pg_dump');
    $psql->AddIncludeDir('src\backend');
<<<<<<< HEAD
    if ($solution->{options}->{readline})
    {
		$psql->AddIncludeDir($solution->{options}->{readline} . '\include');
		$psql->AddLibrary($solution->{options}->{readline} . '\lib\readline.lib');
		$psql->AddLibrary($solution->{options}->{readline} . '\lib\history.lib');

    }
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    $psql->AddFile('src\bin\psql\psqlscan.l');

    my $pgdump = AddSimpleFrontend('pg_dump', 1);
    $pgdump->AddIncludeDir('src\backend');
    $pgdump->AddFile('src\bin\pg_dump\pg_dump.c');
    $pgdump->AddFile('src\bin\pg_dump\common.c');
    $pgdump->AddFile('src\bin\pg_dump\pg_dump_sort.c');
<<<<<<< HEAD
    $pgdump->AddFile('src\bin\pg_dump\keywords.c');
    $pgdump->AddFile('src\backend\parser\kwlookup.c');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

    my $pgdumpall = AddSimpleFrontend('pg_dump', 1);
    $pgdumpall->{name} = 'pg_dumpall';
    $pgdumpall->AddIncludeDir('src\backend');
    $pgdumpall->AddFile('src\bin\pg_dump\pg_dumpall.c');
<<<<<<< HEAD
    $pgdumpall->AddFile('src\bin\pg_dump\keywords.c');
    $pgdumpall->AddFile('src\backend\parser\kwlookup.c');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

    my $pgrestore = AddSimpleFrontend('pg_dump', 1);
    $pgrestore->{name} = 'pg_restore';
    $pgrestore->AddIncludeDir('src\backend');
    $pgrestore->AddFile('src\bin\pg_dump\pg_restore.c');
<<<<<<< HEAD
    $pgrestore->AddFile('src\bin\pg_dump\keywords.c');
    $pgrestore->AddFile('src\backend\parser\kwlookup.c');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

    my $zic = $solution->AddProject('zic','exe','utils');
    $zic->AddFiles('src\timezone','zic.c','ialloc.c','scheck.c','localtime.c');
    $zic->AddReference($libpgport);

    if ($solution->{options}->{xml})
    {
        $contrib_extraincludes->{'pgxml'} = [
            $solution->{options}->{xml} . '\include',
            $solution->{options}->{xslt} . '\include',
            $solution->{options}->{iconv} . '\include'
        ];

        $contrib_extralibs->{'pgxml'} = [
            $solution->{options}->{xml} . '\lib\libxml2.lib',
            $solution->{options}->{xslt} . '\lib\libxslt.lib'
        ];
    }
    else
    {
        push @contrib_excludes,'xml2';
    }

    if (!$solution->{options}->{openssl})
    {
        push @contrib_excludes,'sslinfo';
    }

    if ($solution->{options}->{uuid})
    {
       $contrib_extraincludes->{'uuid-ossp'} = [ $solution->{options}->{uuid} . '\include' ];
       $contrib_extralibs->{'uuid-ossp'} = [ $solution->{options}->{uuid} . '\lib\uuid.lib' ];
    }
	 else
    {
       push @contrib_excludes,'uuid-ossp';
    }

    # Pgcrypto makefile too complex to parse....
    my $pgcrypto = $solution->AddProject('pgcrypto','dll','crypto');
    $pgcrypto->AddFiles(
        'contrib\pgcrypto','pgcrypto.c','px.c','px-hmac.c',
        'px-crypt.c','crypt-gensalt.c','crypt-blowfish.c','crypt-des.c',
        'crypt-md5.c','mbuf.c','pgp.c','pgp-armor.c',
        'pgp-cfb.c','pgp-compress.c','pgp-decrypt.c','pgp-encrypt.c',
        'pgp-info.c','pgp-mpi.c','pgp-pubdec.c','pgp-pubenc.c',
        'pgp-pubkey.c','pgp-s2k.c','pgp-pgsql.c'
    );
    if ($solution->{options}->{openssl})
    {
        $pgcrypto->AddFiles('contrib\pgcrypto', 'openssl.c','pgp-mpi-openssl.c');
    }
    else
    {
        $pgcrypto->AddFiles(
            'contrib\pgcrypto', 'md5.c','sha1.c','sha2.c',
            'internal.c','internal-sha2.c','blf.c','rijndael.c',
            'fortuna.c','random.c','pgp-mpi-internal.c','imath.c'
        );
    }
    $pgcrypto->AddReference($postgres);
    $pgcrypto->AddLibrary('wsock32.lib');
    my $mf = Project::read_file('contrib/pgcrypto/Makefile');
    GenerateContribSqlFiles('pgcrypto', $mf);

    my $D;
    opendir($D, 'contrib') || croak "Could not opendir on contrib!\n";
    while (my $d = readdir($D))
    {
        next if ($d =~ /^\./);
        next unless (-f "contrib/$d/Makefile");
<<<<<<< HEAD
        next if (grep {/^$d\s*$/} @contrib_excludes);
=======
        next if (grep {/^$d$/} @contrib_excludes);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
        AddContrib($d);
    }
    closedir($D);

    $mf = Project::read_file('src\backend\utils\mb\conversion_procs\Makefile');
    $mf =~ s{\\s*[\r\n]+}{}mg;
    $mf =~ m{DIRS\s*=\s*(.*)$}m || die 'Could not match in conversion makefile' . "\n";
    foreach my $sub (split /\s+/,$1)
    {
        my $mf = Project::read_file('src\backend\utils\mb\conversion_procs\\' . $sub . '\Makefile');
        my $p = $solution->AddProject($sub, 'dll', 'conversion procs');
        $p->AddFile('src\backend\utils\mb\conversion_procs\\' . $sub . '\\' . $sub . '.c');
<<<<<<< HEAD
        if ($mf =~ m{^SRCS\s*\+=\s*(.*)\s*$}m)
=======
        if ($mf =~ m{^SRCS\s*\+=\s*(.*)$}m)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
        {
            $p->AddFile('src\backend\utils\mb\conversion_procs\\' . $sub . '\\' . $1);
        }
        $p->AddReference($postgres);
    }

    $mf = Project::read_file('src\bin\scripts\Makefile');
    $mf =~ s{\\s*[\r\n]+}{}mg;
    $mf =~ m{PROGRAMS\s*=\s*(.*)$}m || die 'Could not match in bin\scripts\Makefile' . "\n";
    foreach my $prg (split /\s+/,$1)
    {
        my $proj = $solution->AddProject($prg,'exe','bin');
        $mf =~ m{$prg\s*:\s*(.*)$}m || die 'Could not find script define for $prg' . "\n";
        my @files = split /\s+/,$1;
        foreach my $f (@files)
        {
<<<<<<< HEAD
			$f =~ s/\.o$/\.c/;
            if ($f eq 'keywords.c')
=======
            if ($f =~ /\/keywords\.o$/)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
            {
                $proj->AddFile('src\backend\parser\keywords.c');
                $proj->AddIncludeDir('src\backend');
            }
<<<<<<< HEAD
            elsif ($f eq '$(top_builddir)/src/backend/parser/keywords.c')
            {
                $proj->AddFile('src\backend\parser\keywords.c');
                $proj->AddIncludeDir('src\backend');
            }
            elsif ($f eq 'kwlookup.c')
            {
                $proj->AddFile('src\backend\parser\kwlookup.c');
            }
            elsif ($f eq 'dumputils.c')
            {
                $proj->AddFile('src\bin\pg_dump\dumputils.c');
            }
            elsif ($f =~ /print\.c$/)
            { # Also catches mbprint.c
                $proj->AddFile('src\bin\psql\\' . $f);
            }
            else
            {
                $proj->AddFile('src\bin\scripts\\' . $f);
=======
            else
            {
                $f =~ s/\.o$/\.c/;
                if ($f eq 'dumputils.c')
                {
                    $proj->AddFile('src\bin\pg_dump\dumputils.c');
                }
                elsif ($f =~ /print\.c$/)
                { # Also catches mbprint.c
                    $proj->AddFile('src\bin\psql\\' . $f);
                }
                else
                {
                    $proj->AddFile('src\bin\scripts\\' . $f);
                }
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
            }
        }
        $proj->AddIncludeDir('src\interfaces\libpq');
        $proj->AddIncludeDir('src\bin\pg_dump');
        $proj->AddIncludeDir('src\bin\psql');
        $proj->AddReference($libpq,$libpgport);
        $proj->AddResourceFile('src\bin\scripts','PostgreSQL Utility');
    }

    # Regression DLL and EXE
    my $regress = $solution->AddProject('regress','dll','misc');
    $regress->AddFile('src\test\regress\regress.c');
<<<<<<< HEAD
    $regress->AddLibrary('wsock32.lib');
    $regress->AddLibrary('ws2_32.lib');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    $regress->AddReference($postgres);

    my $pgregress = $solution->AddProject('pg_regress','exe','misc');
    $pgregress->AddFile('src\test\regress\pg_regress.c');
    $pgregress->AddFile('src\test\regress\pg_regress_main.c');
    $pgregress->AddIncludeDir('src\port');
    $pgregress->AddDefine('HOST_TUPLE="i686-pc-win32vc"');
<<<<<<< HEAD
    $pgregress->AddDefine('FRONTEND');
    $pgregress->AddLibrary('wsock32.lib');
    $pgregress->AddLibrary('ws2_32.lib');
    $pgregress->AddReference($libpgport);

    $solution->Save();

    # Upgrade the files to work with Visual Studio newer than 2005
    system("devenv /upgrade pgsql.sln");
=======
    $pgregress->AddReference($libpgport);

    $solution->Save();
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
}

#####################
# Utility functions #
#####################

# Add a simple frontend project (exe)
sub AddSimpleFrontend
{
    my $n = shift;
    my $uselibpq= shift;

    my $p = $solution->AddProject($n,'exe','bin');
    $p->AddDir('src\bin\\' . $n);
<<<<<<< HEAD
    $p->AddDefine('FRONTEND');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    $p->AddReference($libpgport);
    if ($uselibpq)
    {
        $p->AddIncludeDir('src\interfaces\libpq');
        $p->AddReference($libpq);
    }
    return $p;
}

# Add a simple contrib project
sub AddContrib
{
    my $n = shift;
<<<<<<< HEAD
    if (length($n) > 1 && (substr($n,length($n)-1) eq "\r"))
        {
			chop $n;
		}
    my $mf = Project::read_file('contrib\\' . $n . '\Makefile');

    if ($mf =~ m/^MODULE_big\s*=\s*(.*)\s*$/mg)
    {
        my $dn = $1;
       
			
		if (length($dn) > 1 && (substr($dn,length($dn)-1) eq "\r"))
        {
			chop $dn;
		}	
        $mf =~ s{\\\s*[\r\n]+}{}mg;
        $dn =~ s{\\\s*[\r\n]+}{}mg;
        
        if (length($dn) > 1 && (substr($dn,length($dn)-1) eq "\r"))
        {
			chop $dn;
		}	

=======
    my $mf = Project::read_file('contrib\\' . $n . '\Makefile');

    if ($mf =~ /^MODULE_big\s*=\s*(.*)$/mg)
    {
        my $dn = $1;
        $mf =~ s{\\\s*[\r\n]+}{}mg;
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
        my $proj = $solution->AddProject($dn, 'dll', 'contrib');
        $mf =~ /^OBJS\s*=\s*(.*)$/gm || croak "Could not find objects in MODULE_big for $n\n";
		my $objs = $1;
        while ($objs =~ /\b([\w-]+\.o)\b/g)
        {
			my $o = $1;
            $o =~ s/\.o$/.c/;
            $proj->AddFile('contrib\\' . $n . '\\' . $o);
        }
        $proj->AddReference($postgres);
<<<<<<< HEAD
        if ($mf =~ /^SUBDIRS\s*:?=\s*(.*)\s*$/mg)
=======
        if ($mf =~ /^SUBDIRS\s*:?=\s*(.*)$/mg)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
        {
            foreach my $d (split /\s+/, $1)
            {
                my $mf2 = Project::read_file('contrib\\' . $n . '\\' . $d . '\Makefile');
                $mf2 =~ s{\\\s*[\r\n]+}{}mg;
<<<<<<< HEAD
                $mf2 =~ /^SUBOBJS\s*=\s*(.*)\s*$/gm
=======
                $mf2 =~ /^SUBOBJS\s*=\s*(.*)$/gm
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
                  || croak "Could not find objects in MODULE_big for $n, subdir $d\n";
                $objs = $1;
				while ($objs =~ /\b([\w-]+\.o)\b/g)
				{
					my $o = $1;
                    $o =~ s/\.o$/.c/;
                    $proj->AddFile('contrib\\' . $n . '\\' . $d . '\\' . $o);
                }
            }
        }
        AdjustContribProj($proj);
    }
<<<<<<< HEAD
    elsif ($mf =~ m/^MODULES\s*=\s*(.*)\s*$/mg)
=======
    elsif ($mf =~ /^MODULES\s*=\s*(.*)$/mg)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    {
        foreach my $mod (split /\s+/, $1)
        {
            my $proj = $solution->AddProject($mod, 'dll', 'contrib');
            $proj->AddFile('contrib\\' . $n . '\\' . $mod . '.c');
            $proj->AddReference($postgres);
            AdjustContribProj($proj);
        }
    }
<<<<<<< HEAD
    elsif ($mf =~ m/^PROGRAM\s*=\s*(.*)\s*$/mg)
=======
    elsif ($mf =~ /^PROGRAM\s*=\s*(.*)$/mg)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    {
        my $proj = $solution->AddProject($1, 'exe', 'contrib');
        $mf =~ /^OBJS\s*=\s*(.*)$/gm || croak "Could not find objects in MODULE_big for $n\n";
        my $objs = $1;
        while ($objs =~ /\b([\w-]+\.o)\b/g)
        {
			my $o = $1;
            $o =~ s/\.o$/.c/;
            $proj->AddFile('contrib\\' . $n . '\\' . $o);
        }
        AdjustContribProj($proj);
    }
    else
    {
        croak "Could not determine contrib module type for $n\n";
    }

    # Are there any output data files to build?
    GenerateContribSqlFiles($n, $mf);
}

sub GenerateContribSqlFiles
{
    my $n = shift;
    my $mf = shift;
<<<<<<< HEAD
    if ($mf =~ m/^DATA_built\s*=\s*(.*)\s*$/mg)
=======
    if ($mf =~ /^DATA_built\s*=\s*(.*)$/mg)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    {
        my $l = $1;

        # Strip out $(addsuffix) rules
        if (index($l, '$(addsuffix ') >= 0)
        {
            my $pcount = 0;
            my $i;
            for ($i = index($l, '$(addsuffix ') + 12; $i < length($l); $i++)
            {
                $pcount++ if (substr($l, $i, 1) eq '(');
                $pcount-- if (substr($l, $i, 1) eq ')');
                last if ($pcount < 0);
            }
            $l = substr($l, 0, index($l, '$(addsuffix ')) . substr($l, $i+1);
        }

        # Special case for contrib/spi
        $l = "autoinc.sql insert_username.sql moddatetime.sql refint.sql timetravel.sql"
          if ($n eq 'spi');

        foreach my $d (split /\s+/, $l)
        {
<<<<<<< HEAD
            next unless (length($d) > 0);
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
            my $in = "$d.in";
            my $out = "$d";

            if (Solution::IsNewer("contrib/$n/$out", "contrib/$n/$in"))
            {
                print "Building $out from $in (contrib/$n)...\n";
                my $cont = Project::read_file("contrib/$n/$in");
                my $dn = $out;
                $dn =~ s/\.sql$//;
                if ($mf =~ /^MODULE_big\s*=\s*(.*)$/m) { $dn = $1 }
                $cont =~ s/MODULE_PATHNAME/\$libdir\/$dn/g;
                my $o;
                open($o,">contrib/$n/$out") || croak "Could not write to contrib/$n/$d";
                print $o $cont;
                close($o);
            }
        }
    }
}

sub AdjustContribProj
{
    my $proj = shift;
    my $n = $proj->{name};

    if ($contrib_defines->{$n})
    {
        foreach my $d ($contrib_defines->{$n})
        {
            $proj->AddDefine($d);
        }
    }
<<<<<<< HEAD
    if (grep {/^$n\s*$/} @contrib_uselibpq)
=======
    if (grep {/^$n$/} @contrib_uselibpq)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    {
        $proj->AddIncludeDir('src\interfaces\libpq');
        $proj->AddReference($libpq);
    }
<<<<<<< HEAD
    if (grep {/^$n\s*$/} @contrib_uselibpgport)
=======
    if (grep {/^$n$/} @contrib_uselibpgport)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
    {
        $proj->AddReference($libpgport);
    }
    if ($contrib_extralibs->{$n})
    {
        foreach my $l (@{$contrib_extralibs->{$n}})
        {
            $proj->AddLibrary($l);
        }
    }
    if ($contrib_extraincludes->{$n})
    {
        foreach my $i (@{$contrib_extraincludes->{$n}})
        {
            $proj->AddIncludeDir($i);
        }
    }
    if ($contrib_extrasource->{$n})
    {
        $proj->AddFiles('contrib\\' . $n, @{$contrib_extrasource->{$n}});
    }
<<<<<<< HEAD
    $proj->AddLibrary('wsock32.lib');
    $proj->AddLibrary('ws2_32.lib');
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
}

1;
