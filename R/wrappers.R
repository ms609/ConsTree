#' Strict consensus tree
#'
#' `Strict()` returns the strict consensus of a set of trees: the tree that
#' contains exactly those splits (clades) present in every input tree
#' \insertCite{Day1985}{ConsTree}.
#'
#' This is an alias of [`TreeTools::Consensus()`] with `p = 1`.
#'
#' @param trees A list of trees, or a `multiPhylo` object. All entries must
#' share the same leaf labels.
#'
#' @return `Strict()` returns the consensus tree, an object of class `phylo`,
#' rooted as in the first entry of `trees`.
#'
#' @examples
#' trees <- ape::as.phylo(0:5, 8)
#' Strict(trees)
#'
#' @seealso Less conservative summaries: [`Majority()`].
#' @family consensus methods
#' @references \insertAllCited{}
#' @importFrom TreeTools Consensus
#' @export
Strict <- function(trees) {
  Consensus(trees, p = 1)
}

#' Majority-rule consensus tree
#'
#' `Majority()` is a simple alias of [TreeTools::Consensus()], which computes
#' the majority-rule consensus \insertCite{MargushMcMorris1981}{ConsTree}:
#' the tree that contains each split that occurs in more than `p` of the input
#' trees.
#'
#' @inheritParams Strict
#' @param p Numeric between 0.5 and 1: the minimum proportion of trees that must
#' contain a split for it to be retained.
#'
#' @return `Majority()` returns an object of class `phylo` denoting the
#' majority rule consensus, rooted as in the first entry of `trees`.
#'
#' @examples
#' trees <- ape::as.phylo(0:5, 8)
#' Majority(trees, p = 0.6)
#'
#' @family consensus methods
#' @references \insertAllCited{}
#' @importFrom TreeTools Consensus
#' @export
Majority <- function(trees, p = 0.5) {
  Consensus(trees, p = p)
}

#' @rdname Majority
#' @export
MajorityRule <- Majority

#' @rdname Majority
#' @export
MR <- Majority

