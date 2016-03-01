@echo off
<<<<<<< HEAD
REM src/tools/msvc/vcregress.bat
=======
REM $PostgreSQL: pgsql/src/tools/msvc/vcregress.bat,v 1.15 2007/09/27 21:13:11 adunstan Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
REM all the logic for this now belongs in vcregress.pl. This file really
REM only exists so you don't have to type "perl vcregress.pl"
REM Resist any temptation to add any logic here.
@perl vcregress.pl %*
