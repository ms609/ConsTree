# Changelog

## ConsTree 1.0.0.9000 (development)

- Fix omissions in references and attribution.

## ConsTree 1.0.0 (2026-09-07)

CRAN release: 2026-09-15

First public release.

Split-based consensus methods:

- [`Strict()`](https://constree.github.io/dev/reference/Strict.md),
  [`Majority()`](https://constree.github.io/dev/reference/Majority.md) /
  [`MajorityRule()`](https://constree.github.io/dev/reference/Majority.md):
  wrappers around
  [`TreeTools::Consensus()`](https://ms609.github.io/TreeTools/reference/Consensus.html).
- [`Loose()`](https://constree.github.io/dev/reference/Loose.md): loose
  (semi-strict / combinable-component) consensus.
- [`Greedy()`](https://constree.github.io/dev/reference/Greedy.md):
  greedy (extended majority-rule) consensus.
- [`MajorityPlus()`](https://constree.github.io/dev/reference/MajorityPlus.md):
  majority-rule (+) consensus.
- [`Frequency()`](https://constree.github.io/dev/reference/Frequency.md):
  frequency-difference consensus.

Rooted-tree consensus methods:

- [`Adams()`](https://constree.github.io/dev/reference/Adams.md): Adams
  consensus.
- [`RStar()`](https://constree.github.io/dev/reference/RStar.md): R\*
  consensus, assembled from the strong clusters of the majority
  resolved-triplet set (Jansson, Sung, Vu & Yiu 2016).
- [`Local()`](https://constree.github.io/dev/reference/Local.md): local
  consensus (MinRLC / MinILC).

Distance- and branch-length-based summaries:

- [`Average()`](https://constree.github.io/dev/reference/Average.md):
  distance-based average consensus.
- [`Quartet()`](https://constree.github.io/dev/reference/Quartet.md):
  tree minimizing the summed symmetric quartet distance to the inputs
  (Takazawa et al. 2026).
- Billera-Holmes-Vogtmann treespace summaries:
  [`BHVDistance()`](https://constree.github.io/dev/reference/BHVDistance.md)
  (Owen-Provan geodesic distance), `BHVPairwiseDistances()`,
  [`BHVMean()`](https://constree.github.io/dev/reference/BHVMean.md)
  (Fréchet mean) and
  [`BHVVariance()`](https://constree.github.io/dev/reference/BHVMean.md).

An introductory vignette (`ConsTree`) demonstrates each method family on
worked exemplars.
