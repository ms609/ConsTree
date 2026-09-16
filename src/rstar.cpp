#include <Rcpp.h>
#include <vector>
#include <string>
#include <numeric>    // std::iota
#include <algorithm>  // std::min, std::sort, std::fill, std::copy
#include <array>
#include <utility>    // std::pair
#include <cstdint>    // uint8_t, uint16_t, uint32_t, uint64_t
#include <atomic>
#include <unordered_map>
#include "fact_tree.h"
#ifdef _OPENMP
#include <omp.h>
#endif
#include <R_ext/Utils.h>  // R_CheckUserInterrupt, R_ToplevelExec

// =============================================================================
// R* consensus tree (Degnan et al. 2009), computed after Jansson, Sung, Vu & Yiu
// (2016, Algorithmica 76:1224-1244; "JSVY" below).
//
// Definition.  Given k rooted trees on the same leaf set L (n = |L|):
//   * #ab|c = number of input trees in which the resolved triplet ab|c is
//     consistent (fan triplets count for NOTHING -- they have no impact);
//   * the majority resolved-triplet set is
//        R_maj = { ab|c : #ab|c > max(#ac|b, #bc|a) }   (strict PLURALITY);
//   * the R* tree is the unique tree tau with r(tau) subset of R_maj maximising
//     internal nodes.  Lemma 1.1: its clusters are EXACTLY the STRONG CLUSTERS
//     of R_maj, where A subset of L is strong iff  aa'|x in R_maj  for every pair
//     a,a' in A and every x not in A.
//
// CONSTRUCTION follows JSVY's Algorithm R*_consensus_tree (their Fig. 2):
//   1. similarity s(a,b) = #{w : ab|w in R_maj};
//   2. candidate clusters = the single-linkage dendrogram of s (maximum spanning
//      tree, edges in decreasing weight).  This is a laminar superset of the
//      Apresjan clusters of s, which contain every strong cluster;
//   3. keep the candidates that are strong clusters of R_maj;
//   4. the kept clusters are the R* tree (Lemma 1.1).
// Steps 1 and 3 depend on k:
//
//   k = 2  O(n^2) time and memory, no R_maj.  With I[u][v] = |L(T1^u) n L(T2^v)|
//          (JSVY Lemma 2.2), u_i = lca_i(a,b) and p_i, q_i the children of u_i
//          towards a and b, the three cases of ab|w in R_maj (resolved in both
//          trees; resolved in one, fan in the other) sum by inclusion-exclusion
//          to one O(1) expression per pair (twoTreeSimilarity).  A candidate A
//          with u_i = lca_i(A) is strong iff, in both trees, A is a union of
//          child clusters of u_i, and I[u_1][u_2] = |A| (Jansson & Sung 2013,
//          Algorithmica 66:329-345, Lemma 13).  Each test is O(n), over at
//          most n - 1 candidates.
//
//   k >= 3 O(k n^3) time, O(k n^2) memory.  Identical input trees are merged
//          and weighted by multiplicity.  Per-tree matrices hold the preorder
//          rank of each leaf pair's LCA (Euler tour + sparse table); ranks
//          compare as depths do along a root path, so one vectorised pass per
//          tree over each leaf pair's row tallies s over the C(n,3) triples,
//          without materialising R_maj (tallyTriples).
//          Strong-cluster tests (on rows reordered so each candidate is a
//          contiguous column range), cheapest first:
//            - a clade of trees carrying more than half the weight is strong;
//            - a cross-block pair needs s(a,a') >= |L \ A|;
//            - a pair holds for every outsider that intrudes on the clade of
//              lca_t(a,a') in trees carrying less than half the weight;
//            - otherwise, count the smaller side of A (JSVY Lemma 4.4:
//              s(a,a') = |L \ A| + #{x in A : aa'|x in R_maj}).
//          O(k n^3) worst case.  Both tallies and large candidates' pair
//          tests run on nThreads OpenMP threads.

// COMPLEXITY.  JSVY give O(n^2) for k = 2, O(n^2 log^{4/3} n) for k = 3
// (offline 3-D orthogonal range counting) and O(n^2 log^{k+2} n) for unbounded
// k (nested balanced search trees).  The last is exponential in k and beats
// O(k n^3) only when k < log n / (log log n)^{1+eps} (JSVY p. 1229), so k >= 3
// uses the direct tally.  k = 3 is not specialised.
//
// Taxa are 0-indexed internally; tip ape-node v <-> taxon v-1 <-> fact node v-1.
// Output is integer-label Newick (1-indexed) with NO trailing ';'.
// =============================================================================

// Has the user interrupted?  Safe to call from the master thread of a parallel
// region: R's longjmp is contained by R_ToplevelExec rather than thrown.
static void checkInterruptFn(void*) { R_CheckUserInterrupt(); }
static bool userInterrupted() {
  return R_ToplevelExec(checkInterruptFn, NULL) == FALSE;
}

// Disjoint-set find with path halving.
static int ufFind(std::vector<int>& f, int x) {
  while (f[x] != x) { f[x] = f[f[x]]; x = f[x]; }
  return x;
}

