# Red-Team Findings — ConsTree (open items)

Non-trivial bugs / perf / hardening issues, filed **after verification**.
Trivial issues are fixed inline and not listed here.

Rows below are **carried forward** from the pre-tier-system reviews under
`reviews/<area>/` (each was verified in its originating review) and reconciled
against `main` on **2026-06-15**. Confidence varies by row — the reconciliation
pass (a single read-only sweep) was **wrong on 2 of 2 rows that were spot-checked**
(package-infra F1/F2, both already fixed), so its verdicts are not authoritative:

- **Spot-verified present on main 2026-06-15** (code read directly): C-001, C-002,
  C-003, C-006, C-007, B-001. These are real, currently-open (minor/info) items.
- **Flagged by reconciliation, not yet re-verified**: C-004, C-005 (INFO),
  A-001, A-002, and the T-001 test-quality cluster. Treat as candidates until the
  area's next `/red-team` round confirms with the finder/verifier pair.

Headline "MAJOR" items from the *old* reviews were re-checked and found **already
fixed** (see *Resolved on reconciliation*). The 2026-06-15 area-9 (Transfer)
round then found **one new HIGH correctness bug, TC-002** (see the next section) —
verified by execution against an installed build.

## Area 9 — Transfer consensus (2026-06-15 round, sonnet finder + opus/haiku verifiers)

Full record: `reviews/transfer/review.md`. **TC-003 was REFUTED** (dup labels are
caught, not silently corrupted).

**2026-06-15 fix pass (Round 2) — TC-001/002/004/005/006/007 all FIXED** and
verified against an installed build (full `test-transfer.R`: 150 pass / 0 fail).
Committed e4e0aa9 (code) + d4e9e52 (records).

**2026-06-15 Round 3 (ASAN-driven) — TC-008 was a REAL bug, now FIXED.** A
gcc-AddressSanitizer CI run caught a stack-buffer-overflow in `PooledSplits`'s
constructor. Root cause (**TC-009**): `PooledSplits`/`SplitHash`/`SplitEqual`
were each defined at namespace scope, with *different layouts*, in BOTH
`src/transfer_consensus.cpp` and `src/Quartet.cpp` — an ODR violation. The
compiler-emitted special members are COMDAT symbols mangled by type name only,
so the linker folded them and used Quartet's 136-byte ctor to construct
transfer's 120-byte `pool`, writing one slot past its end. This — *not* a
TreeTools header skew — was also the true cause of the earlier `load_all`
"segfault on the happy path" (previously mislabelled TC-008 as a dev-infra
caveat). FIXED by wrapping each file's TU-private types in an anonymous
namespace (the existing `cons_*.cpp` pattern). `load_all` now runs
Transfer+Quartet cleanly; transfer + Quartet test suites pass. Real ASAN-green
confirmation requires a push (CI). Fixes *not yet committed* at time of writing.

| id | severity | status | area | title | file:line — detail | source |
|----|----------|--------|------|-------|--------------------|--------|
| TC-002 | **HIGH** | **FIXED** | 9 Transfer | Mismatched tip-label sets silently accepted → wrong consensus | `R/transfer.R` — added `anyDuplicated` + `setequal`-across-trees guard (mirrors `R/rstar.R:103`); subset/dup inputs now `stop()`. Regression test added (`test-transfer.R` "Transfer validates tip-label sets"). Verified: subset → error, reorder → ok. | reviews/transfer (executed) |
| TC-001 | MED | **FIXED** | 9 Transfer | `.CheckMaxTips` guard dropped in the port | `R/transfer.R` — added internal `.CheckMaxTips()` (cap 32767, mirrors TreeDist) called from `Transfer()` and `tc_profile()`. | reviews/transfer (opus-verified) |
| TC-004 | LOW-MED | **FIXED** | 9/10 | `M*M` 32-bit overflow in transfer matrices | `src/transfer_consensus.cpp` — all 16 flat-index sites (`M*M`, `i*M+j`, …) now compute the product in `std::size_t` via `static_cast`. Compiles clean; tests pass. | reviews/transfer (opus-verified) |
| TC-005 | MED | **FIXED** | 12 Tests | Transfer R-vs-C++ bridge test non-discriminating | `tests/testthat/test-transfer.R` — bridge test now pins the canonical label-based split set of the C++ path. | reviews/transfer |
| TC-006 | MED | **FIXED** | 12 Tests | Transfer tests exercise the R reimpl, not the C++ path | `tests/testthat/test-transfer.R` — bridge test now asserts the shipped C++ greedy path's splits are **identical** to the pure-R reference (content-level, not just tip labels). | reviews/transfer |
| TC-007 | LOW | **FIXED** | 9 Transfer | Comment is provably false | `R/transfer.R:70-72` — false comment replaced (validation now happens above; `as.Splits` renumbers into tipLabels order). | reviews/transfer |
| TC-009 | **HIGH** | **FIXED** | 9/10 | Cross-TU ODR violation: `PooledSplits`/`SplitHash`/`SplitEqual` duplicated at namespace scope | `src/transfer_consensus.cpp` + `src/Quartet.cpp` defined these 3 types at namespace scope with **different layouts**; COMDAT special-member folding linked Quartet's 136-byte `PooledSplits` ctor over transfer's 120-byte `pool` → stack-buffer-overflow (gcc-ASAN CI run 27196858592). UB / memory corruption; **CRAN ASAN blocker.** Fixed: anonymous namespaces in both files (matches `cons_*.cpp`). Verified: ASAN-failing path now clean under `load_all`; transfer+Quartet tests pass. | gcc-ASAN CI + reviews/transfer (executed) |
| TC-008 | **HIGH** | **FIXED** | 9/10 | `load_all` segfaults on the Transfer path | **Re-diagnosed Round 3:** NOT a TreeTools header skew / dev-infra artifact. Same memory corruption as the ASAN stack-buffer-overflow; root cause is the cross-TU ODR violation **TC-009**, fixed with it. `load_all` now runs Transfer/Quartet cleanly. (Earlier "installed package is fine" was luck — benign overwrite under that link order; UB regardless.) | reviews/transfer |

