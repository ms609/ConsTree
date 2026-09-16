# Profiling log

## Round 1 — 2026-09-16 — RStar (`src/rstar.cpp`)

Driver: `drivers/rstar.R` (k = 2, n = 4000; k = 10, n = 400; mostly congruent
trees).  Also timed near-identical / similar / random regimes on a grid.
R overhead < 10 %; everything below is C++.

**Hotspots before.**  k = 2: per-pair LCA arrays (27 %), per-pair formula (26 %),
Prim MST (18 %).  k >= 3: branchy per-triple, per-tree `closePair` tally (~75 %);
on similar trees the strong-cluster filter (per-pair scalar R_maj scans) was as
costly as the tally.

**Changes (all verified: identical clade sets to the pre-round build on 500
random inputs, 1 and 8 threads, incl. polytomies, duplicates, forced general
path for k = 2).**
- k = 2: walk T1 in leaf blocks with hoisted table rows, one packed T2 record
  per leaf, fill row a only; single-pass Prim over a compacted vertex list.  ~2x.
- k >= 3 tally: tree-major rank matrix, branchless `omp simd` counting over
  the third leaf, uint8 counts; unit-weight kernel specialisation (+16 %).
- Identical input trees merged and weighted.
- Strong-cluster filter: dendrogram-order column permutation so candidates are
  contiguous ranges; majority-clade shortcut; per-pair intruder certificate
  (LCA preorder ranks stored instead of depths identify each pair's clade);
  vectorised range counts only as a fallback; `s < |L \ A|` prefilter first.
- OpenMP (`ConsTree.threads` / `mc.cores`): tally, k = 2 similarity, and
  large candidates' pair tests.

**Rejected (measured, no gain):** signed int16 rank storage (~3 %, noise);
two-counter difference kernel (0 %).  Kernel is at its SSE2 limit
single-threaded (AT-LIMIT for further constant-factor work).

**Result** (median of 3, seconds; before -> 1 thread / 8 threads):

| k | n | regime | before | 1 thr | 8 thr |
|---|---|--------|-------:|------:|------:|
| 2 | 8000 | similar | 1.31 | 0.66 | 0.38 |
| 5 | 600 | near-identical | 1.19 | 0.11 | 0.07 |
| 10 | 1200 | near-identical | 9.65 | 1.46 | 0.31 |
| 10 | 1200 | similar | 10.88 | 1.58 | 0.39 |
| 10 | 1200 | random | 13.85 | 1.53 | 0.39 |
| 50 | 300 | similar | 0.58 | 0.14 | 0.06 |
| 200 | 150 | similar | 0.28 | 0.12 | 0.07 |

**Tooling pitfalls found this round** (now fixed at source).
- `build-symboled-lib.ps1` put `-g` in `PKG_CXXFLAGS`, replacing the package's
  `$(SHLIB_OPENMP_CXXFLAGS)`: no `-fopenmp`, `omp simd` ignored, ~3x slow.  The
  script now sets `CXXFLAGS`/`CXXnnFLAGS` instead.
- `roxygenise()` leaves `-O0` objects in `src/` that `R CMD INSTALL .` reuses;
  AGENTS.md now installs with `--preclean`.
- VTune collection inflates wall time ~3x; never read timings from a collected run.

No GitHub issue filed: every finding was fixed this round.

last_focus: 1