// ---------------------------------------------------------------------------
// Triplet counts, k >= 3.  D holds one row of LCA ranks per (tree, leaf); a
// row's entries are in column-position order.  Two of a triple's three LCA
// ranks coincide and the third is at least as deep (larger), so tree t gives
//   ij|l iff d(i,j) > d(i,l);  il|j iff d(i,l) > d(i,j);  jl|i iff d(j,l) > d(i,j)
// and none of them for a fan.  For leaves i, j (j at column posJ) and each
// column l in [from, to), accumulate over distinct trees, weighted by
// multiplicity, how many trees give each triplet.  Count holds the total
// number of trees.
// ---------------------------------------------------------------------------
template <typename Count>
static inline void countTriplets(const uint16_t* D, int n,
                                 const std::vector<int>& weight,
                                 int i, int j, int posJ, int from, int to,
                                 Count* ij, Count* il, Count* jl) {
  std::fill(ij + from, ij + to, 0);
  std::fill(il + from, il + to, 0);
  std::fill(jl + from, jl + to, 0);
  for (size_t t = 0; t < weight.size(); ++t) {
    const Count w = (Count)weight[t];
    const uint16_t* ri = &D[(t * n + i) * n];
    const uint16_t* rj = &D[(t * n + j) * n];
    const uint16_t dij = ri[posJ];
    if (w == 1) {
      #pragma omp simd
      for (int l = from; l < to; ++l) {
        ij[l] += (Count)(dij > ri[l]);
        il[l] += (Count)(ri[l] > dij);
        jl[l] += (Count)(rj[l] > dij);
      }
    } else {
      #pragma omp simd
      for (int l = from; l < to; ++l) {
        ij[l] += w & (Count)-(Count)(dij > ri[l]);
        il[l] += w & (Count)-(Count)(ri[l] > dij);
        jl[l] += w & (Count)-(Count)(rj[l] > dij);
      }
    }
  }
}

// s(a,b) = #{x : ab|x in R_maj} over the C(n,3) triples.  Triple (i, j, l),
// i < j < l, adds to rows i and j of s; each thread accumulates into a private
// matrix (counts are at most n, so uint16_t) that is added to s at the end.
template <typename Count>
static void tallyTriples(const uint16_t* D, int n, const std::vector<int>& weight,
                         std::vector<int>& s, int nThreads) {
  std::atomic<bool> stop(false);
  #ifdef _OPENMP
  #pragma omp parallel num_threads(nThreads)
  #endif
  {
    std::vector<Count> cIJ(n), cIL(n), cJL(n);
    Count* ij = cIJ.data();
    Count* il = cIL.data();
    Count* jl = cJL.data();
    std::vector<uint16_t> part((size_t)n * n, 0);
    #ifdef _OPENMP
    #pragma omp for schedule(dynamic, 1)
    #endif
    for (int i = 0; i < n; ++i) {
      if (stop) continue;
      #ifdef _OPENMP
      if (omp_get_thread_num() == 0 && userInterrupted()) stop = true;
      #else
      if (userInterrupted()) stop = true;
      #endif
      uint16_t* si = &part[(size_t)i * n];
      for (int j = i + 1; j < n - 1; ++j) {
        countTriplets(D, n, weight, i, j, j, j + 1, n, ij, il, jl);
        uint16_t* sj = &part[(size_t)j * n];
        int winsIJ = 0;
        #pragma omp simd reduction(+:winsIJ)
        for (int l = j + 1; l < n; ++l) {
          const Count x = ij[l], y = il[l], z = jl[l];
          winsIJ += (x > y) & (x > z);
          si[l] += (uint16_t)((y > x) & (y > z));
          sj[l] += (uint16_t)((z > x) & (z > y));
        }
        si[j] += (uint16_t)winsIJ;
      }
    }
    #ifdef _OPENMP
    #pragma omp critical
    #endif
    {
      for (size_t c = 0; c < part.size(); ++c) s[c] += part[c];
    }
  }
  if (stop) throw Rcpp::internal::InterruptedException();  // # nocov
}

// #{x at columns [from, to) : ab|x in R_maj}.  ab|a and ab|b never win.
template <typename Count>
static int majorityWins(const uint16_t* D, int n, const std::vector<int>& weight,
                        int a, int b, int posB, int from, int to, Count* buf) {
  Count* ab = buf;
  Count* ax = buf + n;
  Count* bx = buf + 2 * n;
  countTriplets(D, n, weight, a, b, posB, from, to, ab, ax, bx);
  int wins = 0;
  #pragma omp simd reduction(+:wins)
  for (int x = from; x < to; ++x) wins += (ab[x] > ax[x]) & (ab[x] > bx[x]);
  return wins;
}

// ---------------------------------------------------------------------------
// k = 2 machinery.
// ---------------------------------------------------------------------------
// Per-node traversal data.  Leaves under node u are leafOrder[lo[u] .. hi[u]).
struct TreeWalk {
  std::vector<int> parent, depth, size, lo, hi, leafOrder, preorder;
  std::vector<int> rank;         // node -> index in preorder
  std::vector<int> rankSize;     // preorder index -> cluster size
};

static TreeWalk walkTree(const fact::Tree& tr, int n) {
  const int m = tr.cnt;
  TreeWalk w;
  w.parent.assign(m, -1); w.depth.assign(m, 0); w.size.assign(m, 0);
  w.lo.assign(m, 0); w.hi.assign(m, 0);
  w.leafOrder.reserve(n); w.preorder.reserve(m);
  std::vector<std::pair<int, int> > st;
  st.reserve(m);
  st.push_back(std::make_pair(tr.root, 0));
  while (!st.empty()) {
    const int node = st.back().first;
    const int ci = st.back().second;
    if (ci == 0) {
      w.preorder.push_back(node);
      w.lo[node] = (int)w.leafOrder.size();
      if (node < n) w.leafOrder.push_back(node);
    }
    if (ci < (int)tr.G[node].size()) {
      const int c = tr.G[node][ci];
      st.back().second = ci + 1;
      w.parent[c] = node;
      w.depth[c] = w.depth[node] + 1;
      st.push_back(std::make_pair(c, 0));
    } else {
      w.hi[node] = (int)w.leafOrder.size();
      w.size[node] = w.hi[node] - w.lo[node];
      st.pop_back();
    }
  }
  w.rank.assign(m, 0);
  w.rankSize.assign(m, 0);
  for (int r = 0; r < m; ++r) {
    w.rank[w.preorder[r]] = r;
    w.rankSize[r] = w.size[w.preorder[r]];
  }
  return w;
}

