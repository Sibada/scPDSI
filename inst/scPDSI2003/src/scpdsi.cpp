// #include <stdlib.h>
// #include <math.h>
#include <cstring>
// #include <stdio.h>
// #include <ctype.h>
#include "pdsi.h"

// This defines the type number as a double.  This is used to easily change
// the PDSI's variable types.
typedef double number;
typedef int flag;
#define min(a,b) ((a) < (b) ? (a) : (b));
#define MISSING -99.00

//-----------------------------------------------------------------------------
//**********                       MAIN PROGRAM                       *********
//-----------------------------------------------------------------------------
// The main program takes in command line arguments and passes them to the
// constructor of a pdsi variable tester.  It then calls Calcpdsi to calculate
// the pdsi values.  Finally it calls write to output these values to file.
//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    pdsi PDSI;
    PDSI.set_flags(argc, argv); // Sets the flags of PDSI
    PDSI.WeeklyPDSI();         // Calculates the weekly pdsi values for PDSI
    PDSI.Write("weekly/1");
    PDSI.WeeklyPDSI(2);
    PDSI.Write("weekly/2");
    PDSI.WeeklyPDSI(4);
    PDSI.Write("weekly/4");
    PDSI.WeeklyPDSI(13);
    PDSI.Write("weekly/13");
    PDSI.MonthlyPDSI();
    PDSI.Write("monthly/original");
    PDSI.SCMonthlyPDSI();
    PDSI.Write("monthly/self_cal");
    PDSI.WeeklyCMI();
    PDSI.Write("weekly/CMI");
    return 0;
}
//-----------------------------------------------------------------------------
//**********                       PROGRAMS END                       *********
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//**********   START OF FUNCTION DEFINITIONS FOR CLASS:  llist        *********
//-----------------------------------------------------------------------------
// The pdsi constructor sets all flags to default, scans the temperature file
// to get the starting and ending years of the data, and reads in the values
// from the parameter file.
//-----------------------------------------------------------------------------
pdsi::pdsi() {
    strcpy(input_dir, "./");
    strcpy(output_dir, "./");

    //set several parameters to their defaults
    period_length = 1;
    num_of_periods = 52;
    verbose = 1;
    bug = 0;
    output_mode = 0;
    tolerance = 0.00001;
    metric = 0;
    nadss = 0;
    setCalibrationStartYear = 0;
    setCalibrationEndYear = 0;
}
//-----------------------------------------------------------------------------
// The destructor deletes the temporary files used in storing various items
//-----------------------------------------------------------------------------
pdsi::~pdsi() {
    if (extra != 3 && extra != 9) {
        remove("potentials");  // Used for storing potential values for later use
        remove("dvalue");  // Used to store the d values for use after summing them
    }
    remove("bigTable.tbl");
    if (verbose > 0)
        printf("Calculations Complete\n");
}
//-----------------------------------------------------------------------------
// The check_input function will check to make sure there is enough data in
// file represented by the file pointer in.  The basic requirement is that
// there should be 25 years of data, which means 300 months or 1300 weeks that
// are not MISSING.
// returns -1 if the file doesn't exist (in == NULL)
// returns 0 if the file doesn't have enough data
// reutrns 1 if the file is okay
//-----------------------------------------------------------------------------
int pdsi::check_input(FILE *in) {
    float temp;
    int count = 0;
    int min_years = 25;
    if (in == NULL)
        return -1;
    while (fscanf(in, "%f", &temp) != EOF) {
        //if temp is either MISSING, or a year, don't count it.
        if (temp != MISSING && temp <= 999)
            count++;
    }

    if (Weekly) {
        if (count < (min_years * 52) )
            return 0;
        else
            return 1;
    }
    else {
        if (count < (min_years * 12) )
            return 0;
        else
            return 1;
    }
}
//-----------------------------------------------------------------------------
// The initialize function clears all arrays and linked lists.
// it also checks to make sure all the necessary input files exist.
// returns 1 if they do, and -1 if they do not.
// returns 0 if there is too much missing data.
//  -- there should be 25 years worth of data. (300 months or 1300 weeks)
//-----------------------------------------------------------------------------
int pdsi::initialize() {
    char filename[170], base[170];
    FILE* test;
    while (!Xlist.is_empty())
        Xlist.tail_remove();
    while (!altX1.is_empty())
        altX1.tail_remove();
    while (!altX2.is_empty())
        altX2.tail_remove();
    while (!XL1.is_empty())
        XL1.tail_remove();
    while (!XL2.is_empty())
        XL2.tail_remove();
    while (!XL3.is_empty())
        XL3.tail_remove();
    while (!ProbL.is_empty())
        ProbL.tail_remove();
    while (!ZIND.is_empty())
        ZIND.tail_remove();
    while (!PeriodList.is_empty())
        PeriodList.tail_remove();
    while (!YearList.is_empty())
        YearList.tail_remove();
    while (!CMIList.is_empty())
        CMIList.tail_remove();
    //check to make sure the necessary input files exist
    if (strlen(input_dir) > 1)
        strcpy(base, input_dir);
    else
        strcpy(base, "./");
    if (Weekly) {
        strcpy(filename, base);
        strcat(filename, "wk_T_normal");
        if ((test = fopen(filename, "r")) == NULL) {
            sprintf(filename, "%s%s", base, "T_normal");
            if ((test = fopen(filename, "r")) == NULL)
                return -1;
            else if (numEntries(test) != 52) {
                fclose(test);
                if (verbose) {
                    printf("Error: T_normal not in correct format ");
                    printf("for Weekly calculations\n");
                }
                return -1;
            }
        }
        fclose(test);
        strcpy(filename, base);
        strcat(filename, "parameter");
        if ((test = fopen(filename, "r")) == NULL)
            return -1;
        fclose(test);
        strcpy(filename, base);
        strcat(filename, "weekly_P");
        if ((test = fopen(filename, "r")) == NULL)
            return -1;
        else {
            switch (check_input(test)) {
            case -1:
                fclose(test);
                return -1;
                break;
            case 0:
                if (verbose > 0)
                    printf("Error: too much missing data.\n");
                if (verbose > 1)
                    printf("       1300 weeks (25 yrs) of good data.\n");
                fclose(test);
                return 0;
                break;
            }
            fclose(test);
        }
        strcpy(filename, base);
        strcat(filename, "weekly_T");
        if ((test = fopen(filename, "r")) == NULL)
            return -1;
        else {
            switch (check_input(test)) {
            case -1:
                fclose(test);
                return -1;
                break;
            case 0:
                if (verbose > 0)
                    printf("Error: too much missing data.\n");
                if (verbose > 1)
                    printf("       1300 weeks (25 yrs) of good data.\n");
                fclose(test);
                return 0;
                break;
            }
            fclose(test);
        }
    }
    else if (Monthly || SCMonthly) {
        strcpy(filename, base);
        strcat(filename, "mon_T_normal");
        if ((test = fopen(filename, "r")) == NULL) {
            sprintf(filename, "%s%s", base, "T_normal");
            if ((test = fopen(filename, "r")) == NULL)
                return -1;
            else if (numEntries(test) != 12) {
                if (verbose) {
                    printf("Error: T_normal not in correct format ");
                    printf("for Monthly calculations\n");
                }
                fclose(test);
                return -1;
            }
        }
        fclose(test);
        strcpy(filename, base);
        strcat(filename, "parameter");
        if ((test = fopen(filename, "r")) == NULL)
            return -1;
        fclose(test);
        strcpy(filename, base);
        strcat(filename, "monthly_P");
        if ((test = fopen(filename, "r")) == NULL)
            return -1;
        else {
            switch (check_input(test)) {
            case -1:
                fclose(test);
                return -1;
                break;
            case 0:
                if (verbose > 0)
                    printf("Error: too much missing precip data.\n");
                if (verbose > 1)
                    printf("       300 months (25 yrs) of good data.\n");
                fclose(test);
                return 0;
                break;
            }
            fclose(test);
        }
        strcpy(filename, base);
        strcat(filename, "monthly_T");
        if ((test = fopen(filename, "r")) == NULL)
            return -1;
        else {
            switch (check_input(test)) {
            case -1:
                fclose(test);
                return -1;
                break;
            case 0:
                if (verbose > 0)
                    printf("Error: too much missing temp data.\n");
                if (verbose > 1)
                    printf("       300 months (25 yrs) of good data.\n");
                fclose(test);
                return 0;
                break;
            }
            fclose(test);
        }
    }
    else {
        if (verbose) {
            printf("Error.  Invalid type of PDSI calculation\n");
            printf("Either the 'Weekly', 'Monthly', or 'SCMonthly' ");
            printf("flags must be set.\n");
        }
        exit(1);
    }
    return 1;
}
//-----------------------------------------------------------------------------
// The set_flags function takes an argument list (flags) and a number of
// arguments (num_flags).  It then sets the various pdsi flags accordingly
//-----------------------------------------------------------------------------
void pdsi::set_flags(int num_flags, char *flags[]) {
    int read_from = 1;
    unsigned int n;

    // These flags are used for two argument flags
    flag week = 0, year = 0, both = 0, out = 0, e_flag = 0, t_year = 0;
    flag in_dir = -1, out_dir = -1;
    // Initializes the output years flags to 0
    s_year = 0;
    e_year = 0;

    // This loop checks all strings in flags to make sure they are valid flags
    for (int i = read_from; i < num_flags; i++) {
        // This checks for the various single letter flags that are started with
        // the '-' character.  Note: multiply flags can be specified after the -
        if (strncmp(flags[i], "-", 1) == 0) {
            for (unsigned int j = 1; j < strlen(flags[i]); j++) {
                switch (flags[i][j]) {
                case 'm':
                    // A m flags means all input in metric units
                    metric = 1;
                    break;

                case 'v':
                    // A v flag sets the verbosity to maximum
                    verbose = 2;
                    break;

                case 's':
                    // A s flag sets the verbosity to minimum
                    verbose = 0;
                    break;

                case 'e':
                    // An e flag shows that totalyears should be counted from the
                    // e_year flag or that a given year is meant to be the e_year
                    e_flag = 1;
                    break;

                case 'b':
                    // A b flag causes the program to reproduce an error in the
                    // FORTRAN code for easy comparison
                    bug = 1;
                    break;

                /* added on 9/21/05 to allow for user-define calibration start year (jdokulil) */
                case 'c':
                    j++;
                    if (flags[i][j] == 's') {
                        setCalibrationStartYear = 1;
                        i++;
                        if (is_int(flags[i], strlen(flags[i]))) {
                            calibrationStartYear = atoi(flags[i]);
                        } else {
                            printf("INVALID CALIBRATION START YEAR: %s, must be of the form: yyyy (Ex: 1956)\n", flags[i]);
                            printf("pdsi --help for usage.\n");
                            exit(1);
                        }
                        calibrationStartYear = atoi(flags[i]);
                    } else if (flags[i][j] == 'e') {
                        setCalibrationEndYear = 1;
                        i++;
                        if (is_int(flags[i], strlen(flags[i]))) {
                            calibrationEndYear = atoi(flags[i]);
                        } else {
                            printf("INVALID CALIBRATION END YEAR: %s, must be of the form: yyyy (Ex: 2005)\n", flags[i]);
                            printf("pdsi --help for usage.\n");
                            exit(1);
                        }
                    } else {
                        printf("INVALID FLAG: %s\n", flags[i]);
                        printf("pdsi --help for usage.\n");
                        exit(1);
                    }
                    j = strlen(flags[i]);
                    break;
                /* end addition */

                case 'f':
                    // An o flag is used to set the output format of the pdsi
                    out = 1;
                    break;

                case 'x':
                    // An x flag is used to output extra data, such as the water
                    // balance coefficients or PE
                    extra = 0;
                    break;

                case 'i':
                    // A i flag is used to signify the directory location of
                    // the input data
                    // the flag must be in the form of: -iInputDir/sub/
                    // so the rest of the string flags[i] is the
                    // directory
                    for (n = j + 1; n < strlen(flags[i]); n++)
                        input_dir[n - (j + 1)] = flags[i][n];
                    input_dir[n - 2] = '\0';
                    j = n;
                    break;

                case 'o':
                    // A o flag is used to signify the directory location where
                    // the output files should be placed.
                    // the flag must be in the form of: -oOnputDir/sub/
                    // so the rest of the string flags[i] is the
                    // directory
                    for (n = j + 1; n < strlen(flags[i]); n++)
                        output_dir[n - (j + 1)] = flags[i][n];
                    output_dir[n - 2] = '\0';
                    j = n;
                    break;

                case 'n':
                    // this is the nadss flag.  Any options that are
                    // unique to our implemenation should be controled
                    // but it.  Currently, this only affects the way
                    // the parameter file is read.
                    nadss = 1;
                    break;

                case '-':
                    // A string starting with '--' is a special flag such as
                    // the help flag.
                    if (strcmp(flags[i], "--help") == 0) {
                        printf("pdsi <flags> <Total Years>\n");
                        printf("  -b  Causes pdsi to reproduce the bug found in the FORTRAN program\n");
                        printf("  -v  Verbose mode outputs all text to the screen that the FORTRAN program did\n");
                        printf("  -e  Specifies the given year is an end year and not a beginning year\n");
                        printf("  -cs <yyyy> Specifies the calibration start year\n");
                        printf("  -ce <yyyy> Specifies the calibration end year\n");
                        printf("  -s  Silent mode turns off all error outputs\n");
                        printf("  -f  [table|column|both]  Sets the output mode used.\n");
                        printf("      table - Create only files in a table format\n");
                        printf("      column - Create only files in a column format\n");
                        printf("      both - Create both table and column files\n");
                        printf("  -x  [wb|Xtable|potentials|all]  generates extra output files.\n");
                        printf("      wb - Outputs water balance coefficients to \"WB.tbl\" and to the screen.\n");
                        printf("      Xtable - Outputs Year, Month/Week, Z, Prob(end), X1, X2, and X3 to\n        \"bigTable.tbl\".\n");
                        printf("      potentials - Outputs P, PE, PR, PRO, PL and P-PE to \"potentials\".\n");
                        printf("      all - Outputs to \"WB.tbl\", \"bigTable.tbl\" and \"potentials\".\n");
                        printf("\n");
                        exit(1);
                    }
                    else {
                        printf("INVALID FLAG: %s.\n", flags[i]);
                        printf("pdsi --help for usage.\n");
                        exit(1);
                    }
                    break;

                default:
                    // If it is none of the above then the flag is invalid.
                    printf("INVALID FLAG: %s.\n", flags[i]);
                    printf("pdsi --help for usage.\n");
                    exit(1);
                    break;
                }// End switch (argv[i][j])
            }// End for(int j=1; j<strlen(argv[i]); j++)
        }// End if(strncmp(argv[i],"-",1)==0)
        else if (is_int(flags[i], strlen(flags[i]))) {
            // If the argument is an integer it is either a starting/ending year
            // specifier or a total year specifier for the output function. It is
            // also possible for the argument to be the tolerance specifier of 0.
            int temp;
            temp = atoi(flags[i]);
            if (temp > 9999) {
                // If the number is more than 4 digits it is invalid
                printf("INVALID YEAR SPECIFIER: %d", temp);
                printf("pdsi --help for usage.\n");
                exit(1);
            }// End if(temp>9999 || temp<0)
            else if (temp > 999) {
                // Otherwise if it is 4 digits then it must be a year specifier
                if (s_year == 0) {
                    // If this is the first year specifier store it in s_year
                    s_year = temp;
                }// End if(s_year==0)
                else if (s_year > 0 && e_year == 0) {
                    // If this is the second year specifier decide which should be start
                    // year and which should be end year.
                    if (temp < s_year) {
                        e_year = s_year;
                        s_year = temp;
                    }// End if(temp<s_year)
                    else
                        e_year = temp;
                }// End else if(s_year>0&&e_year==0)
                else {
                    // If two year specifiers have already been given then there must be
                    // an invalid flag.
                    printf("Confused by $d.  Start year and end year already specified.\n", temp);
                    printf("pdsi --help for usage.\n");
                    exit(1);
                }// End else
            }// End else if(temp > 999)
            else if (temp > 0) {
                // If temp is not a year and it is not zero then it must be a total
                // specifier.
                t_year = temp;
            }// End else if(temp>0)
            else if (temp == 0)
                // Otherwise it is a tolerance specifier of 0
                tolerance = 0;
        }// End else if(is_int(argv[i],strlen(argv[i])))
        else if (is_flt(flags[i], strlen(flags[i]))) {
            // If the flag is a floating point number then it is probably meant as a
            // tolerance specifier.
            float tol;
            tol = float(atof(flags[i]));
            // A tolerance of much more than 0.005 will have drastic results on the
            // PDSI and is not a very likely want
            if (tol < 1)
                tolerance = tol;
            else {
                printf("Unsure of %f.  Tolerance specifier should be < 1.\n", tol);
                printf("pdsi --help for usage.\n");
                exit(1);
            }
        }//end of else if(is_flt...)

        // If the flag is year then it is an output mode specifier
        else if (strcmp(flags[i], "table") == 0) {
            // This checks that there have been no other output mode specifiers
            if (!week && !both && out == 1)
                year = 1; // Okay to set the output mode to year
            else if (out == 0) {
                printf("Confused by \"table\".  No -f flag detected.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
            else {
                printf("Confused by \"table\".  Output mode already specified.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }

        }
        // If the flag is week then it is an output mode specifier
        else if (strcmp(flags[i], "column") == 0) {
            // This checks that there have been no other output mode specifiers
            if (!year && !both && out == 1)
                week = 1; // Okay to set the ouput mode to week
            else if (out == 0) {
                printf("Confused by \"column\".  No -f flag detected.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
            else {
                printf("Confused by \"column\".  Output mode already specified.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
        }
        // If the flag is year then it is an output mode specifier
        else if (strcmp(flags[i], "both") == 0) {
            // This checks that there have been no other output mode specifiers
            if (!year && !week && out == 1)
                both = 1; // Okay to set the ouput mode to both
            else if (out == 0) {
                printf("Confused by \"both\".  No -f flag detected.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
            else {
                printf("Confused by \"both\".  Output mode already specified.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
        }

        //check for extra output....  wb, Xtable, potentials, or all
        else if (strcmp(flags[i], "wb") == 0) {
            if (extra == 0)
                extra = 1; //this will output the water balance coefficients to a file.
            else if (extra < 0) {
                printf("Confused by \"wb\".  No -x flag detected.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
            else {
                printf("Confused by \"wb\".  Extra output already set.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }

        }
        else if (strcmp(flags[i], "Xtable") == 0) {
            if (extra == 0) {
                //this will output Z, Prob, X1, X2, X3 and PDSI in a table.
                extra = 2;
            }
            else if (extra < 0) {
                printf("Confused by \"Xtable\".  No -x flag detected.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
            else {
                printf("Confused by \"Xtable\".  Extra output already set.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
        }
        else if (strcmp(flags[i], "potentials") == 0) {
            if (extra == 0)
                extra = 3;   //don't delete the potentials file in destructor
            else if (extra < 0) {
                printf("Confused by \"potentials\".  No -x flag detected.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
            else {
                printf("Confused by \"potentials\".  Extra output already set.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
        }
        else if (strcmp(flags[i], "all") == 0) {
            if (extra == 0) {
                extra = 9;   //don't delete the potentials file, and write
                //to both bigTable.tbl and WB.tbl
            }
            else if (extra < 0) {
                printf("Confused by \"all\".  No -x flag detected.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
            else {
                printf("Confused by \"all\".  Extra output already set.\n");
                printf("pdsi --help for usage.\n");
                exit(1);
            }
        }
    }// End for(int i=1; i<argc; i++) -->done checking all flags

    // check input_dir and output_dir for correct format
    //make sure last character is a slash '/'
    //and make sure the input directory exists
    if (strlen(input_dir) > 1) {
        if (input_dir[strlen(input_dir) - 1] != '/')
            strcat(input_dir, "/");
        if (dir_exists(input_dir) == -1) {
            if (verbose)
                printf("Input directory does not exist: %s\n\n", input_dir);
            exit(1);
        }
    }
    else
        strcpy(input_dir, "./");

    if (strlen(output_dir) > 1) {
        if (output_dir[strlen(output_dir) - 1] != '/')
            strcat(output_dir, "/");
    }
    else
        strcpy(output_dir, "./");

    if (verbose > 1) {
        printf("Input Directory is:  %s\n", input_dir);
        printf("Output Directory is: %s\n\n", output_dir);
    }

    // This block opens weekly_T to scan for the start and end year of the data
    // If weekly_T is not present, tries to open monthly_T
    float t;
    FILE * scn;
    char filename[170];
    if (strlen(input_dir) > 1) {
        strcpy(filename, input_dir);
        strcat(filename, "weekly_T");
    }
    else
        strcpy(filename, "weekly_T");

    if ((scn = fopen(filename, "r")) == NULL) {
        if (verbose > 1)
            printf("Error opening %s... trying for %smonthly_T\n", filename, input_dir);
        if (strlen(input_dir) > 1) {
            strcpy(filename, input_dir);
            strcat(filename, "monthly_T");
        }
        else
            strcpy(filename, "monthly_T");

        strcpy(filename, input_dir);
        strcat(filename, "monthly_T");
        if ((scn = fopen(filename, "r")) == NULL) {
            printf("error opening file to get startyear\n");
            printf("tried both %sweekly_T and %s\n", input_dir, filename);
            exit(1);
        }
        else {
            //scan monthly_T for start and end year
            fscanf(scn, "%d", &startyear);
            while ((fscanf(scn, "%f", &t)) != EOF) {
                if (t > 999)
                    endyear = (int)t;
            }
        }
    }
    else {
        //scan weekly_T for start and end year
        fscanf(scn, "%d", &startyear);
        while ((fscanf(scn, "%f", &t)) != EOF) {
            if (t > 999)
                endyear = (int)t;
        }
    }
    fclose(scn);
    totalyears = endyear - startyear + 1;

    if (t_year > totalyears) {
        printf("Span given is greater than span of input data.\n");
        printf("pdsi --help for usage.\n");
        exit(1);
    }


    //check to see if s_year is a valid year according to data
    if (s_year != 0 && (s_year < startyear || s_year > endyear)) {
        printf("The data files only contain data from %d to %d\n", startyear, endyear);
        printf("%d falls outside that range.\n", s_year);
        exit(1);
    }
    //check to see if e_year is a valid year according to data
    if (e_year != 0 && (e_year < startyear || e_year > endyear)) {
        printf("The data files only contain data from %d to %d\n", startyear, endyear);
        printf("%d falls outside that range.\n", e_year);
        exit(1);
    }
    // This checks to see if calculations should be done from end year
    if (e_flag) {
        // If they are, then set e_year to the specified year
        if (s_year == 0) e_year = endyear;
        else if (e_year == 0) e_year = s_year;
        // Set s_year to the start year
        s_year = startyear;
        // If t_year has been specified and keeps the output within the range of
        // the input then reset s_year accordingly
        if (t_year > 0 && (e_year - t_year + 1) > startyear) {
            s_year = e_year - t_year + 1;
        }
    }
    else {
        // Otherwise calculations are done from s_year.
        if (s_year == 0) s_year = startyear;
        if (e_year == 0) e_year = endyear;
        if (t_year > 0 && (s_year + t_year - 1) < endyear) {
            e_year = s_year + t_year - 1;
        }
    }


    /* SG 6/5/06: set user-defined calibration interval variables */
    /* based on code by jdokulil to allow for user-define calibration start and end years */
    if (setCalibrationStartYear == 1) {
        if (calibrationStartYear < startyear || calibrationStartYear > endyear) {
            printf("Warning: invalid calibration start year: %d.\n", calibrationStartYear);
            calibrationStartYear = startyear;
        }
    } else {  /* no calibration start year set, so set it to startyear */
        calibrationStartYear = startyear;
    }
    /* currentCalibrationStartYear is reset for various PDSI calculations (e.g., weekly,
    ** monthly, sc-monthly, CMI, etc.  It is initialized here and reset for each type
    ** of PDSI calculation, where it is used to control some loops.
    */
    currentCalibrationStartYear = calibrationStartYear;

    if (setCalibrationEndYear == 1) {
        if (calibrationEndYear < calibrationStartYear || calibrationEndYear > endyear) {
            printf("Warning: invalid calibration end year: %d.\n", calibrationEndYear);
            calibrationEndYear = endyear;
        }
    } else {  /* no calibration end year set, so set it to startyear */
        calibrationEndYear = endyear;
    }
    /* currentCalibrationEndYear is reset for various PDSI calculations (e.g., weekly,
    ** monthly, sc-monthly, CMI, etc.  It is initialized here and reset for each type
    ** of PDSI calculation, where it is used to control some loops.
    */
    currentCalibrationEndYear = calibrationEndYear;

    /* Set the number of years to skip at the beginning of the history
    ** This will be 0 when calibration start year is not specified
    */
    nStartYearsToSkip = currentCalibrationStartYear - startyear;
    /* Now set the number of years to skip at the end of the history
    ** This will be 0 when calibration end year is not specified
    */

    nEndYearsToSkip = endyear - currentCalibrationEndYear;
    nCalibrationYears = currentCalibrationEndYear - currentCalibrationStartYear + 1;
    /* num_of_periods is set in pdsi constructor and reset in places with the number of periods in a year */
    nStartPeriodsToSkip = nStartYearsToSkip * num_of_periods;
    nEndPeriodsToSkip = nEndYearsToSkip * num_of_periods;
    nCalibrationPeriods = nCalibrationYears * num_of_periods;
    /* SG 6/5/06: end addition */

    // If out then set the output_mode flag according to the output specifier
    if (year && out)
        output_mode = 0;
    else if (week && out)
        output_mode = 1;
    else if (both && out)
        output_mode = 2;
    else if (out) {
        printf("Missing output specifier with -o flag.\n");
        printf("pdsi --help for usage.\n");
        exit(1);
    }

}// End set_flags
//-----------------------------------------------------------------------------
//WeeklyCMI calls the functions necessary to compute the Crop Moisture Index
//(CMI).  The CMI is based on subcalculation of the PDSI.  It is only done
//on a 1-week time scale.
//-----------------------------------------------------------------------------
void pdsi::WeeklyCMI() {
    FILE * param;
    char filename[170];
    int i;

    period_length = 1;
    num_of_periods = 52;

    /* SG 6/5/06: The WeeklyCMI does not (yet) support a calibration interval, so clear the vars */
    currentCalibrationStartYear = startyear;
    currentCalibrationEndYear = endyear;
    nEndYearsToSkip = 0;
    nStartYearsToSkip = 0;
    nCalibrationYears = totalyears;
    nStartPeriodsToSkip = 0;
    nEndPeriodsToSkip = 0;
    nCalibrationPeriods = nCalibrationYears * num_of_periods;
    /* SG 6/5/06: end addition */


    Weekly = true;
    Monthly = false; SCMonthly = false;
    extra = 1;
    if (initialize() < 1) {
        if (verbose > 0)
            printf("%4s Cannot calculate the weekly CMI.\n", "*");
        if (verbose > 1) {
            printf("The necessary input files ");
            printf("were not found in the directory %s\n", input_dir);
            printf("The following files are required:\n");
            printf("weekly_T \nweekly_P\nparameter\nwk_T_normal\n\n");
        }
        return;
    }

    // This block opens the parameter file and sets the initial Su and TLA values
    // must be called after the variable period_length is determined in the
    // set_flags function

    if (strlen(input_dir) > 1)
        strcpy(filename, input_dir);
    else
        strcpy(filename, "./");
    strcat(filename, "parameter");
    if ((param = fopen(filename, "r")) == NULL) {
        printf("Unable to open parameter file.\n");
        printf("File name: %s\n", filename);
        exit(1);
    }
    GetParam(param);
    fclose(param);

    // Output seen only in maximum verbose mode
    if (verbose > 1)
        printf ("processing station 1\n");
    // SumAll is called to compute the sums for the 8 water balance variables
    SumAll();
    // This outputs those sums to the screen
    if (verbose > 1) {
        printf ("STATION = %5d %18c SUMMATION OF WEEKLY VALUES OVER %d YEARS\n", 0, ' ', totalyears);
        printf ("%36c CALIBRATION YEARS:\n", ' ');
        printf ("%4s %7s %8s %8s %8s %8s %8s %8s %8s %8s %10s", "PER", "P", "S", "PR", "PE", "PL", "ET", "R", "L", "RO", "DEP\n\n");
    }
    for (i = 0; i < num_of_periods; i++) {
        DEPSum[i] = ETSum[i] + RSum[i] - PESum[i] + ROSum[i];
        if (verbose > 1) {
            printf ("%4d", (period_length * i) + 1);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", PSum[i], PROSum[i], PRSum[i], PESum[i], PLSum[i]);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", ETSum[i], RSum[i], LSum[i], ROSum[i], DEPSum[i]);
            printf ("\n");
        }
        DSSqr[i] = 0;
    }

    // CalcWBCoef is then called to calculate alpha, beta, gamma, and delta
    CalcWBCoef();

    CalcCMI();

}
//-----------------------------------------------------------------------------
// WeeklyPDSI calls all functions necessary to compute the final PDSI.  This is
// done using the 4 functions SumAll, CalcWBCoef, Calcd, and CalcK.
// Produces a weekly, calibrated PDSI
//-----------------------------------------------------------------------------
void pdsi::WeeklyPDSI() {
    FILE * param;
    char filename[170];
    int i;

    Weekly = true;
    Monthly = false; SCMonthly = false;
    if (initialize() < 1) {
        if (verbose > 0)
            printf("%4s Cannot calculate the weekly PDSI.\n", "*");
        if (verbose > 1) {
            printf("The necessary input files ");
            printf("were not found in the directory %s\n", input_dir);
            printf("The following files are required:\n");
            printf("weekly_T \nweekly_P\nparameter\nwk_T_normal\n\n");
        }
        return;
    }

    // This block opens the parameter file and sets the initial Su and TLA values
    // must be called after the variable period_length is determined in the
    // set_flags function

    if (strlen(input_dir) > 1)
        strcpy(filename, input_dir);
    else
        strcpy(filename, "./");
    strcat(filename, "parameter");
    if ((param = fopen(filename, "r")) == NULL) {
        printf("Unable to open parameter file.\n");
        printf("File name: %s\n", filename);
        exit(1);
    }
    GetParam(param);
    fclose(param);

    // Output seen only in maximum verbose mode
    if (verbose > 1)
        printf ("processing station 1\n");
    // SumAll is called to compute the sums for the 8 water balance variables
    SumAll();
    // This outputs those sums to the screen
    if (verbose > 1) {
        printf ("STATION = %5d %18c SUMMATION OF WEEKLY VALUES OVER %d YEARS\n", 0, ' ', totalyears);
        printf ("%36c CALIBRATION YEARS:\n", ' ');
        printf ("%4s %7s %8s %8s %8s %8s %8s %8s %8s %8s %10s", "PER", "P", "S", "PR", "PE", "PL", "ET", "R", "L", "RO", "DEP\n\n");
    }
    for (i = 0; i < num_of_periods; i++) {
        DEPSum[i] = ETSum[i] + RSum[i] - PESum[i] + ROSum[i];
        if (verbose > 1) {
            printf ("%4d", (period_length * i) + 1);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", PSum[i], PROSum[i], PRSum[i], PESum[i], PLSum[i]);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", ETSum[i], RSum[i], LSum[i], ROSum[i], DEPSum[i]);
            printf ("\n");
        }
        DSSqr[i] = 0;
    }

    // CalcWBCoef is then called to calculate alpha, beta, gamma, and delta
    CalcWBCoef();
    // Next Calcd is called to calculate the weekly departures from normal
    Calcd();
    // CalcK is called to compute the K values
    CalcK();
    // CalcZ is called to compute the Z index
    CalcZ();

    //the calibration process begins
    if (verbose > 1)
        printf("\nCalibrating Index.\n");
    //calculate the duration factors
    CalcDurFact(wetm, wetb, 1);
    CalcDurFact(drym, dryb, -1);
    if (verbose > 1) {
        printf("duration factors:\n");
        printf("wetm = %.3f  wetb = %.3f --> ", wetm, wetb);
        printf("p = %.3f and q = %.3f \n", 1 - (wetm / (wetm + wetb)), 1 / (wetm + wetb));
        printf("drym = %.3f  dryb = %.3f --> ", drym, dryb);
        printf("p = %.3f and q = %.3f \n", 1 - (drym / (drym + dryb)), 1 / (drym + dryb));
    }
    //Calculate the PDSI values
    CalcX();
    //Calibrate the Index
    Calibrate();

    // Now that all calculations have been done they can be output to the screen
    /* SG 6/5/06: changed totalyears to nCalibrationYears means to support
    **            user defined calibration intervals. When not used
    **            nCalibrationYears==totalyears; hence no change then
    */
    if (verbose > 1) {
        int i;
        printf ("STATION = %5d %24c PARAMETERS AND MEANS OF MONTHLY VALUE FOR %d YEARS\n\n", 0, ' ', nCalibrationYears); /* SG 6/5/06: May want to clarify this is number Calibration Years when so specified */
        printf ("%4s %8s %8s %8s %8s %8s %8s %7s %8s", "PER", "ALPHA", "BETA", "GAMMA", "DELTA", "Wet K", "Dry K", "P", "S");
        printf ("%4s %8s %8s %8s %8s %8s %8s %7s %8s", "PER", "ALPHA", "BETA", "GAMMA", "DELTA", "Wet K", "Dry K", "P", "S");
        printf ("%9s %8s %8s %8s %8s %8s %8s\n\n", "PR", "PE", "PL", "ET", "R", "L", "RO");
        for (i = 0; i < num_of_periods; i++) {
            printf ("%4d %8.4f %8.4f %8.4f %8.4f", (period_length * i) + 1, Alpha[i], Beta[i], Gamma[i], Delta[i]);
            printf ("%9.3f %8.3f %8.2f %8.2f %8.2f", k[i]*wet_ratio, k[i]*dry_ratio, PSum[i] / nCalibrationYears, PROSum[i] / nCalibrationYears, PRSum[i] / nCalibrationYears);
            printf ("%9.2f %8.2f %8.2f %8.2f", PESum[i] / nCalibrationYears, PLSum[i] / nCalibrationYears, ETSum[i] / nCalibrationYears, RSum[i] / nCalibrationYears);
            printf ("%9.2f %8.2f\n", LSum[i] / nCalibrationYears, ROSum[i] / nCalibrationYears);
        }
        printf ("\n\n\n%4s %8s %8s %8s %8s %8s\n\n", "PER", "D-ABS", "SIG-D", "DEP", "S-DEP", "SIG-S");
        for (i = 0; i < num_of_periods; i++) {
            printf ("%4d %8.3f %8.2f %8.2f ", (period_length * i) + 1, D[i], sqrt(DSSqr[i] / (nCalibrationYears - 1)), DEPSum[i] / nCalibrationYears);
            if (i == 7) {
                number E, DE;
                E = SD / nCalibrationYears;
                DE = sqrt((SD2 - E * SD) / (nCalibrationYears - 1));
                printf ("%8.2f %8.2f", E, DE);
            }
            printf ("\n");
        }
    }
}//end of WeeklyPDSI()
void pdsi::WeeklyPDSI(int length) {
    period_length = length;
    switch (period_length) {
    case 1:
        num_of_periods = 52;
        break;
    case 2:
        num_of_periods = 26;
        break;
    case 4:
        num_of_periods = 13;
        break;
    case 13:
        num_of_periods = 4;
        break;
    default:
        num_of_periods = 52;
        period_length = 1;
        if (verbose) {
            printf("Possible Error in reading length of time scale.\n");
            printf("Calculating single week PDSI....\n");
        }
        break;
    }

    /* num_of_periods is set in pdsi constructor and reset in here with the number of periods in a year */
    currentCalibrationStartYear = calibrationStartYear;
    currentCalibrationEndYear = calibrationEndYear;
    nCalibrationYears = currentCalibrationEndYear - currentCalibrationStartYear + 1;
    nStartYearsToSkip = currentCalibrationStartYear - startyear;
    nEndYearsToSkip = endyear - currentCalibrationEndYear;
    nStartPeriodsToSkip = nStartYearsToSkip * num_of_periods;
    nEndPeriodsToSkip = nEndYearsToSkip * num_of_periods;
    nCalibrationPeriods = nCalibrationYears * num_of_periods;
    /* SG 6/5/06: end addition */

    //call WeeklyPDSI();
    WeeklyPDSI();
}// end of WeeklyPDSI(int length)
//-----------------------------------------------------------------------------
// MonthlyPDSI calls all functions necessary to compute the final PDSI.
// This is done using the 4 functions SumAll, CalcWBCoef, Calcd, and CalcK.
// Produces a the original PDSI
//-----------------------------------------------------------------------------
void pdsi::MonthlyPDSI() {
    int i;
    FILE *param;
    char filename[170];

    Monthly = true;
    Weekly = false; SCMonthly = false;
    if (initialize() < 1) {
        if (verbose > 0)
            printf("%4s Cannot calculate the Monthly PDSI.\n", "*");
        if (verbose > 1) {
            printf("The necessary input files ");
            printf("were not found in the directory %s\n", input_dir);
            printf("The following files are required:\n");
            printf("monthly_T \nmonthly_P \nparameter \nmon_T_normal\n\n");
        }
        return;
    }
    //preserve period_length and num_of_periods for multiple week PDSI's.

    period_length = 1;
    num_of_periods = 12;

    /* SG 6/5/06: The original Monthly PDSI should not support a calibration interval, so clear the vars */
    currentCalibrationStartYear = startyear;
    currentCalibrationEndYear = endyear;
    nEndYearsToSkip = 0;
    nStartYearsToSkip = 0;
    nCalibrationYears = totalyears;
    nStartPeriodsToSkip = 0;
    nEndPeriodsToSkip = 0;
    nCalibrationPeriods = nCalibrationYears * num_of_periods;
    /* SG 6/5/06: end addition */

    // This block opens the parameter file and sets the initial Su and TLA values
    // must be called after the variable period_length is determined in the
    // set_flags function

    if (strlen(input_dir) > 1)
        strcpy(filename, input_dir);
    else
        strcpy(filename, "./");
    strcat(filename, "parameter");
    if ((param = fopen(filename, "r")) == NULL) {
        printf("Unable to open parameter file.\n");
        printf("File name: %s\n", filename);
        exit(1);
    }
    GetParam(param);
    fclose(param);

    // Output seen only in maximum verbose mode
    if (verbose > 1)
        printf ("processing station 1\n");
    // SumAll is called to compute the sums for the 8 water balance variables
    SumAll();
    // This outputs those sums to the screen
    if (verbose > 1) {
        printf ("STATION = %5d %18c SUMMATION OF MONTHLY VALUES OVER %4d YEARS\n", 0, ' ', totalyears);
        printf ("%36c CALIBRATION YEARS:\n", ' ');
        printf ("%4s %7s %8s %8s %8s %8s %8s %8s %8s %8s %10s", "PER", "P", "S", "PR", "PE", "PL", "ET", "R", "L", "RO", "DEP\n\n");
    }
    for (i = 0; i < num_of_periods; i++) {
        DEPSum[i] = ETSum[i] + RSum[i] - PESum[i] + ROSum[i];
        if (verbose > 1) {
            printf ("%4d", (period_length * i) + 1);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", PSum[i], PROSum[i], PRSum[i], PESum[i], PLSum[i]);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", ETSum[i], RSum[i], LSum[i], ROSum[i], DEPSum[i]);
            printf ("\n");
        }
        DSSqr[i] = 0;
    }

    // CalcWBCoef is then called to calculate alpha, beta, gamma, and delta
    CalcWBCoef();
    // Next Calcd is called to calculate the monthly departures from normal
    Calcd();
    // Finally CalcK is called to compute the K and Z values.  CalcX is called
    // within CalcK.
    CalcOrigK();

}
//-----------------------------------------------------------------------------
// SCMonthlyPDSI calls all functions necessary to compute the final PDSI.  This is
// done using the 4 functions SumAll, CalcWBCoef, Calcd, and CalcK.
// Produces a the original PDSI
//-----------------------------------------------------------------------------
void pdsi::SCMonthlyPDSI() {
    int i;
    FILE *param;
    char filename[170];

    SCMonthly = true;
    Monthly = false;  Weekly = false;
    if (initialize() < 1) {
        if (verbose > 0)
            printf("%4s Cannot calculate the Self-Calibrated Monthly PDSI.\n", "*");
        if (verbose > 1) {
            printf("The necessary input files ");
            printf("were not found in the directory %s\n", input_dir);
            printf("The following files are required:\n");
            printf("monthly_T \nmonthly_P \nparameter \nmon_T_normal\n\n");
        }
        return;
    }

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

    if (verbose > 1) { /* output calibration interval vars if max verbose is set */
        printf("\nstartyear = %d\n", startyear);
        printf("endyear = %d\n", endyear);
        printf("totalyears = %d\n", totalyears);
        printf("setCalibrationStartYear = %d\n", setCalibrationStartYear);
        printf("setCalibrationEndYear = %d\n", setCalibrationEndYear);
        printf("calibrationStartYear = %d\n", calibrationStartYear);
        printf("calibrationEndYear = %d\n", calibrationEndYear);
        printf("currentCalibrationStartYear = %d\n", currentCalibrationStartYear);
        printf("currentCalibrationEndYear = %d\n", currentCalibrationEndYear);
        printf("num_of_periods = %d\n", num_of_periods);
        printf("nStartYearsToSkip = %d\n", nStartYearsToSkip);
        printf("nStartPeriodsToSkip = %d\n", nStartPeriodsToSkip);
        printf("nEndYearsToSkip = %d\n", nEndYearsToSkip);
        printf("nEndPeriodsToSkip = %d\n", nEndPeriodsToSkip);
        printf("nCalibrationYears = %d\n", nCalibrationYears);
        printf("nCalibrationPeriods = %d\n", nCalibrationPeriods);
    }
    /* SG 6/5/06: end addition */

    // This block opens the parameter file and sets the initial Su and TLA values
    // must be called after the variable period_length is determined in the
    // set_flags function

    if (strlen(input_dir) > 1)
        strcpy(filename, input_dir);
    else
        strcpy(filename, "./");
    strcat(filename, "parameter");
    if ((param = fopen(filename, "r")) == NULL) {
        printf("Unable to open parameter file.\n");
        printf("File name: %s\n", filename);
        exit(1);
    }
    GetParam(param);
    fclose(param);

    // Output seen only in maximum verbose mode
    if (verbose > 1)
        printf ("processing station 1\n");
    // SumAll is called to compute the sums for the 8 water balance variables
    SumAll();
    // This outputs those sums to the screen
    if (verbose > 1) {
        printf ("STATION = %5d %18c SUMMATION OF MONTHLY VALUES OVER %4d YEARS\n", 0, ' ', totalyears);
        printf ("%36c CALIBRATION YEARS:\n", ' ');
        printf ("%4s %7s %8s %8s %8s %8s %8s %8s %8s %8s %10s", "PER", "P", "S", "PR", "PE", "PL", "ET", "R", "L", "RO", "DEP\n\n");
    }
    for (i = 0; i < num_of_periods; i++) {
        /* DEPSum will only include calibration interval data since the ET, R, PE, and RO
        ** sum variables only include data from the calibration interval.
        */
        DEPSum[i] = ETSum[i] + RSum[i] - PESum[i] + ROSum[i];
        if (verbose > 1) {
            printf ("%4d", (period_length * i) + 1);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", PSum[i], PROSum[i], PRSum[i], PESum[i], PLSum[i]);
            printf ("%9.2f %8.2f %8.2f %8.2f %8.2f", ETSum[i], RSum[i], LSum[i], ROSum[i], DEPSum[i]);
            printf ("\n");
        }
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
    CalcK();
    // CalcZ is called to compute the Z index
    CalcZ();

    //the calibration process begins
    if (verbose > 1)
        printf("\nCalibrating Index.\n");
    //calculate the duration factors
    CalcDurFact(wetm, wetb, 1);
    CalcDurFact(drym, dryb, -1);
    if (verbose > 1) {
        printf("duration factors:\n");
        printf("wetm = %.3f  wetb = %.3f --> ", wetm, wetb);
        printf("p = %.3f and q = %.3f \n", 1 - (wetm / (wetm + wetb)), 1 / (wetm + wetb));
        printf("drym = %.3f  dryb = %.3f --> ", drym, dryb);
        printf("p = %.3f and q = %.3f \n", 1 - (drym / (drym + dryb)), 1 / (drym + dryb));
    }
    //Calculate the PDSI values
    CalcX();
    //Calibrate the Index
    Calibrate();
    // Now that all calculations have been done they can be output to the screen
    /* SG 6/5/06: changed totalyears to nCalibrationYears means to support
    **            user defined calibration intervals. When not used
    **            nCalibrationYears==totalyears; hence no change then
    */
    if (verbose > 1) {
        int i;
        printf ("STATION = %5d %24c PARAMETERS AND MEANS OF MONTHLY VALUE FOR %d YEARS\n\n", 0, ' ', nCalibrationYears); /* SG 6/5/06: May want to clarify this is number Calibration Years when so specified */
        printf ("%4s %8s %8s %8s %8s %8s %8s %7s %8s", "PER", "ALPHA", "BETA", "GAMMA", "DELTA", "Wet K", "Dry K", "P", "S");
        printf ("%9s %8s %8s %8s %8s %8s %8s\n\n", "PR", "PE", "PL", "ET", "R", "L", "RO");
        for (i = 0; i < num_of_periods; i++) {
            printf ("%4d %8.4f %8.4f %8.4f %8.4f", (period_length * i) + 1, Alpha[i], Beta[i], Gamma[i], Delta[i]);
            printf ("%9.3f %8.3f %8.2f %8.2f %8.2f", k[i]*wet_ratio, k[i]*dry_ratio, PSum[i] / nCalibrationYears, PROSum[i] / nCalibrationYears, PRSum[i] / nCalibrationYears);
            printf ("%9.2f %8.2f %8.2f %8.2f", PESum[i] / nCalibrationYears, PLSum[i] / nCalibrationYears, ETSum[i] / nCalibrationYears, RSum[i] / nCalibrationYears);
            printf ("%9.2f %8.2f\n", LSum[i] / nCalibrationYears, ROSum[i] / nCalibrationYears);
        }
        printf ("\n\n\n%4s %8s %8s %8s %8s %8s\n\n", "PER", "D-ABS", "SIG-D", "DEP", "S-DEP", "SIG-S");
        for (i = 0; i < num_of_periods; i++) {
            printf ("%4d %8.3f %8.2f %8.2f ", (period_length * i) + 1, D[i], sqrt(DSSqr[i] / (nCalibrationYears - 1)), DEPSum[i] / nCalibrationYears);
            if (i == 7) {
                number E, DE;
                E = SD / nCalibrationYears;
                DE = sqrt((SD2 - E * SD) / (nCalibrationYears - 1));
                printf ("%8.2f %8.2f", E, DE);
            }
            printf ("\n");
        }
    }
}//end of SCMonthlyPDSI()
//-----------------------------------------------------------------------------
// This function reads in a years worth of data from file In and places those
// values in array A.  It's been modified to average the input data to the
// correct time scale.  Because of this modification, it only works for
// temperature data; precip data must be summed, not averaged.
//-----------------------------------------------------------------------------
int pdsi::GetTemp(FILE *In, number *A, int max) {
    float t[52], t2[52], temp;
    int i, j, year, read, bad_weeks;
    char line[4096];
    char letter;

    for (i = 0; i < 52; i++)
        A[i] = 0;

    fgets(line, 4096, In);
    read = sscanf(line, "%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &year, &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], &t[6], &t[7], &t[8], &t[9], &t[10], &t[11], &t[12], &t[13], &t[14], &t[15], &t[16], &t[17], &t[18], &t[19], &t[20], &t[21], &t[22], &t[23], &t[24], &t[25], &t[26], &t[27], &t[28], &t[29], &t[30], &t[31], &t[32], &t[33], &t[34], &t[35], &t[36], &t[37], &t[38], &t[39], &t[40], &t[41], &t[42], &t[43], &t[44], &t[45], &t[46], &t[47], &t[48], &t[49], &t[50], &t[51]);

    //place values read into array t2 to be summarized
    if (read == max + 1) {
        //a full year's worth of data was read
        for (i = 0; i < max; i++)
            t2[i] = t[i];
    }
    else {
        //check to see if it is the end of file
        if ( (letter = fgetc(In)) != EOF ) {
            //it's not the end of the file
            //so place partial year's data at end of array
            for (i = 0 ; i < max - (read - 1); i++)
                t2[i] = MISSING;
            for (i; i < max; i++)
                t2[i] = t[i - (max - read + 1)];
            ungetc(letter, In);
        }
        else {
            //it's the end of the file, place partial year's data
            //at beginning on array
            for (i = 0; i < read - 1; i++)
                t2[i] = t[i];
            for (i; i < max; i++)
                t2[i] = MISSING;
        }
    }
    for (i = 0; i < num_of_periods; i++) {
        bad_weeks = 0;
        temp = 0;
        for (j = 0; j < period_length; j++) {
            if (t2[i * period_length + j] != MISSING)
                temp += t2[i * period_length + j];
            else
                bad_weeks++;
        }
        if (bad_weeks < period_length)
            A[i] = temp / (period_length - bad_weeks);
        else
            A[i] = MISSING;
    }
    if (metric) {
        for (i = 0; i < num_of_periods; i++) {
            if (A[i] != MISSING)
                A[i] = A[i] * (9.0 / 5.0) + 32;
        }
    }

    return year;

}
//-----------------------------------------------------------------------------
// This function is a modified version of GetTemp() function for precip only.
//-----------------------------------------------------------------------------
int pdsi::GetPrecip(FILE *In, number *A, int max) {
    float t[52], t2[52], temp;
    int i, j, year, read, bad_weeks;
    char line[4096];
    char letter;

    for (i = 0; i < 52; i++)
        A[i] = 0;

    fgets(line, 4096, In);
    read = sscanf(line, "%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &year, &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], &t[6], &t[7], &t[8], &t[9], &t[10], &t[11], &t[12], &t[13], &t[14], &t[15], &t[16], &t[17], &t[18], &t[19], &t[20], &t[21], &t[22], &t[23], &t[24], &t[25], &t[26], &t[27], &t[28], &t[29], &t[30], &t[31], &t[32], &t[33], &t[34], &t[35], &t[36], &t[37], &t[38], &t[39], &t[40], &t[41], &t[42], &t[43], &t[44], &t[45], &t[46], &t[47], &t[48], &t[49], &t[50], &t[51]);

    //place values read into array t2 to be summarized
    if (read == max + 1) {
        //a full year's worth of data was read
        for (i = 0; i < max; i++)
            t2[i] = t[i];
    }
    else {
        //check to see if it is the end of file
        if ( (letter = fgetc(In)) != EOF ) {
            //it's not the end of the file
            //so place partial year's data at end of array
            for (i = 0 ; i < max - (read - 1); i++)
                t2[i] = MISSING;
            for (i; i < max; i++)
                t2[i] = t[i - (max - read + 1)];
            ungetc(letter, In);
        }
        else {
            //it's the end of the file, place partial year's data
            //at beginning on array
            for (i = 0; i < read - 1; i++)
                t2[i] = t[i];
            for (i; i < max; i++)
                t2[i] = MISSING;
        }
    }
    //now summaraize data in t2 into A
    for (i = 0; i < num_of_periods; i++) {
        bad_weeks = 0;
        temp = 0;
        for (j = 0; j < period_length; j++) {
            if (t2[i * period_length + j] != MISSING)
                temp += t2[i * period_length + j];
            else
                bad_weeks++;
        }
        if (bad_weeks < period_length)
            A[i] = temp;
        else
            A[i] = MISSING;
    }
    if (metric) {
        for (i = 0; i < num_of_periods; i++) {
            if (A[i] != MISSING)
                A[i] = A[i] / 25.4;
        }
    }

    return year;
}
//-----------------------------------------------------------------------------
// This function reads in the 2 initializing values of Su and TLA
//-----------------------------------------------------------------------------
void pdsi::GetParam(FILE * Param) {
    float scn1, scn2;
    double lat;
    double PI = 3.1415926535;
    fscanf(Param, "%f %f", &scn1, &scn2);
    AWC = double(scn1);
    TLA = double(scn2);
    if (metric)
        AWC = AWC / 25.4;
    if (AWC <= 0) {
        printf("Invalid value for AWC: %f\n", Su);
        exit(0);
    }
    Ss = 1.0;   //assume the top soil can hold 1 inch
    if (AWC < Ss) {
        //always assume the top layer of soil can
        //hold at least the Ss value of 1 inch.
        AWC = Ss;
    }
    Su = AWC - Ss;
    if (Su < 0)
        Su = 0;
    if (nadss) {
        if (TLA > 0) {
            if (verbose > 1)
                printf("TLA is positive, assuming location is in Southern Hemisphere. TLA: %f\n", TLA);
            south = 1;
            TLA = -TLA;
        }
        else
            south = 0;
    }
    else {
        lat = TLA;
        TLA = -tan(PI * lat / 180);
        if (lat >= 0) {
            if (verbose > 1)
                printf("TLA is positive, assuming location is in Southern Hemisphere. TLA: %f\n", TLA);
            south = 0;
        }
        else {
            south = 1;
            TLA = -TLA;
        }
    }
    if (Weekly)
        I = CalcWkThornI();
    else if (Monthly || SCMonthly)
        I = CalcMonThornI();
    else {
        if (verbose) {
            printf("Error.  Invalid type of PDSI calculation\n");
            printf("Either the 'Weekly', 'Monthly', ");
            printf("or 'SCMonthly' flags must be set.\n");
        }
        exit(1);
    }

    A = CalcThornA(I);
    if (verbose > 1) {
        printf ("AWC = % .9f  TLA = % .8f\n", AWC, TLA);
        printf ("HEAT INDEX, THORNTHWAITE A: %14.5f %11.5f\n", I, A);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pdsi::CalcWkPE(int period, int year) {
    number Phi[52] = { -.39796983, -.38279708, -.36154002, -.33468535, -.30281503, -.26657407, -.22664177, -.18370885, -.13846106, -.09156812, -.04368365, 0.00456585, .05256951, .09973421, .14548036, .18923931, .23045239, .26857222, .30306721, .33342965, .35918741, .37991898, .39527047, .40497267, .40885593, .40686015, .3990384, .38555316, .36666599, .34272229, .31413363, .28136006, .24489444, .20524994, .16295153, .11853109, .0725259, .02547986, -.02205355, -.06950714, -.11629575, -.16181268, -.20542804, -.24649005, -.28433068, -.31827697, -.3476688, -.37188342, -.3903656, -.40266061, -.40844587, -.40755601};
    //Phi is a daylight constant based on what day of the week January 1st falls
    //on.  The method of calculating Phi was taken directly from palmcode.f, the
    //NCDC's weekly palmer program.
    //For this program however, Phi was calculated by first calculating the
    //Phi values for a year with January 1st falling on each day of the week and
    //then averaging those results.  This was found to have little to no effect
    //on the final results.
    number Dum, Dk;
    number adjusted_Phi[52];
    number temp;
    int i, j;
    int offset;
    if (south) {
        offset = 26;
    }
    else
        offset = 0;


    //need to average Phi[] into an array of num_of_period elements
    //if calculated in southern hemisphere, this will also shift
    //the values by 6 months worth to make up for the opposite seasons.
    for (i = 0; i < (52 / period_length); i++) {
        temp = 0;
        for (j = 0; j < period_length; j++)
            temp += Phi[( (i * period_length) + j + offset ) % 52];
        temp = temp / period_length;
        adjusted_Phi[i] = temp;
    }

    if (T[period] <= 32)
        PE = 0;
    else {
        Dum = adjusted_Phi[period] * TLA;
        Dk = atan(sqrt(1 - Dum * Dum) / Dum);
        if (Dk < 0)
            Dk += 3.141593;
        Dk = (Dk + .0157) / 1.57;
        if (T[period] >= 80) {
            PE = (sin(T[period] / 57.3 - .166) - .76) * Dk;
        }
        else {
            Dum = log(T[period] - 32);
            PE = (exp(-3.863233 + A * 1.715598 - A * log(I) + A * Dum)) * Dk;
        }
    }
    PE = PE * period_length * 7;
}
void pdsi::CalcMonPE(int month, int year) {
    number Phi[] = { -.3865982, -.2316132, -.0378180, .1715539, .3458803, .4308320, .3916645, .2452467, .0535511, -.15583436, -.3340551, -.4310691};
    //these values of Phi[] come directly from the fortran program.
    int Days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    number Dum, Dk;
    int offset;
    if (south)
        offset = 6;
    else
        offset = 0;

    if (T[month] <= 32)
        PE = 0;
    else {
        Dum = Phi[(month + offset) % 12] * TLA;
        Dk = atan(sqrt(1 - Dum * Dum) / Dum);
        if (Dk < 0)
            Dk += 3.141593;
        Dk = (Dk + .0157) / 1.57;
        if (T[month] >= 80)
            PE = (sin(T[month] / 57.3 - .166) - .76) * Dk;
        else {
            Dum = log(T[month] - 32);
            PE = (exp(-3.863233 + A * 1.715598 - A * log(I) + A * Dum)) * Dk;
        }
    }

    // This calculation of leap year follows the FORTRAN program
    // It does not take into account factors of 100 or 400
    /*
    if (year%4==0 && month==1)
      PE=PE*29;
    else
      PE=PE*Days[month];
    */
    //this calculation has been updated to accurately follow leap years
    if (month == 1) {
        if (year % 400 == 0)
            PE = PE * 29;
        else if (year % 4 == 0 && year % 100 != 0)
            PE = PE * 29;
        else
            PE = PE * 28;
    }
    else
        PE = PE * Days[month];

}
//-----------------------------------------------------------------------------
// This function calculates the Thornthwaite heat index I.  This is done by
// reading in the weekly normal temperature from file.  Any above freezing
// temperatures are adjusted and then added to the index.  The equations have
// been modified to handle temperature in degrees Fahrenheit
//-----------------------------------------------------------------------------
number pdsi::CalcWkThornI() {
    number I = 0;
    int i = 0, j = 0;
    float t[53];
    FILE *fin;
    char filename[150];
    if (strlen(input_dir) > 1) {
        strcpy(filename, input_dir);
        strcat(filename, "wk_T_normal");
    }
    else
        strcpy(filename, "wk_T_normal");

    // The file containing the normal temperatures is opened for reading.
    if ((fin = fopen(filename, "r")) == NULL) {
        if (verbose > 1) {
            printf("Warning:  Failed opening file for normal temperatures.\n");
            printf("          filename: %s\n", filename);
        }
        if (strlen(input_dir) > 1)
            sprintf(filename, "%s%s", input_dir, "T_normal");
        else
            strcpy(filename, "T_normal");
        if ((fin = fopen(filename, "r")) == NULL) {
            if (verbose > 0) {
                printf("Fatal Error: Failed to open file for normal temperatures.\n");
                printf("             filename: %s\n", filename);
            }
            exit(0);
        }
    }

    // The weekly temperatures are read in to a temparary array.
    // This was done because the fscanf function was unable to handle an array
    // of type double with the %f.  However, this will also allow for different
    // types of variables to be used in place of double.
    fscanf(fin, "%f %f %f %f %f %f", &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
    fscanf(fin, "%f %f %f %f %f %f", &t[6], &t[7], &t[8], &t[9], &t[10], &t[11]);
    fscanf(fin, "%f %f %f %f %f %f", &t[12], &t[13], &t[14], &t[15], &t[16], &t[17]);
    fscanf(fin, "%f %f %f %f %f %f", &t[18], &t[19], &t[20], &t[21], &t[22], &t[23]);
    fscanf(fin, "%f %f %f %f %f %f", &t[24], &t[25], &t[26], &t[27], &t[28], &t[29]);
    fscanf(fin, "%f %f %f %f %f %f", &t[30], &t[31], &t[32], &t[33], &t[34], &t[35]);
    fscanf(fin, "%f %f %f %f %f %f", &t[36], &t[37], &t[38], &t[39], &t[40], &t[41]);
    fscanf(fin, "%f %f %f %f %f %f", &t[42], &t[43], &t[44], &t[45], &t[46], &t[47]);
    fscanf(fin, "%f %f %f %f", &t[48], &t[49], &t[50], &t[51]);
    //check to make sure the file only has 52 entries
    if (fscanf(fin, "%f", &t[53]) != EOF) {
        printf("Warning: Normal Temperature file, %s, is the wrong format.\n");
        printf("         Results may not be accurate.\n");
    }

    // Then we move the temperatures to the TNorm array and calclulate I
    for (i = 0; i < 52; i++) {
        if (metric)
            TNorm[i] = t[i] * (9.0 / 5.0) + 32;
        else
            TNorm[i] = t[i];

        // Prints the normal temperatures to the screen
        if (verbose > 1)
            printf (" %.2f", TNorm[i]);
        // Adds the modified temp to heat if the temp is above freezing
        if (TNorm[i] > 32)
            I = I + pow((TNorm[i] - 32) / 9, 1.514);
    }
    if (I == 0) {
        if (verbose) {
            printf("Error in calculating Thornthwaite heat index I.\n");
            printf("I == 0 \n\n");
        }
        fclose(fin);
        exit(1);
    }
    // Prints a newline to the screen and closes the input file
    if (verbose > 1)
        printf ("\n");
    fclose(fin);
    //adjust I for weeks by mult by (12/52) and return
    return (I / 52 * 12);
}
//-----------------------------------------------------------------------------
//This function calculates the Thornthwaite heat index I for montly PDSI's.
//-----------------------------------------------------------------------------
number pdsi::CalcMonThornI() {
    number I = 0;
    int i = 0, j = 0;
    float t[13];
    FILE *fin;
    char filename[150];
    if (strlen(input_dir) > 1) {
        sprintf(filename, "%s%s", input_dir, "mon_T_normal");
    }
    else
        strcpy(filename, "mon_T_normal");
    // The file containing the normal temperatures is opened for reading.
    if ((fin = fopen(filename, "r")) == NULL) {
        if (verbose > 1) {
            printf("Warning:  Failed opening file for normal temperatures.\n");
            printf("          filename: %s\n", filename);
        }
        if (strlen(input_dir) > 1)
            sprintf(filename, "%s%s", input_dir, "T_normal");
        else
            strcpy(filename, "T_normal");
        if ((fin = fopen(filename, "r")) == NULL) {
            if (verbose > 0) {
                printf("Fatal Error: Failed to open file for normal temperatures.\n");
                printf("             filename: %s\n", filename);
            }
            exit(0);
        }
    }

    // The monthly temperatures are read in to a temparary array.
    // This was done because the fscanf function was unable to handle an array
    // of type double with the %f.  There might be something that could be used
    // in place of the %f to get a double to work.
    fscanf(fin, "%f %f %f %f %f %f", &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
    fscanf(fin, "%f %f %f %f %f %f", &t[6], &t[7], &t[8], &t[9], &t[10], &t[11]);
    //check to make sure file only had 12 entries
    if (fscanf(fin, "%f", &t[13]) != EOF) {
        printf("Warning: Normal Temperature file, %s, is the wrong format.\n", filename);
        printf("         Results may not be accurate.\n");
    }
    // Then we move the temperatures to the TNorm array and calclulate I
    for (i = 0; i < 12; i++) {
        if (metric)
            TNorm[i] = t[i] * (9.0 / 5.0) + 32;
        else
            TNorm[i] = t[i];
        // Prints the normal temperatures to the screen
        if (verbose > 1)
            printf (" %.2f", TNorm[i]);
        // Adds the modified temp to heat if the temp is above freezing
        if (TNorm[i] > 32)
            I = I + pow((TNorm[i] - 32) / 9, 1.514);
    }
    // Prints a newline to the screen and closes the input file
    if (verbose > 1)
        printf ("\n");
    fclose(fin);
    return I;
}
//-----------------------------------------------------------------------------
// CalcThornA calculates the Thornthwaite exponent a based on the heat index I.
//-----------------------------------------------------------------------------
number pdsi::CalcThornA(number I) {
    number A;
    A = 6.75 * (pow(I, 3)) / 10000000 - 7.71 * (pow(I, 2)) / 100000 + 0.0179 * I + 0.49;
    return A;
}
//-----------------------------------------------------------------------------
// This function calculates d, the moisture departure, for each period of every
// year examined.
//
// d = P - Phat
//
// Phat = (alpha * PE) + (beta * PR) + (gamma * PRO) - (delta * PL)
//
// where PE, PR, PRO, and PL are read in from a file named "potentials"
// where each line has year, month/week, P, PE, PR, PRO, PL and P-PE
//
// this function also calculates the mean of the absolute values of d
// over all the years examined, called D, which is used to
// calculate k and K
//
// Note:  Phat is P with a ^ on it in the mathematical equations explaining
// the Palmer Drought Index
//-----------------------------------------------------------------------------
void pdsi::Calcd() {
    FILE *fin;        // File pointer for the temp. input file potentials
    FILE *fout;       // File pointer for the temp. output file dvalue
    int per;           // The period in question
    int yr;           // The year in question
    number p;         // The precip for that period
    float scn1, scn2, scn3, scn4, scn5, scn6; // Temp. variables for fscanf
    char letter = ' ';
    int i = 0;
    // These variables are used in calculating terminal outputs and are not
    // important to the final PDSI
    number D_sum[52];
    number DSAct[52];
    number SPhat[52];

    for (i = 0; i < 52; i++) {
        D_sum[i] = 0.0;
        DSAct[i] = 0.0;
        SPhat[i] = 0.0;
    }

    // The potentials file is opened for reading in the previously stored values
    if ((fin = fopen("potentials", "r")) == NULL) {
        if (verbose > 0)
            printf ("Error opening temp file with all the potential values.\n");
        exit(1);
    }

    if ((fout = fopen("dvalue", "w")) == NULL) {
        if (verbose > 0)
            printf ("Error opening temp file for d values.\n");
        exit(1);
    }

    // This reads in the first line, which contains column headers.
    while (letter != '\n')
        letter = fgetc(fin);
    // This reads in the values previously stored in "potentials"
    while (fscanf(fin, "%d %d %f %f %f %f %f %f", &yr, &per, &scn1, &scn2, &scn3, &scn4, &scn5, &scn6) != EOF) {
        per = (per - 1) / period_length; //adjust the period # for use in arrays.

        p = scn1;
        PE = scn2;
        PR = scn3;
        PRO = scn4;
        PL = scn5;
        //scn6 is P - PE, which can be ignored for calculations.

        if (p != MISSING && PE != MISSING && PR != MISSING && PRO != MISSING && PL != MISSING) {
            // Then the calculations for Phat and d are done
            Phat = (Alpha[per] * PE) + (Beta[per] * PR) + (Gamma[per] * PRO) - (Delta[per] * PL);
            d = p - Phat;

            // The values of d are output to a temp file for later use.
            fprintf (fout, "%d %d %f\n", yr, (period_length * per) + 1, d);

            /* SG 6/5/06: Need to only update statistical values when in
            **            user defined calibration interval. When not used
            **            nCalibrationYears==totalyears; hence no change then
            */
            if (yr >= currentCalibrationStartYear && yr <= currentCalibrationEndYear) {
                // D_sum is the sum of the absolute values of d
                // and is used to find D
                // D_sum[per] += abs(d);
                if (d < 0.0)
                    D_sum[per] += -(d);
                else
                    D_sum[per] += d;


                // The statistical values are updated
                DSAct[per] += d;
                DSSqr[per] += d * d;
                SPhat[per] += Phat;
            }
        }
        else {
            d = MISSING;
            fprintf (fout, "%d %d %f\n", yr, (period_length * per) + 1, d);
        }
    }

    // If the user specifies, the various sums are output to the screen
    if (verbose > 1) {
        printf ("\n%34c CHECK SUMS OF ESTIMATED VARIABLES\n\n", ' ');
        printf ("%4s %8s %8s %8s", "PER", "SCET", "SCR", "SCRO");
        printf ("%9s %8s %8s\n\n", "SCL", "SCP", "SCD");
    }
    for (i = 0; i < num_of_periods; i++) {
        if (verbose > 1) {
            printf ("%4d%9.2f%9.2f", (period_length * i) + 1, Alpha[i]*PESum[i], Beta[i]*PRSum[i]);
            printf ("%9.2f%9.2f", Gamma[i]*PROSum[i], Delta[i]*PLSum[i]);
            printf ("%9.2f%9.2f\n", SPhat[i], DSAct[i]);
        }
        // D becomes the mean of D_sum
        /* SG 6/5/06: changed totalyears to nCalibrationYears to support
        **            user defined calibration intervals. When not used
        **            nCalibrationYears==totalyears; hence no change then
        */
        D[i] = D_sum[i] / nCalibrationYears;
    }
    // The files are closed
    fclose(fin);
    fclose(fout);
}// end of Calcd()
//-----------------------------------------------------------------------------
// This function uses previously calculated sums to find K, which is the
// weighting factor used in the Palmer Index calculation.
//-----------------------------------------------------------------------------
void pdsi::CalcK() {
    number sums;        //used to calc k

    // Calculate k, which is K', or Palmer's second approximation of K

    for (int per = 0; per < num_of_periods; per++) {
        if (PSum[per] + LSum[per] == 0)
            sums = 0;//prevent div by 0
        else
            sums = (PESum[per] + RSum[per] + ROSum[per]) / (PSum[per] + LSum[per]);

        if (D[per] == 0)
            k[per] = 0.5;//prevent div by 0
        else
            k[per] = (1.5) * log10((sums + 2.8) / D[per]) + 0.5;
    }

}//end of CalcK()

//-----------------------------------------------------------------------------
// This function uses previously calculated sums to find K, which is the
// original weighting factor used in the Palmer Index calculation.
//-----------------------------------------------------------------------------
void pdsi::CalcOrigK() {
    int month, year;
    number sums;        //used to calc k
    float dtemp;
    DKSum = 0;

    FILE * inputd; // File pointer for input file dvalue
    // The dvalue file is open for reading.
    if ((inputd = fopen("dvalue", "r")) == NULL) {
        if (verbose > 0)
            printf ("Error reading the file with d values.\n");
        exit(1);
    }
    // Calculate k, which is K', or Palmer's second approximation of K
    for (int per = 0; per < num_of_periods; per++) {
        if (PSum[per] + LSum[per] == 0)
            sums = 0;//prevent div by 0
        else
            sums = (PESum[per] + RSum[per] + ROSum[per]) / (PSum[per] + LSum[per]);

        if (D[per] == 0)
            k[per] = 0.5;//prevent div by 0
        else
            k[per] = (1.5) * log10((sums + 2.8) / D[per]) + 0.5;

        DKSum += D[per] * k[per];
    }

    if (Weekly) {
        //set duration factors to CPC's
        drym = 2.925;
        dryb = 0.075;
    }
    else {
        // Set duration factors to Palmer's original duration factors
        drym = .309;
        dryb = 2.691;
    }
    wetm = drym;
    wetb = dryb;


    // Initializes the book keeping indices used in finding the PDSI
    Prob = 0.0;
    X1 = 0.0;
    X2 = 0.0;
    X3 = 0.0;
    X = 0.0;
    V = 0.0;
    Q = 0.0;

    // open file point to bigTable.tbl if necessary
    FILE* table;
    if (extra == 2 || extra == 9) {
        table = fopen("bigTable.tbl", "w");
        if (table == NULL) {
            if (verbose > 0)
                printf("Error opening file \"bigTable.tbl\"\n");
        }
        else {
            //write column headers
            if (Weekly) {
                fprintf(table, "YEAR  WEEK      Z     %Prob     ");
                fprintf(table, "X1       X2      X3\n");
            }
            else {
                fprintf(table, "YEAR  MONTH     Z     %Prob     ");
                fprintf(table, "X1       X2      X3\n");
            }
        }
    }
    else
        table = NULL;
    // Reads in all previously calclulated d values and calculates Z
    // then calls CalcX to compute the corresponding PDSI value
    while ((fscanf(inputd, "%d %d %f", &year, &month, &dtemp)) != EOF) {
        PeriodList.insert(month);
        YearList.insert(year);
        d = dtemp;
        month--;
        K = (17.67 / DKSum) * k[month];
        if (d != MISSING)
            Z = d * K;
        else
            Z = MISSING;

        ZIND.insert(Z);
        CalcOneX(table, month, year);
    }
    fclose(inputd);
    if (table)
        fclose(table);
    // Now that all calculations have been done they can be output to the screen
    if (verbose > 1) {
        int i;
        if (Weekly)
            printf ("STATION = %5d %24c PARAMETERS AND MEANS OF WEEKLY VALUE FOR %d YEARS\n\n", 0, ' ', totalyears);
        else
            printf ("STATION = %5d %24c PARAMETERS AND MEANS OF MONTHLY VALUE FOR %d YEARS\n\n", 0, ' ', totalyears);
        printf ("%4s %8s %8s %8s %8s %8s %7s %8s", "MO", "ALPHA", "BETA", "GAMMA", "DELTA", "K", "P", "S");
        printf ("%9s %8s %8s %8s %8s %8s %8s\n\n", "PR", "PE", "PL", "ET", "R", "L", "RO");
        for (i = 0; i < num_of_periods; i++) {
            printf ("%4d %8.4f %8.4f %8.4f %8.4f", (period_length * i) + 1, Alpha[i], Beta[i], Gamma[i], Delta[i]);
            printf ("%9.3f %8.2f %8.2f %8.2f", 17.67 / DKSum * k[i], PSum[i] / totalyears, PROSum[i] / totalyears, PRSum[i] / totalyears);
            printf ("%9.2f %8.2f %8.2f %8.2f", PESum[i] / totalyears, PLSum[i] / totalyears, ETSum[i] / totalyears, RSum[i] / totalyears);
            printf ("%9.2f %8.2f\n", LSum[i] / totalyears, ROSum[i] / totalyears);
        }
        printf ("\n\n\n%4s %8s %8s %8s %8s %8s\n\n", "PER", "D-ABS", "SIG-D", "DEP", "S-DEP", "SIG-S");
        for (i = 0; i < num_of_periods; i++) {
            printf ("%4d %8.3f %8.2f %8.2f ", (period_length * i) + 1, D[i], sqrt(DSSqr[i] / (totalyears - 1)), DEPSum[i] / totalyears);
            if (i == 7) {
                number E, DE;
                E = SD / totalyears;
                DE = sqrt((SD2 - E * SD) / (totalyears - 1));
                printf ("%8.2f %8.2f", E, DE);
            }
            printf ("\n");
        }
    }
}//end of CalcOrigK()

void pdsi::CalcZ() {

    int year, per;
    float dtemp;
    llist tempZ, tempPer, tempyear;
    DKSum = 0.0; //sum of all D[i] and k[i]; used to calc K

    FILE * inputd; // File pointer for input file dvalue
    // The dvalue file is open for reading.
    if ((inputd = fopen("dvalue", "r")) == NULL) {
        if (verbose > 0)
            printf ("Error reading the file with d values.\n");
        exit(1);
    }

    for (per = 0; per < num_of_periods; per++)
        DKSum += D[per] * k[per];

    // Reads in all previously calclulated d values and calculates Z
    // then calls CalcX to compute the corresponding PDSI value
    while ((fscanf(inputd, "%d %d %f", &year, &per, &dtemp)) != EOF) {
        PeriodList.insert(per);
        YearList.insert(year);
        per = (per - 1) / period_length; //adjust for use in arrays

        d = dtemp;
        K = k[per];
        if (d != MISSING) {
            // now that K and d have both been calculated for this per,
            // Z can be computed.
            Z = d * k[per];
        }
        else {
            Z = MISSING;
        }
        ZIND.insert(Z);
    }
    fclose(inputd);
}//end of CalcZ()
void pdsi::Calibrate() {

    llist tempZ, tempweek, tempyear;
    double cal_range;
    int size;

    /* SG 6/5/06: Changes made to set dry_ratio and wet_ratio based only
    **            on values in the user-defined calibration interval.
    **            If no calibration interval is set, then full history
    **            is used
    */
    llist tmpXlist; // temporary copy of Xlist, which is unscaled PDSI values
    if ((setCalibrationStartYear == 1) || (setCalibrationEndYear == 1)) {
        copy(tmpXlist, Xlist);

        /* SG 6/5/06: Remove the periods from the list until we get to the
        **            start of the calibration interval
        */
        for (int i = 0; (i < nStartPeriodsToSkip) && (!tmpXlist.is_empty()) ; i++) {
            tmpXlist.tail_remove(); /* remove periods before the start of the interval */
        }
        /* SG 6/5/06: We now have a list that begins at the calibration interval.
        **            However, if the list has more periods than the length of the
        **            calibration interval, we must be sure to not go past the
        **            calibration interval length.
        **
        **            So, we now remove the periods from the head of list until we get to the
        **            end of the calibration interval.
        */
        for (int i = 0; (i < nEndPeriodsToSkip) && (!tmpXlist.is_empty()) ; i++) {
            tmpXlist.head_remove(); /* remove periods before the end of the interval */
        }

        /* SG 6/5/06: Now verify that we have the correct number of periods left in the temp Xlist.
        **            If not, print a warning and continue.  The result will be off, but we not much
        **            we can do. So it is better to print the warning and finish.  This is mostly
        **            a debug statement monitoring a runtime error that should never happen.
        */
        size = tmpXlist.get_size();
        if (nCalibrationPeriods != size) {
            //printf("Warning: invalid calibration peiords left in Xlist:\n");
            //printf("\t Calibration periods left = %d, Xlist size = %d.\n", nCalibrationPeriods, size);
        }
        //calibrate using upper and lower 2% of values within the user-defined calibration interval
        cal_range = 4.0;
        dry_ratio = (-cal_range / tmpXlist.safe_percentile(0.02));
        wet_ratio = (cal_range / tmpXlist.safe_percentile(0.98));
        /* SG 6/5/06: That ends the changes needed for implementing self-calibration intervals. */
    } else {
        //calibrate using upper and lower 2%
        cal_range = 4.0;
        dry_ratio = (-cal_range / Xlist.safe_percentile(0.02));
        wet_ratio = (cal_range / Xlist.safe_percentile(0.98));
    }

    // K_d = K_d * dry_ratio;
    // K_w = K_w * wet_ratio;

    if (verbose > 1) {
        //printf("adjusting Z-index using:\n");
        //printf("dry ratio = %.2f/",-cal_range);
        //printf("%f = %f\n",-cal_range/dry_ratio,dry_ratio);
        //printf("wet ratio = %.2f/",cal_range);
        //printf("%f = %f\n",cal_range/wet_ratio,wet_ratio);
    }

    //adjust the Z-index values
    while (!ZIND.is_empty()) {
        Z = ZIND.tail_remove();
        if (Z != MISSING) {
            if (Z >= 0)
                Z = Z * wet_ratio;
            else
                Z = Z * dry_ratio;
        }
        tempZ.insert(Z);
    }
    copy(ZIND, tempZ);

    CalcX();
}//end of calibrate()

void pdsi::CalcX() {
    llist tempZ, tempPer, tempYear;
    int year, per;
    FILE* table;
    if (extra == 2 || extra == 9) {
        table = fopen("bigTable.tbl", "w");
        if (table == NULL) {
            if (verbose > 0)
                printf("Error opening file \"bigTable.tbl\"\n");
        }
        else {
            // writes the column headers to bigTable.tbl if needed
            if (Weekly) {
                fprintf(table, "YEAR  WEEK      Z     %Prob     ");
                fprintf(table, "X1       X2      X3\n");
            }
            else {
                fprintf(table, "YEAR  MONTH     Z     %Prob     ");
                fprintf(table, "X1       X2      X3\n");
            }
        }
    }
    else
        table = NULL;

    //empty all X lists
    while (!Xlist.is_empty())
        Xlist.head_remove();
    while (!XL1.is_empty())
        XL1.head_remove();
    while (!XL2.is_empty())
        XL2.head_remove();
    while (!XL3.is_empty())
        XL3.head_remove();
    while (!altX1.is_empty())
        altX1.head_remove();
    while (!altX2.is_empty())
        altX2.head_remove();
    while (!ProbL.is_empty())
        ProbL.head_remove();

    // Initializes the book keeping indices used in finding the PDSI
    Prob = 0.0;
    X1 = 0.0;
    X2 = 0.0;
    X3 = 0.0;
    X = 0.0;
    V = 0.0;
    Q = 0.0;

    copy(tempZ, ZIND);
    copy(tempPer, PeriodList);
    copy(tempYear, YearList);
    while (!tempZ.is_empty()) {
        Z = tempZ.tail_remove();
        per = (int)tempPer.tail_remove();
        year = (int)tempYear.tail_remove();

        CalcOneX(table, per, year);
    }
    if (table)
        fclose(table);
}//end of CalcX()
//-----------------------------------------------------------------------------
// This function calculates X, X1, X2, and X3
//
// X1 = severity index of a wet spell that is becoming "established"
// X2 = severity index of a dry spell that is becoming "established"
// X3 = severity index of any spell that is already "established"
//
// newX is the name given to the pdsi value for the current week.
// newX will be one of X1, X2 and X3 depending on what the current
// spell is, or if there is an established spell at all.
//-----------------------------------------------------------------------------
void pdsi::CalcOneX(FILE* table, int period_number, int year) {
    number newV;    //These variables represent the values for
    number newProb; //corresponding variables for the current period.
    number newX;    //They are kept seperate because many calculations
    number newX1;   //depend on last period's values.
    number newX2;
    number newX3;
    number ZE;      //ZE is the Z value needed to end an established spell

    number m, b, c;

    flag wd;        //wd is a sign changing flag.  It allows for use of the same
    //equations during both a wet or dry spell by adjusting the
    //appropriate signs.

    if (X3 >= 0) {
        m = wetm;
        b = wetb;
    }
    else {
        m = drym;
        b = dryb;
    }
    c = 1 - (m / (m + b));


    if (Z != MISSING) {
        // This sets the wd flag by looking at X3
        if (X3 >= 0) wd = 1;
        else wd = -1;
        // If X3 is 0 then there is no reason to calculate Q or ZE, V and Prob
        // are reset to 0;
        if (X3 == 0) {
            newX3 = 0;
            newV = 0;
            newProb = 0;
            ChooseX(newX, newX1, newX2, newX3, bug);
        }
        // Otherwise all calculations are needed.
        else {
            newX3 = (c * X3 + Z / (m + b));
            ZE = (m + b) * (wd * 0.5 - c * X3);
            Q = ZE + V;
            newV = Z - wd * (m * 0.5) + wd * min(wd * V + tolerance, 0);

            if ((wd * newV) > 0) {
                newV = 0;
                newProb = 0;
                newX1 = 0;
                newX2 = 0;
                newX = newX3;
                while (!altX1.is_empty())
                    altX1.head_remove();
                while (!altX2.is_empty())
                    altX2.head_remove();
            }
            else {
                newProb = (newV / Q) * 100;
                if (newProb >= 100 - tolerance) {
                    newX3 = 0;
                    newV = 0;
                    newProb = 100;
                }
                ChooseX(newX, newX1, newX2, newX3, bug);
            }
        }

        if (table != NULL) {
            //output stuff to a table
            //year, period, z, newProb, newX1, newX2, newX3
            fprintf(table, "%5d %5d %7.2f %7.2f ", year, period_number, Z, newProb);
            fprintf(table, "%7.2f %7.2f %7.2f\n", newX1, newX2, newX3);
        }

        //update variables for next month:
        V = newV;
        Prob = newProb;
        X1 = newX1;
        X2 = newX2;
        X3 = newX3;

        //add newX to the list of pdsi values
        Xlist.insert(newX);
        XL1.insert(X1);
        XL2.insert(X2);
        XL3.insert(X3);
        ProbL.insert(Prob);
    }
    else {
        //This month's data is missing, so output MISSING as PDSI.
        //All variables used in calculating the PDSI are kept from
        //the previous month.  Only the linked lists are changed to make
        //sure that if backtracking occurs, a MISSING value is kept
        //as the PDSI for this month.

        if (table != NULL) {
            //output stuff to a table
            //year, period, z, newProb, newX1, newX2, newX3
            fprintf(table, "%5d %5d %7.2f %7.2f ", year, period_number, Z, MISSING);
            fprintf(table, "%7.2f %7.2f %7.2f\n", MISSING, MISSING, MISSING);
        }
        Xlist.insert(MISSING);
        XL1.insert(MISSING);
        XL2.insert(MISSING);
        XL3.insert(MISSING);
        ProbL.insert(MISSING);
    }

}//end of CalcOneX
//-----------------------------------------------------------------------------
// This function calculates the sum of all actual and potential values for each
// indivual period over the given period of years.  To do this it must calculate
// these values for each period in the period.  The potential values are then
// stored for future use.
//-----------------------------------------------------------------------------
void pdsi::SumAll() {
    FILE * fout;
    FILE * input_temp, *input_prec;
    char Temp[150], Precip[150];
    int actyear;
    number DEP = 0;
    SD = 0;
    SD2 = 0;
    /* SG 6/5/06: add variable to support a calibration interval */
    int nCalibrationPeriodsLeft = nCalibrationPeriods; /* init periods left */

    // Initializes the sums to 0;
    for (int i = 0; i < 52; i++) {
        ETSum[i] = 0;
        RSum[i] = 0;
        LSum[i] = 0;
        ROSum[i] = 0;
        PSum[i] = 0;
        PESum[i] = 0;
        PRSum[i] = 0;
        PLSum[i] = 0;
        PROSum[i] = 0;
    }

    // Opens the temperature and precipitation files for reading the input and
    // potentials file for temparary storage.

    if ((fout = fopen("potentials", "w")) == NULL) {
        if (verbose > 0)
            printf ("Error in opening temporary storage file for potentials.\n");
        exit(1);
    }

    if (Weekly) {

        if (strlen(input_dir) > 1) {
            sprintf(Temp, "%s%s", input_dir, "weekly_T");
            sprintf(Precip, "%s%s", input_dir, "weekly_P");
        }
        else {
            strcpy(Temp, "weekly_T");
            strcpy(Precip, "weekly_P");
        }

        if ((input_temp = fopen(Temp, "r")) == NULL) {
            if (verbose > 0) {
                printf ("Error reading temperature file.\n");
                printf ("filename = %s\n", Temp);
            }
            exit(1);
        }
        if ((input_prec = fopen(Precip, "r")) == NULL) {
            if (verbose > 0) {
                printf ("Error reading precipitation file.\n");
                printf ("filename = %s\n", Precip);
            }
            exit(1);
        }
        //print column headers:
        fprintf(fout, " Year  Week        P         PE         PR         PRO");
        fprintf(fout, "        PL        P-PE \n");
    }
    else if (Monthly || SCMonthly) {

        if (strlen(input_dir) > 1) {
            sprintf(Temp, "%s%s", input_dir, "monthly_T");
            sprintf(Precip, "%s%s", input_dir, "monthly_P");
        }
        else {
            strcpy(Temp, "monthly_T");
            strcpy(Precip, "monthly_P");
        }

        if ((input_temp = fopen(Temp, "r")) == NULL) {
            if (verbose > 0) {
                printf ("Error reading temperature file.\n");
                printf ("filename = %s\n", Temp);
            }
            exit(1);
        }
        if ((input_prec = fopen(Precip, "r")) == NULL) {
            if (verbose > 0) {
                printf ("Error reading precipitation file.\n");
                printf ("filename = %s\n", Precip);
            }
            exit(1);
        }
        //print column headers:
        fprintf(fout, " Year  MONTH       P         PE         PR         PRO");
        fprintf(fout, "        PL        P-PE \n");

    }
    else {
        if (verbose) {
            printf("Error.  Invalid type of PDSI calculation\n");
            printf("Either the 'Weekly', 'Monthly', or 'SCMonthly' flags must be set.\n");
        }
        exit(1);
    }

    // This loop runs to read in and calculate the values for all years
    for (int year = 1; year <= totalyears; year++) {
        // Get a year's worth of temperature and precipitation data
        // Also, get the current year from the temperature file.

        if (Weekly) {
            actyear = GetTemp(input_temp, T, 52);
            GetPrecip(input_prec, P, 52);
        }
        else {
            actyear = GetTemp(input_temp, T, 12);
            GetPrecip(input_prec, P, 12);
        }

        // This loop runs for each per in the year
        for (int per = 0; per < num_of_periods; per++) {
            if (P[per] >= 0 && T[per] != MISSING) {
                // calculate the Potential Evapotranspiration first
                // because it's needed in later calculations
                if (Weekly)
                    CalcWkPE(per, actyear);
                else
                    CalcMonPE(per, actyear);

                CalcPR();         // calculate Potential Recharge, Potential Runoff,
                CalcPRO();        // and Potential Loss
                CalcPL();
                CalcActual(per);  // Calculate Evapotranspiration, Recharge, Runoff,
                // and Loss

                // Calculates some statistical variables for output
                // to the screen in the most verbose mode (verbose > 1)
                if (Weekly) {
                    if (per > (17 / period_length) && per < (35 / period_length)) {
                        DEP = DEP + P[per] + L - PE;
                        if (per > (30 / period_length) && per < (35 / period_length)) {
                            SD = SD + DEP;
                            SD2 = SD2 + DEP * DEP;
                            DEP = 0;
                        }
                    }
                }
                else {
                    if (per > 4 && per < 8) {
                        DEP = DEP + P[per] + L - PE;
                        if (per == 7) {
                            SD = SD + DEP;
                            SD2 = SD2 + DEP * DEP;
                            DEP = 0;
                        }
                    }
                }

                /* SG 6/5/06: add code to support a calibration interval */
                /* SG 6/4/06: Allow for user-defined calibration interval by not Summing during
                **            years before the calibration interval or after the end of the calibration
                **            interval.
                */
                if (year > nStartYearsToSkip) {
                    if (nCalibrationPeriodsLeft > 0) { /* Within the calibration interval, so update Sums */
                        // Reduce number of calibration years left by one for each year actually summed
                        nCalibrationPeriodsLeft--;

                        // Update the sums by adding the current water balance values
                        ETSum[per] += ET;
                        RSum[per] += R;
                        ROSum[per] += RO;
                        LSum[per] += L;
                        PSum[per] += P[per];
                        PESum[per] += PE;
                        PRSum[per] += PR;
                        PROSum[per] += PRO;
                        PLSum[per] += PL;
                    }
                }

                //P, PE, PR, PRO, and PL will be used later, so
                //these variables need to be stored to an outside file
                //where they can be accessed at a later time
                fprintf(fout, "%5d %5d %10.6f ", actyear, (period_length * per) + 1, P[per]);
                fprintf(fout, "%10.6f %10.6f %10.6f %10.6f ", PE, PR, PRO, PL);
                fprintf(fout, "%10.6f\n", P[per] - PE);
            }//matches if(P[per]>= 0 && T[per] != MISSING)
            else {
                fprintf(fout, "%5d %5d %f ", actyear, (period_length * per) + 1, MISSING);
                fprintf(fout, "%10.6f %10.6f %10.6f ", MISSING, MISSING, MISSING);
                fprintf(fout, "%10.6f %10.6f\n", MISSING, MISSING);
            }
        }//end of period loop
    }//end of year loop

    // We are done with these files for now so close them
    fclose(fout);
    fclose(input_temp);
    fclose(input_prec);
}
//-----------------------------------------------------------------------------
//CalcCMI is a lot like SumAll
//Equations used come from a Memorandum dated March 29, 1968
// from: H. B. Harshbarger, Director, Office of Field Services,
//                          Environmental Data Service,
//                          Environmental Science Services Administation
//                          U.S. Deptartment of Commerce
// to:   All Regional and State Climatologists
// Subject: A Crop Moisture Map to be published in the weekly weather and crop
//          bulletin, national summary.
//-----------------------------------------------------------------------------
void pdsi::CalcCMI() {
    FILE * fout;
    FILE * input_temp, *input_prec;
    char Temp[150], Precip[150];
    int actyear;

    number DE;          //relative evapotranspiration anomaly for the week
    number Yprime;      //1st approx to Y
    number old_Yprime;  //last week's Yprime;
    number Y;           //index of evapotranspiration deficit
    number M;           //average % of field capacity
    number G;           //index of excessive moisture
    number old_G;       //last week's G
    number H;           //a 'return to normal' factor
    number CMI;         //the CMI.

    //initialize soil values back to initial value
    // (they were changed when SumAll was run to calc Alpha)
    Ss = 1;
    Su = AWC - Ss;
    if (Su < 0)
        Su = 0;

    // Initializes the sums to 0;
    for (int i = 0; i < 52; i++) {
        ETSum[i] = 0;
        RSum[i] = 0;
        LSum[i] = 0;
        ROSum[i] = 0;
        PSum[i] = 0;
        PESum[i] = 0;
        PRSum[i] = 0;
        PLSum[i] = 0;
        PROSum[i] = 0;
    }

    // open output file.
    if ((fout = fopen("CMI_calc.tbl", "w")) == NULL) {
        if (verbose > 0)
            printf ("Error in opening temporary storage file for potentials.\n");
        exit(1);
    }

    // Opens the temperature and precipitation files for reading the input
    if (strlen(input_dir) > 1) {
        sprintf(Temp, "%s%s", input_dir, "weekly_T");
        sprintf(Precip, "%s%s", input_dir, "weekly_P");
    }
    else {
        strcpy(Temp, "weekly_T");
        strcpy(Precip, "weekly_P");
    }

    if ((input_temp = fopen(Temp, "r")) == NULL) {
        if (verbose > 0) {
            printf ("Error reading temperature file.\n");
            printf ("filename = %s\n", Temp);
        }
        exit(1);
    }
    if ((input_prec = fopen(Precip, "r")) == NULL) {
        if (verbose > 0) {
            printf ("Error reading precipitation file.\n");
            printf ("filename = %s\n", Precip);
        }
        exit(1);
    }

    //print column headers
    fprintf(fout, "%5s %5s %7s %7s %7s %7s %7s %7s %7s %7s %7s ",
            "year", "week", "PET", "ET", "Alpha", "R", "RO", "Ss", "Su", "M", "DE");
    fprintf(fout, "%7s %7s %7s %7s %7s\n",
            "Yprime", "Y", "H", "G", "CMI");

    old_G = 0;
    old_Yprime = 0;

    // This loop runs to read in and calculate the values for all years
    for (int year = 1; year <= totalyears; year++) {
        // Get a year's worth of temperature and precipitation data
        // Also, get the current year from the temperature file.

        actyear = GetTemp(input_temp, T, 52);
        GetPrecip(input_prec, P, 52);

        // This loop runs for each per in the year
        for (int per = 0; per < num_of_periods; per++) {
            if (P[per] >= 0 && T[per] != MISSING) {
                // calculate the Potential Evapotranspiration first
                // because it's needed in later calculations
                CalcWkPE(per, actyear);
                CalcPR();         // calculate Potential Recharge, Potential Runoff,
                CalcPRO();        // and Potential Loss
                CalcPL();
                CalcActual(per);  // Calculate Evapotranspiration, Recharge, Runoff,
                // and Loss

                //this formula is the original equation:
                //M = (PRO + Ss + Su) / (2 * (PRO+PR));
                //given PRO = (Ss + Su) and PR = AWC - PRO,
                //it reduces to this, which makes more sense:
                M = (Ss + Su) / AWC;

                DE = (ET - Alpha[per] * PE) / sqrt(Alpha[per]);
                Yprime = 0.67 * old_Yprime + 1.8 * DE;
                if (Yprime < 0)
                    Y = Yprime;
                else
                    Y = M * Yprime;

                if (old_G == 0)
                    H = 0;
                else if (old_G < 0.5)
                    H = old_G;
                else if (old_G < 1.0)
                    H = 0.5;
                else
                    H = 0.5 * old_G;

                G = old_G - H + (M * R) + RO;

                CMI = Y + G;

                old_Yprime = Yprime;
                old_G = G;
            }
            else {
                CMI = MISSING;
            }

            CMIList.insert(CMI);
            YearList.insert(actyear);
            PeriodList.insert(per + 1);

            //output values to file
            fprintf(fout, "%5d %5d %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f ",
                    actyear, per + 1, PE, ET, Alpha[per], R, RO, Ss, Su, M, DE);
            fprintf(fout, "%7.4f %7.4f %7.4f %7.4f %7.4f\n",
                    Yprime, Y, H, G, CMI);


        }//end of period loop
    }//end of year loop

    // We are done with these files for now so close them
    fclose(fout);
    fclose(input_temp);
    fclose(input_prec);
}
//-----------------------------------------------------------------------------
// CalcPR calculates the Potential Recharge of the soil for one period of the
// year being examined.  PR = Soils Max Capacity - Soils Current Capacity or
// AWC - (SU + Ss)
//-----------------------------------------------------------------------------
void pdsi::CalcPR() {
    PR = AWC - (Su + Ss);
}
//-----------------------------------------------------------------------------
// CalcPRO calculates the Potential Runoff for a given period of the year being
// examined.  PRO = Potential Precip - PR. Palmer arbitrarily set the Potential
// Precip to the AWC making PRO = AWC - (AWC - (Su + Ss)). This then simplifies
// to PRO = Su + Ss
//-----------------------------------------------------------------------------
void pdsi::CalcPRO() {
    PRO = Ss + Su;
}
//-----------------------------------------------------------------------------
// CalcPL calculates the Potential Loss of moisture in the soil for a period of
// one period of the year being examined. If the Ss capacity is enough to
// handle all PE, PL is simple PE.  Otherwise, potential loss from Su occurs at
// the rate of (PE-Ss)/AWC*Su.  This means PL = Su*(PE - Ss)/AWC + Ss
//-----------------------------------------------------------------------------
void pdsi::CalcPL() {
    if (Ss >= PE)
        PL = PE;
    else {
        PL = ((PE - Ss) * Su) / (AWC) + Ss;
        if (PL > PRO) // If PL>PRO then PL>water in the soil.  This isn't
            PL = PRO;   // possible so PL is set to the water in the soil
    }
}
//-----------------------------------------------------------------------------
// CalcActual calculates the actual values of evapotranspiration,soil recharge,
// runoff, and soil moisture loss.  It also updates the soil moisture in both
// layers for the next period depending on current weather conditions.
//-----------------------------------------------------------------------------
void pdsi::CalcActual(int per) {
    number R_surface = 0.0;   // recharge of the surface layer
    number R_under = 0.0;    // recharge of the underlying layer
    number surface_L = 0.0;   // loss from surface layer
    number under_L = 0.0;    // loss from underlying layer
    number new_Su, new_Ss;    // new soil moisture values


    if (P[per] >= PE) {
        // The precipitation exceeded the maximum possible evapotranspiration
        // (excess moisture)
        ET = PE;   // Enough moisture for all potential evapotranspiration to occur
        L = 0.0;   // with no actual loss of soil moisture

        if ((P[per] - PE) > (1.0 - Ss)) {
            // The excess precip will recharge both layers. Note: (1.0 - SS) is the
            // amount of water needed to saturate the top layer of soil assuming it
            // can only hold 1 in. of water.
            R_surface = 1.0 - Ss;
            new_Ss = 1.0;

            if ((P[per] - PE - R_surface) < ((AWC - 1.0) - Su)) {
                // The entire amount of precip can be absorbed by the soil (no runoff)
                // and the underlying layer will receive whats left after the top layer
                // Note: (AWC - 1.0) is the amount able to be stored in lower layer
                R_under = (P[per] - PE - R_surface);
                RO = 0.0;
            }
            else {
                // The underlying layer is fully recharged and some runoff will occur
                R_under = (AWC - 1.0) - Su;
                RO = P[per] - PE - (R_surface + R_under);
            }
            new_Su = Su + R_under;
            R = R_surface + R_under;//total recharge
        }
        else {
            // There is only enough moisture to recharge some of the top layer.
            R = P[per] - PE;
            new_Ss = Ss + R;
            new_Su = Su;
            RO = 0.0;
        }
    }// End of if(P[per] >= PE)
    else {
        // The evapotranspiration is greater than the precipitation received.  This
        // means some moisture loss will occur from the soil.
        if (Ss > (PE - P[per])) {
            // The moisture from the top layer is enough to meet the remaining PE so
            // only the top layer losses moisture.
            surface_L = PE - P[per];
            under_L = 0.0;
            new_Ss = Ss - surface_L;
            new_Su = Su;
        }
        else {
            // The top layer is drained, so the underlying layer loses moisture also.
            surface_L = Ss;
            under_L = (PE - P[per] - surface_L) * Su / AWC;
            if (Su < under_L)
                under_L = Su;
            new_Ss = 0.0;
            new_Su = Su - under_L;
        }
        R = 0;// No recharge occurs
        L = under_L + surface_L;// Total loss
        RO = 0.0;// No extra moisture so no runoff
        ET = P[per] + L;// Total evapotranspiration
    }
    Ss = new_Ss;//update soil moisture values
    Su = new_Su;
}//end of CalcActual(int per)
//-----------------------------------------------------------------------------
// This function calculates Alpha, Beta, Gamma, and Delta, the normalizing
// climate coefficients in the water balance equation.
// If the user desires, the results are output to the screen and a file.
//-----------------------------------------------------------------------------
void pdsi::CalcWBCoef() {

    FILE *wb;

    // The coefficients are calculated by per
    for (int per = 0; per < num_of_periods; per++) {

        //calculate alpha:
        if (PESum[per] != 0.0)
            Alpha[per] = ETSum[per] / PESum[per];
        else if (ETSum[per] == 0.0)
            Alpha[per] = 1.0;
        else
            Alpha[per] = 0.0;

        //calculate beta:
        if (PRSum[per] != 0.0)
            Beta[per] = RSum[per] / PRSum[per];
        else if (RSum[per] == 0.0)
            Beta[per] = 1.0;
        else
            Beta[per] = 0.0;

        //calculate gamma:
        if (PROSum[per] != 0.0)
            Gamma[per] = ROSum[per] / PROSum[per];
        else if (ROSum[per] == 0.0)
            Gamma[per] = 1.0;
        else
            Gamma[per] = 0.0;

        //calculate delta:
        if (PLSum[per] != 0.0)
            Delta[per] = LSum[per] / PLSum[per];
        else
            Delta[per] = 0.0;
    }

    if (extra == 1 || extra == 9) {
        //output water balance coefficients
        wb = fopen("WB.tbl", "w");
        fprintf(wb, "PERIOD   ALPHA     BETA    GAMMA    DELTA\n");
        if (verbose > 1)
            printf ("\nPERIOD   ALPHA     BETA    GAMMA    DELTA\n");
        for (int i = 0; i < num_of_periods; i++) {
            fprintf(wb, "%3d %10.4f %8.4f %8.4f %8.4f \n", (period_length * i) + 1, Alpha[i], Beta[i], Gamma[i], Delta[i]);
            if (verbose > 1)
                printf ("%3d %10.4f %8.4f %8.4f %8.4f \n", (period_length * i) + 1, Alpha[i], Beta[i], Gamma[i], Delta[i]);
        }
        fclose(wb);
    }
}//end CalcWBCoef()
//-----------------------------------------------------------------------------
// The Write() function will write the PDSI to the default directory.
// If it is a weekly PDSI, it will go to the "weekly/" directory
// If it is a monthly PDSI, it will go to the "monthly/" directory
// If it is a sc_monthly PDSI, it will go to the "sc_monthly/" directory
//-----------------------------------------------------------------------------
void pdsi::Write() {
    char full_path[128];

    if (Xlist.is_empty()) {
        if (CMIList.is_empty()) {
            if (verbose > 1)
                printf("No PDSI or CMI values have been calculated.\n");
            return;
        }
        else {
            Write("weekly/CMI/");
        }
    }
    if (Weekly) {
        sprintf(full_path, "weekly/%d/", period_length);
        Write(full_path);
    }
    else if (Monthly)
        Write("monthly/original");
    else if (SCMonthly)
        Write("monthly/self_cal");

}
//-----------------------------------------------------------------------------
// The Write(char* directory) function will write the PDSI to the specified
// directory.  If the directory does not exist, it will create it.
// The directory path should be relative to the output directory (if any)that
// was specified as a command line argument.
// If no output directory  was specified on the command line, the path will
// be treated as a relative path to the current directory.
//-----------------------------------------------------------------------------
void pdsi::Write(char *directory) {

    unsigned int i = 0;
    int e = 0;                            //error flag
    char base[128];                       //string for base dir path
    char my_dir[128];                     //string for directory
    char full_path[256];                  //string for full path to dir

    if (Xlist.is_empty() && CMIList.is_empty() ) {
        if (verbose > 1)
            printf("No PDSI or CMI values have been calculated.\n");
        return;
    }

    strcpy(my_dir, directory);
    //make sure last character of directory is '/'
    if (my_dir[strlen(my_dir) - 1] != '/')
        strcat(my_dir, "/");

    if (strlen(output_dir) > 1)
        strcpy(base, output_dir);
    else
        strcpy(base, "./");

    sprintf(full_path, "%s%s", base, my_dir);
    switch (create_dir(full_path)) {
    case -1:
        //the directory was not created
        if (verbose > 0)
            printf("Error creating directory: %s \n", full_path);
        //replace all '/' with '_' in the directory
        for (i = 0; i < strlen(my_dir); i++) {
            if (my_dir[i] == '/')
                my_dir[i] = '_';
        }
        sprintf(full_path, "%s%s", base, my_dir);
        if (verbose > 0)
            printf("Output files will now have the prefix: %s \n\n", my_dir);
        break;

    case 0:
        //the directory was successfully created
        if (verbose > 1)
            printf("Outputting to the new directory: %s\n\n", full_path);
        break;

    case 1:
        //the directory already exists
        if (verbose > 1)
            printf("Outputting to existing directory: %s\n\n", full_path);
        break;
    }

    //finally, call FinalWrite().  If no errors occurred, the string
    //dir will be the correct directory (ending in '/').  Otherwise,
    //it will be the directory structure with all '/' replaced by '_'.

    if (Xlist.is_empty()) {
        WriteCMI(full_path);
        MoveCMIFiles(full_path);
    }
    else {
        FinalWrite(full_path);
        MoveFiles(full_path);
    }
}
void pdsi::WriteCMI(char* dir) {
    int cyear = startyear;
    int prev, cur, saved_per, change_per;
    number c;
    llist tempCMI, tempweek;
    FILE *cmi0, *cmi1;
    char filename[80];

    if (CMIList.is_empty()) {
        if (verbose > 0)
            printf("no CMI values have been calculated\n");
        return;
    }

    sprintf(filename, "%s%s", dir, "CMI.tbl");
    remove(filename);
    if (output_mode == 0 || output_mode == 2) {
        cmi0 = fopen(filename, "w");
    }

    sprintf(filename, "%s%s", dir, "CMI.clm");
    remove(filename);
    if (output_mode == 1 || output_mode == 2) {
        cmi1 = fopen(filename, "w");
    }

    copy(tempCMI, CMIList);
    copy(tempweek, PeriodList);
    change_per = 1;
    prev = 0;
    while (!tempCMI.is_empty() && cyear <= e_year) {
        if (change_per)
            cur = (int)tempweek.tail_remove();
        else
            cur = saved_per;

        //check to make sure there is not missing values
        //by making sure the data is continuous
        if ((prev == 0 && cur == 1) || cur == prev + period_length || cur == 1 && ( (Weekly && prev == (52 - period_length + 1)) || ((Monthly || SCMonthly) && prev == (12 - period_length + 1)) ) ) {
            change_per = 1;

            c = tempCMI.tail_remove();
        }
        else {
            if (verbose > 1)
                printf("missing data before period %d in %d\n", cur, cyear);
            saved_per = cur;
            cur = prev + period_length;
            change_per = 0;
            c = MISSING;
        }
        if (cur == 1) {
            if ((output_mode == 0 || output_mode == 2) && cyear >= s_year) {
                fprintf (cmi0, "%5d", cyear);
            }
        }
        if ((output_mode == 1 || output_mode == 2) && cyear >= s_year) {
            fprintf (cmi1, "%5d%5d%7.2f\n", cyear, cur, c);
        }
        if ((output_mode == 0 || output_mode == 2) && cyear >= s_year) {
            fprintf (cmi0, "%7.2f", c);
        }
        if (cur >= 52 - period_length + 1) {
            // Outputs a newline after a years worth of data in the table files
            if ((output_mode == 0 || output_mode == 2) && cyear >= s_year) {
                fprintf (cmi0, "\n");
            }
            cyear++;
            cur = 1 - period_length;
        }
        prev = cur;
    }
    if (verbose > 0)
        printf("%4s Weekly CMI written to %s\n", "*", dir);
}
//-----------------------------------------------------------------------------
// The FinalWrite function creates 4 or 8 output files based on the output_mode
// flag.  These files are the PDSI, the hydro PDSI (PHDI), the Z index (ZIND),
// and the weighted PDSI (WPLM).  The calculations for the PHDI and WPLM are
// also done within this function.
//-----------------------------------------------------------------------------
void pdsi::FinalWrite(char* dir) {
    int cyear = startyear;
    int prev, cur, saved_per, change_per;
    number x, x1, x2, x3, p, wp, ph, z;
    llist tempX, tempX1, tempX2, tempX3, tempZ, tempP, tempweek;
    FILE *pdsi0, *pdsi1, *phdi0, *phdi1, *zind0, *zind1, *wplm0, *wplm1;
    char filename[80];

    if (Xlist.is_empty()) {
        if (verbose > 1)
            printf("No PDSI values have been calculated.\n");
        return;
    }

    //remove any existing output files:
    sprintf(filename, "%s%s", dir, "PDSI.tbl");
    remove(filename);
    sprintf(filename, "%s%s", dir, "PDSI.clm");
    remove(filename);
    sprintf(filename, "%s%s", dir, "PHDI.tbl");
    remove(filename);
    sprintf(filename, "%s%s", dir, "PHDI.clm");
    remove(filename);
    sprintf(filename, "%s%s", dir, "WPLM.tbl");
    remove(filename);
    sprintf(filename, "%s%s", dir, "WPLM.clm");
    remove(filename);
    sprintf(filename, "%s%s", dir, "ZIND.tbl");
    remove(filename);
    sprintf(filename, "%s%s", dir, "ZIND.clm");
    remove(filename);

    // If output_mode is 0 or 2 the table files are opened for writing
    if (output_mode == 0 || output_mode == 2) {
        //open files
        sprintf(filename, "%s%s", dir, "PDSI.tbl");
        pdsi0 = fopen(filename, "w");
        sprintf(filename, "%s%s", dir, "PHDI.tbl");
        phdi0 = fopen(filename, "w");
        sprintf(filename, "%s%s", dir, "WPLM.tbl");
        wplm0 = fopen(filename, "w");
        sprintf(filename, "%s%s", dir, "ZIND.tbl");
        zind0 = fopen(filename, "w");
    }
    // If the output_mode is 1 or 2 the column files are opened for writing
    if (output_mode == 1 || output_mode == 2) {
        //open files
        sprintf(filename, "%s%s", dir, "PDSI.clm");
        pdsi1 = fopen(filename, "w");
        sprintf(filename, "%s%s", dir, "PHDI.clm");
        phdi1 = fopen(filename, "w");
        sprintf(filename, "%s%s", dir, "WPLM.clm");
        wplm1 = fopen(filename, "w");
        sprintf(filename, "%s%s", dir, "ZIND.clm");
        zind1 = fopen(filename, "w");
    }

    // Copies of the needed linked lists are made in order to preserve the lists
    // for future use.
    copy(tempX, Xlist);
    copy(tempX1, XL1);
    copy(tempX2, XL2);
    copy(tempX3, XL3);
    copy(tempZ, ZIND);
    copy(tempP, ProbL);
    copy(tempweek, PeriodList);

    change_per = 1;
    prev = 0;
    // The temp lists are then emptied and output
    while (!tempX.is_empty() && cyear <= e_year) {
        if (change_per)
            cur = (int)tempweek.tail_remove();
        else
            cur = saved_per;

        //check to make sure there is not missing values
        //by making sure the data is continuous
        if ((prev == 0 && cur == 1) || cur == prev + period_length || cur == 1 && ( (Weekly && prev == (52 - period_length + 1)) || ((Monthly || SCMonthly) && prev == (12 - period_length + 1)) ) ) {
            change_per = 1;
            // The last item in the linked list is removed and set to the
            // corresponding variable for calculation of the PHDI and WPLM
            x = tempX.tail_remove();
            x1 = tempX1.tail_remove();
            x2 = tempX2.tail_remove();
            x3 = tempX3.tail_remove();
            p = tempP.tail_remove();
            z = tempZ.tail_remove();

            // The probability is converted from a percentage to a ratio
            p = p / 100;
            // The current PHDI value starts at X3
            ph = x3;
            if (x3 == 0) {
                // There is not an established wet or dry spell so PHDI = PDSI (ph=x)
                // and the WPLM value is the maximum absolute value of X1 or X2
                ph = x;
                wp = x1;
                if (-x2 > (x1 + tolerance))
                    wp = x2;
            }
            else if (p > (0 + tolerance / 100) && p < (1 - tolerance / 100)) {
                // There is an established spell but there is a possibility it has or is
                // ending.  The WPLM is then a weighted average between X3 and X1 or X2
                if (x3 < 0)
                    // X3 is negative so WPLM is weighted average of X3 and X1
                    wp = (1 - p) * x3 + p * x1;
                else
                    // X3 is positive so WPLM is weighted average of X3 and X2
                    wp = (1 - p) * x3 + p * x2;
            }
            else
                // There is an established spell without possibility of end meaning the
                // WPLM is simply X3
                wp = x3;
        }
        else {
            if (verbose > 1)
                printf("missing data before period %d in %d\n", cur, cyear);
            saved_per = cur;
            cur = prev + period_length;
            change_per = 0;
            x = MISSING;
            ph = MISSING;
            z = MISSING;
            wp = MISSING;
        }
        //---------------------------------------
        //beginning of actual output of results:
        //---------------------------------------
        // If printing table files start each line with the year
        if (cur == 1) {
            if ((output_mode == 0 || output_mode == 2) && cyear >= s_year) {
                fprintf (pdsi0, "%5d", cyear);
                fprintf (phdi0, "%5d", cyear);
                fprintf (zind0, "%5d", cyear);
                fprintf (wplm0, "%5d", cyear);
            }
        }
        // Outputs the current values in a column format: Year Week Value
        if ((output_mode == 1 || output_mode == 2) && cyear >= s_year) {
            fprintf (pdsi1, "%5d%5d%7.2f\n", cyear, cur, x);
            fprintf (phdi1, "%5d%5d%7.2f\n", cyear, cur, ph);
            fprintf (zind1, "%5d%5d%7.2f\n", cyear, cur, z);
            fprintf (wplm1, "%5d%5d%7.2f\n", cyear, cur, wp);
        }

        // Outputs the current values in a table format: Year num_of_periodsWeeklyValues
        if ((output_mode == 0 || output_mode == 2) && cyear >= s_year) {
            fprintf (pdsi0, "%7.2f", x);
            fprintf (phdi0, "%7.2f", ph);
            fprintf (zind0, "%7.2f", z);
            fprintf (wplm0, "%7.2f", wp);
        }

        if (Weekly && cur >= 52 - period_length + 1 || (Monthly || SCMonthly) && cur >= 12 - period_length + 1) {
            // Outputs a newline after a years worth of data in the table files
            if ((output_mode == 0 || output_mode == 2) && cyear >= s_year) {
                fprintf (pdsi0, "\n");
                fprintf (phdi0, "\n");
                fprintf (zind0, "\n");
                fprintf (wplm0, "\n");
            }
            cyear++;
            cur = 1 - period_length;
        }
        prev = cur;
    }
    if (verbose > 0) {
        if (Weekly) {
            if (period_length == 1)
                printf("%4s Self-Calibrated Weekly PDSI written to %s\n", "*", dir);
            else if (period_length == 2)
                printf("%4s Self-Calibrated 2-Week PDSI written to %s\n", "*", dir);
            else if (period_length == 4)
                printf("%4s Self-Calibrated 4-Week PDSI written to %s\n", "*", dir);
            else if (period_length == 13)
                printf("%4s Self-Calibrated 13-Week PDSI written to %s\n", "*", dir);
        }
        if (Monthly)
            printf("%4s Monthly PDSI written to %s\n", "*", dir);
        if (SCMonthly)
            printf("%4s Self-Calibrated Monthly PDSI written to %s\n", "*", dir);
    }
}// void pdsi::FinalWrite()
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pdsi::MoveCMIFiles(char* dir) {
    char filename[170];
    sprintf(filename, "%s%s", dir, "CMI_calc.tbl");
    remove(filename);
    if (rename("CMI_calc.tbl", filename) == -1) {
        if (ReprintFile("CMI_calc.tbl", filename) == -1 && verbose > 0)
            printf("unable to rename CMI_calc.tbl as \"%s\".\n", filename);
    }
    sprintf(filename, "%s%s", dir, "potentials");
    remove(filename);
    if (rename("potentials", filename) == -1) {
        if (ReprintFile("potentials", filename) == -1 && verbose > 0)
            printf("Unable to rename potentials file as \"%s\".\n", filename);
    }
    sprintf(filename, "%s%s", dir, "dvalue");
    remove(filename);
    sprintf(filename, "%s%s", dir, "bigTable.tbl");
    remove(filename);
    sprintf(filename, "%s%s", dir, "WB.tbl");
    remove(filename);
    if (rename("WB.tbl", filename) == -1 ) {
        if (ReprintFile("WB.tbl", filename) == -1 && verbose > 0)
            printf("Unable to rename WB.tbl file as \"%s\".\n", filename);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pdsi::MoveFiles(char* dir) {
    char filename[170];
    //remove any files that might already be in the director
    sprintf(filename, "%s%s", dir, "potentials");
    remove(filename);
    sprintf(filename, "%s%s", dir, "dvalue");
    remove(filename);
    sprintf(filename, "%s%s", dir, "bigTable.tbl");
    remove(filename);
    sprintf(filename, "%s%s", dir, "WB.tbl");
    remove(filename);

    //move files
    if (extra == 3 || extra == 9) {
        //move potentials and dvalue
        sprintf(filename, "%s%s", dir, "potentials");
        if (rename("potentials", filename) == -1) {
            if (ReprintFile("potentials", filename) == -1 && verbose > 0)
                printf("Unable to rename potentials file as \"%s\".\n", filename);
        }
        sprintf(filename, "%s%s", dir, "dvalue");
        if (rename("dvalue", filename) == -1) {
            if (ReprintFile("dvalue", filename) == -1 && verbose > 0)
                printf("Unable to rename dvalue file as \"%s\".\n", filename);
        }
    }
    if (extra == 2 || extra == 9) {
        //move bigTable.tbl
        sprintf(filename, "%s%s", dir, "bigTable.tbl");
        if (rename("bigTable.tbl", filename) == -1) {
            if (ReprintFile("bigTable.tbl", filename) == -1 && verbose > 0)
                printf("Unable to rename bigTable.tbl file as \"%s\".\n", filename);
        }
    }
    if (extra == 1 || extra == 9) {
        //move WB.tbl
        sprintf(filename, "%s%s", dir, "WB.tbl");
        if (rename("WB.tbl", filename) == -1 ) {
            if (ReprintFile("WB.tbl", filename) == -1 && verbose > 0)
                printf("Unable to rename WB.tbl file as \"%s\".\n", filename);
        }
    }
}
int pdsi::ReprintFile(char* src, char *des) {
    FILE *in, *out;
    char line[4096];
    in = fopen(src, "r");
    if (in == NULL)
        return -1;
    out = fopen (des, "w");
    if (out == NULL)
        return 1;
    while (fgets(line, 4096, in))
        fprintf(out, "%s", line);
    if (feof(in)) {
        fclose(out);
        fclose(in);
        remove(src);
        return 1;
    }
    else {
        fclose(in);
        fclose(out);
        return -1;
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pdsi::Backtrack(number X1, number X2) {
    number num1, num2;
    node * ptr = NULL;
    num1 = X1;
    while (!altX1.is_empty() && !altX2.is_empty()) {
        if (num1 > 0) {
            num1 = altX1.head_remove();
            num2 = altX2.head_remove();
        }
        else {
            num1 = altX2.head_remove();
            num2 = altX1.head_remove();
        }
        if (-tolerance <= num1 && num1 <= tolerance) num1 = num2;
        ptr = Xlist.set_node(ptr, num1);
    }
}//end of backtrack()
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pdsi::ChooseX(number& newX, number& newX1, number& newX2, number& newX3, int bug)
{
    number m, b;
    number wetc, dryc;

    if (X3 >= 0) {
        m = wetm;
        b = wetb;
    }
    else {
        m = drym;
        b = dryb;
    }

    wetc = 1 - (wetm / (wetm + wetb));
    dryc = 1 - (drym / (drym + wetb));

    newX1 = (wetc * X1 + Z / (wetm + wetb));
    if (newX1 < 0)
        newX1 = 0;
    newX2 = X2;

    if (bug == 0) {
        newX2 = (dryc * X2 + Z / (drym + dryb));
        if (newX2 > 0)
            newX2 = 0;
    }

    if ((newX1 >= 0.5) && (newX3 == 0)) {
        Backtrack(newX1, newX2);
        newX = newX1;
        newX3 = newX1;
        newX1 = 0;
    }
    else {
        newX2 = (dryc * X2 + Z / (drym + dryb));
        if (newX2 > 0)
            newX2 = 0;

        if ((newX2 <= -0.5) && (newX3 == 0)) {
            Backtrack(newX2, newX1);
            newX = newX2;
            newX3 = newX2;
            newX2 = 0;
        }
        else if (newX3 == 0) {
            if (newX1 == 0) {
                Backtrack(newX2, newX1);
                newX = newX2;
            }
            else if (newX2 == 0) {
                Backtrack(newX1, newX2);
                newX = newX1;
            }
            else {
                altX1.insert(newX1);
                altX2.insert(newX2);
                newX = newX3;
            }
        }

        else {
            //store X1 and X2 in their linked lists for possible use later
            altX1.insert(newX1);
            altX2.insert(newX2);
            newX = newX3;
        }
    }
}//end of chooseX


void pdsi::CalcDurFact(number &slope, number &intercept, int sign) {
    //calculates m and b, which are used to calculated X(i)
    //based on the Z index.  These constants will determine the
    //weight that the previous PDSI value and the current Z index
    //will have on the current PDSI value.  This is done by finding
    //several of the driest periods at this station and assuming that
    //those periods represents an extreme drought.  Then a linear
    //regression is done to determine the relationship between length
    //of a dry (or wet) spell and the accumulated Z index during that
    //same period.
    //
    //it appears that there needs to be a different weight given to
    //negative and positive Z values, so the variable 'sign' will
    //determine whether the driest or wettest periods are looked at.

    int num_list = 10;
    number sum[10];
    int length[10];
    int i;

    if (Weekly) {
        if (period_length == 1) {
            length[0] = 13;
            length[1] = 26;
            length[2] = 39;
            length[3] = 52;
            length[4] = 78;
            length[5] = 104;
            length[6] = 130;
            length[7] = 156;
            length[8] = 182;
            length[9] = 208;
        }
        else if (period_length == 2) {
            length[0] = 6;
            length[1] = 13;
            length[2] = 19;
            length[3] = 26;
            length[4] = 39;
            length[5] = 52;
            length[6] = 65;
            length[7] = 78;
            length[8] = 91;
            length[9] = 104;
        }
        else if (period_length == 4) {
            length[0] = 3;
            length[1] = 6;
            length[2] = 10;
            length[3] = 13;
            length[4] = 20;
            length[5] = 26;
            length[6] = 33;
            length[7] = 39;
            length[8] = 46;
            length[9] = 52;
        }
        else if (period_length == 13) {
            length[0] = 2;
            length[1] = 3;
            length[2] = 4;
            length[3] = 5;
            length[4] = 6;
            length[5] = 8;
            length[6] = 10;
            length[7] = 12;
            length[8] = 14;
            length[9] = 16;
        }
    }
    else {
        length[0] = 3;
        length[1] = 6;
        length[2] = 9;
        length[3] = 12;
        length[4] = 18;
        length[5] = 24;
        length[6] = 30;
        length[7] = 36;
        length[8] = 42;
        length[9] = 48;
    }

    for (i = 0; i < num_list; i++) {
        sum[i] = get_Z_sum(length[i], sign);
        //printf("sum[%d] = %f\n",i,sum[i]);
    }
    //if(verbose > 1){
    //printf("Points used in linear regression for Duration Factors:\n");
    //for(i=0;i<num_list;i++)
    //printf("%7d  ",length[i]);
    //printf("\n");
    //for(i=0;i<num_list;i++)
    //printf("%7.2f  ",sum[i]);
    //printf("\n");
    //}

    LeastSquares(length, sum, num_list, sign, verbose, slope, intercept);
    //printf("the line is: y = %f * x + %f\n",slope,intercept);
    //now divide m and b by 4 or -4 becuase that line represents
    //pdsi of either 4.0 or -4.0

    slope = slope / (sign * 4);
    intercept = intercept / (sign * 4);
}//end of CalcDurFact()

number pdsi::get_Z_sum(int length, int sign) {
    number sum, max_sum, z;
    llist tempZ, list_to_sum, list_of_sums;

    number highest_reasonable;
    number percentile;
    number reasonable_tol = 1.25;
    /* SG 6/5/06: Add variable to implement user-defined calibration interval */
    int nCalibrationPeriodsLeft;

    copy(tempZ, ZIND);
    sum = 0;

    /* SG 6/5/06: Remove the periods from the list until we get to the
    **            start of the calibration interval
    */
    for (int i = 0; (i < nStartPeriodsToSkip) && (!tempZ.is_empty()) ; i++) {
        tempZ.tail_remove(); /* remove periods before the start of the interval */
    }
    /* SG 6/5/06: We now have a list that begins at the calibration interval.
    **            However, if the list has more periods than the length of the
    **            calibration interval, we must be sure to not go past the
    **            calibration interval length
    */
    nCalibrationPeriodsLeft = nCalibrationPeriods; /* init periods left */
    //first fill the list to be summed
    for (int i = 0; i < length; i++) {
        if (tempZ.is_empty()) {
            //printf("Error: tempZ is empty.\n");
            i = length;
        }
        else {
            z = tempZ.tail_remove();
            nCalibrationPeriodsLeft--; /* reduce by one period for each remove */
            /* assumes that nCalibrationPeriods is >= length, reasonable
                            ** This is a reasonable assumption and does not hurt if
                            ** anything if false--just calibrate over a slightly longer
                            ** interval, which is already way too short in that case */
            if (z != MISSING) {
                sum += z;
                list_to_sum.insert(z);
            }
            else {
                i--;
            }
        }
        //printf("i = %d  z= %f  sum = %f\n",i,z,sum);
    }


    //now for each remaining Z value,
    //recalculate the sum based on last value in the
    //list to sum and the next Z value
    max_sum = sum;
    list_of_sums.insert(sum);
    while (!tempZ.is_empty() && nCalibrationPeriodsLeft > 0) {
        z = tempZ.tail_remove();
        nCalibrationPeriodsLeft--; /* reduce by one period for each remove */
        if (z != MISSING) {
            sum -= list_to_sum.tail_remove();
            sum += z;
            list_to_sum.insert(z);
            list_of_sums.insert(sum);
        }
        if (sign * sum > sign * max_sum)
            max_sum = sum;
    }

    //highest reasonable is the highest (or lowest)
    //value that is not due to some freak anomaly in the
    //data.
    //"freak anomaly" is defined as a value that is either
    //   1) 25% higher than the 98th percentile
    //   2) 25% lower than the 2nd percentile
    //
    highest_reasonable = 0;
    if (sign == 1)
        percentile = list_of_sums.safe_percentile(.98);
    if (sign == -1)
        percentile = list_of_sums.safe_percentile(.02);

    while (!list_of_sums.is_empty()) {
        sum = list_of_sums.tail_remove();
        if (sign * sum > 0 ) {
            if ( (sum / percentile) < reasonable_tol ) {
                if (sign * sum > sign * highest_reasonable )
                    highest_reasonable = sum;
            }
        }
    }

    if (sign == -1)
        return max_sum;
    else if (sign == 1)
        //return max_sum;
        return highest_reasonable;
    else
        return MISSING;
}//end of get_Z_sum()

number pdsi::getPDSI(int period, int year) {
    return getValue(Xlist, period, year);
}
number pdsi::getZIND(int period, int year) {
    return getValue(ZIND, period, year);
}
number pdsi::getWPLM(int period, int year) {

    number x1, x2, x3, p, wp;

    x1 = getValue(XL1, period, year);
    x2 = getValue(XL2, period, year);
    x3 = getValue(XL3, period, year);
    p = getValue(ProbL, period, year);
    if (x1 == MISSING || x2 == MISSING || x3 == MISSING || p == MISSING)
        wp = MISSING;
    else {
        p = p / 100;
        if (x3 == 0) {
            // There is not an established wet or dry spell so PHDI = PDSI (ph=x)
            // and the WPLM value is the maximum absolute value of X1 or X2
            wp = x1;
            if (-x2 > (x1 + tolerance))
                wp = x2;
        }
        else if (p > (0 + tolerance / 100) && p < (1 - tolerance / 100)) {
            // There is an established spell but there is a possibility it has or is
            // ending.  The WPLM is then a weighted average between X3 and X1 or X2
            if (x3 < 0)
                // X3 is negative so WPLM is weighted average of X3 and X1
                wp = (1 - p) * x3 + p * x1;
            else
                // X3 is positive so WPLM is weighted average of X3 and X2
                wp = (1 - p) * x3 + p * x2;
        }
        else
            // There is an established spell without possibility of end meaning the
            // WPLM is simply X3
            wp = x3;
    }
    return wp;
}

number pdsi::getPHDI(int period, int year) {
    number x, x3;

    x = getValue(Xlist, period, year);
    x3 = getValue(XL3, period, year);
    if (x == MISSING || x3 == MISSING)
        return MISSING;
    if (x3 == 0)
        return x;
    else
        return x3;
}

number pdsi::getValue(llist &List, int period, int year) {
    llist tempPer, tempYear, tempList;
    number per, yr, val;
    bool loop_exit = false;

    copy(tempList, List);
    copy(tempPer, PeriodList);
    copy(tempYear, YearList);

    while (! loop_exit) {
        if (tempList.is_empty())
            loop_exit = true;
        if (tempPer.is_empty())
            loop_exit = true;
        if (YearList.is_empty())
            loop_exit = true;

        val = tempList.head_remove();
        per = tempPer.head_remove();
        yr = tempYear.head_remove();

        if (yr == year && per == period)
            return val;
    }
    return MISSING;
}


number* pdsi::getYearArray(int &size) {
    size = YearList.get_size();
    return YearList.returnArray();
}
number* pdsi::getPerArray(int &size) {
    size = PeriodList.get_size();
    return PeriodList.returnArray();
}
number* pdsi::getCMIArray(int &size) {
    size = CMIList.get_size();
    return CMIList.returnArray();
}
number* pdsi::getCMIArray(int start_per, int start_yr,
                          int end_per, int end_yr, int &size) {
    return getSubArray(CMIList, start_per, start_yr, end_per, end_yr, size);
}
number* pdsi::getPDSIArray(int &size) {
    size = Xlist.get_size();
    return Xlist.returnArray();
}
number* pdsi::getPDSIArray(int start_per, int start_yr,
                           int end_per, int end_yr, int &size) {
    return getSubArray(Xlist, start_per, start_yr, end_per, end_yr, size);
}
number* pdsi::getZINDArray(int &size) {
    size = ZIND.get_size();
    return ZIND.returnArray();
}
number* pdsi::getZINDArray(int start_per, int start_yr,
                           int end_per, int end_yr, int &size) {
    return getSubArray(ZIND, start_per, start_yr, end_per, end_yr, size);
}


number* pdsi::getPHDIArray(int &size) {
    number *x, *x3;
    x = Xlist.returnArray();
    x3 = XL3.returnArray();
    size = Xlist.get_size();
    number *A = new number[size];
    if (A == NULL) {
        size = 0;
        return A;
    }
    for (int i = 0; i < size; i++) {
        if (x[i] != MISSING) {
            if (x3[i] == 0)
                A[i] = x[i];
            else
                A[i] = x3[i];
        }
        else
            A[i] = MISSING;
    }
    delete [] x;
    delete [] x3;
    return A;
}


number* pdsi::getPHDIArray(int start_per, int start_yr,
                           int end_per, int end_yr, int &size) {
    number *x, *x3;
    int tempsize;
    x = getSubArray(Xlist, start_per, start_yr, end_per, end_yr, size);
    x3 = getSubArray(XL3, start_per, start_yr, end_per, end_yr, tempsize);
    number *A = new number[size];
    if (A == NULL) {
        size = 0;
        return A;
    }
    for (int i = 0; i < size; i++) {
        if (x[i] != MISSING) {
            if (x3[i] == 0)
                A[i] = x[i];
            else
                A[i] = x3[i];
        }
        else
            A[i] = MISSING;
    }
    delete [] x;
    delete [] x3;
    return A;
}


number* pdsi::getWPLMArray(int &size) {
    number *A;
    number *x1Array, *x2Array, *x3Array, *pArray;
    number x1, x2, x3, p, wp;

    x1Array = XL1.returnArray();
    x2Array = XL2.returnArray();
    x3Array = XL3.returnArray();
    pArray = ProbL.returnArray();

    size = XL1.get_size();

    A = new number[size];
    if (A == NULL) {
        size = 0;
        return A;
    }

    for (int i = 0; i < size; i++) {
        x1 = x1Array[i];
        x2 = x2Array[i];
        x3 = x3Array[i];
        p = pArray[i];

        if (x1 == MISSING || x2 == MISSING || x3 == MISSING || p == MISSING)
            wp = MISSING;
        else {
            p = p / 100;
            if (x3 == 0) {
                // There is not an established wet or dry spell so PHDI = PDSI (ph=x)
                // and the WPLM value is the maximum absolute value of X1 or X2
                wp = x1;
                if (-x2 > (x1 + tolerance))
                    wp = x2;
            }
            else if (p > (0 + tolerance / 100) && p < (1 - tolerance / 100)) {
                // There is an established spell but there is a possibility it has or is
                // ending.  The WPLM is then a weighted average between X3 and X1 or X2
                if (x3 < 0)
                    // X3 is negative so WPLM is weighted average of X3 and X1
                    wp = (1 - p) * x3 + p * x1;
                else
                    // X3 is positive so WPLM is weighted average of X3 and X2
                    wp = (1 - p) * x3 + p * x2;
            }
            else
                // There is an established spell without possibility of end meaning the
                // WPLM is simply X3
                wp = x3;
        }
        A[i] = wp;
    }
    delete [] x1Array;
    delete [] x2Array;
    delete [] x3Array;
    delete [] pArray;
    return A;
}


number* pdsi::getWPLMArray(int start_per, int start_yr,
                           int end_per, int end_yr, int &size) {
    number *A;
    number *x1Array, *x2Array, *x3Array, *pArray;
    number x1, x2, x3, p, wp;
    int tempsize;

    x1Array = getSubArray(XL1, start_per, start_yr, end_per, end_yr, size);
    x2Array = getSubArray(XL2, start_per, start_yr, end_per, end_yr, tempsize);
    x3Array = getSubArray(XL3, start_per, start_yr, end_per, end_yr, tempsize);
    pArray = getSubArray(ProbL, start_per, start_yr, end_per, end_yr, tempsize);

    A = new number[size];
    if (A == NULL) {
        size = 0;
        return A;
    }

    for (int i = 0; i < size; i++) {
        x1 = x1Array[i];
        x2 = x2Array[i];
        x3 = x3Array[i];
        p = pArray[i];

        if (x1 == MISSING || x2 == MISSING || x3 == MISSING || p == MISSING)
            wp = MISSING;
        else {
            p = p / 100;
            if (x3 == 0) {
                // There is not an established wet or dry spell so PHDI = PDSI (ph=x)
                // and the WPLM value is the maximum absolute value of X1 or X2
                wp = x1;
                if (-x2 > (x1 + tolerance))
                    wp = x2;
            }
            else if (p > (0 + tolerance / 100) && p < (1 - tolerance / 100)) {
                // There is an established spell but there is a possibility it has or is
                // ending.  The WPLM is then a weighted average between X3 and X1 or X2
                if (x3 < 0)
                    // X3 is negative so WPLM is weighted average of X3 and X1
                    wp = (1 - p) * x3 + p * x1;
                else
                    // X3 is positive so WPLM is weighted average of X3 and X2
                    wp = (1 - p) * x3 + p * x2;
            }
            else
                // There is an established spell without possibility of end meaning the
                // WPLM is simply X3
                wp = x3;
        }
        A[i] = wp;
    }
    delete [] x1Array;
    delete [] x2Array;
    delete [] x3Array;
    delete [] pArray;
    return A;
}


number* pdsi::getSubArray(llist &List, int start_per, int start_yr,
                          int end_per, int end_yr, int &size) {

    llist temp;
    number *Array, *year, *period;
    int i, j;
    int cur_per, cur_yr;
    int per_len = 0;
    int num_missing;

    Array = List.returnArray();
    year = YearList.returnArray();
    period = PeriodList.returnArray();

    for (j = 0; j < PeriodList.get_size(); j++) {
        if (period[j] > per_len)
            per_len = (int)period[j];
    }
    //printf("per_len is: %d\n",per_len);
    //printf("size of list is: %d\n",PeriodList.get_size() );

    if ( (start_yr > year[0]) ||
            ( (start_yr == year[0]) && (start_per > period[0]) ) ) {
        i = 0;
        while ( ( ( year[i] < start_yr ) ||
                  ( year[i] == start_yr && period[i] < start_per ) ) &&
                ( i < List.get_size() ) ) {
            i++;
        }
        while ( ( ( year[i] < end_yr ) ||
                  ( year[i] == end_yr && period[i] <= end_per ) ) &&
                ( i < List.get_size() ) ) {
            temp.insert(Array[i]);
            i++;
        }
        if (i == List.get_size()) {
            cur_yr = (int)year[i - 1];
            cur_per = (int)period[i - 1];
            if ((cur_per % per_len) == 0) {
                cur_per = 1;
                cur_yr++;
            }
            else
                cur_per++;
            while ( (cur_yr < end_yr) ||
                    ( (cur_yr == end_yr) && (cur_per <= end_per)) ) {
                temp.insert(MISSING);
                if ((cur_per % per_len) == 0) {
                    cur_per = 1;
                    cur_yr++;
                }
                else
                    cur_per++;
            }
        }

    }

    else {
        if (start_yr == year[0])
            num_missing = (int)period[0] - start_per;
        else {
            if (period[0] <= start_per)
                num_missing = ((int)year[0] - start_yr - 1) * per_len + ((int)period[0] - start_per + per_len);
            else
                num_missing = ((int)year[0] - start_yr) * per_len + ((int)period[0] - start_per);
        }
        //printf("num_missing=%d\n",num_missing);
        for (j = 0; j < num_missing; j++)
            temp.insert(MISSING);

        i = 0;
        while ( ( ( year[i] < end_yr ) ||
                  ( year[i] == end_yr && period[i] <= end_per ) ) &&
                ( i < List.get_size() ) ) {
            temp.insert(Array[i]);
            //printf("i=%d  cur_date: %d/%d  end_date: %d/%d\n",i,(int)period[i],(int)year[i],end_per,end_yr);
            i++;
        }
        if (i == List.get_size()) {
            cur_yr = (int)year[i - 1];
            cur_per = (int)period[i - 1];
            if ((cur_per % per_len) == 0) {
                cur_per = 1;
                cur_yr++;
            }
            else
                cur_per++;
            while ( (cur_yr < end_yr) ||
                    ( (cur_yr == end_yr) && (cur_per <= end_per)) ) {
                temp.insert(MISSING);
                //printf("here i=%d  cur_date: %d/%d  end_date: %d/%d\n",i,cur_per,cur_yr,end_per,end_yr);
                if ((cur_per % per_len) == 0) {
                    cur_per = 1;
                    cur_yr++;
                }
                else
                    cur_per++;
            }
        }
    }
    delete [] Array;
    delete [] year;
    delete [] period;

    size = temp.get_size();
    return temp.returnArray();
}
