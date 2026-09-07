#' R* consensus tree
#'
#' `RStar()` returns the R\* consensus \insertCite{Degnan2009}{ConsTree} of a
#' set of rooted trees.
#'
#' The R\* consensus is a rooted-triplet method.  For every set of three leaves it
#' tallies, across the input trees, the three possible resolved rooted triplets
#' (`ab|c`, `ac|b`, `bc|a`) and keeps whichever appears most frequently.
#' Ties are not kept.
#' The kept triplets form the set of majority resolved triplets, \eqn{R_{maj}}.
#' Then R\* is the most resolved tree that displays no resolved triplet
#' outside \eqn{R_{maj}}.
#'
#' R\* is always a refinement of the majority-rule consensus:
#' every majority clade also appears in `RStar()`.
#'
#' @inheritParams Strict
#'
#' @return `RStar()` returns the consensus tree, an object of class `phylo`.
#' It is rooted by construction, but when the resolved triplets leave the
#' deepest divergence unresolved the root is a polytomy.
#'
#' @examples
#' # Five trees whose majority signal recovers the species tree (((a,b),c),d):
#' trees <- c(
#'   ape::read.tree(text = "(((a, b), c), d);"),
#'   ape::read.tree(text = "(((a, b), c), d);"),
#'   ape::read.tree(text = "(((a, b), c), d);"),
#'   ape::read.tree(text = "(((a, c), b), d);"),
#'   ape::read.tree(text = "(((b, c), a), d);")
#' )
#' 
#' # (a, b) wins {a,b,c} by plurality (3 vs 1 vs 1)
#' ape::write.tree(RStar(trees))
#'
#' @seealso Closely related: [`Strict()`], [`Majority()`], [`Adams()`],
#' [`Local()`].
#' @family consensus methods
#' @references \insertAllCited{}
#' @importFrom ape read.tree
#' @importFrom TreeTools Preorder RenumberTips TipLabels
#' @export
RStar <- function(trees) {
  # Coerce to a plain list.
  if (inherits(trees, "phylo")) {
    return(trees)
  }
  if (!is.list(trees)) {
    stop("`trees` must be a list of trees or a `multiPhylo` object.")
  }
  trees <- c(trees)
  trees <- trees[!vapply(trees, is.null, logical(1))]
  nTree <- length(trees)

  if (nTree < 2L) {
    return(if (nTree) trees[[1L]] else NULL)
  }

  labels <- TipLabels(trees[[1L]])
  if (anyDuplicated(labels)) {
    stop("all tip labels must be unique")
  }
  if (any(vapply(trees[-1L], function(tr)
    !setequal(TipLabels(tr), labels), logical(1)))) {
    stop("all trees must have the same tip labels")
  }
  n <- length(labels)

  if (n < 3L) {
    return(trees[[1L]])
  }

  # Relabel every tree 1..n in a shared canonical order and put in Preorder so
  # the C++ LCA pass (which assumes parent precedes child) is valid.
  edgeList <- lapply(trees, function(tr) {
    tr <- Preorder(RenumberTips(tr, labels))
    tr[["edge"]]
  })

  nwk <- rStarConsensus(edgeList, n)

  tree <- read.tree(text = paste0(nwk, ";"))
  tree[["tip.label"]] <- labels[as.integer(tree[["tip.label"]])]
  # Return:
  tree
}