// ---------------------------------------------------------------------------
// Fill row t of D: D[(t*n + a)*n + b] = preorder rank of LCA(tip a, tip b), via
// an Euler tour + sparse-table RMQ (O(n log n) build, O(1) query, O(n^2) fill);
// D[(t*n + a)*n + a] = 0xFFFF.  Every ancestor of a leaf precedes its
// descendants in preorder, so along a root path a larger rank is a deeper node,
// and ranks compare exactly as depths do; the ranks also identify the LCA.  A
// leaf is deeper than any of its ancestors, whence the diagonal.  Rows are
// contiguous in b, so the tally's inner loops read sequentially and vectorise.
// Ranks are < 2n, which the caller keeps below 2^16.
// ---------------------------------------------------------------------------
static void fillLcaRanks(const fact::Tree& tr, const TreeWalk& walk, int n, int t,
                         uint16_t* D) {
  const int nNode = tr.cnt;
  std::vector<int> euler;     euler.reserve((size_t)2 * nNode);
  std::vector<int> firstPos(nNode, -1);
  {
    std::vector<std::pair<int, int> > st;
    st.reserve(nNode);
    st.push_back(std::make_pair(tr.root, 0));
    while (!st.empty()) {
      const int node = st.back().first;
      const int ci = st.back().second;
      if (ci == 0) { firstPos[node] = (int)euler.size(); euler.push_back(node); }
      if (ci < (int)tr.G[node].size()) {
        st.back().second = ci + 1;
        st.push_back(std::make_pair(tr.G[node][ci], 0));
      } else {
        st.pop_back();
        if (!st.empty()) euler.push_back(st.back().first);
      }
    }
  }

  const int m = (int)euler.size();
  std::vector<int> LOG(m + 1, 0);
  for (int i = 2; i <= m; ++i) LOG[i] = LOG[i / 2] + 1;
  const int K = LOG[m] + 1;
  // sp[j][i] = Euler index in [i, i + 2^j) whose node has minimum rank.
  const std::vector<int>& rank = walk.rank;
  std::vector<std::vector<int> > sp(K, std::vector<int>(m));
  for (int i = 0; i < m; ++i) sp[0][i] = i;
  for (int j = 1; j < K; ++j) {
    const int half = 1 << (j - 1);
    for (int i = 0; i + (1 << j) <= m; ++i) {
      int l = sp[j - 1][i], r = sp[j - 1][i + half];
      sp[j][i] = (rank[euler[l]] <= rank[euler[r]]) ? l : r;
    }
  }

  for (int a = 0; a < n; ++a) {
    const int fa = firstPos[a];
    const size_t rowA = ((size_t)t * n + a) * n;
    D[rowA + a] = 0xFFFF;
    for (int b = a + 1; b < n; ++b) {
      int l = fa, r = firstPos[b];
      if (l > r) std::swap(l, r);
      const int j = LOG[r - l + 1];
      const int i1 = sp[j][l], i2 = sp[j][r - (1 << j) + 1];
      const uint16_t lcaRank = (uint16_t)std::min(rank[euler[i1]], rank[euler[i2]]);
      D[rowA + b] = lcaRank;
      D[((size_t)t * n + b) * n + a] = lcaRank;
    }
  }
}

static int lcaWalk(const TreeWalk& w, int x, int y) {
  while (w.depth[x] > w.depth[y]) x = w.parent[x];
  while (w.depth[y] > w.depth[x]) y = w.parent[y];
  while (x != y) { x = w.parent[x]; y = w.parent[y]; }
  return x;
}

// I[(u - n) * (nInt2 + 1) + (v - n)] = |L(T1^u) n L(T2^v)| for internal u, v.
// Built bottom-up over T1: a leaf child adds 1 along its T2 ancestor path; an
// internal child adds its row.  O(n^2).  Each row ends with a column of 1s,
// the intersection with whichever of a or b is a leaf child of an LCA.
static std::vector<int> intersectionTable(const fact::Tree& t1, const TreeWalk& w1,
                                          const TreeWalk& w2, int n, int nInt2) {
  const int nInt1 = t1.cnt - n;
  const size_t width = (size_t)nInt2 + 1;
  std::vector<int> I((size_t)nInt1 * width, 0);
  for (int idx = (int)w1.preorder.size() - 1; idx >= 0; --idx) {
    const int u = w1.preorder[idx];
    if (u < n) continue;
    int* row = &I[(size_t)(u - n) * width];
    for (int c : t1.G[u]) {
      if (c < n) {
        for (int v = w2.parent[c]; v != -1; v = w2.parent[v]) ++row[v - n];
      } else {
        const int* childRow = &I[(size_t)(c - n) * width];
        for (int j = 0; j < nInt2; ++j) row[j] += childRow[j];
      }
    }
    row[nInt2] = 1;
  }
  return I;
}

