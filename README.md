# scPDSI
[![Travis-CI Build Status](https://travis-ci.org/Sibada/scPDSI.svg?branch=master)](https://travis-ci.org/Sibada/scPDSI)
[![CRAN RStudio mirror downloads](http://cranlogs.r-pkg.org/badges/scPDSI)](https://cran.r-project.org/package=scPDSI)
[![CRAN version](http://www.r-pkg.org/badges/version/scPDSI)](https://cran.r-project.org/package=scPDSI)
 * Turn on travis for your repo at https://travis-ci.org/Sibada/scPDSI

This R package is used to calculate the conventional **Palmer Drought Severity Index (PDSI)** and the **Self-Calibrating Palmer Drought Severity Index (scPDSI)**, the widely used drought indicators around the world, at monthly scale. Precipitation and potential evapotranspiration (PE) data are required to calculate the PDSI and scPDSI.

This package is build up on the C++ codes of the scPDSI provided by Nathan Wells, Steve Goddard and Michael J. Hayes in the University of Nebraska-Lincoln.

## Installation

Get it from the CRAN repository:

```R
install.packages('scPDSI')
```

Or install from GitHub (usually the development version):

```R
library(devtools)
install_github('Sibada', 'scPDSI')
```


Please cite these references if you use the scPDSI on your work:

* Palmer W., 1965. Meteorological drought. *U.s.department of Commerce Weather Bureau Research Paper*, <<https://www.ncdc.noaa.gov/temp-and-precip/drought/docs/palmer.pdf>>.

* Wells N., Goddard S., Hayes M. J., 2004. A Self-Calibrating Palmer Drought Severity Index. *Journal of Climate*, **17**(12):2335-2351, <[doi:10.1175/1520-0442(2004)017<2335:ASPDSI>2.0.CO;2](http://dx.doi.org/10.1175/1520-0442(2004)017%3C2335:ASPDSI%3E2.0.CO;2)>.

## Example

This is an example showing how to calculate the scPDSI:

``` r
## P and PE are the vectors of monthly precipitation and PE data.
sc_pdsi <- pdsi(P, PE, start = 1960, sc = TRUE)
plot(sc_pdsi$X) # Plot the calculated PDSI values
```

## Copyright and license

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
