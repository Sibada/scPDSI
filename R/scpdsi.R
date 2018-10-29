# Computation of the conventional Palmer Drought Severity Index (PDSI)
# and Self-calibrating Palmer Drought Severity Index (scPDSI).

#' @useDynLib scPDSI
#' @importFrom Rcpp sourceCpp
NULL

.onLoad <- function(libname, pkgname) {
  ops <- options()
  pdsi.ops <- list(
    # Calculating the conventional PDSI
    # Duration factors
    PDSI.p = 0.897,
    PDSI.q = 1/3,

    # Coefficients of climate characteristic
    PDSI.coe.K1.1 = 1.5,
    PDSI.coe.K1.2 = 2.8,
    PDSI.coe.K1.3 = 0.5,

    PDSI.coe.K2 = 17.67
  )

  toset <- !(names(pdsi.ops) %in% names(ops))
  if(any(toset)) {
    options(pdsi.ops[toset])
  }
}

#' Calculate the (sc)PDSI
#' @description Calculating the monthly conventional Palmer Drought Severity Index
#'              (PDSI) and Self-calibrating PDSI (scPDSI) using the precipitation
#'              and potential evapotranspiration.
#'
#' @param P Monthly precipitation series without NA [mm]. Can be a time series.
#'
#' @param PE Monthly potential evapotranspiration corresponding to the precipitation
#'            series. Can be calculated by the Penman-Monteith or the Thonthwate
#'            equation [mm].
#'
#' @param AWC Available soil water capacity of the soil layer [mm]. Default 100 mm.
#'
#' @param start Integer. Start year of the PDSI to be calculate default 1.
#'
#' @param end Integer. End year of the PDSI to be calculate.
#'
#' @param cal_start Integer. Start year of the calibrate period. Default is start year.
#'
#' @param cal_end Integer. End year of the calibrate period. Default is end year.
#'
#' @param sc Bool. Should use the self-calibrating procedure to calculate the climatical
#'           coefficient (K2 and duration coefficients). If not it would use the default
#'           parameters of Palmer (1965).
#'
#' @details
#'
#' The Palmer Drought Severity Index (PDSI), proposed by Palmer (1965), is a
#' widely used drought indicator to quantify the long-term drought conditions, for
#' an area at a certain time. The PDSI is a semi-physical based drought index calculated
#' using the precipitation and potential evapotranspiration data, based on a simple
#' two-layer bucket water balance model. Conventionally, the constants to calculate
#' the PDSI were firstly empirically derived by using the meteorological records in
#' Kansas and Iowa in middle US with a semi-arid climate conditions, therefore the
#' conventional PDSI usually could not satisfactorily represent the drought conditions
#' for other areas around the world, which also makes spatial comparisons of PDSI values
#' difficult.
#'
#' For this, Wells et al. (2004) proposed a self-calibrating Palmer Drought Severity
#' Index (scPDSI). The scPDSI could automatically adjust the empirical constants
#' in the PDSI computation with dynamically calculated values. Several works have
#' proved that the scPDSI performs better in spatially comparison than the conventional
#' PDSI. For more details please see the works of Wells et al. (2004).
#'
#' This function could also calculate the conventional PDSI with revised constants.
#' Before the scPDSI appeared, the PDSI calculation has also been revised focusing
#' on the local climate characteristics in many area around the world. Those
#' constants could be reset by using the global options before calculating the PDSI
#' e.g.:
#'
#' \preformatted{
#' options(PDSI.coe.K1.1 = 1.6)
#' }
#'
#' And calculation in the PDSI would be:
#' \deqn{Ki0=coeK11 * lg(((PEi+Ri+ROi)/(Pi+Li)+coeK12)/Di)+coeK13}
#' \deqn{Ki=coeK2/(\sum Dj*Kj) * Ki}
#' \deqn{X[i]=p*X[i-1]+q*Z[i]}
#'
#' \eqn{coeK11}, \eqn{coeK12}, \eqn{coeK13}, \eqn{coeK2}, \eqn{p}, and \eqn{q} are
#' corresponding to \code{PDSI.coe.K1.1}, \code{PDSI.coe.K1.2}, \code{PDSI.coe.K1.3},
#' \code{PDSI.coe.K2}, \code{PDSI.p}, and \code{PDSI.q}, respectively.
#'
#' For example, in a national standard in China about meteorological drought level
#' (GB/T 20481-2017), the PDSI was revised by re-adjust the constants in the
#' calculation. To use the PDSI based on this standard should set the global
#' options of R as follows:
#'
#' \preformatted{
#' options(PDSI.coe.K1.1 = 1.6)
#' options(PDSI.coe.K1.3 = 0.4)
#' options(PDSI.coe.K2 = 16.84)
#' options(PDSI.p = 0.755)
#' options(PDSI.q = 1/1.63)
#' }
#'
#' @return
#' This function return an object of class \code{pdsi}.
#'
#' The object of class \code{pdsi} is a list containing the following components:
#'
#' \itemize{
#'   \item call: the call to \code{pdsi} used to generate the object.
#'   \item X: time series of the X values, i.e. the Palmer Drought Severity Index (PDSI).
#'   \item inter.vars: An time series matrix containing the intermediate variables,
#'   including \code{P} (input precipitation), \code{PE} (input potential
#'   evapotranspiration), \code{PR} (potential recharge of soil moisture),
#'   \code{PRO} (potential runoff), \code{PL} (potential loss of soil moisture),
#'   \code{d} (water deficiencies), \code{Z} (water anomoly index, i.e. Z index),
#'   \code{Prob} (probability to end the wet or dry spell), \code{X1}, \code{X2} and
#'   \code{X3} (intermediate variables of calculating the X values).
#'   \item clim.coes: a matrix of the climate coefficients including \code{alpha},
#'   \code{beta}, \code{gamma}, \code{delta}, and \code{K1} coefficient for each month
#'   in a year.
#'   \item calib.coes: a matrix of the coefficients in the self-calibrating procedure
#'   of scPDSI, including \code{m}, \code{b} (slope and intercept of the
#'   duration-accumulated Z index plot), \code{p}, \code{q} (duration factors), and
#'   \code{K2} (ratio to adjust K coefficient) for wet and dry spell, respectively.
#'   Note that the P and PE would be convered from mm to inch in the calculation,
#'   therefore the units of \code{m}, \code{b} would also be inch correspondingly.
#'
#' }
#'
#' @references Palmer W., 1965. Meteorological drought. U.s.department of Commerce
#'             Weather Bureau Research Paper.
#'
#'             Wells, N., Goddard, S., Hayes, M. J., 2004. A Self-Calibrating Palmer
#'             Drought Severity Index. Journal of Climate, 17(12):2335-2351.
#'
#' @examples
#' library(scPDSI)
#' data(Lubuge)
#'
#' P <- Lubuge$P
#' PE <- Lubuge$PE
#' sc_pdsi <- pdsi(P, PE, start = 1960)
#' plot(sc_pdsi)
#'
#' # Without self-calibrating.
#' ori_pdsi <- pdsi(P, PE, start = 1960, sc = FALSE)
#' plot(ori_pdsi)
#'
#' # Without self-calibrating and use standards of
#' # mainland China. (GB/T 20481-2017)
#' options(PDSI.coe.K1.1 = 1.6)
#' options(PDSI.coe.K1.3 = 0.4)
#' options(PDSI.coe.K2 = 16.84)
#' options(PDSI.p = 0.755)
#' options(PDSI.q = 1/1.63)
#' gb_pdsi <- pdsi(P, PE, start = 1960, sc = FALSE)
#' plot(gb_pdsi)
#'
#' @importFrom stats ts
#'
#' @export
pdsi <- function(P, PE, AWC = 100, start = NULL, end = NULL, cal_start = NULL, cal_end = NULL,
                 sc = TRUE) {

  freq <- 12

  if(is.null(start)) start <-  1;
  if(is.null(end)) end <- start + ceiling(length(P)/freq) - 1

  if(is.null(cal_start)) cal_start <- start
  if(is.null(cal_end)) cal_end <- end

  res <- C_pdsi(P, PE, AWC, start, end, cal_start, cal_end, sc,
                getOption("PDSI.coe.K1.1"),
                getOption("PDSI.coe.K1.2"),
                getOption("PDSI.coe.K1.3"),
                getOption("PDSI.coe.K2"),
                getOption("PDSI.p"),
                getOption("PDSI.q"))

  names(res) <- c("inter.vars", "clim.coes", "calib.coes")

  inter.vars <- res[[1]]
  clim.coes <- res[[2]]
  calib.coes <- res[[3]]
  inter.vars[inter.vars == -999.] <- NA

  out <- list(call = match.call(expand.dots=FALSE),
              X = ts(inter.vars[, 14], start = start, frequency = freq),
              inter.vars = ts(inter.vars[, 3:13], start = start, frequency = freq))

  colnames(out$inter.vars) <- c("P", "PE", "PR", "PRO", "PL", "d", "Z",
                                "Prob", "X1", "X2", "X3")

  dim(calib.coes) <- c(2, 5)
  colnames(calib.coes) <- c("m", "b", "p", "q", "K2")
  rownames(calib.coes) <- c('wet', 'dry')

  colnames(clim.coes) <- c("alpha", "beta", "gamma", "delta", "K1")
  rownames(clim.coes) <- month.name

  out$clim.coes <- clim.coes
  out$calib.coes <- calib.coes

  out$self.calib <- sc
  out$range <- c(start, end)
  out$range.ref <- c(cal_start, cal_end)

  class(out) <- "pdsi"
  out
}

