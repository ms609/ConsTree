#' Adams consensus tree
#'
#' `Adams()` computes the \insertCite{Adams1972;textual}{ConsTree} consensus
#' tree.
#'
#' The Adams consensus places each species in the the smallest group to which
#' it belongs on all input trees. Consequently, it may contain groupings that do
#' not appear in any input tree#'
#' This implementation builds on the algorithm of
#' \insertCite{JanssonLiSung2017}{ConsTree}; please cite this paper where you
#' use this method.
#'
#' @inheritParams Strict
#'
#' @return `Adams()` returns an object of class `phylo` denoting the Adams
#' consensus tree.
#'
#' @examples
#' # Two rooted trees that disagree only on the position of one leaf
#' trees <- c(ape::read.tree(text = "(((a, b), c), d);"),
#'            ape::read.tree(text = "(((a, b), d), c);"))
#' # keeps the clade (a, b); leaves c, d unresolved at the root
#' ape::write.tree(Adams(trees))
#'
#' @family consensus methods
#' @references \insertAllCited{}
#' @importFrom ape read.tree
#' @importFrom TreeTools Preorder RenumberTips TipLabels
#' @export
Adams <- function(trees) {
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
    return(if (nTree) trees[[1]] else NULL)
  }
  labels <- TipLabels(trees[[1]])
  n <- length(labels)
  # Adams is defined only for trees on a common leaf set; RenumberTips() below
  # would otherwise fail with an opaque message, so check up front.
  if (!all(vapply(trees[-1], function(tr) setequal(TipLabels(tr), labels),
                  logical(1)))) {
    stop("All `trees` must describe the same leaves.")
  }
  if (n < 3L) {
    return(trees[[1]])
  }
  # Marshal each tree on its OWN root (Adams is rooted; do not re-root at taxon 1
  # as the split methods do).  RenumberTips aligns ape tip i with labels[i] in
  # every tree, so the integer labels the C++ returns map straight back.
  edgeList <- lapply(trees, function(tr) {
    Preorder(RenumberTips(tr, labels))[["edge"]]
  })
  nwk <- paste0(adamsConsensusCpp(edgeList, n), ";")
  tree <- read.tree(text = nwk)
  tree[["tip.label"]] <- labels[as.integer(tree[["tip.label"]])]
  # Return: rooted by construction -- do not re-root.
  tree
}