## Area 10 — FACT primitive & C++ memory safety (2026-06-15 Round 4, sonnet finder)

Finder swept `fact_tree.{cpp,h}` + every consensus kernel for OOB / overflow /
aliasing, OpenMP races, and cross-TU ODR. **Cross-TU ODR sweep CLEAN** beyond
TC-009 (already fixed). **OpenMP regions race-free** (per-element write ownership;
no R/Rcpp calls inside parallel). **C-001 and C-004 REFUTED.** Carried items
**C-002 / C-003 / C-006 / C-007 CONFIRMED** (all latent or degenerate-input-only;
unreachable via the public R API, which routes through `ape::Preorder`). Only one
new live bug: **MEM-001 (HIGH)** — fixed in this pass.

| id | severity | status | area | title | file:line — detail | source |
|----|----------|--------|------|-------|--------------------|--------|
| MEM-001 | **HIGH** | **FIXED** | 10 | `M*M` 32-bit overflow in Quartet `compat` matrix | `src/Quartet.cpp` — `compat(M*M)` alloc (L383) + `i*M+j`/`j*M+c` indexing (L401/402/601/671) were plain `int*int`; overflow is UB for M>46340 unique splits. The Quartet twin of TC-004, **missed** in that pass (transfer was cast, Quartet was not). Fixed: `static_cast<std::size_t>` on the left operand at all 5 sites. Compiles clean; Quartet+transfer suites pass. **CRAN ASAN/UBSan blocker.** | reviews/area10 (executed) |
| MEM-002 | LOW | OPEN | 10/3 | `reinterpret_cast<bool*>(char*)` strict-aliasing UB | `src/cons_frequency.cpp:1342-1348` — `std::vector<char>` buffers passed as `bool*` to `filter()`. Works on every real platform (`sizeof(bool)==1`, 0=false); formally UB, could miscompile under aggressive aliasing. Fix: use `uint8_t` end-to-end or change `filter` to take `char*`. | reviews/area10 |

## Carried-forward open items (from pre-tier-system reviews)

