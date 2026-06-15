# Red-Team Log — ConsTree

Per-round record for the `/red-team` rotation. Each round notes the area, model
tier, finder yield (count of **confirmed** findings), verifier verdicts, and the
escalation decision. The next invocation reads `last_focus:` at the bottom and
picks area `(last_focus mod 12) + 1`. Per-area maturity is tracked by the most
recent `last_tier:` + `yield:` for that area.

Tiers: finder ladder `sonnet → opus → fable`; verifier `haiku` (low-sev) or
peer-or-higher (high-sev). See the skill for routing.

---

## 2026-06-15 — Consolidation + schedule reconciliation (no review round)

Rebuilt the rotation under the new tier-aware skill. Before today the only record
was `reviews/<area>/review.md` (≈18 areas) plus a stale `tonight-schedule.md`
(2026-06-02/03 six-slot session). Actions:

- **Built `focus-areas.md`** (12 areas, all `start_tier: sonnet`) and **`findings.md`**
  (carried-forward open items, reconciled against `main`).
- **Reconciled the 2026-06-02/03 schedule's open issues** against current main:
  - Slot 2 Quartet (CONDITIONAL) → **fixed** (e07f1d6: pool.count semantics, is_compatible guard, pinned oracle).
  - Slot 3 Frequency (CONDITIONAL, tip-label) → **fixed** (1d77249 + tests 9210fdf).
  - Slot 4 BHV (**BLOCK**, CRITICAL hang + MAJOR singleton) → **does not reproduce on main.**
    Ran both repros against main source (`pkgload::load_all` + OS timeout):
    F1 returns √3=1.732051 without hanging; F2 returns 0. The BLOCK was against
    the abandoned, never-merged `claude/cranky-kare` branch (base 534b349 @ 6329d96),
    not main. F3 (no-edge.length opaque error, MINOR) is itself **fixed** on main
    (`R/BHV.R:28` clear `stop()`); F2's fix (`collapse.singles` at `R/BHV.R:30`) is
    likewise present. Drivers kept: `reviews/claude-cranky-kare/run-against-main-f{1,2}.R`.
  - Slot 5 MajorityPlus/FACT boundary → **REPRO ONLY, no review.md was ever written.**
    The repro (`reviews/majorityplus-fact/repro-01-strict-boundary.R`) confirmed
    strict `>` (not `>=`) is FACT-exact: twoToTwo drops, twoToOne keeps. This
    status existed *only* in the schedule — recorded here before deleting it. Now
    covered by area 5.
  - Slot 6 R*/Local (CONDITIONAL) → **fixed** (1d77249, 9d62a3d). Cross-cutting
    tip-label root cause (F1 in slots 2/3/6) closed by 1d77249.
- **Deleted** the stale `tonight-schedule.md` (all content folded into this log /
  findings.md). Kept `reviews/` — the new files index it.
- Reconciliation also confirmed: package-infra F1/F2 (NAMESPACE/ape import) **fixed**
  (86711f8); cpp-mem F1 **fixed** (b396a6f); test vacuity T1/T5/T8 **fixed** (35869cb).

**No finder/verifier launched today** (scaffold rule: user ratifies the rotation
and tiers first). Recommended first round: **area 9 (Transfer)** — newest code,
entirely unreviewed, with new OpenMP. `last_focus` seeded at 8 so a bare
`/red-team` resumes there.

---

## 2026-06-15 — Round 1: Area 9 (Transfer consensus), sonnet

First live round under the tier system. Finder: `red-team-finder` @ **sonnet**.
Verifiers: high-sev static → `red-team-verifier` @ **opus**; low/med test-quality →
@ **haiku**; execution-dependent claims (TC-002/003) settled by the orchestrator
against an **installed** build (`load_all` is broken on this path — TC-008).

