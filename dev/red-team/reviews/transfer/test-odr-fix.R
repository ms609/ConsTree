# Discriminating test for the PooledSplits/SplitHash/SplitEqual ODR fix
# (anonymous namespaces in transfer_consensus.cpp + Quartet.cpp).
#
# If the cross-TU type collision was the cause of the "TC-008" load_all
# segfault, the Transfer() happy path should now run under load_all without
# crashing. Quartet is exercised too since it shares the fixed type names.
#   Rscript dev/red-team/reviews/transfer/test-odr-fix.R
pkgload::load_all(".", quiet = TRUE, compile = TRUE)
suppressMessages(library(TreeTools))
cat("load_all OK\n"); flush.console()

trees <- as.phylo(0:9, nTip = 8)
tc <- Transfer(trees)
cat("Transfer OK   NSplits =", NSplits(tc), "\n"); flush.console()

# Same-set / different-order (TC-002 path) and a second call to stress the
# shared ctor/dtor on both struct layouts.
t8a <- BalancedTree(letters[1:8])
t8b <- RenumberTips(t8a, rev(letters[1:8]))
tc2 <- Transfer(structure(list(t8a, t8b), class = "multiPhylo"))
cat("Transfer reorder OK   NSplits =", NSplits(tc2), "\n"); flush.console()

# Quartet consensus shares the (now TU-private) PooledSplits/SplitHash names.
qc <- Quartet(trees)
cat("Quartet OK   NSplits =", NSplits(qc), "\n"); flush.console()

cat("ALL OK\n")