// s(a,b) = #{w : ab|w in R_maj} for two trees, in O(n^2).  Writing |x| for a
// cluster size and I(x, y) for |L(T1^x) n L(T2^y)|,
//   s = n - |p1| - |q1| - |p2| - |q2| - I(u1,u2)
//         + I(u1,p2) + I(u1,q2) + I(p1,u2) + I(q1,u2).
// When p_i or q_i is a leaf it is a or b, which lies under the other tree's u,
// so the intersection is 1.
//
// For fixed a, T2's (u2, p2, q2) are recorded per b; T1 is walked from a to the
// root, so (u1, p1, q1) and their table rows are fixed across each block of b
// under one child of an ancestor.  Row a of s is filled for every b != a.
static std::vector<int> twoTreeSimilarity(const fact::Tree& t1, const TreeWalk& w1,
                                          const fact::Tree& t2, const TreeWalk& w2,
                                          const std::vector<int>& I, int n, int nInt2,
                                          int nThreads) {
  const size_t width = (size_t)nInt2 + 1;
  // Column of v in the table: internal v -> v - n; leaf -> the all-1s column.
  auto column = [&](int v) -> int { return v < n ? nInt2 : v - n; };
  const std::vector<int> ones(width, 1);           // "row" of a leaf of T1
  struct Far { int u, p, q, sizes; };              // T2 view of pair (a, b)
  std::vector<int> s((size_t)n * n);
  std::atomic<bool> stop(false);
  #ifdef _OPENMP
  #pragma omp parallel num_threads(nThreads)
  #endif
  {
  std::vector<Far> far(n);
  #ifdef _OPENMP
  #pragma omp for schedule(static)
  #endif
  for (int a = 0; a < n; ++a) {
    if (stop) continue;
    #ifdef _OPENMP
    if (omp_get_thread_num() == 0 && (a & 63) == 0 && userInterrupted()) stop = true;
    #else
    if ((a & 63) == 0 && userInterrupted()) stop = true;
    #endif
    int prev = a;
    for (int u = w2.parent[a]; u != -1; prev = u, u = w2.parent[u]) {
      for (int c : t2.G[u]) {
        if (c == prev) continue;
        const Far f = { u - n, column(prev), column(c), w2.size[prev] + w2.size[c] };
        for (int i = w2.lo[c]; i < w2.hi[c]; ++i) far[w2.leafOrder[i]] = f;
      }
    }
    int* row = &s[(size_t)a * n];
    row[a] = 0;
    prev = a;
    for (int u = w1.parent[a]; u != -1; prev = u, u = w1.parent[u]) {
      const int* rowU = &I[(size_t)(u - n) * width];
      const int* rowP = prev < n ? ones.data() : &I[(size_t)(prev - n) * width];
      for (int c : t1.G[u]) {
        if (c == prev) continue;
        const int* rowQ = c < n ? ones.data() : &I[(size_t)(c - n) * width];
        const int base = n - w1.size[prev] - w1.size[c];
        for (int i = w1.lo[c]; i < w1.hi[c]; ++i) {
          const int b = w1.leafOrder[i];
          const Far& f = far[b];
          row[b] = base - f.sizes - rowU[f.u] + rowU[f.p] + rowU[f.q]
                 + rowP[f.u] + rowQ[f.u];
        }
      }
    }
  }
  }
  if (stop) throw Rcpp::internal::InterruptedException();  // # nocov
  return s;
}