| id | severity | status | area | title | file:line — detail | source |
|----|----------|--------|------|-------|--------------------|--------|
| C-001 | MINOR | **REFUTED** (R4) | 4 Loose | `anc[-1]` deref when root not kept | `src/cons_loose.cpp:~70` — **unreachable.** `precompute()` always hashes the root cluster into `H[1]`, and the op==0 `looseMerge` pass never clears `good[1]` (full-taxon cluster is universally compatible) → `keep[A.root]==1` always; the `anc[-1]` arm is never taken. Latent: no assert guards the `good[1]` dependency. | reviews/area10 |
| C-002 | MINOR | OPEN (R4 confirmed) | 7 R*/Local | `lcaDepth` walks past root −1 | `src/rstar.cpp:~172`, `src/local_consensus.cpp:~48` — `parent[-1]` deref on **disconnected** input only. Unreachable via public API (R wrappers run `ape::Preorder` → connected). Exploitable by direct C++ call / future refactor. | reviews/cpp-memory-safety, area10 |
| C-003 | MINOR | OPEN (R4 confirmed) | 5 MajorityPlus | Leaf `goodLabel` invariant undocumented | `src/cons_majorityplus.cpp:~62` — if a leaf has `goodLabel<=0`, `ret.leaf[label[i]]` = `ret.leaf[-1]` OOB write. Invariant (`updateCounter` increments every leaf) holds now but is unasserted/fragile. | reviews/cpp-memory-safety, area10 |
| C-004 | INFO | **REFUTED** (R4) | 1/10 | `pool.data` pointer stability | `src/Quartet.cpp:214,219` — **resolved.** `total_splits` is `size_t`; `pool.data.reserve(total_splits * n_bytes)` runs before any pointer-keyed `split_map` insert, so the backing store never reallocates and keys stay valid (mirrors transfer). | reviews/area10 |
| C-005 | INFO/perf | OPEN | 1 BHV | O(n²) queue in `gtp_no_common` | `src/bhv.cpp:~167,197` — `vector::erase(begin())` in a loop; fine for current tree sizes | reviews/cpp-memory-safety |
| C-006 | MINOR | OPEN (R4 confirmed) | 10 FACT | Dropped `assert(num==N)` malformed-tree guard | `src/fact_tree.cpp:~79` — forests / duplicate-leaf edge tables leave `H[i]=-1`, garbage `minL/maxL/size`; downstream kernels use them unchecked → silently wrong consensus, no error. Cheapest guard: `if (num != A.N) Rcpp::stop(...)`. | reviews/fact-tree-primitive, area10 |
| C-007 | MINOR | OPEN (R4 confirmed) | 10 FACT | `buildTreeFromEdge` dead root-fallback arm | `src/fact_tree.cpp:~130` — `nRow>0 ? edge(0,0)-1 : 0`; the `:0` (zero-row) arm is dead, and gates a secondary `t.leaf[]` OOB for nTip≥6. Unreachable via wrappers; replace with early `stop()` on nRow==0. | reviews/fact-tree-primitive, area10 |
| B-001 | MINOR | OPEN | 1 BHV | `TOL=1e-10` drop-tolerance is scale-dependent | `src/bhv.cpp:24` — absolute tolerance; very large/small branch lengths could mis-drop edges | reviews/bhv-numerical-precision |
| B-002 | INFO | OPEN | 1 BHV | Fréchet `converged` flag tracks step size, not gradient | `R/BHV.R:~293` (documented) — may report convergence on a slow crawl | reviews/bhv-numerical-precision |
| A-001 | MINOR | OPEN | 8 Average | Non-logical `edgeLengths` falls through silently | `R/Average.R` — no `isTRUE()` validation of the flag | reviews/average-consensus |
| A-002 | INFO | OPEN | 8 Average | NA branch lengths → confusing ape error | `R/Average.R` — no pre-check; deferred ape error is opaque | reviews/average-consensus |
| T-001 | MINOR/quality | OPEN | 12 Tests | Test-suite hardening cluster (~20 gaps) | `tests/testthat/` — discriminating power (inclusion-only assertions T6/T9), heuristic-optimality claims undocumented (T13/T22), RNG drift on shared seed (T19), duplicated `cladeSet` helper (T18), and ~14 more | reviews/test-suite-quality |

## Resolved on reconciliation (2026-06-15) — recorded so they are not re-opened
- **package-infrastructure F1** (was MAJOR, "stale `TreeTools::CompatibleSplits`
  import blocks CRAN") — **FIXED** (86711f8); not present in `NAMESPACE`/`R/`.
- **package-infrastructure F2** (stale `ape::write.tree` in `R/local.R`) — **FIXED** (86711f8).
- **cranky-kare F1** (CRITICAL infinite loop on zero-length incompatible interior
  edge) — **does not reproduce on main**; `BHVDistance` returns √3 without
  hanging. The BLOCK was against the abandoned, never-merged `claude/cranky-kare`
  branch. Repro driver kept at `reviews/claude-cranky-kare/run-against-main-f1.R`.
- **cranky-kare F2** (MAJOR wrong distance on internal singleton) — **does not
  reproduce on main**; `d(B, collapse.singles(B)) = 0`. Driver: `run-against-main-f2.R`.
  (`R/BHV.R:30` `collapse.singles()` at the top of `.TreeToBHV` is the reason.)
- **cranky-kare F3** (MINOR opaque error on missing `edge.length`) — **FIXED**;
  `R/BHV.R:28` now `stop("Trees must have edge.length to compute BHV distances.")`.
- **cpp-memory-safety F1** (MAJOR unchecked depth-index) — **FIXED** (b396a6f).
- **test-suite-quality T1/T5/T8** (MAJOR vacuous lattice tests) — **FIXED** (35869cb).
- **R\*/Local, Quartet, Frequency tip-label validation** — **FIXED** (1d77249, e07f1d6).
