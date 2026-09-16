#' Fast algorithms for phylogenetic consensus trees
#'
#' `ConsTree` provides efficient methods for the construction of phylogenetic
#' consensus trees \insertCite{Bryant2003}{ConsTree}.
#' Methods include strict, majority-rule, majority-rule (+),
#' loose (combinable component / semi-strict), greedy, Adams, frequency
#' difference, R*, and local consensus.
#' 
#' It also incorporates distance-based summary trees using BHV, path-length,
#' quartet and transfer distances
#' \insertCite{LapointeCucumel1997,Takazawa2026,OwenProvan2011,BrownOwen2020}{ConsTree}.
#' 
#' It builds on the tree and split infrastructure of
#' [\pkg{TreeTools}](https://ms609.github.io/TreeTools/), and implements
#' asymptotically efficient consensus algorithms
#' \insertCite{Jansson2016acm,Jansson2017,Jansson2018,Jansson2018itcbab,Jansson2026}{ConsTree}.
#'
#' @keywords internal
#' @useDynLib ConsTree, .registration = TRUE
#' @importFrom Rcpp sourceCpp
"_PACKAGE"

# Suppress "NOTE: Nothing imported from Rdpack":
#' @importFrom Rdpack reprompt
NULL

## usethis namespace: start
## usethis namespace: end
NULL
