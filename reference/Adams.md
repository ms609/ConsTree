# Adams consensus tree

`Adams()` computes the Adams (1972) consensus tree.

## Usage

``` r
Adams(trees)
```

## Arguments

- trees:

  A list of trees, or a `multiPhylo` object. All entries must share the
  same leaf labels.

## Value

`Adams()` returns an object of class `phylo` denoting the Adams
consensus tree.

## Details

The Adams consensus places each species in the the smallest group to
which it belongs on all input trees. Consequently, it may contain
groupings that do not appear in any input tree#' This implementation
builds on the algorithm of (Jansson et al. 2017) ; please cite this
paper where you use this method.

## References

Adams EN (1972). “Consensus techniques and the comparison of taxonomic
trees.” *Systematic Zoology*, **21**(4), 390–397.
[doi:10.2307/2412432](https://doi.org/10.2307/2412432) .  
  
Jansson J, Li Z, Sung W (2017). “On finding the Adams consensus tree.”
*Information and Computation*, **256**, 334–347.
[doi:10.1016/j.ic.2017.08.002](https://doi.org/10.1016/j.ic.2017.08.002)
.

## See also

Other consensus methods:
[`Average()`](https://constree.github.io/reference/Average.md),
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
# Two rooted trees that disagree only on the position of one leaf
trees <- c(ape::read.tree(text = "(((a, b), c), d);"),
           ape::read.tree(text = "(((a, b), d), c);"))
# keeps the clade (a, b); leaves c, d unresolved at the root
ape::write.tree(Adams(trees))
#> [1] "(d,c,(b,a));"
```
