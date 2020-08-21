CREATE SCHEMA orca_static_pruning;
SET search_path TO orca_static_pruning;

CREATE TABLE rp (a int, b int, c int) DISTRIBUTED BY (a) PARTITION BY RANGE (b);
CREATE TABLE rp0 PARTITION OF rp FOR VALUES FROM (MINVALUE) TO (10);
CREATE TABLE rp1 PARTITION OF rp FOR VALUES FROM (10) TO (20);
CREATE TABLE rp2 PARTITION OF rp FOR VALUES FROM (4200) TO (4203);

INSERT INTO rp VALUES (0, 0, 0), (11, 11, 0), (4201, 4201, 0);

SELECT $query$
SELECT *
FROM rp
WHERE b < 4200
$query$ AS qry \gset

SET optimizer_trace_fallback TO on;
EXPLAIN (COSTS OFF, VERBOSE)
:qry ;

:qry ;
