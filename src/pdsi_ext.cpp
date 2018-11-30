#include "pdsi.h"

void pdsi::Rext_init(NumericVector& P, NumericVector& PE,
                     number o_AWC,
                     int s_yr, int e_yr,
                     int calib_s_yr, int calib_e_yr) {
  int nPeriods;
  int input_len = P.length();
  metric = 1;
  verbose = 0;
  num_of_periods = 12;

  if(s_yr >= e_yr)
    Rf_error("Start year (%d) must earlier than end year (%d).", s_yr, e_yr);

  if(calib_s_yr >= calib_e_yr)
    Rf_error("Calibrating start year (%d) must earlier than "
             "calibrating end year (%d).", calib_s_yr, calib_e_yr);

  startyear = s_yr;
  endyear = e_yr;
  s_year = s_yr;
  e_year = e_yr;
  calibrationStartYear = calib_s_yr;
  calibrationEndYear = calib_e_yr;

  if(input_len != PE.length())
    Rf_error("Length of input P (%d) is not equal to input PE (%d).",
             input_len, PE.length());

  if(calibrationStartYear < startyear) {
    Rf_warning("Calibrating start year (%d) is earlier than start year (%d), "
               "it would be set as start year.", calib_s_yr, s_yr);
    calibrationStartYear = startyear;
  }
  if(calibrationEndYear > endyear) {
    Rf_warning("Calibrating end year (%d) is later than end year (%d), "
                 "it would be set as end year.", calib_e_yr, e_yr);
    calibrationEndYear = endyear;
  }

  setCalibrationStartYear = 1;
  setCalibrationEndYear = 1;

  totalyears = endyear - startyear + 1;
  currentCalibrationStartYear = calibrationStartYear;
  currentCalibrationEndYear = calibrationEndYear;
  nStartYearsToSkip = currentCalibrationStartYear - startyear;
  nEndYearsToSkip = endyear - currentCalibrationEndYear;
  nCalibrationYears = currentCalibrationEndYear - currentCalibrationStartYear + 1;

  nStartPeriodsToSkip = nStartYearsToSkip * num_of_periods;
  nEndPeriodsToSkip = nEndYearsToSkip * num_of_periods;
  nCalibrationPeriods = nCalibrationYears * num_of_periods;
  nPeriods = totalyears * num_of_periods;

  if((int)ceil(input_len * 1. / num_of_periods) < totalyears)
    Rf_error("Years of input P (%d years) should not shorter than"
             "years of output settings (%d years).",
             (int)ceil(input_len * 1. / num_of_periods),
             totalyears);

  AWC = o_AWC / 25.4;

  Ss = 1.0;
  if(AWC < Ss){
    AWC = Ss;
  }
  Su = AWC - Ss;
  if(Su < 0)
    Su = 0;

  P_vec = P;
  PE_vec = PE;
  //d_vec = NumericVector(nPeriods);
  //Z_vec = NumericVector(nPeriods);
  vals_mat = NumericMatrix(nPeriods, 16);
  coefs_mat = NumericMatrix(12, 5);
  K_w = 1.;
  K_d = 1.;

  coe_K1_1 = 1.5;
  coe_K1_2 = 2.8;
  coe_K1_3 = 0.5;
  coe_K2 = 17.67;

  coe_p = 0.897;
  coe_q = 1./3.;

  coe_m = (1-coe_p)/coe_q;
  coe_b = coe_p/coe_q;
}

void pdsi::Rext_set_parcoefs(number K1_1, number K1_2, number K1_3, number K2,
                       number p, number q) {
  coe_K1_1 = K1_1;
  coe_K1_2 = K1_2;
  coe_K1_3 = K1_3;
  coe_K2 = K2;

  coe_p = p;
  coe_q = q;

  coe_m = (1-coe_p)/coe_q;
  coe_b = coe_p/coe_q;
}

void pdsi::Rext_get_Rvec(NumericVector R_vec, int year, number* A, int freq) {
  int rng = min(freq, R_vec.length() - (year - 1) * freq);
  for(int i = 0; i < freq; i++) {
    if(i < rng)
      A[i] = R_vec[(year - 1) * freq + i];
    else
      A[i] = MISSING;

    if(metric && A[i] != MISSING) {
        A[i] = A[i]/25.4;
    }
  }

}

