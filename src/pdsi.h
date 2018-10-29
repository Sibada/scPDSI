#ifndef PDSI_H
#define PDSI_H

#include <Rcpp.h>
#include <R.h>

using namespace Rcpp;

// This defines the type number as a double.  This is used to easily change
// the PDSI's variable types.
typedef double number;
typedef int flag;
#define min(a,b) ((a) < (b) ? (a) : (b));
#define MISSING -999.00

//-----------------------------------------------------------------------------
//**********   START OF STRUCTURE DEFINITION FOR STRUCT:  node        *********
//-----------------------------------------------------------------------------
// The node struct is used specifically in the linked list class and is not
// relevant to the actual PDSI.
//-----------------------------------------------------------------------------
struct node {            // A structure for a node
public:
  number key;            // Where the data is stored
  struct node *next;     // Where the next node is
  struct node *previous; // Where the previous node is
};

//-----------------------------------------------------------------------------
//**********   CLOSE OF STRUCTURE DEFINITION FOR STRUCT:  node        *********
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//**********   START OF CLASS DEFINITIONS FOR THE CLASS:  llist       *********
//-----------------------------------------------------------------------------
// The llist class is a dynamic storage class.  It is used in the PDSI to
// eliminate problems with filling static arrays.
//-----------------------------------------------------------------------------
class llist {            // A linked list class
private:
  node *head;            // A pointer to the head of the linked-list
  int size;
  number kthLargest(int k);  //returns the kth largest number in the list
  number long_select(int k); //returns the kth largest number in the list
  //without using an array or sorting.  Used when
  //there is not enough memory to place the entire
  //list in a new array.
  number percentile(double percentage); //returns the specified percentile

public:
  llist();               // The constructor
  ~llist();              // The destructor
  // The insert function takes an argument of type number and places it on
  // the head of the linked list.
  void insert(number x);
  // The remove functions remove from either the head (head_remove) or the
  // tail (tail_remove) of the linked list.
  number head_remove();  // remove the first node and returns its key
  number tail_remove();  // remove the last node and returns its key
  // These are other useful functions used in dealing with linked lists
  int is_empty();// Returns 1 if the llist is empty 0 otherwise
  int get_size();
  number sumlist();  // Sums the items in list
  void sumlist(number &prev_sum, int sign);//sums items in list into prev_sum
  number maxlist();
  number minlist();
  number quartile(int q);        //returns the qth quartile
  number safe_percentile(double percentage); //safe version

  number* returnArray();

  // The set_node function sets the key of the node pointed to by set.  It
  // checks the linked list to make sure set is in the node to prevent
  // runtime errors from occuring.  It was written specifically for the PDSI
  // program and its backtracking function.
  node *set_node(node *set=NULL, number x=0);
  friend void copy(llist &L1,const llist &L2); // Copies L2 to L1
};
//-----------------------------------------------------------------------------
//**********   CLOSE OF CLASS DEFINITIONS FOR THE CLASS:  llist       *********
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//**********   START OF CLASS DEFINITIONS FOR THE CLASS:  pdsi        *********
//-----------------------------------------------------------------------------
// The pdsi class contains and deals with all necessary pdsi variables and
// calculations.  The paramater values and starting/ending years are set upon
// creation, but the actual pdsi values are not calculated until Calcpdsi is
// called.
//-----------------------------------------------------------------------------
//
class pdsi {
public:
  // The pdsi constructor takes in an array of arguments (argv[]) and an
  // integer number of arguments (argc).  These arguments should contain
  // the various flags desired to custimize the performance of the pdsi
  pdsi();
  ~pdsi();

  // The set_flags function takes in an array of flags/arguments (flags[])
  // and an integer number of flags/arguments (num_flags).  It then sets
  // the various pdsi options accordingly.  If incorrect or invalid flags
  // are input, the set_flags function terminates the program.
  void set_flags(int num_flags, char *flags[]);

