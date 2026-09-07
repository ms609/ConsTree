# Average consensus tree

`Average()` returns the average consensus (Lapointe and Cucumel 1997) :
the tree whose path-length (patristic) distances most closely match the
average of the path-length distances of the input trees.

## Usage

``` r
Average(
  trees,
  method = c("fastme.bal", "ls", "nj", "bionj", "fastme.ols"),
  weights = NULL,
  scale = c("none", "max"),
  edgeLengths = NA,
  outgroup = NULL,
  check.labels = TRUE,
  lsControl = list()
)
```

## Arguments

- trees:

  A list of trees, or a `multiPhylo` object. All entries must share the
  same leaf labels.

- method:

  Character specifying how to build the tree from the average distance
  matrix:

  - `"fastme.bal"` (the default) returns the balanced minimum-evolution
    tree (Desper and Gascuel 2002) : a fast, accurate approximation of
    the least-squares tree;

  - `"ls"` searches for the least-squares tree using
    [`TreeSearch::LeastSquaresTree()`](https://ms609.github.io/TreeSearch/reference/LeastSquaresTree.html),
    a non-negative least-squares NNI/SPR search;

  - `"nj"`, `"bionj"` and `"fastme.ols"` return the corresponding
    distance tree (Saitou and Nei 1987; Gascuel 1997) .

- weights:

  Numeric vector specifying the weight of each tree in the average (e.g.
  posterior probabilities), with one entry per tree. If `NULL`, each
  tree is weighted equally.

- scale:

  Character specifying whether to rescale each tree's distance matrix
  before averaging. `"none"` leaves matrices unscaled; `"max"` divides
  each matrix by its largest entry, recommended when absolute distances
  are not comparable (Lapointe and Cucumel 1997) .

- edgeLengths:

  Logical specifying whether to use edge lengths when computing
  path-length distances. `TRUE` requires edge lengths; `FALSE` does not
  use edge lengths; `NA` uses edge lengths when all trees have them, and
  otherwise counts edges.

- outgroup:

  Character vector specifying tip label(s) on which to root the result.
  `NULL` returns an unrooted tree.

- check.labels:

  Logical specifying whether to confirm that every tree contains the
  same leaves.

- lsControl:

  Named list of further arguments for the least-squares search
  (`method = "ls"`), passed to
  [`TreeSearch::LeastSquaresTree()`](https://ms609.github.io/TreeSearch/reference/LeastSquaresTree.html).

## Value

`Average()` returns an object of class `phylo` with fitted branch
lengths denoting the average consensus tree.

## Details

Because the average of several path-length matrices is usually not
itself realisable by any tree, `Average()` approximates it: by default,
with the fast balanced minimum-evolution tree; or if `method = "ls"`, by
NP-hard least-squares search (Day 1987) .

## References

Day WHE (1987). “Computational complexity of inferring phylogenies from
dissimilarity matrices.” *Bulletin of Mathematical Biology*, **49**(4),
461–467. [doi:10.1007/BF02458863](https://doi.org/10.1007/BF02458863)
.  
  
Desper R, Gascuel O (2002). “Fast and accurate phylogeny reconstruction
algorithms based on the minimum-evolution principle.” *Journal of
Computational Biology*, **9**(5), 687–705.
[doi:10.1089/106652702761034136](https://doi.org/10.1089/106652702761034136)
.  
  
Gascuel O (1997). “BIONJ: an improved version of the NJ algorithm based
on a simple model of sequence data.” *Molecular Biology and Evolution*,
**14**(7), 685–695.
[doi:10.1093/oxfordjournals.molbev.a025808](https://doi.org/10.1093/oxfordjournals.molbev.a025808)
.  
  
Lapointe F, Cucumel G (1997). “The average consensus procedure:
combination of weighted trees containing identical or overlapping sets
of taxa.” *Systematic Biology*, **46**(2), 306–312.
[doi:10.1093/sysbio/46.2.306](https://doi.org/10.1093/sysbio/46.2.306)
.  
  
Saitou N, Nei M (1987). “The neighbor-joining method: a new method for
reconstructing phylogenetic trees.” *Molecular Biology and Evolution*,
**4**(4), 406–425.
[doi:10.1093/oxfordjournals.molbev.a040454](https://doi.org/10.1093/oxfordjournals.molbev.a040454)
.

## See also

Split-based summaries:
[`Strict()`](https://constree.github.io/reference/Strict.md),
[`Majority()`](https://constree.github.io/reference/Majority.md).

Other consensus methods:
[`Adams()`](https://constree.github.io/reference/Adams.md),
[`Frequency()`](https://constree.github.io/reference/Frequency.md),
[`Greedy()`](https://constree.github.io/reference/Greedy.md),
[`Local()`](https://constree.github.io/reference/Local.md),
[`Loose()`](https://constree.github.io/reference/Loose.md),
[`Majority()`](https://constree.github.io/reference/Majority.md),
[`MajorityPlus()`](https://constree.github.io/reference/MajorityPlus.md),
[`Quartet()`](https://constree.github.io/reference/Quartet.md),
[`RStar()`](https://constree.github.io/reference/RStar.md),
[`Strict()`](https://constree.github.io/reference/Strict.md),
[`Transfer()`](https://constree.github.io/reference/Transfer.md)

## Examples

``` r
trees <- ape::rmtree(5, 8)    # five random eight-leaf trees
Average(trees)                # fast (balanced minimum evolution) default
#> 
#> Phylogenetic tree with 8 tips and 6 internal nodes.
#> 
#> Tip labels:
#>   t5, t6, t2, t4, t7, t3, ...
#> 
#> Unrooted; includes branch length(s).
# \donttest{
if (requireNamespace("TreeSearch", quietly = TRUE) &&
    exists("LeastSquaresTree", where = asNamespace("TreeSearch"),
           mode = "function")) {
  Average(trees, method = "ls")    # faithful least-squares fit (slower)
  
  # use Fitch-Margoliash weighting:
  Average(trees, method = "ls",
    lsControl = list(spr = FALSE, maxHits = 5L, weight = "fm")
  )
}
#> 
#> Phylogenetic tree with 8 tips and 6 internal nodes.
#> 
#> Tip labels:
#>   t5, t6, t4, t1, t8, t7, ...
#> 
#> Unrooted; includes branch length(s).
# }
```
