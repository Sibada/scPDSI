#include "pdsi.h"

//#include <stdlib.h>
#include <math.h>
#include <cstring>
//#include <stdio.h>
#include <ctype.h>

/**
 * Original scPDSI scripts
 * Version 2003
 */

//-----------------------------------------------------------------------------
// CalcThornA calculates the Thornthwaite exponent a based on the heat index I.
//-----------------------------------------------------------------------------
number pdsi::CalcThornA(number I) {
    number A;
    A = 6.75 * (pow(I, 3)) / 10000000 - 7.71 * (pow(I, 2)) / 100000 + 0.0179 * I + 0.49;
    return A;
}

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
    /*
    if(verbose > 0)
        printf("Calculations Complete\n");
    */
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

    K_d = K_d * dry_ratio; // MODIFIED here
    K_w = K_w * wet_ratio; // MODIFIED here

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
