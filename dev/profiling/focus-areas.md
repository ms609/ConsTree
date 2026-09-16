# Profiling focus areas

Ranked rotation for `/profile`. Only RStar has been profiled so far; run
`/profile init` to rank the remaining methods.

| # | Area | Files | Why hot | Baseline cost | Last profiled | Status |
|---|------|-------|---------|---------------|---------------|--------|
| 1 | `RStar()` | `src/rstar.cpp`, `R/rstar.R` | O(k n^3) triplet tally; O(n^2) two-tree path | k = 10, n = 1200: 1.5 s (1 thread), 0.3–0.4 s (8) | 2026-09-16 | OPTIMISED |
