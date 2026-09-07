# Greedy (extended majority-rule) consensus tree

`Greedy()` computes the greedy consensus, also termed the extended
majority-rule consensus (Bryant 2003) . Distinct splits are considered
in decreasing order of their frequency across the input trees, breaking
ties arbitrarily; each is added to the growing consensus if it is
compatible with every split already accepted.

## Usage

``` r
Greedy(trees)
```

## Arguments

- trees:

  A list of trees, or a `multiPhylo` object. All entries must share the
  same leaf labels.

## Value

`Greedy()` returns an object of class `phylo` denoting the consensus,
rooted as in the first entry of `trees`.

## Details

The implementation builds upon the `greedyConsensusFast` algorithm of
(Jansson et al. 2016) ; please cite that paper when using this method.

## References

Bryant D (2003). “A classification of consensus methods for
phylogenetics.” In Janowitz MF, Lapointe F, McMorris FR, Mirkin B,
Roberts FS (eds.), *Bioconsensus*, volume 61 of *DIMACS Series in
Discrete Mathematics and Theoretical Computer Science*, 163–184.
American Mathematical Society.
[doi:10.1090/dimacs/061/11](https://doi.org/10.1090/dimacs/061/11) .  
  
Jansson J, Shen C, Sung W (2016). “Improved algorithms for constructing
consensus trees.” *Journal of the ACM*, **63**(3), 1–24.
[doi:10.1145/2925985](https://doi.org/10.1145/2925985) .

## See also

Closely related:
[`Strict()`](https://constree.github.io/reference/Strict.md),
[`Majority()`](https://constree.github.io/reference/Majority.md),
[`Loose()`](https://constree.github.io/reference/Loose.md).

Other consensus methods:
[`Adams()`](https://constree.github.io/reference/Adams.md),
[`Average()`](https://constree.github.io/reference/Average.md),
[`Frequency()`](https://constree.github.io/reference/Frequency.md),
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
trees <- ape::as.phylo(0:5, 8)
Greedy(trees)
#> 
#> Phylogenetic tree with 8 tips and 7 internal nodes.
#> 
#> Tip labels:
#>   t1, t4, t6, t7, t2, t8, ...
#> 
#> Rooted; no branch length.
```
