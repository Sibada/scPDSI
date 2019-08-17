// #include <cstring>
// #include "pdsi.h"

// //-----------------------------------------------------------------------------
// //**********                       MAIN PROGRAM                       *********
// //-----------------------------------------------------------------------------
// // The main program takes in command line arguments and passes them to the
// // constructor of a pdsi variable tester.  It then calls Calcpdsi to calculate
// // the pdsi values.  Finally it calls write to output these values to file.
// //-----------------------------------------------------------------------------
// int main(int argc, char *argv[]) {
//     pdsi PDSI;
//     PDSI.set_flags(argc, argv); // Sets the flags of PDSI
//     PDSI.WeeklyPDSI();         // Calculates the weekly pdsi values for PDSI
//     PDSI.Write("weekly/1");
//     PDSI.WeeklyPDSI(2);
//     PDSI.Write("weekly/2");
//     PDSI.WeeklyPDSI(4);
//     PDSI.Write("weekly/4");
//     PDSI.WeeklyPDSI(13);
//     PDSI.Write("weekly/13");
//     PDSI.MonthlyPDSI();
//     PDSI.Write("monthly/original");
//     PDSI.SCMonthlyPDSI();
//     PDSI.Write("monthly/self_cal");
//     PDSI.WeeklyCMI();
//     PDSI.Write("weekly/CMI");
//     return 0;
// }
