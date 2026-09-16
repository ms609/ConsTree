# Majority-rule (+) consensus tree

`MajorityPlus()` computes the majority-rule (+) consensus (Dong et al.
2010) , which displays each clade that occurs in more input trees than
contradict it.

## Usage

``` r
MajorityPlus(trees)
```

## Arguments

- trees:

  A list of trees, or a `multiPhylo` object. All entries must share the
  same leaf labels.

## Value

`MajorityPlus()` returns an object of class `phylo` denoting the
majority-plus consensus tree, rooted as in the first entry of `trees`.

## Details

This implementation uses the algorithm of Jansson et al. (2018) ; please
cite both this and Dong et al. (2010) when using this method.

## References

Dong J, Fernández-Baca D, McMorris FR, Powers RC (2010). “Majority-Rule
(+) consensus trees.” *Mathematical Biosciences*, **228**(1), 10–15.
[doi:10.1016/j.mbs.2010.08.002](https://doi.org/10.1016/j.mbs.2010.08.002)
.  
  
Jansson J, Rajaby R, Shen C, Sung W (2018). “Algorithms for the Majority
Rule (+) Consensus Tree and the Frequency Difference Consensus Tree.”
*IEEE/ACM Transactions on Computational Biology and Bioinformatics*,
**15**(1), 15–26.
[doi:10.1109/TCBB.2016.2609923](https://doi.org/10.1109/TCBB.2016.2609923)
.

## See also

Closely related:
[`Majority()`](https://constree.github.io/dev/reference/Majority.md),
[`Greedy()`](https://constree.github.io/dev/reference/Greedy.md),
[`Loose()`](https://constree.github.io/dev/reference/Loose.md).

Other consensus methods:
[`Adams()`](https://constree.github.io/dev/reference/Adams.md),
[`Average()`](https://constree.github.io/dev/reference/Average.md),
[`Frequency()`](https://constree.github.io/dev/reference/Frequency.md),
[`Greedy()`](https://constree.github.io/dev/reference/Greedy.md),
[`Local()`](https://constree.github.io/dev/reference/Local.md),
[`Loose()`](https://constree.github.io/dev/reference/Loose.md),
[`Majority()`](https://constree.github.io/dev/reference/Majority.md),
[`Quartet()`](https://constree.github.io/dev/reference/Quartet.md),
[`RStar()`](https://constree.github.io/dev/reference/RStar.md),
[`Strict()`](https://constree.github.io/dev/reference/Strict.md),
[`Transfer()`](https://constree.github.io/dev/reference/Transfer.md)

## Examples

``` r
trees <- ape::as.phylo(0:5, 8)
MajorityPlus(trees)
#> 
#> Phylogenetic tree with 8 tips and 5 internal nodes.
#> 
#> Tip labels:
#>   t1, t2, t8, t7, t6, t5, ...
#> 
#> Rooted; no branch length.
```
