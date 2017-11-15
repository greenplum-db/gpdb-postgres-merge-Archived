--
-- CONSTRAINTS (GPDB-specific)
--

--
-- Regression test: ensure we don't trip any assertions when
-- optimizer_enable_dml_triggers is true
--
SET optimizer_enable_dml_triggers = true;
BEGIN;
CREATE TABLE fkc_primary_table1(a int PRIMARY KEY, b text) DISTRIBUTED BY (a);
CREATE TABLE fkc_foreign_table1(a int REFERENCES fkc_primary_table1 ON DELETE RESTRICT ON UPDATE RESTRICT, b text) DISTRIBUTED BY (a);
INSERT INTO fkc_primary_table1 VALUES (1, 'bar');
INSERT INTO fkc_foreign_table1 VALUES (1, 'bar');
COMMIT;
