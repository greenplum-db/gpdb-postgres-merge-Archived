# Configuration arguments for vcbuild.
use strict;
use warnings;

our $config = {
    asserts=>1,			# --enable-cassert
    # integer_datetimes=>1,   # --enable-integer-datetimes - on is now default
    # float4byval=>1,         # --disable-float4-byval, on by default
    # float8byval=>1,         # --disable-float8-byval, on by default
    # blocksize => 32,        # --with-blocksize, 8kB by default
    # ldap=>1,				# --with-ldap
    nls=>undef,				# --enable-nls=<path>
<<<<<<< HEAD
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
=======
    tcl=>'c:\tcl',		# --with-tls=<path>
    perl=>'c:\perl', 			# --with-perl
    python=>'c:\python24', # --with-python=<path>
    krb5=>'c:\prog\pgsql\depend\krb5', # --with-krb5=<path>
    ldap=>1,			# --with-ldap
    openssl=>'c:\openssl', # --with-ssl=<path>
    uuid=>'c:\prog\pgsql\depend\ossp-uuid', #--with-ossp-uuid
    xml=>'c:\prog\pgsql\depend\libxml2',
    xslt=>'c:\prog\pgsql\depend\libxslt',
    iconv=>'c:\prog\pgsql\depend\iconv',
    zlib=>'c:\prog\pgsql\depend\zlib'# --with-zlib=<path>
>>>>>>> 0f855d621b
};

1;
