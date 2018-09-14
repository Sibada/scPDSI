#include "pdsi.h"

// Main function to calculate scPDSI.
// [[Rcpp::export]]
List C_pdsi(NumericVector P, NumericVector PE, double AWC,
              int s_yr, int e_yr, int calib_s_yr, int calib_e_yr,
              bool sc,
              double K1_1, double K1_2, double K1_3, double K2,
              double p, double q) {

  pdsi PDSI;

  PDSI.Rext_init(P, PE, AWC, s_yr, e_yr, calib_s_yr, calib_e_yr);
  PDSI.Rext_set_parcoefs(K1_1, K1_2, K1_3, K2, p, q);

  PDSI.Rext_PDSI_mon(sc);

  List z = List::create(PDSI.vals_mat, PDSI.coefs_mat,
                        PDSI.Rext_out_params());
  return z;
}