void pdsi::Rext_PDSI_mon(bool sc) {
  int i;
  //FILE *param;
  //char filename[170];

  SCMonthly = true;
  Monthly = false;
  Weekly = false;

  /*
  if(initialize() < 1){
    if(verbose > 0)
      printf("%4s Cannot calculate the Self-Calibrated Monthly PDSI.\n","*");
    if(verbose > 1){
      printf("The necessary input files ");
      printf("were not found in the directory %s\n",input_dir);
      printf("The following files are required:\n");
      printf("monthly_T \nmonthly_P \nparameter \nmon_T_normal\n\n");
    }
    return;
  }
  */
  period_length = 1;
  num_of_periods = 12;

  /* num_of_periods is set in pdsi constructor and reset in here with the number of periods in a year */
  currentCalibrationStartYear = calibrationStartYear;
  currentCalibrationEndYear = calibrationEndYear;
  nCalibrationYears = currentCalibrationEndYear - currentCalibrationStartYear + 1;
  nStartYearsToSkip = currentCalibrationStartYear - startyear;
  nEndYearsToSkip = endyear - currentCalibrationEndYear;
  nStartPeriodsToSkip = nStartYearsToSkip * num_of_periods;
  nEndPeriodsToSkip = nEndYearsToSkip * num_of_periods;
  nCalibrationPeriods = nCalibrationYears * num_of_periods;

  /*
  if(verbose > 1){
    printf("\nstartyear = %d\n",startyear);
    printf("endyear = %d\n",endyear);
    printf("totalyears = %d\n",totalyears);
    printf("setCalibrationStartYear = %d\n",setCalibrationStartYear);
    printf("setCalibrationEndYear = %d\n",setCalibrationEndYear);
    printf("calibrationStartYear = %d\n",calibrationStartYear);
    printf("calibrationEndYear = %d\n",calibrationEndYear);
    printf("currentCalibrationStartYear = %d\n",currentCalibrationStartYear);
    printf("currentCalibrationEndYear = %d\n",currentCalibrationEndYear);
    printf("num_of_periods = %d\n",num_of_periods);
    printf("nStartYearsToSkip = %d\n",nStartYearsToSkip);
    printf("nStartPeriodsToSkip = %d\n",nStartPeriodsToSkip);
    printf("nEndYearsToSkip = %d\n",nEndYearsToSkip);
    printf("nEndPeriodsToSkip = %d\n",nEndPeriodsToSkip);
    printf("nCalibrationYears = %d\n",nCalibrationYears);
    printf("nCalibrationPeriods = %d\n",nCalibrationPeriods);
  }
  */
  /* SG 6/5/06: end addition */

  // This block opens the parameter file and sets the initial Su and TLA values
  // must be called after the variable period_length is determined in the
  // set_flags function

  /*
  if(strlen(input_dir)>1)
    strcpy(filename,input_dir);
  else
    strcpy(filename,"./");
  strcat(filename,"parameter");
  if((param=fopen(filename,"r"))==NULL) {
    printf("Unable to open parameter file.\n");
    printf("File name: %s\n",filename);
    exit(1);
  }
  GetParam(param);
  fclose(param);
   */

  // Output seen only in maximum verbose mode
  /*
  if(verbose>1)
    printf ("processing station 1\n");
   */
  // SumAll is called to compute the sums for the 8 water balance variables
  SumAll();
  // This outputs those sums to the screen
  /*
  //if(verbose>1) {
    printf ("STATION = %5d %18c SUMMATION OF MONTHLY VALUES OVER %4d YEARS\n", 0, ' ', totalyears);
    printf ("%36c CALIBRATION YEARS:\n", ' ');
    printf ("%4s %7s %8s %8s %8s %8s %8s %8s %8s %8s %10s", "PER", "P", "S", "PR", "PE", "PL", "ET", "R", "L", "RO", "DEP\n\n");
  //}
  */
  for (i = 0;i < num_of_periods;i++) {
    /* DEPSum will only include calibration interval data since the ET, R, PE, and RO
    ** sum variables only include data from the calibration interval.
    */
    DEPSum[i] = ETSum[i] + RSum[i] - PESum[i] + ROSum[i];
    /*
    //if(verbose>1) {
      printf ("%4d", (period_length*i)+1);
      printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", PSum[i], PROSum[i], PRSum[i], PESum[i], PLSum[i]);
      printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", ETSum[i], RSum[i], LSum[i], ROSum[i], DEPSum[i]);
      printf ("\n");
    //}
    */
    DSSqr[i] = 0;
  }

  // CalcWBCoef is then called to calculate alpha, beta, gamma, and delta
  /* These variables will only include calibration interval data since the other
  ** sum variables only include data from the calibration interval--set in SumALL().
  */
  CalcWBCoef();
  // Next Calcd is called to calculate the monthly departures from normal
  Calcd();
  // CalcK is called to compute the K values
  /* These variables will only include calibration interval data since the other
  ** sum variables only include data from the calibration interval--set in SumALL().
  */

  // self-calibrating or not
  if(sc) {
    CalcK();
    // CalcZ is called to compute the Z index
    CalcZ();

    /*
    //the calibration process begins
    if(verbose > 1)
      printf("\nCalibrating Index.\n");
     */
    //calculate the duration factors
    CalcDurFact(wetm, wetb, 1);
    CalcDurFact(drym, dryb, -1);
    /*
    //if(verbose > 1) {
      printf("duration factors:\n");
      printf("wetm = %.3f  wetb = %.3f --> ",wetm,wetb);
      printf("p = %.3f and q = %.3f \n", 1-(wetm/(wetm+wetb)), 1/(wetm+wetb));
      printf("drym = %.3f  dryb = %.3f --> ",drym,dryb);
      printf("p = %.3f and q = %.3f \n", 1-(drym/(drym+dryb)), 1/(drym+dryb));
    //}
     */
    //Calculate the PDSI values
    CalcX();
    //Calibrate the Index
    for(int i = 0; i < 3; i++)
      Calibrate();
    // Now that all calculations have been done they can be output to the screen
    /* SG 6/5/06: changed totalyears to nCalibrationYears means to support
     **            user defined calibration intervals. When not used
     **            nCalibrationYears==totalyears; hence no change then
     */

  } else {
    CalcOrigK();
  }
  /*
  if(verbose>1) {
    int i;
    printf ("STATION = %5d %24c PARAMETERS AND MEANS OF MONTHLY VALUE FOR %d YEARS\n\n", 0, ' ', nCalibrationYears); // SG 6/5/06: May want to clarify this is number Calibration Years when so specified
    printf ("%4s %8s %8s %8s %8s %8s %8s %7s %8s", "PER", "ALPHA", "BETA", "GAMMA", "DELTA", "Wet K", "Dry K", "P", "S");
    printf ("%9s %8s %8s %8s %8s %8s %8s\n\n", "PR", "PE", "PL", "ET", "R", "L", "RO");
    for (i=0;i<num_of_periods;i++) {
      printf ("%4d %8.4f %8.4f %8.4f %8.4f", (period_length*i)+1, Alpha[i], Beta[i], Gamma[i], Delta[i]);
      printf ("%9.3f %8.3f %8.2f %8.2f %8.2f", k[i]*wet_ratio, k[i]*dry_ratio, PSum[i]/nCalibrationYears, PROSum[i]/nCalibrationYears, PRSum[i]/nCalibrationYears);
      printf ("%9.2f %8.2f %8.2f %8.2f", PESum[i]/nCalibrationYears, PLSum[i]/nCalibrationYears, ETSum[i]/nCalibrationYears, RSum[i]/nCalibrationYears);
      printf ("%9.2f %8.2f\n", LSum[i]/nCalibrationYears, ROSum[i]/nCalibrationYears);
    }
    printf ("\n\n\n%4s %8s %8s %8s %8s %8s\n\n", "PER", "D-ABS", "SIG-D", "DEP", "S-DEP", "SIG-S");
    for (i=0;i<num_of_periods;i++) {
      printf ("%4d %8.3f %8.2f %8.2f ", (period_length*i)+1, D[i], sqrt(DSSqr[i]/(nCalibrationYears-1)), DEPSum[i]/nCalibrationYears);
      if (i==7) {
        number E, DE;
        E=SD/nCalibrationYears;
        DE=sqrt((SD2-E*SD)/(nCalibrationYears-1));
        printf ("%8.2f %8.2f", E, DE);
      }
      printf ("\n");
    }
  }
*/
  Rext_output_X();
}