// Is the marked set A a union of child clusters of u?  O(|L(u)|).
static bool unionOfChildClusters(const fact::Tree& tr, const TreeWalk& w, int u,
                                 const std::vector<char>& inA) {
  for (int c : tr.G[u]) {
    int count = 0;
    for (int i = w.lo[c]; i < w.hi[c]; ++i) count += inA[w.leafOrder[i]];
    if (count != 0 && count != w.size[c]) return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// Main Rcpp entry point.  Returns the Newick string (no trailing ';').
// twoTreeFastPath = FALSE forces the general path when k = 2 (for testing).
// [[Rcpp::export]]
std::string rStarConsensus(Rcpp::List edgeList, int nTip,
                           bool twoTreeFastPath = true, int nThreads = 1) {
  if (nThreads < 1) nThreads = 1;
  const int n = nTip;
  const int k = edgeList.size();
  const bool twoTree = twoTreeFastPath && k == 2;

  // Trivial leaf sets (the R wrapper handles n < 3, but stay self-contained).
  // # nocov start
  if (n < 3) {
    std::string out = "(";
    for (int i = 0; i < n; ++i) { if (i) out += ","; out += std::to_string(i + 1); }
    out += ")";
    return out;
  }
  // # nocov end

  std::vector<int> s;

  // k = 2 state.
  fact::Tree t1, t2;
  TreeWalk w1, w2;
  std::vector<int> I;
  int nInt2 = 0;

  // General state: LCA-rank matrices of the distinct trees, tree-major, and
  // each distinct tree's multiplicity.
  std::vector<uint16_t> D;
  std::vector<int> weight;
  std::vector<TreeWalk> walks;
  #define RSTAR_DEP(t, a, b) (D[((size_t)(t) * n + (a)) * n + (b)])

  if (twoTree) {
    Rcpp::IntegerMatrix edge1 = edgeList[0], edge2 = edgeList[1];
    t1 = fact::buildTreeFromEdge(edge1, n);
    t2 = fact::buildTreeFromEdge(edge2, n);
    nInt2 = t2.cnt - n;
    const double bytes = (double)(t1.cnt - n) * (double)(nInt2 + 1) * (double)sizeof(int)
                       + (double)n * (double)n * (double)sizeof(int);
    // # nocov start
    if (bytes > 2.4e9) {
      Rcpp::stop("rStarConsensus: the cluster-intersection table would need "
                 "~%.1f GB (n = %d). Reduce the number of leaves.", bytes / 1e9, n);
    }
    // # nocov end
    w1 = walkTree(t1, n);
    w2 = walkTree(t2, n);
    I = intersectionTable(t1, w1, w2, n, nInt2);
    s = twoTreeSimilarity(t1, w1, t2, w2, I, n, nInt2, nThreads);
  } else {
    // Identical trees have identical preorder edge matrices: keep one of each,
    // weighted by its multiplicity.
    std::vector<int> distinct;                     // index into edgeList
    {
      std::unordered_map<uint64_t, std::vector<int> > byHash;
      for (int t = 0; t < k; ++t) {
        Rcpp::IntegerMatrix edge = edgeList[t];
        uint64_t h = 1469598103934665603ULL;       // FNV-1a
        for (R_xlen_t r = 0; r < edge.size(); ++r) {
          h = (h ^ (uint64_t)(uint32_t)edge[r]) * 1099511628211ULL;
        }
        int found = -1;
        for (int d : byHash[h]) {
          Rcpp::IntegerMatrix seen = edgeList[distinct[d]];
          if (seen.nrow() == edge.nrow() &&
              std::equal(edge.begin(), edge.end(), seen.begin())) {
            found = d;
            break;
          }
        }
        if (found < 0) {
          byHash[h].push_back((int)distinct.size());
          distinct.push_back(t);
          weight.push_back(1);
        } else {
          ++weight[found];
        }
      }
    }
    const int kDistinct = (int)distinct.size();

    // Memory guard on the LCA-rank matrices (O(k n^2) uint16s); ranks < 2n
    // must also fit in uint16_t.
    const double bytes = (double)kDistinct * (double)n * (double)n * (double)sizeof(uint16_t);
    // # nocov start
    if (n >= 32768) {
      Rcpp::stop("rStarConsensus: more than two trees supports at most 32767 "
                 "leaves (n = %d).", n);
    }
    if (bytes > 2.4e9) {
      Rcpp::stop("rStarConsensus: the per-tree LCA-rank matrices would need "
                 "~%.1f GB (k = %d distinct trees, n = %d). Reduce the number "
                 "of trees or leaves.", bytes / 1e9, kDistinct, n);
    }
    // # nocov end

    D.assign((size_t)kDistinct * n * n, 0);
    for (int t = 0; t < kDistinct; ++t) {
      Rcpp::checkUserInterrupt();
      Rcpp::IntegerMatrix edge = edgeList[distinct[t]];
      fact::Tree tr = fact::buildTreeFromEdge(edge, n);
      walks.push_back(walkTree(tr, n));
      fillLcaRanks(tr, walks.back(), n, t, D.data());
    }

    // Defensive: on small inputs, cross-check the O(1) LCA against a direct
    // ancestor walk for every tree.  Cheap at n <= 50.
    if (n <= 50) {
      for (int t = 0; t < kDistinct; ++t) {
        for (int a = 0; a < n; ++a)
          for (int b = a + 1; b < n; ++b)
            if (walks[t].rank[lcaWalk(walks[t], a, b)] != RSTAR_DEP(t, a, b)) {
              // # nocov start
              Rcpp::stop("rStarConsensus: internal LCA self-check failed "
                         "(tree = %d, a = %d, b = %d).", t, a, b);
              // # nocov end
            }
      }
    }

    s.assign((size_t)n * n, 0);
    if (k < 256)        tallyTriples<uint8_t>(D.data(), n, weight, s, nThreads);
    else if (k < 65536) tallyTriples<uint16_t>(D.data(), n, weight, s, nThreads);  // # nocov
    else                tallyTriples<uint32_t>(D.data(), n, weight, s, nThreads);  // # nocov
    for (int a = 0; a < n; ++a)
      for (int b = a + 1; b < n; ++b)
        s[(size_t)b * n + a] = s[(size_t)a * n + b];
  }

  // ---- Maximum spanning tree of s (Prim, O(n^2)) ------------------------------
  // Vertices outside the tree are kept compacted in (restV, restW, restTo), so
  // each step updates and selects in one pass over the remaining vertices.
  std::vector<std::array<int, 3> > mst;        // (weight, u, v)
  mst.reserve(n - 1);
  std::vector<int> restV(n - 1), restW(n - 1, -1), restTo(n - 1, 0);
  std::iota(restV.begin(), restV.end(), 1);
  int nRest = n - 1;
  for (int u = 0; nRest > 0; ) {
    const int* su = &s[(size_t)u * n];
    int best = 0;
    for (int r = 0; r < nRest; ++r) {
      const int sw = su[restV[r]];
      if (sw > restW[r]) { restW[r] = sw; restTo[r] = u; }
      if (restW[r] > restW[best]) best = r;
    }
    std::array<int, 3> e = { restW[best], restTo[best], restV[best] };
    mst.push_back(e);
    u = restV[best];
    --nRest;
    restV[best] = restV[nRest]; restW[best] = restW[nRest]; restTo[best] = restTo[nRest];
  }
  std::sort(mst.begin(), mst.end(),
            [](const std::array<int, 3>& a, const std::array<int, 3>& b) {
              return a[0] > b[0];   // decreasing weight -> single-linkage merges
            });

  // ---- Dendrogram: node n + e joins the two sets merged by MST edge e ---------
  std::vector<int> dChild((size_t)2 * (n - 1));
  {
    std::vector<int> uf(n), top(n);
    std::iota(uf.begin(), uf.end(), 0);
    std::iota(top.begin(), top.end(), 0);
    for (int e = 0; e < n - 1; ++e) {
      const int ru = ufFind(uf, mst[e][1]), rv = ufFind(uf, mst[e][2]);
      dChild[(size_t)2 * e] = top[ru];
      dChild[(size_t)2 * e + 1] = top[rv];
      uf[rv] = ru;
      top[ru] = n + e;
    }
  }
  // Leaf order of the dendrogram: each candidate is the contiguous block
  // order[lo[node] .. hi[node]).
  std::vector<int> order, pos(n), lo((size_t)2 * n - 1), hi((size_t)2 * n - 1);
  order.reserve(n);
  {
    std::vector<std::pair<int, bool> > stk(1, std::make_pair(2 * n - 2, false));
    while (!stk.empty()) {
      const int node = stk.back().first;
      const bool done = stk.back().second;
      stk.pop_back();
      if (node < n) {
        lo[node] = (int)order.size();
        pos[node] = lo[node];
        order.push_back(node);
        hi[node] = lo[node] + 1;
      } else if (done) {
        hi[node] = (int)order.size();
      } else {
        lo[node] = (int)order.size();
        stk.push_back(std::make_pair(node, true));
        stk.push_back(std::make_pair(dChild[(size_t)2 * (node - n) + 1], false));
        stk.push_back(std::make_pair(dChild[(size_t)2 * (node - n)], false));
      }
    }
  }

  // General path: reorder each rank row's columns into dendrogram order, so
  // every candidate's inside and outside are contiguous column ranges.
  std::vector<uint8_t> buf8;
  std::vector<uint16_t> buf16;
  std::vector<uint32_t> buf32;
  if (!twoTree) {
    std::vector<uint16_t> tmp(n);
    for (size_t t = 0; t < weight.size(); ++t) {
      for (int a = 0; a < n; ++a) {
        uint16_t* row = &D[(t * n + a) * n];
        for (int c = 0; c < n; ++c) tmp[c] = row[order[c]];
        std::copy(tmp.begin(), tmp.end(), row);
      }
    }
    const size_t bufSize = (size_t)3 * n * nThreads;  // three rows per thread
    if (k < 256) buf8.resize(bufSize);
    else if (k < 65536) buf16.resize(bufSize);  // # nocov
    else buf32.resize(bufSize);                 // # nocov
  }
  // #{x at columns [from, to) : ab|x in R_maj}, using thread tid's buffer.
  auto pairWins = [&](int a, int b, int from, int to, int tid) -> int {
    if (from >= to) return 0;
    const size_t at = (size_t)3 * n * tid;
    if (k < 256) {
      return majorityWins(D.data(), n, weight, a, b, pos[b], from, to, &buf8[at]);
    }
    // # nocov start
    if (k < 65536) {
      return majorityWins(D.data(), n, weight, a, b, pos[b], from, to, &buf16[at]);
    }
    return majorityWins(D.data(), n, weight, a, b, pos[b], from, to, &buf32[at]);
    // # nocov end
  };

  // ---- Candidates in merge order -> filter to strong -> build ---------------
  std::vector<int> dpar(n);   std::iota(dpar.begin(), dpar.end(), 0);   // dendrogram UF
  std::vector<int> apar(n);   std::iota(apar.begin(), apar.end(), 0);   // accepted-block UF
  std::vector<int> repNode(n);                                          // block root -> out node
  std::iota(repNode.begin(), repNode.end(), 0);                         // leaf i -> out leaf i
  std::vector<int> blockStamp(n, -1), blockIndex(n, 0);

  // k = 2: lca of each dendrogram set in each tree (leaf i starts at node i).
  std::vector<int> setLca1, setLca2;
  if (twoTree) {
    setLca1.resize(n); std::iota(setLca1.begin(), setLca1.end(), 0);
    setLca2.resize(n); std::iota(setLca2.begin(), setLca2.end(), 0);
  }

  // General path: lca of each dendrogram set in each distinct tree.
  const size_t kDistinct = weight.size();
  std::vector<int> setLca, lcaNow(kDistinct);
  if (!twoTree) {
    setLca.resize(kDistinct * n);
    for (size_t t = 0; t < kDistinct; ++t) {
      std::iota(setLca.begin() + t * n, setLca.begin() + (t + 1) * n, 0);
    }
  }
  // Members of the current candidate in each clade, indexed by preorder rank.
  const size_t rankWidth = (size_t)2 * n;
  std::vector<int> inClade(twoTree ? 0 : kDistinct * rankWidth);
  // Outsiders under a clade ("intruders"), listed for clades with few of them:
  // intruderAt[t * rankWidth + r] indexes intruderPool when intruderStamp
  // matches the current candidate.
  const int maxIntruders = 64;
  std::vector<int> intruderStamp(twoTree ? 0 : kDistinct * rankWidth, -1);
  std::vector<int> intruderAt(twoTree ? 0 : kDistinct * rankWidth);
  std::vector<int> intruderPool;
  std::vector<int> intrusion(twoTree ? 0 : (size_t)n * nThreads, 0);

  std::vector<std::vector<int> > outChildren((size_t)2 * n);            // out node -> children
  int nextId = n;                                                       // internal ids >= n
  std::vector<char> inA(n, 0);

  for (int e = 0; e < n - 1; ++e) {
    Rcpp::checkUserInterrupt();
    const int ru = ufFind(dpar, mst[e][1]);
    const int rv = ufFind(dpar, mst[e][2]);
    const int node = n + e;
    const int from = lo[node], to = hi[node];
    const int sz = to - from;                        // candidate A = order[from, to)
    int lcaA1 = -1, lcaA2 = -1;
    if (twoTree) {
      lcaA1 = lcaWalk(w1, setLca1[ru], setLca1[rv]);
      lcaA2 = lcaWalk(w2, setLca2[ru], setLca2[rv]);
    } else {
      for (size_t t = 0; t < kDistinct; ++t) {
        lcaNow[t] = lcaWalk(walks[t], setLca[t * n + ru], setLca[t * n + rv]);
      }
    }

    if (sz <= n - 1) {
      for (int c = from; c < to; ++c) inA[order[c]] = 1;

      // partition A into current accepted blocks (distinct accUF roots)
      std::vector<int> blockRoot;
      std::vector<std::vector<int> > blockMem;
      for (int c = from; c < to; ++c) {
        const int a = order[c];
        const int r = ufFind(apar, a);
        if (blockStamp[r] != e) {
          blockStamp[r] = e;
          blockIndex[r] = (int)blockRoot.size();
          blockRoot.push_back(r);
          if (!twoTree) blockMem.push_back(std::vector<int>());
        }
        if (!twoTree) blockMem[blockIndex[r]].push_back(a);
      }
      const int p = (int)blockRoot.size();

      bool strong = (p >= 2);
      if (twoTree) {
        strong = strong
          && I[(size_t)(lcaA1 - n) * ((size_t)nInt2 + 1) + (lcaA2 - n)] == sz
          && unionOfChildClusters(t1, w1, lcaA1, inA)
          && unionOfChildClusters(t2, w2, lcaA2, inA);
      } else {
        // strong iff every cross-block pair (a,a') has aa'|x in R_maj for all x
        // not in A (within-block pairs are certified by the accepted block).
        const int need = n - sz;                     // = |L \ A|

        // A clade of trees carrying more than half the weight is strong: every
        // aa'|x holds in those trees.
        int cladeWeight = 0;
        for (size_t t = 0; t < kDistinct; ++t) {
          if (walks[t].size[lcaNow[t]] == sz) cladeWeight += weight[t];
        }

        if (2 * cladeWeight <= k) {
          for (size_t t = 0; t < kDistinct; ++t) {
            const TreeWalk& wt = walks[t];
            int* c = &inClade[t * rankWidth];
            const int nRank = (int)wt.preorder.size();
            std::fill(c, c + nRank, 0);
            for (int r = nRank - 1; r > 0; --r) {
              const int node = wt.preorder[r];
              if (node < n) c[r] = inA[node];
              c[wt.rank[wt.parent[node]]] += c[r];
            }
          }
          // List the intruders of clade r of tree t, if there are few.
          auto listIntruders = [&](size_t t, int r) {
            const size_t cell = t * rankWidth + r;
            intruderStamp[cell] = e;
            intruderAt[cell] = (int)intruderPool.size();
            const TreeWalk& wt = walks[t];
            const int node = wt.preorder[r];
            for (int i = wt.lo[node]; i < wt.hi[node]; ++i) {
              const int x = wt.leafOrder[i];
              if (!inA[x]) intruderPool.push_back(x);
            }
          };

          // Is cross pair (a, ap) consistent with A being strong?
          auto pairHolds = [&](int a, int ap, int tid) -> bool {
            const int sab = s[(size_t)a * n + ap];
            if (sab < need) return false;              // necessary condition

            // Outsider x fails to intrude on the clade of lca_t(a,a') in tree t
            // only if aa'|x holds there.  So if, for every outsider, the trees
            // in which it intrudes carry less than half the weight, aa'|x wins
            // for all of them.  Trees whose clade has many (or unlisted)
            // intruders count against every outsider.
            const size_t cell = (size_t)a * n + pos[ap];
            int crowded = 0;
            bool anyListed = false;
            for (size_t t = 0; t < kDistinct; ++t) {
              const int r = D[t * (size_t)n * n + cell];
              const int nIntruders = walks[t].rankSize[r] - inClade[t * rankWidth + r];
              if (nIntruders == 0) continue;
              if (nIntruders <= maxIntruders) {
                if (intruderStamp[t * rankWidth + r] != e && tid < 0) listIntruders(t, r);
                if (intruderStamp[t * rankWidth + r] == e) { anyListed = true; continue; }
              }
              crowded += weight[t];
            }
            if (2 * crowded < k) {
              int worst = 0;
              if (anyListed) {
                int* count = &intrusion[(size_t)n * (tid < 0 ? 0 : tid)];
                for (size_t t = 0; t < kDistinct; ++t) {
                  const int r = D[t * (size_t)n * n + cell];
                  const size_t at = t * rankWidth + r;
                  const int nIntruders = walks[t].rankSize[r] - inClade[at];
                  if (nIntruders == 0 || intruderStamp[at] != e) continue;
                  const int* list = &intruderPool[intruderAt[at]];
                  for (int i = 0; i < nIntruders; ++i) {
                    const int c = count[list[i]] += weight[t];
                    if (c > worst) worst = c;
                  }
                }
                for (size_t t = 0; t < kDistinct; ++t) {
                  const int r = D[t * (size_t)n * n + cell];
                  const size_t at = t * rankWidth + r;
                  const int nIntruders = walks[t].rankSize[r] - inClade[at];
                  if (nIntruders == 0 || intruderStamp[at] != e) continue;
                  const int* list = &intruderPool[intruderAt[at]];
                  for (int i = 0; i < nIntruders; ++i) count[list[i]] = 0;
                }
              }
              if (2 * (crowded + worst) < k) return true;
            }
            if (tid < 0) tid = 0;

            // Otherwise count the smaller side of A in vectorised column
            // ranges: the outside directly, or the inside, deriving the
            // outside via Lemma 4.4 (s(a,a') = |L \ A| + inside wins).
            if (need <= sz) {
              return pairWins(a, ap, 0, from, tid) + pairWins(a, ap, to, n, tid) == need;
            }
            return sab - pairWins(a, ap, from, to, tid) == need;
          };

          // Members of all but the last block; each pairs with later blocks.
          std::vector<std::pair<int, int> > left;
          double crossPairs = 0;
          for (int bi = 0; bi + 1 < p; ++bi) {
            for (int a : blockMem[bi]) left.push_back(std::make_pair(a, bi));
          }
          {
            double later = 0;
            for (int bi = p - 1; bi >= 0; --bi) {
              crossPairs += (double)blockMem[bi].size() * later;
              later += (double)blockMem[bi].size();
            }
          }
          const bool parallel = nThreads > 1 &&
            crossPairs * (double)kDistinct * std::min(need, sz) > 1e6;
          intruderPool.clear();
          bool prefilterPasses = true;
          if (parallel) {
            for (int bi = 0; bi < p && prefilterPasses; ++bi)
              for (int bj = bi + 1; bj < p && prefilterPasses; ++bj)
                for (int a : blockMem[bi]) {
                  const int* sa = &s[(size_t)a * n];
                  for (int ap : blockMem[bj]) {
                    if (sa[ap] < need) { prefilterPasses = false; break; }
                  }
                  if (!prefilterPasses) break;
                }
          }
          if (!prefilterPasses) {
            strong = false;
          } else if (parallel) {
            for (int bi = 0; bi < p; ++bi)
              for (int bj = bi + 1; bj < p; ++bj)
                for (int a : blockMem[bi])
                  for (int ap : blockMem[bj]) {
                    const size_t cell = (size_t)a * n + pos[ap];
                    for (size_t t = 0; t < kDistinct; ++t) {
                      const int r = D[t * (size_t)n * n + cell];
                      const int nIntruders = walks[t].rankSize[r] - inClade[t * rankWidth + r];
                      if (nIntruders > 0 && nIntruders <= maxIntruders &&
                          intruderStamp[t * rankWidth + r] != e) {
                        listIntruders(t, r);
                      }
                    }
                  }
          }
          // Do all pairs of left[idx] with later blocks hold?
          auto leftHolds = [&](size_t idx, int tid) -> bool {
            const int a = left[idx].first;
            for (int bj = left[idx].second + 1; bj < p; ++bj) {
              for (int ap : blockMem[bj]) {
                if (!pairHolds(a, ap, tid)) return false;
              }
            }
            return true;
          };
          if (!strong) {
            // rejected by the necessary condition
          } else if (parallel) {
            std::atomic<bool> failed(false);
            #ifdef _OPENMP
            #pragma omp parallel for schedule(dynamic, 4) num_threads(nThreads)
            #endif
            for (size_t idx = 0; idx < left.size(); ++idx) {
              if (failed) continue;
              #ifdef _OPENMP
              const int tid = omp_get_thread_num();
              #else
              const int tid = 0;
              #endif
              if (!leftHolds(idx, tid)) failed = true;
            }
            strong = !failed;
          } else {
            for (int bi = 0; bi < p && strong; ++bi)
              for (int bj = bi + 1; bj < p && strong; ++bj)
                for (size_t ia = 0; ia < blockMem[bi].size() && strong; ++ia)
                  for (size_t ja = 0; ja < blockMem[bj].size() && strong; ++ja)
                    strong = pairHolds(blockMem[bi][ia], blockMem[bj][ja], -1);
          }
        }
      }

      if (strong) {                                  // accept: create an R* node
        int id = nextId++;
        for (int bi = 0; bi < p; ++bi) outChildren[id].push_back(repNode[blockRoot[bi]]);
        int base = blockRoot[0];
        for (int bi = 1; bi < p; ++bi) apar[ufFind(apar, blockRoot[bi])] = ufFind(apar, base);
        repNode[ufFind(apar, base)] = id;
      }

      for (int c = from; c < to; ++c) inA[order[c]] = 0;
    }

    dpar[rv] = ru;
    if (twoTree) {
      setLca1[ru] = lcaA1; setLca2[ru] = lcaA2;
    } else {
      for (size_t t = 0; t < kDistinct; ++t) setLca[t * n + ru] = lcaNow[t];
    }
  }

  // ---- root: the full leaf set is the root; join all top-level blocks --------
  const int rootId = nextId++;
  {
    std::vector<char> seen(n, 0);
    for (int leaf = 0; leaf < n; ++leaf) {
      int r = ufFind(apar, leaf);
      if (!seen[r]) { seen[r] = 1; outChildren[rootId].push_back(repNode[r]); }
    }
  }

  // ---- emit Newick (iterative post-order) ------------------------------------
  std::vector<std::string> memo(nextId);
  std::vector<int> visit; visit.reserve(nextId);
  {
    std::vector<int> stk(1, rootId);
    while (!stk.empty()) {
      int u = stk.back(); stk.pop_back();
      visit.push_back(u);
      for (int c : outChildren[u]) stk.push_back(c);
    }
  }
  for (int idx = (int)visit.size() - 1; idx >= 0; --idx) {
    int u = visit[idx];
    if (outChildren[u].empty()) {                    // leaf: 1-indexed integer label
      memo[u] = std::to_string(u + 1);
    } else {
      std::string out = "(";
      for (size_t c = 0; c < outChildren[u].size(); ++c) {
        if (c) out += ",";
        out += memo[outChildren[u][c]];
      }
      out += ")";
      memo[u] = out;
    }
  }

  #undef RSTAR_DEP
  return memo[rootId];
}