#' @title plot (sc)PDSI
#'
#' @description plot the timeseries of calculated (sc)PDSI.
#'
#' @param x an object of class \code{pdsi}.
#' @param tit title of the plot.
#' @param ... additional parameters, not used at present.
#'
#' @details  Plot the timeseries of (sc)PDSI using function\code{\link{pdsi}}.
#' Values over 6 or below -6 and NA values would be shown by grey points.
#'
#' @importFrom graphics plot polygon abline lines points
#' @importFrom stats start end ts frequency
#' @seealso
#' \code{\link{pdsi}}
#'
#' @export
plot.pdsi <- function(x, tit = NULL, ...) {
  # Codes are referenced from the plot function of SPEI by Begueria et al.,
  # see package `SPEI`.
  ser <- x$X
  sup <- ifelse(ser > 6, 6, NA)
  sdn <- ifelse(ser < -6, -6, NA)

  ser[is.na(ser)] <- 0
  ser[ser > 6] <- 6
  ser[ser < -6] <- -6
  sna <- ifelse(ser == 0, 0, NA)

  # Copy from source code of SPEI
  if (start(ser)[2]==1) {
    ns <- c(start(ser)[1]-1, 12)
  } else {
    ns <- c(start(ser)[1], start(ser)[2]-1)
  }
  if (end(ser)[2]==12) {
    ne <- c(end(ser)[1]+1, 1)
  } else {
    ne <- c(end(ser)[1], end(ser)[2]+1)
  }

  label <- ifelse(x$self.calib, 'scPDSI', 'PDSI')

  ser2 <- ts(c(0, ser, 0),frequency=frequency(ser), start=ns,end=ne)
  ser2.p <- ifelse(ser2 > 0, ser2, 0)
  ser2.n <- ifelse(ser2 <= 0, ser2, 0)
  plot(ser2, type='n', xlab='', ylab=paste(label, "(X-values)"), main=tit)
  if (!all(x$range == x$range.ref)) {
    k <- ts(8, start = c(x$range.ref[1],1), end = c(x$range.ref[2], 12), frequency=12)
    k[1] <- k[length(k)] <- -8
    polygon(k, col='#cccccc', border=NA, density=15)
    abline(v=x$range.ref[1], col='#888888')
    abline(v=x$range.ref[2]+1, col='#888888')
  }
  #grid(col='#666666', nx = NA, ny = NULL, lty=2)
  polygon(ser2.p, col = 'black', border=NA)
  polygon(ser2.n, col = 'black', border=NA)
  lines(ser2)
  abline(h=0)
  points(sna, pch=21, col='white', bg='#888888')
  points(sup, pch=21, col='white', bg='#888888')
  points(sdn, pch=21, col='white', bg='#888888')
}
