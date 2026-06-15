# Red-Team Review — Area 9: Transfer consensus

- **Date:** 2026-06-15
- **Tier:** sonnet (finder) + opus/haiku (severity-matched verifiers)
- **Scope:** `src/transfer_consensus.cpp`, `R/transfer.R`, `tests/testthat/test-transfer.R`
- **Oracle:** upstream TreeDist at `C:\Users\pjjg18\GitHub\TreeDist` (installed; `TransferConsensus` works on the test inputs below)
- **Verdict:** CONDITIONAL — one confirmed correctness bug (TC-002), one real regression (TC-001),
  one latent overflow shared with the oracle (TC-004), three test-quality gaps (TC-005/6/7).
  ~~No shipping crash.~~ **Corrected Round 3: there WAS a shipping crash** (TC-009, below).
- **RESOLUTION (2026-06-15 fix pass):** TC-001/002/004/005/006/007 all **FIXED** and verified
  against an installed build (full `test-transfer.R`: 150 pass / 0 fail). See `verify-fixes.R`
  and the log.md "Round 2" entry.
- **RESOLUTION 2 (2026-06-15 Round 3, ASAN-driven):** A gcc-AddressSanitizer CI run caught a
  stack-buffer-overflow in `PooledSplits`'s ctor. Root cause **TC-009**: `PooledSplits`/
  `SplitHash`/`SplitEqual` were defined at namespace scope with different layouts in BOTH
  `transfer_consensus.cpp` and `Quartet.cpp` → COMDAT special-member folding linked Quartet's
  136-byte ctor over transfer's 120-byte `pool`. **FIXED** via anonymous namespaces in both
  files. This was also the true cause of the "TC-008 load_all segfault" (NOT a header skew).
  `load_all` now runs Transfer+Quartet cleanly; suites pass. See `test-odr-fix.R`, `run-tests.R`
  and the log.md "Round 3" entry. ASAN-green confirmation pending a push.

## Confirmed findings

| id | severity | verdict | summary |
|----|----------|---------|---------|
| **TC-002** | **HIGH** | **REAL (executed)** | `Transfer()` silently accepts trees whose tip-label sets differ (subset case) and returns a wrong consensus with **no error or warning**. Same cross-cutting validation gap that `1d77249` closed in RStar/Local/Quartet/Frequency — missed in Transfer (ported later, 2026-06-09). |
| TC-001 | MED | REAL (static, opus-verified) | The `.CheckMaxTips(nTip)` guard present in the TreeDist oracle (`TreeDist/R/transfer_consensus.R:60,109`) was dropped — `.CheckMaxTips` is defined **nowhere** in ConsTree. Port regression. Practical harm only at nTip > 32767 (gates TC-004). |
| TC-004 | LOW-MED | REAL but LATENT | `M * M` computed in 32-bit `int` at `src/transfer_consensus.cpp:221,302`; overflow (UB / under-alloc / OOB) for M > 46340 unique splits. **Byte-identical in the TreeDist oracle** → not a port regression; reachable only at very large inputs (nTrees unbounded). |
| TC-005 | MED | REAL | The only R-vs-C++ bridge test (`tests/testthat/test-transfer.R:708-713`) asserts only sorted tip-label equality + `NSplits >= 1`; does not pin split content. Non-discriminating. |
| TC-006 | MED | REAL | Unit tests exercise the pure-R reimplementation (`.DoAdd`/`.GreedyBest`/… in `R/transfer.R`), not the shipped C++ greedy path (`greedy_best`/`greedy_first`). C++-only regressions would pass. |
| TC-007 | LOW | REAL | Comment `R/transfer.R:71` "as.Splits() will error if a tree's tips don't match tipLabels" is **provably false** (TC-002). Trivial fix, but bundle with TC-002. |

## Refuted

| id | verdict |
|----|---------|
| TC-003 | **REFUTED** — duplicate labels in the first tree are caught: `RenumberTips` errors "Tree labels `a` repeated in `tipOrder`". No silent corruption. At most a nit (cryptic message; no explicit wrapper guard). |

## Empirical evidence (against the INSTALLED package — `R CMD INSTALL` of main HEAD)

```
sanity 6+6                 -> EXIT=0  OK 2 splits
TC-002a 6-tip then 5-tip   -> EXIT=0  RET 2     (silent wrong result — NO error)   <-- the bug
TC-002b 5-tip then 6-tip   -> EXIT=0  "Missing in `tipOrder`: f"  (errors — asymmetric)
TC-003  dup label 'a'      -> EXIT=0  "labels a repeated"         (errors — not silent)
```

Drivers: `repro-tip-validation.R`, `repro-segfault-isolate.R` (these use `pkgload::load_all`
and must be adapted to an installed lib — see caveat).

## ~~CAVEAT — `load_all` is broken~~ — RETRACTED Round 3: it was the TC-009 ODR bug

**This earlier hypothesis was WRONG.** Rounds 1–2 attributed the `load_all` "segfault on
the happy path" to a TreeTools `SplitList.h` byte-width skew (a supposed dev-infra artifact),
and concluded the installed package was safe. Round 3 disproved both:

- The real cause is the **cross-TU ODR violation TC-009** (duplicated `PooledSplits`/
  `SplitHash`/`SplitEqual` at namespace scope in `transfer_consensus.cpp` + `Quartet.cpp`).
  Under `load_all`'s link order the folded COMDAT ctor corrupted the stack → segfault; a
  gcc-ASAN install caught the same corruption deterministically as a stack-buffer-overflow.
- "Installed package works / returns NSplits=2" was **luck**, not safety — the same UB, with
  a link order whose overwrite happened to land on benign padding. Not something to rely on.
- After fixing TC-009 (anonymous namespaces), **`load_all` runs `Transfer()` and `Quartet()`
  cleanly** (NSplits 3 each on `as.phylo(0:9, nTip=8)`); transfer+Quartet suites pass.

**Consequence (updated):** `load_all` is now the correct, working dev loop for this path.
Drivers: `test-odr-fix.R` (happy-path under load_all), `run-tests.R` (suites). The old
"use an installed build" workaround is no longer needed.

## Ruled out (no issue)
- OpenMP data races in all three parallel regions (`transfer_dist_mat`, `compute_td`,
  `compat_mat`) — each output cell written by exactly one thread; deterministic across
  thread counts; no R/Rcpp API in parallel regions. (Crash is identical at OMP_NUM_THREADS=1.)
- Star-tree / n<4-tip / single-orthant edge cases.
- Signature sync: C++ export, both RcppExports, and the R call site all agree (6 args).

## Notes for next reviewer of area 9 (still yielding — stay at sonnet)
- TC-002 fix: add `setequal(TipLabels(tr), tipLabels)` + `anyDuplicated` validation in
  `Transfer()` (mirror `R/rstar.R:103`), and fix the TC-007 comment in the same patch.
- The greedy C++ path has **no** discriminating test (TC-005/6); a pinned-consensus oracle
  test (compare to TreeDist on fixed inputs) would close TC-005, TC-006 together.
- Re-confirm TC-004 reachability vs any practical cap on unique-split count.
