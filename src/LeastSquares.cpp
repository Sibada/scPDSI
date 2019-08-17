#include "pdsi.h"

void pdsi::LeastSquares(int *x, number *y, int n, int sign, int verbose, number &slope, number &intercept) {
    number sumX, sumX2, sumY, sumY2, sumXY;
    number SSX, SSY, SSXY;
    number xbar, ybar;

    number correlation = 0;
    number c_tol = 0.85;

    number max = 0;
    number max_diff = 0;
    int max_i = 0;

    number this_x, this_y;
    int i;

    sumX = 0; sumY = 0; sumX2 = 0; sumY2 = 0; sumXY = 0;
    for (i = 0; i < n; i++) {
        this_x = x[i];
        this_y = y[i];

        sumX += this_x;
        sumY += this_y;
        sumX2 += this_x * this_x;
        sumY2 += this_y * this_y;
        sumXY += this_x * this_y;
    }

    xbar = sumX / n;
    ybar = sumY / n;

    SSX = sumX2 - (sumX * sumX) / n;
    SSY = sumY2 - (sumY * sumY) / n;
    SSXY = sumXY - (sumX * sumY) / n;

    correlation = SSXY / (sqrt(SSX) * sqrt(SSY));

    if (verbose > 1 && (sign * correlation) < c_tol ) {
        //printf("original correlation = %.4f \n",correlation);
    }

    i = n - 1;
    while ((sign * correlation) < c_tol && i > 3) {
        //when the correlation is off, it appears better to
        //take the earlier sums rather than the later ones.
        this_x = x[i];
        this_y = y[i];

        sumX -= this_x;
        sumY -= this_y;
        sumX2 -= this_x * this_x;
        sumY2 -= this_y * this_y;
        sumXY -= this_x * this_y;

        SSX = sumX2 - (sumX * sumX) / i;
        SSY = sumY2 - (sumY * sumY) / i;
        SSXY = sumXY - (sumX * sumY) / i;

        xbar = sumX / i;
        ybar = sumY / i;

        correlation = SSXY / (sqrt(SSX) * sqrt(SSY));
        i--;
    }

    if (verbose > 1) {
        //printf("final correlation =  %.4f\n\n",correlation);
    }
    slope = SSXY / SSX;

    n = i + 1;
    for (i = 0; i < n; i++) {
        if (sign * (y[i] - slope * x[i]) > sign * max_diff) {
            max_diff = y[i] - slope * x[i];
            max_i = i;
            max = y[i];
        }
    }
    intercept = max - slope * x[max_i];
}//end of LeastSquares()
