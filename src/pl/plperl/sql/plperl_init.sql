-- test plperl.on_plperl_init errors are fatal

<<<<<<< HEAD
-- Must load plperl before we can set on_plperl_init
=======
-- This test tests setting on_plperl_init after loading plperl
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
LOAD 'plperl';

SET SESSION plperl.on_plperl_init = ' system("/nonesuch"); ';

SHOW plperl.on_plperl_init;

DO $$ warn 42 $$ language plperl;
