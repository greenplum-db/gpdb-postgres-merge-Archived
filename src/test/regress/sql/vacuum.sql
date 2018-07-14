--
-- VACUUM
--

CREATE TABLE vactst (j INT DEFAULT 1,i INT);
INSERT INTO vactst(i) VALUES (1);
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst(i) VALUES (0);
SELECT count(*) FROM vactst;
DELETE FROM vactst WHERE i != 0;
SELECT i FROM vactst ORDER BY 1;
VACUUM FULL vactst;
UPDATE vactst SET i = i + 1;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst SELECT * FROM vactst;
INSERT INTO vactst(i) VALUES (0);
SELECT count(*) FROM vactst;
DELETE FROM vactst WHERE i != 0;
VACUUM (FULL) vactst;
DELETE FROM vactst;
SELECT i FROM vactst ORDER BY 1;

VACUUM (FULL, FREEZE) vactst;
VACUUM (ANALYZE, FULL) vactst;

CREATE TABLE vaccluster (i INT PRIMARY KEY);
ALTER TABLE vaccluster CLUSTER ON vaccluster_pkey;
INSERT INTO vaccluster SELECT i FROM vactst;
CLUSTER vaccluster;

VACUUM FULL pg_am;
VACUUM FULL pg_class;
VACUUM FULL pg_database;
VACUUM FULL vaccluster;
VACUUM FULL vactst;

CREATE TABLE vacstat_test (a int);
INSERT INTO vacstat_test SELECT i FROM generate_series(1,10) i ;
VACUUM vacstat_test;

-- Confirm that VACUUM has updated stats from all nodes
SELECT true FROM pg_class WHERE oid='vacstat_test'::regclass
AND relpages > 0
AND reltuples > 0
AND relallvisible > 0;

SELECT true FROM pg_class WHERE oid='vacstat_test'::regclass
AND relpages =
	(SELECT SUM(relpages) FROM gp_dist_random('pg_class')
	 WHERE oid='vacstat_test'::regclass)
AND reltuples =
	(SELECT SUM(reltuples) FROM gp_dist_random('pg_class')
	 WHERE oid='vacstat_test'::regclass)
AND relallvisible =
	(SELECT SUM(relallvisible) FROM gp_dist_random('pg_class')
	 WHERE oid='vacstat_test'::regclass);


DROP TABLE vaccluster;
DROP TABLE vactst;
DROP TABLE vacstat_test;
