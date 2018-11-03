# scPDSI 0.1.2

* Try to fix the bug on compile error on solaris platform again. I feel sorry to mistake the reason for that the `floor` function is also overloaded in `R.h` or `Rcpp.h` therefore I simply commented out the including of `math.h`. The major challenge is that solaris platform is inavailable for us thus we could only do that tentatively.
Now I try to revised it to use the explicit namespace prefixes (`std::floor(...)`) according to suggestion of "Writing R Extensions" and I hope it works.

Many thanks again for your review on my R package.

Best regards.

# scPDSI 0.1.1

* Add a function `plot.pdsi` to draw the timeseries of calculated PDSI.

## Test environments
* ubuntu 12.04 (on travis-ci), R 3.2.3
* win-builder (devel and release)

## R CMD check results

0 errors | 0 warnings | 0 note

## Reverse dependencies

This is a new release, so there are no reverse dependencies.

