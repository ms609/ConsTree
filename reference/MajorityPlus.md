# Majority-rule (+) consensus tree

`MajorityPlus()` computes the majority-rule (+) consensus (Jansson et
al. 2016) , which displays each clade that occurs in more input trees
than contradict it.

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

## References

Jansson J, Shen C, Sung W (2016). “Improved algorithms for constructing
consensus trees.” *Journal of the ACM*, **63**(3), 1–24.
[doi:10.1145/2925985](https://doi.org/10.1145/2925985) .

## See also

Closely related:
[`Majority()`](https://constree.github.io/reference/Majority.md),
[`Greedy()`](https://constree.github.io/reference/Greedy.md),
[`Loose()`](https://constree.github.io/reference/Loose.md).

Other consensus methods:
[`Adams()`](https://constree.github.io/reference/Adams.md),
[`Average()`](https://constree.github.io/reference/Average.md),
[`Frequency()`](https://constree.github.io/reference/Frequency.md),
[`Greedy()`](https://constree.github.io/reference/Greedy.md),
[`Local()`](https://constree.github.io/reference/Local.md),
[`Loose()`](https://constree.github.io/reference/Loose.md),
[`Majority()`](https://constree.github.io/reference/Majority.md),
[`Quartet()`](https://constree.github.io/reference/Quartet.md),
[`RStar()`](https://constree.github.io/reference/RStar.md),
[`Strict()`](https://constree.github.io/reference/Strict.md),
[`Transfer()`](https://constree.github.io/reference/Transfer.md)

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
