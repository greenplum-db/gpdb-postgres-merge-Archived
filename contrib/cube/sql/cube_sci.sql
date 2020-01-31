---
--- Testing cube output in scientific notation.  This was put into separate
--- test, because has platform-depending output.
---

SELECT '1e27'::cube AS cube;
SELECT '-1e27'::cube AS cube;
SELECT '1.0e27'::cube AS cube;
SELECT '-1.0e27'::cube AS cube;
SELECT '1e+27'::cube AS cube;
SELECT '-1e+27'::cube AS cube;
SELECT '1.0e+27'::cube AS cube;
SELECT '-1.0e+27'::cube AS cube;
SELECT '1e-7'::cube AS cube;
SELECT '-1e-7'::cube AS cube;
SELECT '1.0e-7'::cube AS cube;
SELECT '-1.0e-7'::cube AS cube;
<<<<<<< HEAD
SELECT '1e-700'::cube AS cube;
SELECT '-1e-700'::cube AS cube;
=======
SELECT '1e-300'::cube AS cube;
SELECT '-1e-300'::cube AS cube;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
SELECT '1234567890123456'::cube AS cube;
SELECT '+1234567890123456'::cube AS cube;
SELECT '-1234567890123456'::cube AS cube;