- **Finder yield: 6 confirmed, 1 refuted** (7 candidates).
- **TC-002 (HIGH, REAL)** — `Transfer()` silently accepts mismatched tip-label
  sets (subset case) → wrong consensus, no error. The cross-cutting validation
  gap `1d77249` closed elsewhere, missed in the later-ported Transfer. Confirmed
  by execution (installed pkg: 6-tip+5-tip input returns 2 splits, no error).
- **TC-001 (MED, REAL)** — `.CheckMaxTips` guard dropped vs TreeDist oracle.
- **TC-004 (LOW-MED, REAL but LATENT)** — `M*M` int overflow; byte-identical in
  oracle, not a port regression; very-large-input only.
- **TC-005/006 (MED, REAL)** — test suite tests the R reimpl, not the C++ path;
  bridge test non-discriminating.
- **TC-007 (LOW, REAL)** — false comment at `R/transfer.R:71` (TC-002 disproves it).
- **TC-003 — REFUTED**: duplicate labels are caught by RenumberTips, not silently
  corrupted.
- **TC-008 (INFO/dev)** — the finder's "CRITICAL segfault on the happy path" was a
  `load_all` build artifact (inconsistent TreeTools `SplitList.h`), **not a
  shipping bug**: installed `Transfer()` and the TreeDist oracle both work. This
  near-miss is why the round verified by execution against an installed build.