  // These functions are work horses of the pdsi class.  They call all
  // functions necessary to calculate all aspects of the pdsi from the
  // temperature and precipitation files.
  // WeeklyPDSI() calculates a self-calibrating weekly PDSI
  // MonthlyPDSI() calculates the original PDSI
  // SCMonthlyPDSI() calculates a self-calibrating monthly PDSI
  void WeeklyPDSI();
  void WeeklyPDSI(int length);
  void MonthlyPDSI();
  void SCMonthlyPDSI();
  void WeeklyCMI();
  // The write function creates output files based on the flags sent to the
  // constructor (possible need a set_flags function).
  void Write();
  void Write(char* dir);

  number getPDSI(int period, int year);
  number getZIND(int period, int year);
  number getPHDI(int period, int year);
  number getWPLM(int period, int year);
  number getCMI(int period, int year);

  number* getPDSIArray(int &size);
  number* getPDSIArray(int start_per, int start_yr,
                       int end_per, int end_yr, int &size);
  number* getZINDArray(int &size);
  number* getZINDArray(int start_per, int start_yr,
                       int end_per, int end_yr, int &size);
  number* getWPLMArray(int &size);
  number* getWPLMArray(int start_per, int start_yr,
                       int end_per, int end_yr, int &size);
  number* getPHDIArray(int &size);
  number* getPHDIArray(int start_per, int start_yr,
                       int end_per, int end_yr, int &size);

  number* getCMIArray(int &size);
  number* getCMIArray(int start_per, int start_yr,
                      int end_per, int end_yr, int &size);

  number* getYearArray(int &size);
  number* getPerArray(int &size);

  /* Added functions and variables for run in R */

  /* Added fields for run in R */
  NumericVector P_vec;
  NumericVector PE_vec;

  NumericMatrix vals_mat;
  NumericMatrix coefs_mat;

  //NumericVector d_vec;
  //NumericVector Z_vec;

  number K_w;
  number K_d;

  number coe_K1_1;
  number coe_K1_2;
  number coe_K1_3;
  number coe_K2;

  number coe_p;
  number coe_q;
  number coe_m;
  number coe_b;

  void Rext_init(NumericVector& P, NumericVector& PE,
  	             number AWC,
                 int s_yr, int e_yr,
                 int calib_s_yr, int calib_e_yr);
  void Rext_set_parcoefs(number K1_1, number K1_2, number K1_3, number K2,
  	number p, number q);

  void Rext_PDSI_mon(bool SC);

  void Rext_get_Rvec(NumericVector R_vec, int year, number* A, int freq);

  void Rext_output_X();

  NumericVector Rext_out_params();

private:
  //these variables keep track of what type of PDSI is being calculated.
  bool Weekly;
  bool Monthly;
  bool SCMonthly;

  // The variables for storing the starting year of calculation and the
  // total number of years to calculate
  int startyear;
  int endyear;
  int totalyears;

  int period_length;        //set to 1 for monthly, otherwise, legth of period
  int num_of_periods;       //number of periods of period_length in a year.

  // The variables used as flags to the pdsi class
  flag bug;
  flag output_mode;
  flag verbose;
  flag s_year;
  flag e_year;
  flag extra;
  flag metric;
  flag south;
  flag nadss;

  /* added on 9/21/05 to allow for user-define calibration start year (jdokulil) */
  flag setCalibrationStartYear;
  flag setCalibrationEndYear;
  int calibrationStartYear;
  int calibrationEndYear;
  /* end addition */

  /* SG: Steve Goddard modifications */
  /* SG 6/5/06: add variables to allow user-defined calibration intervals */
  int currentCalibrationStartYear;
  int currentCalibrationEndYear;
  int nStartYearsToSkip;
  int nEndYearsToSkip;
  int nCalibrationYears;
  int nStartPeriodsToSkip;
  int nEndPeriodsToSkip;
  int nCalibrationPeriods;
  /* SG 6/5/06: End adding variables to allow user-defined calibration intervals */


