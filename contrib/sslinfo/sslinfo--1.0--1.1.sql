/* contrib/sslinfo/sslinfo--1.0--1.1.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "ALTER EXTENSION sslinfo UPDATE TO '1.1'" to load this file. \quit

<<<<<<< HEAD
CREATE OR REPLACE FUNCTION
=======
CREATE FUNCTION
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
ssl_extension_info(OUT name text,
	OUT value text,
    OUT critical boolean
) RETURNS SETOF record
AS 'MODULE_PATHNAME', 'ssl_extension_info'
LANGUAGE C STRICT;
