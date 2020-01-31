<<<<<<< HEAD
#
# Makefile for Kerberos authentication test.
#
# Most of the logic is in installcheck.sh.
#
=======
#-------------------------------------------------------------------------
#
# Makefile for src/test/kerberos
#
# Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# src/test/kerberos/Makefile
#
#-------------------------------------------------------------------------
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

subdir = src/test/kerberos
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
installcheck:
	PATH=/usr/sbin:/usr/local/sbin:$(PATH) bash installcheck.sh

# Stop the test KDC server, if it's running.
.PHONY: stop-kdc
stop-kdc:
	if [ -e ./krb5kdc.pid ]; then kill `cat ./krb5kdc.pid`; fi || true
	rm -f ./krb5kdc.pid

clean: stop-kdc
	rm -rf test-kdc-db
	rm -f krb5cc server.keytab client.keytab
	rm -f pg_hba.conf.orig pg_hba.conf.kerberized postgresql.conf.orig postgresql.conf.kerberized
=======
export with_gssapi with_krb_srvnam

check:
	$(prove_check)

installcheck:
	$(prove_installcheck)

clean distclean maintainer-clean:
	rm -rf tmp_check
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