  // Various constants used in calculations
  number TLA; // The negative tangent of latitude is used in calculating PE
  number AWC; // The soils water capacity
  number I;   // Thornthwaites heat index
  number A;   // Thornthwaites exponent
  number tolerance; // The tolerance for various comparisons

  // The arrays used to read in the normal temp data, a years worth of
  // actual temp data, and a years worth of precipitation data
  number TNorm[52];
  number T[52];
  number P[52];

  // These variables are used in calculation to store the current period's
  // potential and actual water balance variables as well as the soil
  // moisture levels
  number ET;            // Actual evapotranspiration
  number R;             // Actual soil recharge
  number L;             // Actual loss
  number RO;            // Actual runoff
  number PE;            // Potential evapotranspiration
  number PR;            // Potential soil recharge
  number PL;            // Potential Loss
  number PRO;           // Potential runoff
  number Su;            // Underlying soil moisture
  number Ss;            // Surface soil moisture

  // These arrays are used to store the monthly or weekly sums of the 8 key
  // water balance variables and the precipitation
  number ETSum[52];
  number RSum[52];
  number LSum[52];
  number ROSum[52];
  number PESum[52];
  number PRSum[52];
  number PLSum[52];
  number PROSum[52];
  number PSum[52];

  // These arrays store the monthly or weekly water balance coefficeints
  number Alpha[52];
  number Beta[52];
  number Gamma[52];
  number Delta[52];

  // The CAFEC percipitation
  number Phat;

  // These variables are used in calculating the z index
  number d;     // Departure from normal for a period
  number D[52]; // Sum of the absolute value of all d values by period
  number k[52]; // Palmer's k' constant by period
  number K;     // The final K value for a period
  number Z;     // The z index for a period (Z=K*d)

  // These variables are used in calculating the PDSI from the Z
  // index.  They determine how much of an effect the z value has on
  // the PDSI based on the climate of the region.
  // They are calculated using CalcDurFact()
  number drym;
  number dryb;
  number wetm;
  number wetb;

  //these two variables weight the climate characteristic in the
  //calibration process
  number dry_ratio;
  number wet_ratio;

  // The X variables are used in book keeping for the computation of
  // the pdsi
  number X1;    // Wet index for a month/week
  number X2;    // Dry index for a month/week
  number X3;    // Index for an established wet or dry spell
  number X;     // Current period's pdsi value before backtracking

  // These variables are used in calculating the probability of a wet
  // or dry spell ending
  number Prob;  // Prob=V/Q*100
  number V;     // Sumation of effective wetness/dryness
  number Q;     // Z needed for an end plus last period's V

  // These variables are statistical variables computed and output in
  // verbose mode
  number DSSqr[52];
  number DEPSum[52];
  number DKSum;
  number SD;
  number SD2;

  // linked lists to store X values for backtracking when computing X
  llist Xlist;//final list of PDSI values
  llist altX1;//list of X1 values
  llist altX2;//list of X2 values

  // These linked lists store the Z, Prob, and 3 X values for
  // outputing the Z index, Hydro Palmer, and Weighted Palmer
  llist XL1;
  llist XL2;
  llist XL3;
  llist ProbL;
  llist ZIND;
  llist PeriodList;
  llist YearList;

  // This linked list stores the CMI;
  llist CMIList;

  //the directory path to the directory containing the input files
  char input_dir[128];
  char output_dir[128];

  // Class Functions

  // These funcitons initialize some variables and linked lists
  // and check to make sure all the necessary data is there.
  int initialize();
  int check_input(FILE *in);

  // These functions read in the input files
  int GetTemp(FILE * In, number *A, int max); // Gets the Temp
  int GetPrecip(FILE *In, number *A, int max); // Gets the Precip
  void GetParam(FILE * Param);          // Gets Paramaters Su and TLA