void pdsi::Rext_output_X() {
  int n = 0;
  number x, x1, x2, x3, p, wp, ph;
  llist tempX, tempX1, tempX2, tempX3, tempP;
  copy(tempX, Xlist);
  copy(tempX1, XL1);
  copy(tempX2, XL2);
  copy(tempX3, XL3);
  copy(tempP, ProbL);

  while(!tempX.is_empty() && n < vals_mat.nrow()) {
    x = tempX.tail_remove();
    x1 = tempX1.tail_remove();
    x2 = tempX2.tail_remove();
    x3 = tempX3.tail_remove();
    p = tempP.tail_remove()/100.;

    ph = x3;
    if (x3==0) {
      // There is not an established wet or dry spell so PHDI = PDSI (ph=x)
      // and the WPLM value is the maximum absolute value of X1 or X2
      ph = x;
      wp = x1;
      if (-x2 > (x1 + tolerance))
        wp=x2;
    }
    else if (p > (0+tolerance/100) && p < (1-tolerance/100)) {
      // There is an established spell but there is a possibility it has or is
      // ending.  The WPLM is then a weighted average between X3 and X1 or X2
      if (x3 < 0)
        // X3 is negative so WPLM is weighted average of X3 and X1
        wp = (1-p)*x3 + p*x1;
      else
        // X3 is positive so WPLM is weighted average of X3 and X2
        wp = (1-p)*x3 + p*x2;
    }
    else
      // There is an established spell without possibility of end meaning the
      // WPLM is simply X3
      wp=x3;

    vals_mat(n, 13) = x;
    vals_mat(n, 14) = ph;
    vals_mat(n, 15) = wp;
    n++;
  }
}

NumericVector pdsi::Rext_out_params() {
  NumericVector outp(10);
  outp[0] = wetm;
  outp[1] = drym;
  outp[2] = wetb;
  outp[3] = dryb;
  outp[4] = 1-wetm/(wetm+wetb);
  outp[5] = 1-drym/(drym+dryb);
  outp[6] = 1/(wetm+wetb);
  outp[7] = 1/(drym+dryb);
  outp[8] = K_w;
  outp[9] = K_d;

  return outp;
}
