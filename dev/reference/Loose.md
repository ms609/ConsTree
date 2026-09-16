# Loose consensus tree

`Loose()` computes the loose consensus, also known as the semi-strict or
combinable-component consensus (Bremer 1990) . It contains every split
that is not contradicted by any input tree.

## Usage

``` r
Loose(trees)
```

## Arguments

- trees:

  A list of trees, or a `multiPhylo` object. All entries must share the
  same leaf labels.

## Value

`Loose()` returns the consensus tree, an object of class `phylo`, rooted
as in the first entry of `trees`.

## Details

The loose consensus refines the strict consensus
([`Strict()`](https://constree.github.io/dev/reference/Strict.md)). In
contrast to the majority-rule consensus
([`Majority()`](https://constree.github.io/dev/reference/Majority.md)),
a split present in most trees may still be contradicted by a minority,
and so be excluded from the loose consensus; yet a split that occurs in
just one tree will be is retained if no other tree contradicts it.

This implementation builds on the `looseConsensusFast` algorithm of
(Jansson et al. 2016) ; please cite that paper when using this method.

## References

Bremer K (1990). “Combinable component consensus.” *Cladistics*,
**6**(4), 369–372.
[doi:10.1111/j.1096-0031.1990.tb00551.x](https://doi.org/10.1111/j.1096-0031.1990.tb00551.x)
.  
  
Jansson J, Sung W, Vu H, Yiu S (2016). “Faster algorithms for computing
the R\* consensus tree.” *Algorithmica*, **76**(4), 1224–1244.
[doi:10.1007/s00453-016-0122-2](https://doi.org/10.1007/s00453-016-0122-2)
.

## See also

Closely related:
[`Strict()`](https://constree.github.io/dev/reference/Strict.md),
[`Majority()`](https://constree.github.io/dev/reference/Majority.md),
[`Greedy()`](https://constree.github.io/dev/reference/Greedy.md).

Other consensus methods:
[`Adams()`](https://constree.github.io/dev/reference/Adams.md),
[`Average()`](https://constree.github.io/dev/reference/Average.md),
[`Frequency()`](https://constree.github.io/dev/reference/Frequency.md),
[`Greedy()`](https://constree.github.io/dev/reference/Greedy.md),
[`Local()`](https://constree.github.io/dev/reference/Local.md),
[`Majority()`](https://constree.github.io/dev/reference/Majority.md),
[`MajorityPlus()`](https://constree.github.io/dev/reference/MajorityPlus.md),
[`Quartet()`](https://constree.github.io/dev/reference/Quartet.md),
[`RStar()`](https://constree.github.io/dev/reference/RStar.md),
[`Strict()`](https://constree.github.io/dev/reference/Strict.md),
[`Transfer()`](https://constree.github.io/dev/reference/Transfer.md)

## Examples

``` r
trees <- ape::as.phylo(0:5, 8)
Loose(trees)
#> 
#> Phylogenetic tree with 8 tips and 5 internal nodes.
#> 
#> Tip labels:
#>   t1, t2, t8, t7, t6, t5, ...
#> 
#> Rooted; no branch length.
```