  // These functions calculate the Potentials needed
  // Calculates Potential Evapotranspiration from Thornthwaite
  void CalcWkPE(int period, int year);
  void CalcMonPE(int month, int year);
  void CalcPR();// Calculates Potential Recharge
  void CalcPL();// Calculates Potential Loss
  void CalcPRO();// Calculates Potential Runoff

  // This function calculates the actual values from the potentials
  void CalcActual(int per);// Calculates Actual values

  // These functions do calculations on a yearly basis
  void SumAll(); // Creates sums of Actual and Potential values
  void Calcd();     // Finds the values for d, phat and then finds D
  void CalcK();     // Calculates K
  void CalcOrigK();  // This function does it all. It computes K, the Z-index
  // and the X values.  Used for uncalibrated PDSI.
  void CalcZ();     // Calculates the Z-index
  void CalcX();     // Calculates the PDSI and X1, X2, and X3
  //void CalcOneX(FILE* table, int period_number, int year);
  void CalcOneX(int period_number, int year);
  //calculates the PDSI and X1, X2, and X3 for one period.
  void Calibrate(); // Calibrates the PDSI

  void CalcCMI();
  void WriteCMI(char* dir);
  void MoveCMIFiles(char* dir);


  // This function backtracks through the X values
  // and replaces them with the appropriate value of X1 or X2
  // when necessary
  void Backtrack(number X1, number X2);
  void ChooseX(number& newX, number& newX1, number& newX2,
               number& newX3, int bug);

  // These functions calculate Thornthwaite coefficients
  number CalcWkThornI();// Calculates Thornthwaite Heat Index
  number CalcMonThornI();
  number CalcThornA(number I);// Calculates Thornthwaite Exponent

  // This function calculates the coefficients for the Water Balance Equation
  void CalcWBCoef();

  // This function calculates the constants used in calculating the PDSI value
  // from the Z index.  These constants affect how much influence the Z index
  // has on the PDSI value.
  void CalcDurFact(number &slope, number &intercept, int sign);
  number get_Z_sum(int length, int sign);
  void LeastSquares(int *x, number *y, int n, int sign, number &slope, number &intercept);

  // This function writes the PDSI values
  void FinalWrite(char* dir);

  // This function will take care of any extra files created by either
  // renaming them or moving them to the appropriate directory
  void MoveFiles(char *dir);
  int ReprintFile(char* src, char *des);

  number getValue(llist &List, int period, int year);

  number* getSubArray(llist &List, int start_per, int start_yr,
                      int end_per, int end_yr, int &size);

  // These are simple functions to determine if the characters of a string
  // form an integer or a floating point number.
  inline int is_int(char *string,int length);
  inline int is_flt(char *string,int length);
};
//-----------------------------------------------------------------------------
//**********   CLOSE OF CLASS DEFINITIONS FOR THE CLASS:  pdsi        *********
//-----------------------------------------------------------------------------
//
//The dir_exists function is a function used to test to see if a directory
//exists.
//The create_dir function will create directories recursively until the path
//given as an argument exists.  Example:  create_dir("dir/sub1/sub2/")
//will create three directories: 'dir', 'sub1', and 'sub2'.
//-----------------------------------------------------------------------------
int dir_exists(char *dir);
int create_dir(char *path);
//-----------------------------------------------------------------------------
//These three functions, partition(), select(), and exch() are used to select
//the kth largest number in an array.
//-----------------------------------------------------------------------------
void select(number a[], int l, int r, int k);
int partition(number a[], int left, int right);
void exch(number &x, number &y);
//-----------------------------------------------------------------------------
//The numEntries() function returns the number of entries in the file
//It is used to check the T_normal file for the correct format because the
//program allows that filename to be used in place of either wk_T_normal
//or mon_T_normal.
int numEntries(FILE *in);
#endif
