# Configuration arguments for vcbuild.
use strict;
use warnings;

our $config = {
<<<<<<< HEAD
    asserts=>1,			# --enable-cassert
    # integer_datetimes=>1,   # --enable-integer-datetimes - on is now default
    # float4byval=>1,         # --disable-float4-byval, on by default
    # float8byval=>1,         # --disable-float8-byval, on by default
    # blocksize => 32,        # --with-blocksize, 8kB by default
    # ldap=>1,				# --with-ldap
=======
    asserts=>0,			# --enable-cassert
    integer_datetimes=>1,   # --enable-integer-datetimes
>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238
    nls=>undef,				# --enable-nls=<path>
    tcl=>undef,				# --with-tls=<path>
    perl=>undef, 			# --with-perl
    python=>undef,			# --with-python=<path>
    krb5=>undef,			# --with-krb5=<path>
    openssl=>undef,			# --with-ssl=<path>
    uuid=>undef,			# --with-ossp-uuid
    xml=>undef,				# --with-libxml=<path>
    xslt=>undef,			# --with-libxslt=<path>
    iconv=>undef,			# (not in configure, path to iconv)
    zlib=>'c:\zlib64',			# --with-zlib=<path>  (GPDB needs zlib)
    pthread=>'c:\pthreads',  		# gpdb needs pthreads 
    curl=>'c:\zlib', 			# gpdb needs libcurl
    bzlib=>'c:\pgbuild\bzlib'
    #readline=>'c:\progra~1\GnuWin32' 	# readline for windows?
};

1;
