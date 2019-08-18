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
library(plyr)

## 2004 example
indir <- system.file("scPDSI2003/data/example", package = "scPDSI")
outdir <- dirname(indir)

files <- dir(indir, full.names = TRUE) %>% set_names(., basename(.))
lst_files <- split_files(files)
lst <- map_depth(lst_files, 2, read_pdsi_input)

## MONTHLY ---------------------------------------------------------------------
{
    r <- read_pdsi_ouput(outdir, "monthly") %>% map(tidy_pdsi_df)
    df <- r$`original/potentials`

    y0_pdsi <- r$`self_cal/PDSI.tbl`
    # y0_scpdsi <- r$`self_cal/PDSI.tbl`
    y0 <- y0_pdsi

    sc_pdsi <- with(df,
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

    diff <- y0 - y1
    plot(diff, type = "l")
    abline(h = 0, col = "red"); grid()

    ## check abnorm
    d0 <- r$`self_cal/potentials`[, 1:5]
    d1 <- data.table(sc_pdsi$inter.vars[, 1:5])
    a <- cbind(d0, d1, d1 - d0) %>% cbind(diff = as.numeric(diff))# %>% as.tibble()

    # I <- which(abs(diff) > 0.05)
    # plot(y0[I], type = "b")
    # plot(y1[I], type = "b")
}

## WEEKLY ----------------------------------------------------------------------
{
    ## scales
    # df <- lst$week$
    r <- read_pdsi_ouput(outdir, "weekly") %>% map(tidy_pdsi_df)
    df <- r$`1/potentials`

    {
        y0_pdsi <- r$`1/PDSI.tbl`
        # y0_scpdsi <- r$`self_cal/PDSI.tbl`
        y0 <- y0_pdsi

        sc_pdsi <- with(df,
                        pdsi(P*25.4, PE*25.4, start = 1893, AWC = 12.18*25.4,
                             sc = TRUE, cal_start = 1961, cal_end = 1990, num_of_periods = 52))
        y1 <- sc_pdsi$X
        # plot(sc_pdsi) # plot PDSI
        par(mfcol = c(3, 1), mar = c(2, 3, 0.5, 1))
        plot(y0, type = "l");
        abline(h = 0, col = "red"); grid()

        # plot(y0_scpdsi, type = "l")
        plot(y1, type = "l")
        abline(h = 0, col = "red"); grid()

        diff <- y0 - y1
        plot(diff, type = "l")
        abline(h = 0, col = "red"); grid()
    }
}

## test about scPDSI
# {
#     y0 <- r$`original/PDSI.tbl`
#     y0_scpdsi <- r$`self_cal/PDSI.tbl`

#     # plot(sc_pdsi) # plot PDSI
#     par(mfcol = c(3, 1), mar = c(2, 3, 0.5, 1))
#     plot(y0, type = "l");
#     abline(h = 0, col = "red"); grid()

#     # plot(y0_scpdsi, type = "l")
#     plot(y0_scpdsi, type = "l")
#     abline(h = 0, col = "red"); grid()

#     plot(y0_scpdsi - y0, type = "l")
#     abline(h = 0, col = "red"); grid()
# }

