-- test plperl.on_plperl_init errors are fatal

<<<<<<< HEAD
-- Must load plperl before we can set on_plperl_init
LOAD 'plperl';

SET SESSION plperl.on_plperl_init = ' system("/nonesuch"); ';
=======
-- Avoid need for custom_variable_classes = 'plperl'
LOAD 'plperl';

SET SESSION plperl.on_plperl_init = ' system("/nonesuch") ';
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

SHOW plperl.on_plperl_init;

DO $$ warn 42 $$ language plperl;
