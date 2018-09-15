# scPDSI
[![Travis-CI Build Status](https://travis-ci.org/Sibada/scPDSI.svg?branch=master)](https://travis-ci.org/Sibada/scPDSI)
 * Turn on travis for your repo at https://travis-ci.org/Sibada/scPDSI

This R package is used to calculate the conventional **Palmer Drought Severity Index (PDSI)** and the **Self-Calibrating Palmer Drought Severity Index (scPDSI)**, the widely used drought indicators around the world, at monthly scale. Precipitation and potential evapotranspiration (PE) data are required to calculate the PDSI and scPDSI.

This package is build up on the C++ codes of the scPDSI provided by Nathan Wells, Steve Goddard and Michael J. Hayes in the University of Nebraska-Lincoln.

## Installation

```r
library(devtools)
install_github('Sibada', 'scPDSI')
```

## References

Please cite these references if you use the scPDSI on your work:

* Palmer W., 1965. Meteorological drought. U.s.department of Commerce Weather Bureau Research Paper.

* Wells N., Goddard S., Hayes M. J., 2004. A Self-Calibrating Palmer Drought Severity Index. Journal of Climate, 17(12):2335-2351.

## Example

This is an example showing how to calculate the scPDSI:

``` r
## P and PE are the vectors of monthly precipitation and PE data.
sc_pdsi <- pdsi(P, PE, start = 1960)
plot(sc_pdsi$X) # Plot the calculated PDSI values
```

## Copyright and license

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
