#' Read the input data of v2003 scPDSI to R
#'
#' @param file Character, input file path.
#'
#' @keywords internal
#'
#' @import magrittr
#' @importFrom utils read.table
#' @export
read_pdsi_input <- function(file){
    d <- read.table(file)
    tidy_pdsi_df(d)
}

tidy_pdsi_df <- function(d, missing = -99){
    m <- ncol(d)
    n <- nrow(d)
    d[d == -99] <- NA
    
    if (m %in% c(12, 52)) {
        return(as.numeric(d))
    } else if (m == 2) {
        colnames(d) <- c("Su", "TLA")
        return(d)
    } else {
        by <- switch(as.character(m-1),
                 "12" = "month",
                 "52" = "week")

        if (colnames(d)[1] == "V1" ) {
            if (m == 3) d <- d[, -1] # for dvalue
            val  <- as.matrix(d[, -1]) %>% t() %>% as.numeric()
            # date <- seq.Date(make_date(d$V1[1], 1, 1), make_date(d$V1[n], 12, 31), by)
            # res  <- zoo(val, date)
            val
        } else {
            if (colnames(d)[1] == "Year") d <- d[, -1]
            d[, -1]
        }
        # fprintf("%s by = %s\n", file, by)
        # print(by)
    }
}

# split input files input monthly and weekly, and return a list
split_files <- function(files){
    I_p <- grep("param", files)
    I_mon <- grep("mon", files)
    I_week <- grep("week", files)

    list(month = files[c(I_p, I_mon)],
        week = files[c(I_p, I_week)])
}

#' Read the output data of v2003 scPDSI to R
#' 
#' @param outdir Directory of output data.
#' @param scale Character, `monthly` or `weekly`.
#' 
#' @keywords internal
#' 
#' @importFrom purrr is_empty map
#' @importFrom data.table fread
#' @export
read_pdsi_ouput <- function(outdir, scale = "monthly") {

    FUN <- function(scale){
    dir <- sprintf("%s/%s", outdir, scale)

        files <- list.files(dir, full.names = TRUE, recursive = TRUE) %>%
            set_names(sprintf("%s/%s", basename(dirname(.)), basename(.)))

        if (!is_empty(files)){
            lst <- map(files, fread)
        }
        return(lst)
    }

    scale %>% intersect(c("monthly", "weekly")) %<>% set_names(., .)
    res <- map(scale, FUN)
    if (length(scale) == 1) res <- res[[1]]
    return(res)
}
