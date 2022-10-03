Greenplum (https://github.com/greenplum-db/gpdb) is fork of
PostgreSQL. Greenplum team performs merge with PostgreSQL major/minor
versions into Greenplum to keep Greenplum codebase up to speed.

This repository is used as development space for these merge
iterations. Plus it act as archive for all past merge iterations
frequently referenced by Greenplum Developers. This repository houses
the iterations branches which track individual commits during
PostgreSQL merge work to Greenplum. No production code directly ships
from this repository. Once the merge work in complete, Pull Request is
raised against https://github.com/greenplum-db/gpdb repository and
code merged via merge commit to GPDB repository. Given this purpose,
this repository doesn't house any "main" branch as each iteration
branch have their own originating points.
