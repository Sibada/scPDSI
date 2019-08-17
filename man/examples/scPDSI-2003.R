# library(scPDSI)
# data(Lubuge)
#
# P <- Lubuge$P
# PE <- Lubuge$PE
# sc_pdsi <- pdsi(P, PE, start = 1960)
# plot(sc_pdsi) # plot PDSI

library(magrittr)
library(purrr)
library(lubridate)
library(zoo)
library(plyr)
## 2004 example
indir <- system.file("backup/data/example", package = "scPDSI")
outdir <- dirname(indir)

files <- dir(indir, full.names = TRUE) %>% set_names(., basename(.))
lst_files <- split_files(files)
lst <- map_depth(lst_files, 2, read_pdsi_input)

## scales
r <- read_pdsi_ouput(outdir, "monthly") %>% map(tidy_pdsi_df)

{
    y0_pdsi <- r$`original/PDSI.tbl`
    y0_scpdsi <- r$`self_cal/PDSI.tbl`
    y0 <- y0_scpdsi

    sc_pdsi <- with(r$`original/potentials`,
                    pdsi(P*25.4, PE*25.4, start = 1893, AWC = 12.18*25.4,
                         sc = TRUE, cal_start = 1961, cal_end = 1990))
    y1 <- sc_pdsi$X
    # plot(sc_pdsi) # plot PDSI
    par(mfcol = c(3, 1), mar = c(2, 3, 0.5, 1))
    plot(y0, type = "l");
    abline(h = 0, col = "red"); grid()

    # plot(y0_scpdsi, type = "l")
    plot(y1, type = "l")
    abline(h = 0, col = "red"); grid()

    diff <- y1 - y0
    plot(diff, type = "l")
    abline(h = 0, col = "red"); grid()
}

## test about scPDSI
{
    y0 <- r$`original/PDSI.tbl`
    y0_scpdsi <- r$`self_cal/PDSI.tbl`

    # plot(sc_pdsi) # plot PDSI
    par(mfcol = c(3, 1), mar = c(2, 3, 0.5, 1))
    plot(y0, type = "l");
    abline(h = 0, col = "red"); grid()

    # plot(y0_scpdsi, type = "l")
    plot(y0_scpdsi, type = "l")
    abline(h = 0, col = "red"); grid()

    plot(y0_scpdsi - y0, type = "l")
    abline(h = 0, col = "red"); grid()
}

