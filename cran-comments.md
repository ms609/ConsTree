## Test environments

* Local PC:
  - Windows 10, R devel

* [GitHub Actions](https://github.com/ms609/TreeTools/actions)
  - Ubuntu latest
    - R 4.1
    - R release (tests, examples & vignettes run with valgrind)
    - R devel
  - Mac OS X latest, R release
  - Microsoft Windows Server latest, R release
  - RHub checks

## R CMD check results

There were no ERRORs or WARNINGs.
There were XXX NOTES:


> Possibly misspelled words in DESCRIPTION:
>   combinable (21:10)

False positive: technical term, included in inst/WORDLIST.

  
> Found the following (possibly) invalid DOIs:
>   DOI: 10.32614/CRAN.package.ConsTree
>     From: inst/CITATION
>     Status: 404
>     Message: Not Found
    
I anticipate that the doi will be registered automatically once the package is
available on CRAN.
    
## Downstream dependencies

There are no downstream dependencies.