- **No inline fixes applied** (TC-002's guard is a behaviour change wanting a test;
  recommend bundling TC-007's comment fix with it).
- **Seam status: still yielding** → next visit to area 9 stays at **sonnet** (mine
  the immature seam with a fresh agent before escalating).

last_tier(area 9): sonnet
yield(area 9): 6 confirmed (1 HIGH)

Full record: `reviews/transfer/review.md`. Filed: TC-001/002/004/005/006/007/008
in `findings.md`.

---

## 2026-06-15 — Round 2: Area 9 fix pass

Applied fixes for every actionable Round-1 finding (TC-008 is dev-infra, left as
a caveat). Mechanical C++ widening delegated to a `red-team-finder` @ **sonnet**;
R + test work done by the orchestrator. Verified against an **installed** build
(`R CMD INSTALL` → full `test-transfer.R`: **150 pass / 0 fail**).

- **TC-002 (HIGH)** — `Transfer()` now validates tip labels: `anyDuplicated` +
  `setequal`-across-trees guard (mirrors `R/rstar.R:103`). Subset/duplicate inputs
  `stop()`; same-set-different-order still works. New regression test added.
- **TC-001 (MED)** — added internal `.CheckMaxTips()` (cap 32767, mirrors TreeDist),
  called from `Transfer()` and `tc_profile()`.
- **TC-004 (LOW-MED)** — all 16 flat-index sites in `src/transfer_consensus.cpp`
  now do the `M*M`/`i*M+j` multiplication in `std::size_t`. Loop counters left
  `int` (no sign-compare warnings). Compiles clean.
- **TC-005/006 (MED)** — the R-vs-C++ bridge test now asserts the shipped C++
  greedy path's splits are **identical** to the pure-R reference and pins the
  canonical consensus.
- **TC-007 (LOW)** — false comment at `R/transfer.R:71` corrected.

Empirical drivers kept: `reviews/transfer/verify-fixes.R`. Fixes committed
e4e0aa9 (code) + d4e9e52 (records). Area 9 seam: the structural finds are now
closed; a future revisit should target the greedy heuristic's optimality
(untested) — may warrant escalation past sonnet.

---

## 2026-06-15 — Round 3: ASAN-driven (areas 9/10), cross-TU ODR fix

Triggered by a red gcc-AddressSanitizer CI job (run 27196858592, "AddressSanitizer
tests"): **stack-buffer-overflow** constructing `PooledSplits` in the Transfer
path. Diagnosed by the orchestrator (opus); fix verified locally; advisor-reviewed
before editing.

- **TC-009 (HIGH, REAL — ASAN-confirmed)** — **cross-TU ODR violation.**
  `PooledSplits`, `SplitHash`, `SplitEqual` were each defined at *namespace scope*
  with **different layouts** in both `src/transfer_consensus.cpp` (120-byte
  `PooledSplits`) and `src/Quartet.cpp` (136-byte, extra `tips_on_side1`). Their
  implicit special members are COMDAT/weak symbols mangled by type name only, so
  the linker folded them: Quartet's 136-byte ctor was used to construct transfer's
  120-byte `pool`, writing `tree_members` one slot (offset 120) past the end —
  the exact ASAN report. UB / memory corruption; a **CRAN ASAN/UBSAN blocker.**
- **Fix:** wrapped each file's TU-private types + helpers in an anonymous
  namespace (`namespace { … }`), the pattern already used by
  `cons_adams/greedy/loose/majorityplus/frequency.cpp`. Exported `[[Rcpp::export]]`
  functions left outside. Minimal, no logic change.
- **TC-008 re-diagnosed → FIXED.** The earlier `load_all` "segfault on the happy
  path" was NOT a TreeTools header skew / dev-infra artifact. It was the same ODR
  memory corruption (different link order under `load_all`). The Round-1/2 caveat
  ("installed package is fine, test via installed build") was luck, not safety.
- **Cross-TU sweep (area-10 work):** grepped every `src/*.cpp` for namespace-scope
  type names defined in >1 TU. Only other hit was `Tree` (bhv.cpp `namespace bhv`
  vs cons_frequency.cpp anonymous namespace) — **safe, distinct mangled names.**
  No other collisions among struct/class names.
- **Verification (Windows, R-devel):** `pkgload::load_all` — which *previously
  segfaulted* on this path — now runs `Transfer()` (NSplits 3) and `Quartet()`
  (NSplits 3) cleanly; transfer + Quartet test suites pass (1 on-CRAN skip).
  Drivers: `reviews/transfer/test-odr-fix.R`, `run-tests.R`. **Real ASAN-green
  needs a push** (CI) — pending user authorisation.

A `red-team-finder` @ **sonnet** was launched for the rest of area 10 (FACT
primitive `src/fact_tree.cpp/.h` + cross-cutting bounds + OpenMP races; confirm/
refute carried C-001..C-007). Its findings are recorded in Round 4 below.

---

## 2026-06-15 — Round 4: Area 10 (FACT primitive & C++ memory safety), sonnet

Finder: `red-team-finder` @ **sonnet** (launched during Round 3's ASAN fix).
Reviewed `fact_tree.{cpp,h}` + all 12 consensus kernels for OOB / overflow /
aliasing, OpenMP races, and a fresh cross-TU ODR sweep. Orchestrator (opus)
verified the HIGH finding by reading the code directly.

- **Finder yield: 1 new confirmed HIGH (fixed), 1 new LOW (open); 4 carried
  confirmed; 2 carried refuted.**
- **MEM-001 (HIGH, REAL) — FIXED this round.** `src/Quartet.cpp` computed the
  `compat` matrix size and indices with plain `int*int` (`M*M` alloc L383;
  `i*M+j`/`j*M+c` at L401/402/601/671) → overflow UB for M>46340 unique splits.
  The Quartet twin of TC-004, missed when transfer was fixed. Fixed with
  `static_cast<std::size_t>` at all 5 sites; compiles clean, Quartet+transfer
  suites pass. **CRAN ASAN/UBSan blocker.**
- **MEM-002 (LOW, open)** — `reinterpret_cast<bool*>(char*)` strict-aliasing UB
  in `cons_frequency.cpp:1342-1348`; never misfires (`sizeof(bool)==1`), portable
  fix is `uint8_t`.
- **C-001 REFUTED** — `anc[-1]` in `contract()` unreachable (root always hashed
  into `H[1]`; `good[1]` never cleared). Latent: dependency unasserted.
- **C-004 REFUTED** — Quartet `pool.data` pointer stability OK (`total_splits`
  is `size_t`; `reserve` before pointer-keyed inserts).
- **C-002 / C-003 / C-006 / C-007 CONFIRMED but latent/degenerate-only** — all
  require malformed/disconnected input unreachable through the R API
  (`ape::Preorder` → connected, well-formed). C-006 (dropped `assert(num==N)`)
  is the one worth a cheap `Rcpp::stop` guard for defence-in-depth.
- **ODR sweep CLEAN** beyond TC-009 (`quartet_index`/`quartet_state_from_sides`
  are `inline` in Quartet only; `Tree` distinct via `bhv::`/anon ns; all else
  `static`/anon/named). **OpenMP regions race-free** (per-element ownership; no
  R/Rcpp in parallel).
- **Seam status: still yielding** (MEM-001 was a live HIGH) → area 10 stays
  **sonnet** on revisit. Next angle: a deterministic large-M regression test for
  MEM-001, and the C-002/3/6/7 defensive guards if hardening for CRAN UBSan.

Full record: `reviews/area10-memory-safety/review.md`. Filed: MEM-001 (fixed),
MEM-002; updated C-001/C-004 → refuted, C-002/3/6/7 → confirmed.

last_tier(area 10): sonnet
yield(area 10): 1 confirmed HIGH (fixed) + 1 LOW; 4 carried confirmed, 2 refuted

---

## Prior reviews (pre-tier-system, folded from `reviews/`)

These predate the `sonnet/opus/fable` tier system; treat `tier: legacy`. Each has
a full `review.md` under `reviews/<dir>/`. Listed newest-first by review date.

| date | area (dir) | verdict | surviving open items |
|------|-----------|---------|----------------------|
| 2026-06-03 | bhv-numerical-precision | ship w/ nits | B-001 (TOL scale), B-002 (converged flag) |
| 2026-06-03 | package-infrastructure | fixed since | none (F1/F2 fixed 86711f8) |
| 2026-06-03 | cons-loose | ship | none (re-verified pass) |
| 2026-06-03 | average-consensus | conditional → fixed | A-001, A-002 (F1/F2 fixed b9e2cf4) |
| 2026-06-03 | fact-tree-primitive | ship w/ nits | C-006, C-007 |
| 2026-06-03 | cpp-memory-safety | conditional | C-001..C-005 (F1 fixed b396a6f) |
| 2026-06-03 | cons-majorityplus | ship | C-003 (goodLabel doc) |
| 2026-06-03 | test-suite-quality | conditional | T-001 cluster (T1/T5/T8 fixed 35869cb) |
| 2026-06-03 | rstar-local-consensus (slot 6) | conditional → fixed | none (1d77249, 9d62a3d) |
| 2026-06-03 | feature-adams-fast | ship | none (F1–F5 fixed 7fc9128/2cfe45f) |
| 2026-06-03 | feature-rstar-fast | conditional → fixed | none (9d62a3d) |
| 2026-06-02/03 | feature-frequency-fast (slot 3) | conditional → fixed | none (1d77249) |
| 2026-06-02 | feature-quartet (slot 2) | conditional → fixed | T13 (oracle-doc, in T-001) |
| 2026-06-02 | feature-loose-fast (slot 1) | ship (nits) | none (F1–F6 hardened 8ffa321) |
| 2026-06-02/03 | claude-cranky-kare (slot 4) | BLOCK (off-main branch) | none on main (repros pass) |
| 2026-06-02/03 | majorityplus-fact (slot 5) | repro only | none (boundary FACT-exact) |

---

last_focus: 10
last_focus_set: 2026-06-15 (area 10 FACT/memory-safety reviewed at sonnet, yielded MEM-001 HIGH — next bare /red-team picks area 11 R wrappers & package infrastructure; area 10 seam still yielding, stays sonnet on revisit)
