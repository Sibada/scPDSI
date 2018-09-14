# scPDSI
[![Travis-CI Build Status](https://travis-ci.org/Sibada/scPDSI.svg?branch=master)](https://travis-ci.org/Sibada/scPDSI)
 * Turn on travis for your repo at https://travis-ci.org/Sibada/scPDSI

This R package is used to calculate the conventional **Palmer Drought Severity Index (PDSI)** and the **Self-Calibrating Palmer Drought Severity Index (scPDSI)**, the widely used drought indicators around the world, at monthly scale. Precipitation and potential evapotranspiration (PE) data are required to calculate the PDSI and scPDSI.
This package is build up on the C++ codes of the scPDSI provided by Nathan Wells, Steve Goddard and Michael J. Hayes in the University of Nebraska¡ªLincoln.

## Installation

```r
library(devtools)
install_github('Sibada', 'scPDSI')
```

## References

Please cite these references if you use the scPDSI on your work:

* Palmer W. Meteorological drought[J]. U.s.department of Commerce Weather Bureau Research Paper, 1965.

* Wells N, Goddard S, Hayes M J. A Self-Calibrating Palmer Drought Severity Index[J]. Journal of Climate, 2010, 17(12):2335-2351.

## Example

This is an example showing how to calculate the scPDSI:

``` r
## P and PE are the vectors of monthly precipitation and PE data.
sc_pdsi <- pdsi(P, PE, start = 1960)
plot(sc_pdsi$X) # Plot the calculated PDSI values
```
