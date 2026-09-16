# Majority-rule consensus tree

`Majority()` is a simple alias of
[`TreeTools::Consensus()`](https://ms609.github.io/TreeTools/reference/Consensus.html),
which computes the majority-rule consensus (Margush and McMorris 1981) :
the tree that contains each split that occurs in more than `p` of the
input trees.

## Usage

``` r
Majority(trees, p = 0.5)

MajorityRule(trees, p = 0.5)

MR(trees, p = 0.5)
```

## Arguments

- trees:

  A list of trees, or a `multiPhylo` object. All entries must share the
  same leaf labels.

- p:

  Numeric between 0.5 and 1: the minimum proportion of trees that must
  contain a split for it to be retained.

## Value

`Majority()` returns an object of class `phylo` denoting the majority
rule consensus, rooted as in the first entry of `trees`.

## References

Margush T, McMorris FR (1981). “Consensus n-trees.” *Bulletin of
Mathematical Biology*, **43**(2), 239–244.
[doi:10.1007/BF02459446](https://doi.org/10.1007/BF02459446) .

## See also

Other consensus methods:
[`Adams()`](https://constree.github.io/dev/reference/Adams.md),
[`Average()`](https://constree.github.io/dev/reference/Average.md),
[`Frequency()`](https://constree.github.io/dev/reference/Frequency.md),
[`Greedy()`](https://constree.github.io/dev/reference/Greedy.md),
[`Local()`](https://constree.github.io/dev/reference/Local.md),
[`Loose()`](https://constree.github.io/dev/reference/Loose.md),
[`MajorityPlus()`](https://constree.github.io/dev/reference/MajorityPlus.md),
[`Quartet()`](https://constree.github.io/dev/reference/Quartet.md),
[`RStar()`](https://constree.github.io/dev/reference/RStar.md),
[`Strict()`](https://constree.github.io/dev/reference/Strict.md),
[`Transfer()`](https://constree.github.io/dev/reference/Transfer.md)

## Examples

``` r
trees <- ape::as.phylo(0:5, 8)
Majority(trees, p = 0.6)
#> 
#> Phylogenetic tree with 8 tips and 5 internal nodes.
#> 
#> Tip labels:
#>   t1, t2, t3, t4, t5, t6, ...
#> 
#> Rooted; no branch length.
```
