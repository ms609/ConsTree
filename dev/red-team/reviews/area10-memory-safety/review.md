# Red-Team Review — Area 10: FACT primitive & C++ memory safety

- **Date:** 2026-06-15 (Round 4, tier system)
- **Tier:** sonnet (finder) + opus (orchestrator verified the HIGH finding by direct read)
- **Scope:** `src/fact_tree.{cpp,h}` and cross-cutting memory safety across all 12
  `src/*.cpp` consensus kernels: OOB reads/writes, signed-int overflow in
  allocation/index math, pointer/iterator invalidation, lifetime, `reinterpret_cast`
  aliasing, OpenMP races, and a fresh cross-TU ODR sweep.
- **Trigger:** launched during the Round-3 ASAN fix (the cross-TU ODR sweep is
  area-10 work; folded the whole area into one round).
- **Verdict:** ONE new live HIGH bug (MEM-001, **fixed**); one new LOW (MEM-002).
  Two carried items refuted (C-001, C-004), four confirmed-but-latent
  (C-002/3/6/7). ODR sweep clean; OpenMP clean. Seam still yielding → stays sonnet.

## Confirmed findings

| id | severity | verdict | summary |
|----|----------|---------|---------|
| **MEM-001** | **HIGH** | **REAL (executed) — FIXED** | `src/Quartet.cpp` `compat` matrix used plain `int*int`: alloc `compat(M*M)` (L383) and indices `i*M+j`/`j*M+c` (L401/402/601/671). Overflow UB for M>46340 unique splits. The Quartet **twin of TC-004**, missed when transfer was widened. Fixed: `static_cast<std::size_t>` on the left operand at all 5 sites. Compiles clean; Quartet+transfer suites pass. **CRAN ASAN/UBSan blocker.** |
| MEM-002 | LOW | REAL (static) | `reinterpret_cast<bool*>(char*)` strict-aliasing UB at `cons_frequency.cpp:1342-1348`. Works on every real platform (`sizeof(bool)==1`, 0=false); formally UB. Fix: `uint8_t` end-to-end or `filter(char*)`. |
| C-002 | MINOR | REAL but unreachable | `lcaDepth`/`lcaDepthWalk` deref `parent[-1]` on **disconnected** input (`rstar.cpp:~172`, `local_consensus.cpp:~48`). R API routes through `ape::Preorder` → connected; reachable only by direct C++ call or future refactor. |
| C-003 | MINOR | REAL but latent | `cons_majorityplus.cpp:~62` — leaf with `goodLabel<=0` ⇒ `ret.leaf[-1]` OOB write. Invariant (`updateCounter` increments every leaf) holds today, unasserted/fragile. |
| C-006 | MINOR | REAL but degenerate | `fact_tree.cpp:~79` dropped `assert(num==N)`; forests / duplicate-leaf edge tables leave `H[i]=-1` + garbage `minL/maxL/size`, used unchecked downstream → silently wrong output, no error. Cheap guard: `if (num != A.N) Rcpp::stop(...)`. |
| C-007 | LOW | REAL but dead | `fact_tree.cpp:~130` `nRow>0 ? edge(0,0)-1 : 0`; the zero-row `:0` arm is dead and gates a secondary `t.leaf[]` OOB (nTip≥6). Unreachable via wrappers; replace with early `stop()`. |

## Refuted

| id | verdict |
|----|---------|
| C-001 | **REFUTED** — `anc[-1]` in `contract()` (`cons_loose.cpp:~70`) is unreachable. `precompute()` always hashes the root cluster into `H[1]`; the op==0 `looseMerge` pass never clears `good[1]` (full-taxon cluster is universally compatible) ⇒ `keep[A.root]==1` always. Latent: dependency unasserted. |
| C-004 | **REFUTED** — Quartet `pool.data` pointer stability OK. `total_splits` is `size_t` and `pool.data.reserve(total_splits * n_bytes)` (L214/219) runs before any pointer-keyed `split_map` insert; backing store never reallocates. |

## Ruled out (no issue)
- **Cross-TU ODR** beyond TC-009: `quartet_index` / `quartet_state_from_sides` are
  `inline` in `Quartet.cpp` only; `Tree` is distinct (`bhv::Tree` vs anon-ns in
  `cons_frequency.cpp`); everything else is `static`, anonymous-, or named-namespace.
  No collisions.
- **OpenMP races** in `transfer_consensus.cpp` (`transfer_dist_mat`, `compute_td`,
  `compat_mat`): per-element write ownership by outer index; no R/Rcpp API in any
  parallel region.

## Fix applied this round
- **MEM-001** — `src/Quartet.cpp` L383/401/402/601/671 widened to `std::size_t`.
  Verified: `load_all` recompiles clean; `test_dir(filter="transfer|Quartet")`
  passes (1 on-CRAN skip). Driver: `reviews/transfer/run-tests.R`.

## Notes for next reviewer of area 10 (still yielding — stay sonnet)
- No test covers the large-M case for either Quartet or transfer; a deterministic
  high-split-count regression (e.g. many diverse-topology bootstrap trees) would
  pin MEM-001 / TC-004 and is the obvious next artifact.
- C-002/3/6/7 are defence-in-depth: cheap `Rcpp::stop`/`assert` guards would make
  the package robust under CRAN UBSan even on malformed direct-C++ input. Bundle
  if hardening for submission.
- `cpp_quartet_consensus` caps `n_tips<=100` but not `n_tree`; M grows with both
  tree count and topological diversity — understand that ceiling before claiming
  MEM-001 is purely theoretical.
