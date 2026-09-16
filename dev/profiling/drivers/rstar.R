# RStar() driver: two-tree O(n^2) path and general O(kn^3) path.
# Usage: Rscript dev/profiling/drivers/rstar.R [lib] [which: k2|general|both]
args <- commandArgs(TRUE)
lib <- if (length(args) >= 1 && nzchar(args[[1]])) args[[1]] else ".agent-cons"
which <- if (length(args) >= 2) args[[2]] else "both"
suppressMessages({
  library(ConsTree, lib.loc = lib)
  library(TreeTools)
})
set.seed(5813)

# Mostly congruent inputs: a base tree with a few leaf swaps per replicate,
# the gene-tree-like regime R* is meant for.
Perturb <- function(tr, m) {
  for (i in seq_len(m)) {
    ij <- sample(NTip(tr), 2)
    tr[["tip.label"]][ij] <- tr[["tip.label"]][rev(ij)]
  }
  tr
}
Timed <- function(label, expr) {
  t0 <- proc.time()[["elapsed"]]
  force(expr)
  cat(sprintf("%-28s %6.2f s\n", label, proc.time()[["elapsed"]] - t0))
}

if (which %in% c("k2", "both")) {
  n <- 4000
  base <- RandomTree(n, root = TRUE)
  k2 <- list(base, Perturb(base, 80))
  Timed("k = 2, n = 4000, x5", for (i in 1:5) RStar(k2))
}
if (which %in% c("general", "both")) {
  n <- 400
  base <- RandomTree(n, root = TRUE)
  k10 <- lapply(1:10, function(i) Perturb(base, 10))
  Timed("k = 10, n = 400, x3", for (i in 1:3) RStar(k10))
}
