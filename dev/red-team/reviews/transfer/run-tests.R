pkgload::load_all(".", quiet = TRUE, compile = FALSE)
suppressMessages(library(testthat))
testthat::test_dir("tests/testthat", filter = "transfer|Quartet",
                   reporter = "summary", stop_on_failure = FALSE)
