# Local consensus tree

`Local()` returns the local consensus (Kannan et al. 1998) of a set of
rooted trees. The local consensus is the most conservative tree
consistent with the rooted triplets shared by every input tree. The
minimum rooted local consensus (MinRLC, `type = "rooted"`) and the
minimum induced local consensus (MinILC, `type = "induced"`) differ in
how the resolution of the result is scored.

## Usage

``` r
Local(trees, type = c("rooted", "induced"))
```

## Arguments

- trees:

  A list of trees, or a `multiPhylo` object. All entries must share the
  same leaf labels.

- type:

  Character specifying whether to compute the minimum rooted local
  consensus (`"rooted"`, the default; MinRLC) or the minimum induced
  local consensus (`"induced"`; MinILC).

## Value

`Local()` returns an object of class `phylo` denoting the local
consensus tree. When there is no valid consensus (as no common triplets
separate any pair of trees), a star tree is returned.

## Details

The implementation builds on the algorithms of (Jansson et al. 2018) ,
which generalize the "RV-II" type consensus of Kannan et al. (1998) ;
please cite both papers when using this method.

Because the algorithm is exponential, `Local()` is limited to `n <= 20`
leaves. Running time is faster when input trees are more congruent.

## References

Jansson J, Rajaby R, Sung W (2018). “Minimal phylogenetic supertrees and
local consensus trees.” *AIMS Medical Science*, **5**(2), 181–203.
[doi:10.3934/medsci.2018.2.181](https://doi.org/10.3934/medsci.2018.2.181)
.  
  
Kannan S, Warnow T, Yooseph S (1998). “Computing the Local Consensus of
Trees.” *SIAM Journal on Computing*, **27**(6), 1695–1724.
[doi:10.1137/S0097539795287642](https://doi.org/10.1137/S0097539795287642)
.

## See also

Closely related:
[`Strict()`](https://constree.github.io/dev/reference/Strict.md),
[`Majority()`](https://constree.github.io/dev/reference/Majority.md),
[`Adams()`](https://constree.github.io/dev/reference/Adams.md).

Other consensus methods:
[`Adams()`](https://constree.github.io/dev/reference/Adams.md),
[`Average()`](https://constree.github.io/dev/reference/Average.md),
[`Frequency()`](https://constree.github.io/dev/reference/Frequency.md),
[`Greedy()`](https://constree.github.io/dev/reference/Greedy.md),
[`Loose()`](https://constree.github.io/dev/reference/Loose.md),
[`Majority()`](https://constree.github.io/dev/reference/Majority.md),
[`MajorityPlus()`](https://constree.github.io/dev/reference/MajorityPlus.md),
[`Quartet()`](https://constree.github.io/dev/reference/Quartet.md),
[`RStar()`](https://constree.github.io/dev/reference/RStar.md),
[`Strict()`](https://constree.github.io/dev/reference/Strict.md),
[`Transfer()`](https://constree.github.io/dev/reference/Transfer.md)

## Examples

``` r
# Two trees that agree on one cherry but disagree on overall topology
t1 <- ape::read.tree(text = "(1,((2,3),4));")
t2 <- ape::read.tree(text = "(1,((2,4),3));")
Local(list(t1, t2), "rooted")  # keeps clade {2,3,4} only
#> 
#> Phylogenetic tree with 4 tips and 2 internal nodes.
#> 
#> Tip labels:
#>   1, 2, 3, 4
#> 
#> Rooted; no branch length.
```
