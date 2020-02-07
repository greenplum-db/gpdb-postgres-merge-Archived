-- All GPDB-added functions are here, instead of pg_proc.h. pg_proc.h should
-- kept as close as possible to the upstream version, to make merging easier.
--
-- This file is translated into DATA rows by catullus.pl. See
-- README.add_catalog_function for instructions on how to run it.

 CREATE FUNCTION pg_resqueue_status() RETURNS SETOF record LANGUAGE internal VOLATILE STRICT PARALLEL RESTRICTED AS 'pg_resqueue_status' WITH (OID=6030, DESCRIPTION="Return resource queue information");

 CREATE FUNCTION pg_resqueue_status_kv() RETURNS SETOF record LANGUAGE internal VOLATILE STRICT PARALLEL RESTRICTED AS 'pg_resqueue_status_kv' WITH (OID=6069, DESCRIPTION="Return resource queue information");
 

