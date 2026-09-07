# ConsTree

<!-- badges: start -->
[![Project Status: WIP – Initial development is in progress, but there has not yet been a stable, usable release suitable for the public.](https://www.repostatus.org/badges/latest/active.svg)](https://www.repostatus.org/#active)
[![R-CMD-check](https://github.com/ms609/ConsTree/actions/workflows/R-CMD-check.yml/badge.svg)](https://github.com/ms609/ConsTree/actions/workflows/R-CMD-check.yml)
[![codecov](https://codecov.io/gh/ms609/ConsTree/branch/main/graph/badge.svg)](https://app.codecov.io/gh/ms609/ConsTree)
<!-- badges: end -->

'ConsTree' is an R package providing a comprehensive, efficient suite of
methods for summarizing a collection of phylogenetic trees — for example a
bootstrap or Bayesian posterior sample — as a single **consensus tree**.
tr
## Consensus methods

### Split-selection methods

These methods take a list of trees (or a `multiPhylo`) that share the same
leaves, and return a single `phylo` object.
Methods differ in which groupings (splits or clusters) the consensus tree
retains:

| Function | Objective |
|----------|--------------------------|
| `Strict()` | Retains groupings that occur in **every** tree |
| `Majority()` / `MajorityRule()` | Retains groupings that occur in **most** trees (tunable via `p`) |
| `Loose()` | Retains groupings that no tree **contradicts** (semi-strict / combinable-component) |
| `MajorityPlus()` | Retains groupings that more trees display than contradict |
| `Frequency()` | Retains groupings that are more frequent than every conflicting grouping (frequency-difference) |
| `Greedy()` | Adds groupings greedily, most frequent first, when compatible with those already kept (extended majority-rule) |
| `Adams()` | Constructed from the finest root-level partition shared by every tree (may introduce novel groupings; rooted) |
| `Local()` | Built from rooted triplets shared by every tree (minimum rooted/induced local consensus; ≤ 20 leaves) |
| `RStar()` | Includes each rooted triplet grouping that wins a **plurality** against each alternative separately |

### Distance and branch-length summaries

These methods summarize trees through a distance or tree-space criterion:

| Function | Objective |
|----------|---------|
| `Average()` | The tree best fitting the **mean path-length** (patristic) distances of the inputs |
| `Quartet()` | An approximate median minimizing the total **quartet distance** to the inputs; often more resolved than majority-rule |
| `Transfer()` | A greedy consensus minimizing total **transfer distance** to the inputs; often more resolved than majority-rule |
| `BHVMean()` | the Fréchet **mean tree** in Billera–Holmes–Vogtmann treespace, with branch lengths; `BHVDistance()`, `BHVPairwiseDistances()` and `BHVVariance()` provide the supporting geodesic distances and dispersion |

## Usage

```r
library("ConsTree")

trees <- ape::as.phylo(1:100, 8)   # 100 eight-leaf trees

Strict(trees)        # most conservative
Majority(trees)      # the familiar 50% majority-rule tree
Loose(trees)         # everything not actively contradicted
Frequency(trees)     # frequency-difference: often more resolved than majority
Greedy(trees)        # most resolved of the split-based summaries
Transfer(trees)      # minimizes transfer distance; often more resolved than majority-rule
```

## Installation

Install from CRAN (from Sept 2026) with:

```r
install.packages("ConsTree")
```

Install the development version from GitHub:

```r
if (!require("pak")) install.packages("pak")
pak::pkg_install("ms609/ConsTree")
```

## Relationship to other packages

'ConsTree' builds on [TreeTools](https://ms609.github.io/TreeTools/) (the fast
engine for strict and majority-rule consensus calculation) and ['TreeDist'](https://ms609.github.io/TreeDist/) (tree distances and
information-theoretic consensus).

['TreeDist'](https://ms609.github.io/TreeDist/)'s `median.multiPhylo` offers a
complementary summary: the tree within a sample that has the lowest median
clustering information distance to the others.

The quartet machinery underlying `Quartet()` builds on the
['Quartet'](https://ms609.github.io/Quartet/) package, which counts the
resolved- and shared-quartet statistics between trees; and the BHV summaries
relate to ['distory'](https://cran.r-project.org/package=distory), which
computes geodesic distances in the same treespace.

['Rogue'](https://ms609.github.io/Rogue/) identifies unstable ('rogue') leaves
whose removal can improve the resolution and support of a consensus tree;
dropping rogues before summarizing with 'ConsTree' often yields a
better-resolved result.

## Citation and attribution

The manual page for each function details the literature that underpins each
method; please cite this literature alongside this package
(type `citation("ConsTree")`).

Please note that this project is released with a
[Contributor Code of Conduct](https://ms609.github.io/TreeTools/CODE_OF_CONDUCT.html).
By contributing, you agree to abide by its terms.

