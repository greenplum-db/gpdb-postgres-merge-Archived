-- Use ONLY plperlu tests here. For plperl/plerlu combined tests
-- see plperl_plperlu.sql

<<<<<<< HEAD
-- Must load plperl before we can set on_plperlu_init
=======
-- Avoid need for custom_variable_classes = 'plperl'
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
LOAD 'plperl';

-- Test plperl.on_plperlu_init gets run
SET plperl.on_plperlu_init = '$_SHARED{init} = 42';
DO $$ warn $_SHARED{init} $$ language plperlu;

--
-- Test compilation of unicode regex - regardless of locale.
-- This code fails in plain plperl in a non-UTF8 database.
--
CREATE OR REPLACE FUNCTION perl_unicode_regex(text) RETURNS INTEGER AS $$
  return ($_[0] =~ /\x{263A}|happy/i) ? 1 : 0; # unicode smiley
$$ LANGUAGE plperlu;
