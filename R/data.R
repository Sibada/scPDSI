
#' @name Datasets
#'
#' @aliases Sample climatic data of Lubuge
#'
#' @title Datasets for illustrating the functions in the scPDSI package.
#'
#' @description
#' This is the sample dataset used in the examples of the scPDSI package.
#' It includes monthly precipitation (`P`, in mm) and potential
#' evapotranspiration (`PE`) for the Lubuge Bouyei and Miao Minority
#' Autonomous County (104.5 degree E, 24.75 degree N) from 1960 to 2016
#' (total 57 years). Lubuge is also the centroid of the severe drought
#' disaster of 2009-2010 in the southwest China.
#'
#' @details See description.
#'
#' @usage
#' data(Lubuge)
#'
#' @format
#' `Lubuge` dataset:
#' 
#' * `P` monthly precipitation totals `[mm]`.
#' * `PE` monthly potential evapotranspiration totals `[mm]`.
#' @source
#' Data of `Lubuge` were obtained from the CRU TS v4.01 datasets released by the
#' Climatic Research Unit (CRU)
#' (<https://crudata.uea.ac.uk/cru/data/hrg/cru_ts_4.01/>).
#'
#' @author Data were ported to R by Ruida Zhong.
#'
#' @examples
#' data(Lubuge)
#'
"Lubuge"
