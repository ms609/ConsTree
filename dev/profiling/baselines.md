# Baselines

Refreshed by each `/profile` round; `/profile regress` flags > 10 % slowdowns.
Machine: i7-10700 (8 cores), Windows, Rtools45 gcc 14.2, `-O2 -fopenmp`.

| Driver / config | Threads | Median (s) | Top hotspot | Date |
|-----------------|--------:|-----------:|-------------|------|
| RStar k = 2, n = 8000, similar | 1 | 0.66 | `twoTreeSimilarity` | 2026-09-16 |
| RStar k = 10, n = 1200, similar | 1 | 1.58 | `countTriplets` (tally, ~80 %) | 2026-09-16 |
| RStar k = 10, n = 1200, similar | 8 | 0.39 | `countTriplets` | 2026-09-16 |
| RStar k = 50, n = 300, similar | 1 | 0.14 | `countTriplets` | 2026-09-16 |
