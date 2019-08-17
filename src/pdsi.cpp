#include "pdsi.h"

//#include <stdlib.h>
#include <math.h>
#include <cstring>
//#include <stdio.h>
#include <ctype.h>


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
    //FILE *fin;        // File pointer for the temp. input file potentials
    //FILE *fout;       // File pointer for the temp. output file dvalue
    int per;           // The period in question
    int yr;           // The year in question
    number p;         // The precip for that period
    //float scn1, scn2, scn3, scn4, scn5, scn6; // Temp. variables for fscanf
    //char letter = ' ';
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

    // This reads in the first line, which contains column headers.
    //while(letter != '\n')
    //  letter = fgetc(fin);
    // This reads in the values previously stored in "potentials"
    //while(fscanf(fin,"%d %d %f %f %f %f %f %f", &yr, &per, &scn1, &scn2, &scn3, &scn4, &scn5, &scn6) != EOF) {
    for (i = 0; i < vals_mat.nrow(); i++) {
        yr = vals_mat(i, 0);
        per = vals_mat(i, 1);
        per = (per - 1) / period_length; //adjust the period # for use in arrays.

        p = vals_mat(i, 2);
        PE = vals_mat(i, 3);
        PR = vals_mat(i, 4);
        PRO = vals_mat(i, 5);
        PL = vals_mat(i, 6);
        //scn6 is P - PE, which can be ignored for calculations.

        if (p != MISSING && PE != MISSING && PR != MISSING && PRO != MISSING && PL != MISSING) {
            // Then the calculations for Phat and d are done
            Phat = (Alpha[per] * PE) + (Beta[per] * PR) + (Gamma[per] * PRO) - (Delta[per] * PL);
            d = p - Phat;

            // The values of d are output to a temp file for later use.
            //fprintf (fout, "%d %d %f\n", yr, (period_length*per)+1, d);
            vals_mat(i, 7) = d;

            /* SG 6/5/06: Need to only update statistical values when in
            **            user defined calibration interval. When not used
            **            nCalibrationYears==totalyears; hence no change then
            */
            if (yr + startyear - 1 >= currentCalibrationStartYear
                    && yr + startyear - 1 <= currentCalibrationEndYear) {
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
            vals_mat(i, 7) = d;
            //fprintf (fout, "%d %d %f\n", yr, (period_length*per)+1, d);
        }
    }

    // If the user specifies, the various sums are output to the screen
    for (i = 0; i < num_of_periods; i++) {
        // D becomes the mean of D_sum
        /* SG 6/5/06: changed totalyears to nCalibrationYears to support
        **            user defined calibration intervals. When not used
        **            nCalibrationYears==totalyears; hence no change then
        */
        D[i] = D_sum[i] / nCalibrationYears;
    }
    // The files are closed
    //fclose(fin);
    //fclose(fout);
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
            k[per] = coe_K1_3;//prevent div by 0
        else
            k[per] = coe_K1_1 * log10((sums + coe_K1_2) / D[per]) + coe_K1_3;

        coefs_mat(per, 4) = k[per];
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

    // Calculate k, which is K', or Palmer's second approximation of K
    for (int per = 0; per < num_of_periods; per++) {
        if (PSum[per] + LSum[per] == 0)
            sums = 0;//prevent div by 0
        else
            sums = (PESum[per] + RSum[per] + ROSum[per]) / (PSum[per] + LSum[per]);

        if (D[per] == 0)
            k[per] = coe_K1_3;//prevent div by 0
        else
            k[per] = coe_K1_1 * log10((sums + coe_K1_2) / D[per]) + coe_K1_3;

        coefs_mat(per, 4) = k[per];
        DKSum += D[per] * k[per];
    }

    if (Weekly) {
        //set duration factors to CPC's
        drym = 2.925;
        dryb = 0.075;
    } else {
        // Set duration factors to Palmer's original duration factors
        drym = coe_m;
        dryb = coe_b;
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
    // Reads in all previously calclulated d values and calculates Z
    // then calls CalcX to compute the corresponding PDSI value
    for (int i = 0; i < vals_mat.nrow(); i++) {
        year = vals_mat(i, 0);
        month = vals_mat(i, 1);
        dtemp = vals_mat(i, 7);

        PeriodList.insert(month);
        YearList.insert(year);
        d = dtemp;
        month--;
        K_w = coe_K2 / DKSum;
        K_d = K_w;
        K = K_w * k[month];
        if (d != MISSING)
            Z = d * K;
        else
            Z = MISSING;

        ZIND.insert(Z);
        CalcOneX(month, year);
    }
    // Now that all calculations have been done they can be output to the screen
}//end of CalcOrigK()

void pdsi::CalcZ() {

    int year, per;
    float dtemp;
    llist tempZ, tempPer, tempyear;
    DKSum = 0.0; //sum of all D[i] and k[i]; used to calc K

    for (per = 0; per < num_of_periods; per++)
        DKSum += D[per] * k[per];

    // Reads in all previously calclulated d values and calculates Z
    // then calls CalcX to compute the corresponding PDSI value
    //while((fscanf(inputd,"%d %d %f", &year, &per, &dtemp))!=EOF) {
    for (int i = 0; i < vals_mat.nrow(); i++) {
        year = vals_mat(i, 0);
        per = vals_mat(i, 1);
        dtemp = vals_mat(i, 7);

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
    //fclose(inputd);
}//end of CalcZ()


void pdsi::CalcX() {
    llist tempZ, tempPer, tempYear;
    int year, per;

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

        CalcOneX(per, year);
    }
    //if(table)
    //  fclose(table);
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
void pdsi::CalcOneX(int period_number, int year) {
    number newV;    //These variables represent the values for
    number newProb; //corresponding variables for the current period.
    number newX;    //They are kept seperate because many calculations
    number newX1;   //depend on last period's values.
    number newX2;
    number newX3;
    number ZE;      //ZE is the Z value needed to end an established spell

    number m, b, c;
    int n = (year - 1) * num_of_periods + period_number;

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

        vals_mat(n, 8) = Z;
        vals_mat(n, 9) = newProb;
        vals_mat(n, 10) = newX1;
        vals_mat(n, 11) = newX2;
        vals_mat(n, 12) = newX3;

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

        vals_mat(n, 8) = Z;
        vals_mat(n, 9) = MISSING;
        vals_mat(n, 10) = MISSING;
        vals_mat(n, 11) = MISSING;
        vals_mat(n, 12) = MISSING;

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
    //FILE * fout;
    //FILE * input_temp, *input_prec;
    //char Temp[150], Precip[150];
    //int actyear;
    int n;
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

    // This loop runs to read in and calculate the values for all years
    for (int year = 1; year <= totalyears; year++) {
        // Get a year's worth of temperature and precipitation data
        // Also, get the current year from the temperature file.

        if (Weekly) {
            // NOTE: Here T is the vector of PE, instead of tempreature.
            Rext_get_Rvec(PE_vec, year, T, 52);
            Rext_get_Rvec(P_vec, year, P, 52);
        }
        else {
            Rext_get_Rvec(PE_vec, year, T, 12);
            Rext_get_Rvec(P_vec, year, P, 12);
        }

        // This loop runs for each per in the year
        for (int per = 0; per < num_of_periods; per++) {
            if (P[per] >= 0 && T[per] != MISSING) {
                // calculate the Potential Evapotranspiration first
                // because it's needed in later calculations
                /*
                    if(Weekly)
                      CalcWkPE(per,actyear);
                    else
                      CalcMonPE(per,actyear);
                */
                PE = T[per]; // NOTE: Here T is the vector of PE, instead of tempreature.
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

                n = (year - 1) * num_of_periods + per;
                vals_mat(n, 0) = year;
                vals_mat(n, 1) = per * period_length + 1;
                vals_mat(n, 2) = P[per];
                vals_mat(n, 3) = PE;
                vals_mat(n, 4) = PR;
                vals_mat(n, 5) = PRO;
                vals_mat(n, 6) = PL;

                //P, PE, PR, PRO, and PL will be used later, so
                //these variables need to be stored to an outside file
                //where they can be accessed at a later time
                //fprintf(fout,"%5d %5d %10.6f ",actyear,(period_length*per)+1,P[per]);
                //fprintf(fout,"%10.6f %10.6f %10.6f %10.6f ",PE,PR,PRO,PL);
                //fprintf(fout,"%10.6f\n",P[per]-PE);
            }//matches if(P[per]>= 0 && T[per] != MISSING)
            else {
                n = (year - 1) * num_of_periods + per;
                vals_mat(n, 0) = year;
                vals_mat(n, 1) = per * period_length + 1;
                for (int i = 2; i < 7; i++)
                    vals_mat(n, i) = MISSING;
            }
        }//end of period loop
    }//end of year loop
}

//-----------------------------------------------------------------------------
// This function calculates Alpha, Beta, Gamma, and Delta, the normalizing
// climate coefficients in the water balance equation.
// If the user desires, the results are output to the screen and a file.
//-----------------------------------------------------------------------------
void pdsi::CalcWBCoef() {
    //FILE *wb;
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

    for (int i = 0; i < num_of_periods; i++) {
        coefs_mat(i, 0) = Alpha[i];
        coefs_mat(i, 1) = Beta[i];
        coefs_mat(i, 2) = Gamma[i];
        coefs_mat(i, 3) = Delta[i];
    }
}//end CalcWBCoef()
