# Consensus tree that minimizes quartet distance

`Quartet()` constructs a consensus tree that minimizes the sum of
symmetric quartet distances to a set of input trees, using a greedy
add-and-prune heuristic.

## Usage

``` r
Quartet(
  trees,
  init = c("majority", "star", "extended"),
  greedy = c("best", "first")
)
```

## Arguments

- trees:

  Object of class `multiPhylo` specifying the input trees. All trees
  must share the same tip labels.

- init:

  Character string specifying the initial tree:

  - `"majority"`: the majority-rule consensus.

  - `"star"`: a fully unresolved star tree.

  - `"extended"`: the extended (greedy) majority-rule consensus.

- greedy:

  Character string specifying the greedy strategy:

  - `"best"`: evaluate all candidates and pick the best action at each
    step.

  - `"first"`: pick the first improving action encountered (faster; may
    give a slightly worse result).

## Value

`Quartet()` returns a consensus tree, an object of class `phylo`,
unrooted.

## Details

Where the majority-rule consensus minimizes the sum of Robinson-Foulds
distances to the input trees, `Quartet()` finds an approximate median
tree under the symmetric quartet distance (Takazawa et al. 2026) , which
counts both false-positive and false-negative resolved quartets equally.

Because the quartet distance gives greater weight to deep branches
(which resolve more quartets), quartet consensus trees tend to be more
resolved than majority-rule trees, especially when phylogenetic signal
is low.

The function supports trees with up to 100 tips.

## References

Takazawa Y, Takeda A, Hayamizu M, Gascuel O (2026). “Outperforming the
majority-rule consensus tree using fine-grained dissimilarity measures.”
*bioRxiv*.
[doi:10.64898/2026.03.16.712085](https://doi.org/10.64898/2026.03.16.712085)
.

## See also

Closely related:
[`Strict()`](https://constree.github.io/reference/Strict.md),
[`Majority()`](https://constree.github.io/reference/Majority.md),
[`Greedy()`](https://constree.github.io/reference/Greedy.md).

Other consensus methods:
[`Adams()`](https://constree.github.io/reference/Adams.md),
[`Average()`](https://constree.github.io/reference/Average.md),
[`Frequency()`](https://constree.github.io/reference/Frequency.md),
[`Greedy()`](https://constree.github.io/reference/Greedy.md),
[`Local()`](https://constree.github.io/reference/Local.md),
[`Loose()`](https://constree.github.io/reference/Loose.md),
[`Majority()`](https://constree.github.io/reference/Majority.md),
[`MajorityPlus()`](https://constree.github.io/reference/MajorityPlus.md),
[`RStar()`](https://constree.github.io/reference/RStar.md),
[`Strict()`](https://constree.github.io/reference/Strict.md),
[`Transfer()`](https://constree.github.io/reference/Transfer.md)

## Examples

``` r
library("TreeTools", quietly = TRUE)

# Generate bootstrap-like trees
trees <- as.phylo(1:30, nTip = 8)

# Quartet consensus
qc <- Quartet(trees)
plot(qc)


# Compare resolution with majority-rule
mr <- UnrootTree(Consensus(trees, p = 0.5))
cat("Majority-rule splits:", NSplits(mr), "\n")
#> Majority-rule splits: 2 
cat("Quartet consensus splits:", NSplits(qc), "\n")
#> Quartet consensus splits: 3 
```
