#include "hardisp.hpp"
#include <stdio.h>

/**
 * @details This function returns the ocean loading displacement amplitude,
 *          frequency, and phase of a set of tidal constituents generated by
 *          the Bos-Scherneck website at http://www.oso.chalmers.se/~loading/.
 *          The variable nin is input as the number wanted, and the variable 
 *          nout is returned as the number provided.  The constituents used
 *          are stored in the arrays idd (Doodson number) and tamp
 *          (Cartwright-Edden amplitude).  The actual amp and phase of each
 *          of these are determined by spline interpolation of the real and
 *          imaginary part of the admittance, as specified at a subset of the
 *          constituents.
 * 
 * @param[in]  ampin  Cartwright-Edden amplitude of tidal constituents
 *                    (size \c nin).
 * @param[in]  idtin  Doodson number of tidal constituents (size 6x\c nin).
 * @param[in]  phin   Phase of tidal constituents
 * @param[in]  nin    Number of harmonics used
 * @param[in]  itm    Date as integer array in UTC. The format should be:
 *                    [year,day_of_year,hours,minutes,seconds].
 * @param[out] amp    Amplitude due to ocean loading
 * @param[out] f      Frequency due to ocean loading
 * @param[out] p      Phase due to ocean loading
 * @param[out] nout   Number of harmonics returned
 * @return            zero
 * 
 * @note
 *     -# The phase is determined for a time set in COMMON block /date/ in
 *        the function TDFRPH.
 *     -# The arrays F and P must be specified as double precision.
 *     -# Status:  Class 1 model
 * 
 * @version 2009 August 19
 * 
 * @cite McCarthy, D. D., Petit, G. (eds.), IERS Conventions (2003),
 *       IERS Technical Note No. 32, BKG (2004)
 * 
 */
int iers2010::hisp::admint (const double* ampin,const int idtin[][6],
        const double* phin,double* amp,double* f,double* p,const int& nin,
        int& nout, const int* itm)
 {
    /*+----------------------------------------------------------------------
    *  The parameters below set the number of harmonics used in the prediction
    *  (nt; This must also be set in the main program) and the number of
    *  constituents whose amp and phase may be specified (ncon)
    *-----------------------------------------------------------------------*/
    constexpr int nt   ( 342 );
    constexpr int ncon ( 20 );
    
    //  Arrays containing information about the subset whose amp and phase may
    //+ be specified, and scratch arrays for the spline routines for which
    //+ at most ncon constituents may be specified. 
    constexpr double dtr ( .01745329252e0 );
    int key[ncon];
    double rl[ncon],aim[ncon],rf[ncon],scr[ncon],
    zdi[ncon],zdr[ncon],di[ncon],dr[ncon],sdi[ncon],sdr[ncon];
    double fr,pr,re,am,sf;
    
    // Arrays containing information about all stored constituents
    static const int idd[/*nt*/][6] = {
        {2, 0, 0, 0, 0, 0},  { 2, 2,-2, 0, 0, 0},  { 2,-1, 0, 1, 0, 0},
        {2, 2, 0, 0, 0, 0},  { 2, 2, 0, 0, 1, 0},  { 2, 0, 0, 0,-1, 0},
        {2,-1, 2,-1, 0, 0},  { 2,-2, 2, 0, 0, 0},  { 2, 1, 0,-1, 0, 0},
        {2, 2,-3, 0, 0, 1},  { 2,-2, 0, 2, 0, 0},  { 2,-3, 2, 1, 0, 0},
        {2, 1,-2, 1, 0, 0},  { 2,-1, 0, 1,-1, 0},  { 2, 3, 0,-1, 0, 0},
        {2, 1, 0, 1, 0, 0},  { 2, 2, 0, 0, 2, 0},  { 2, 2,-1, 0, 0,-1},
        {2, 0,-1, 0, 0, 1},  { 2, 1, 0, 1, 1, 0},  { 2, 3, 0,-1, 1, 0},
        {2, 0, 1, 0, 0,-1},  { 2, 0,-2, 2, 0, 0},  { 2,-3, 0, 3, 0, 0},
        {2,-2, 3, 0, 0,-1},  { 2, 4, 0, 0, 0, 0},  { 2,-1, 1, 1, 0,-1},
        {2,-1, 3,-1, 0,-1},  { 2, 2, 0, 0,-1, 0},  { 2,-1,-1, 1, 0, 1},
        {2, 4, 0, 0, 1, 0},  { 2,-3, 4,-1, 0, 0},  { 2,-1, 2,-1,-1, 0},
        {2, 3,-2, 1, 0, 0},  { 2, 1, 2,-1, 0, 0},  { 2,-4, 2, 2, 0, 0},
        {2, 4,-2, 0, 0, 0},  { 2, 0, 2, 0, 0, 0},  { 2,-2, 2, 0,-1, 0},
        {2, 2,-4, 0, 0, 2},  { 2, 2,-2, 0,-1, 0},  { 2, 1, 0,-1,-1, 0},
        {2,-1, 1, 0, 0, 0},  { 2, 2,-1, 0, 0, 1},  { 2, 2, 1, 0, 0,-1},
        {2,-2, 0, 2,-1, 0},  { 2,-2, 4,-2, 0, 0},  { 2, 2, 2, 0, 0, 0},
        {2,-4, 4, 0, 0, 0},  { 2,-1, 0,-1,-2, 0},  { 2, 1, 2,-1, 1, 0},
        {2,-1,-2, 3, 0, 0},  { 2, 3,-2, 1, 1, 0},  { 2, 4, 0,-2, 0, 0},
        {2, 0, 0, 2, 0, 0},  { 2, 0, 2,-2, 0, 0},  { 2, 0, 2, 0, 1, 0},
        {2,-3, 3, 1, 0,-1},  { 2, 0, 0, 0,-2, 0},  { 2, 4, 0, 0, 2, 0},
        {2, 4,-2, 0, 1, 0},  { 2, 0, 0, 0, 0, 2},  { 2, 1, 0, 1, 2, 0},
        {2, 0,-2, 0,-2, 0},  { 2,-2, 1, 0, 0, 1},  { 2,-2, 1, 2, 0,-1},
        {2,-1, 1,-1, 0, 1},  { 2, 5, 0,-1, 0, 0},  { 2, 1,-3, 1, 0, 1},
        {2,-2,-1, 2, 0, 1},  { 2, 3, 0,-1, 2, 0},  { 2, 1,-2, 1,-1, 0},
        {2, 5, 0,-1, 1, 0},  { 2,-4, 0, 4, 0, 0},  { 2,-3, 2, 1,-1, 0},
        {2,-2, 1, 1, 0, 0},  { 2, 4, 0,-2, 1, 0},  { 2, 0, 0, 2, 1, 0},
        {2,-5, 4, 1, 0, 0},  { 2, 0, 2, 0, 2, 0},  { 2,-1, 2, 1, 0, 0},
        {2, 5,-2,-1, 0, 0},  { 2, 1,-1, 0, 0, 0},  { 2, 2,-2, 0, 0, 2},
        {2,-5, 2, 3, 0, 0},  { 2,-1,-2, 1,-2, 0},  { 2,-3, 5,-1, 0,-1},
        {2,-1, 0, 0, 0, 1},  { 2,-2, 0, 0,-2, 0},  { 2, 0,-1, 1, 0, 0},
        {2,-3, 1, 1, 0, 1},  { 2, 3, 0,-1,-1, 0},  { 2, 1, 0, 1,-1, 0},
        {2,-1, 2, 1, 1, 0},  { 2, 0,-3, 2, 0, 1},  { 2, 1,-1,-1, 0, 1},
        {2,-3, 0, 3,-1, 0},  { 2, 0,-2, 2,-1, 0},  { 2,-4, 3, 2, 0,-1},
        {2,-1, 0, 1,-2, 0},  { 2, 5, 0,-1, 2, 0},  { 2,-4, 5, 0, 0,-1},
        {2,-2, 4, 0, 0,-2},  { 2,-1, 0, 1, 0, 2},  { 2,-2,-2, 4, 0, 0},
        {2, 3,-2,-1,-1, 0},  { 2,-2, 5,-2, 0,-1},  { 2, 0,-1, 0,-1, 1},
        {2, 5,-2,-1, 1, 0},  { 1, 1, 0, 0, 0, 0},  { 1,-1, 0, 0, 0, 0},
        {1, 1,-2, 0, 0, 0},  { 1,-2, 0, 1, 0, 0},  { 1, 1, 0, 0, 1, 0},
        {1,-1, 0, 0,-1, 0},  { 1, 2, 0,-1, 0, 0},  { 1, 0, 0, 1, 0, 0},
        {1, 3, 0, 0, 0, 0},  { 1,-2, 2,-1, 0, 0},  { 1,-2, 0, 1,-1, 0},
        {1,-3, 2, 0, 0, 0},  { 1, 0, 0,-1, 0, 0},  { 1, 1, 0, 0,-1, 0},
        {1, 3, 0, 0, 1, 0},  { 1, 1,-3, 0, 0, 1},  { 1,-3, 0, 2, 0, 0},
        {1, 1, 2, 0, 0, 0},  { 1, 0, 0, 1, 1, 0},  { 1, 2, 0,-1, 1, 0},
        {1, 0, 2,-1, 0, 0},  { 1, 2,-2, 1, 0, 0},  { 1, 3,-2, 0, 0, 0},
        {1,-1, 2, 0, 0, 0},  { 1, 1, 1, 0, 0,-1},  { 1, 1,-1, 0, 0, 1},
        {1, 4, 0,-1, 0, 0},  { 1,-4, 2, 1, 0, 0},  { 1, 0,-2, 1, 0, 0},
        {1,-2, 2,-1,-1, 0},  { 1, 3, 0,-2, 0, 0},  { 1,-1, 0, 2, 0, 0},
        {1,-1, 0, 0,-2, 0},  { 1, 3, 0, 0, 2, 0},  { 1,-3, 2, 0,-1, 0},
        {1, 4, 0,-1, 1, 0},  { 1, 0, 0,-1,-1, 0},  { 1, 1,-2, 0,-1, 0},
        {1,-3, 0, 2,-1, 0},  { 1, 1, 0, 0, 2, 0},  { 1, 1,-1, 0, 0,-1},
        {1,-1,-1, 0, 0, 1},  { 1, 0, 2,-1, 1, 0},  { 1,-1, 1, 0, 0,-1},
        {1,-1,-2, 2, 0, 0},  { 1, 2,-2, 1, 1, 0},  { 1,-4, 0, 3, 0, 0},
        {1,-1, 2, 0, 1, 0},  { 1, 3,-2, 0, 1, 0},  { 1, 2, 0,-1,-1, 0},
        {1, 0, 0, 1,-1, 0},  { 1,-2, 2, 1, 0, 0},  { 1, 4,-2,-1, 0, 0},
        {1,-3, 3, 0, 0,-1},  { 1,-2, 1, 1, 0,-1},  { 1,-2, 3,-1, 0,-1},
        {1, 0,-2, 1,-1, 0},  { 1,-2,-1, 1, 0, 1},  { 1, 4,-2, 1, 0, 0},
        {1,-4, 4,-1, 0, 0},  { 1,-4, 2, 1,-1, 0},  { 1, 5,-2, 0, 0, 0},
        {1, 3, 0,-2, 1, 0},  { 1,-5, 2, 2, 0, 0},  { 1, 2, 0, 1, 0, 0},
        {1, 1, 3, 0, 0,-1},  { 1,-2, 0, 1,-2, 0},  { 1, 4, 0,-1, 2, 0},
        {1, 1,-4, 0, 0, 2},  { 1, 5, 0,-2, 0, 0},  { 1,-1, 0, 2, 1, 0},
        {1,-2, 1, 0, 0, 0},  { 1, 4,-2, 1, 1, 0},  { 1,-3, 4,-2, 0, 0},
        {1,-1, 3, 0, 0,-1},  { 1, 3,-3, 0, 0, 1},  { 1, 5,-2, 0, 1, 0},
        {1, 1, 2, 0, 1, 0},  { 1, 2, 0, 1, 1, 0},  { 1,-5, 4, 0, 0, 0},
        {1,-2, 0,-1,-2, 0},  { 1, 5, 0,-2, 1, 0},  { 1, 1, 2,-2, 0, 0},
        {1, 1,-2, 2, 0, 0},  { 1,-2, 2, 1, 1, 0},  { 1, 0, 3,-1, 0,-1},
        {1, 2,-3, 1, 0, 1},  { 1,-2,-2, 3, 0, 0},  { 1,-1, 2,-2, 0, 0},
        {1,-4, 3, 1, 0,-1},  { 1,-4, 0, 3,-1, 0},  { 1,-1,-2, 2,-1, 0},
        {1,-2, 0, 3, 0, 0},  { 1, 4, 0,-3, 0, 0},  { 1, 0, 1, 1, 0,-1},
        {1, 2,-1,-1, 0, 1},  { 1, 2,-2, 1,-1, 0},  { 1, 0, 0,-1,-2, 0},
        {1, 2, 0, 1, 2, 0},  { 1, 2,-2,-1,-1, 0},  { 1, 0, 0, 1, 2, 0},
        {1, 0, 1, 0, 0, 0},  { 1, 2,-1, 0, 0, 0},  { 1, 0, 2,-1,-1, 0},
        {1,-1,-2, 0,-2, 0},  { 1,-3, 1, 0, 0, 1},  { 1, 3,-2, 0,-1, 0},
        {1,-1,-1, 0,-1, 1},  { 1, 4,-2,-1, 1, 0},  { 1, 2, 1,-1, 0,-1},
        {1, 0,-1, 1, 0, 1},  { 1,-2, 4,-1, 0, 0},  { 1, 4,-4, 1, 0, 0},
        {1,-3, 1, 2, 0,-1},  { 1,-3, 3, 0,-1,-1},  { 1, 1, 2, 0, 2, 0},
        {1, 1,-2, 0,-2, 0},  { 1, 3, 0, 0, 3, 0},  { 1,-1, 2, 0,-1, 0},
        {1,-2, 1,-1, 0, 1},  { 1, 0,-3, 1, 0, 1},  { 1,-3,-1, 2, 0, 1},
        {1, 2, 0,-1, 2, 0},  { 1, 6,-2,-1, 0, 0},  { 1, 2, 2,-1, 0, 0},
        {1,-1, 1, 0,-1,-1},  { 1,-2, 3,-1,-1,-1},  { 1,-1, 0, 0, 0, 2},
        {1,-5, 0, 4, 0, 0},  { 1, 1, 0, 0, 0,-2},  { 1,-2, 1, 1,-1,-1},
        {1, 1,-1, 0, 1, 1},  { 1, 1, 2, 0, 0,-2},  { 1,-3, 1, 1, 0, 0},
        {1,-4, 4,-1,-1, 0},  { 1, 1, 0,-2,-1, 0},  { 1,-2,-1, 1,-1, 1},
        {1,-3, 2, 2, 0, 0},  { 1, 5,-2,-2, 0, 0},  { 1, 3,-4, 2, 0, 0},
        {1, 1,-2, 0, 0, 2},  { 1,-1, 4,-2, 0, 0},  { 1, 2, 2,-1, 1, 0},
        {1,-5, 2, 2,-1, 0},  { 1, 1,-3, 0,-1, 1},  { 1, 1, 1, 0, 1,-1},
        {1, 6,-2,-1, 1, 0},  { 1,-2, 2,-1,-2, 0},  { 1, 4,-2, 1, 2, 0},
        {1,-6, 4, 1, 0, 0},  { 1, 5,-4, 0, 0, 0},  { 1,-3, 4, 0, 0, 0},
        {1, 1, 2,-2, 1, 0},  { 1,-2, 1, 0,-1, 0},  { 0, 2, 0, 0, 0, 0},
        {0, 1, 0,-1, 0, 0},  { 0, 0, 2, 0, 0, 0},  { 0, 0, 0, 0, 1, 0},
        {0, 2, 0, 0, 1, 0},  { 0, 3, 0,-1, 0, 0},  { 0, 1,-2, 1, 0, 0},
        {0, 2,-2, 0, 0, 0},  { 0, 3, 0,-1, 1, 0},  { 0, 0, 1, 0, 0,-1},
        {0, 2, 0,-2, 0, 0},  { 0, 2, 0, 0, 2, 0},  { 0, 3,-2, 1, 0, 0},
        {0, 1, 0,-1,-1, 0},  { 0, 1, 0,-1, 1, 0},  { 0, 4,-2, 0, 0, 0},
        {0, 1, 0, 1, 0, 0},  { 0, 0, 3, 0, 0,-1},  { 0, 4, 0,-2, 0, 0},
        {0, 3,-2, 1, 1, 0},  { 0, 3,-2,-1, 0, 0},  { 0, 4,-2, 0, 1, 0},
        {0, 0, 2, 0, 1, 0},  { 0, 1, 0, 1, 1, 0},  { 0, 4, 0,-2, 1, 0},
        {0, 3, 0,-1, 2, 0},  { 0, 5,-2,-1, 0, 0},  { 0, 1, 2,-1, 0, 0},
        {0, 1,-2, 1,-1, 0},  { 0, 1,-2, 1, 1, 0},  { 0, 2,-2, 0,-1, 0},
        {0, 2,-3, 0, 0, 1},  { 0, 2,-2, 0, 1, 0},  { 0, 0, 2,-2, 0, 0},
        {0, 1,-3, 1, 0, 1},  { 0, 0, 0, 0, 2, 0},  { 0, 0, 1, 0, 0, 1},
        {0, 1, 2,-1, 1, 0},  { 0, 3, 0,-3, 0, 0},  { 0, 2, 1, 0, 0,-1},
        {0, 1,-1,-1, 0, 1},  { 0, 1, 0, 1, 2, 0},  { 0, 5,-2,-1, 1, 0},
        {0, 2,-1, 0, 0, 1},  { 0, 2, 2,-2, 0, 0},  { 0, 1,-1, 0, 0, 0},
        {0, 5, 0,-3, 0, 0},  { 0, 2, 0,-2, 1, 0},  { 0, 1, 1,-1, 0,-1},
        {0, 3,-4, 1, 0, 0},  { 0, 0, 2, 0, 2, 0},  { 0, 2, 0,-2,-1, 0},
        {0, 4,-3, 0, 0, 1},  { 0, 3,-1,-1, 0, 1},  { 0, 0, 2, 0, 0,-2},
        {0, 3,-3, 1, 0, 1},  { 0, 2,-4, 2, 0, 0},  { 0, 4,-2,-2, 0, 0},
        {0, 3, 1,-1, 0,-1},  { 0, 5,-4, 1, 0, 0},  { 0, 3,-2,-1,-1, 0},
        {0, 3,-2, 1, 2, 0},  { 0, 4,-4, 0, 0, 0},  { 0, 6,-2,-2, 0, 0},
        {0, 5, 0,-3, 1, 0},  { 0, 4,-2, 0, 2, 0},  { 0, 2, 2,-2, 1, 0},
        {0, 0, 4, 0, 0,-2},  { 0, 3,-1, 0, 0, 0},  { 0, 3,-3,-1, 0, 1},
        {0, 4, 0,-2, 2, 0},  { 0, 1,-2,-1,-1, 0},  { 0, 2,-1, 0, 0,-1},
        {0, 4,-4, 2, 0, 0},  { 0, 2, 1, 0, 1,-1},  { 0, 3,-2,-1, 1, 0},
        {0, 4,-3, 0, 1, 1},  { 0, 2, 0, 0, 3, 0},  { 0, 6,-4, 0, 0, 0}
    };
    
    static const double tamp[/*nt*/] = { 
        .632208e0, .294107e0, .121046e0, .079915e0, .023818e0,-.023589e0, .022994e0,
        .019333e0,-.017871e0, .017192e0, .016018e0, .004671e0,-.004662e0,-.004519e0,
        .004470e0, .004467e0, .002589e0,-.002455e0,-.002172e0, .001972e0, .001947e0,
        .001914e0,-.001898e0, .001802e0, .001304e0, .001170e0, .001130e0, .001061e0,
       -.001022e0,-.001017e0, .001014e0, .000901e0,-.000857e0, .000855e0, .000855e0,
        .000772e0, .000741e0, .000741e0,-.000721e0, .000698e0, .000658e0, .000654e0,
       -.000653e0, .000633e0, .000626e0,-.000598e0, .000590e0, .000544e0, .000479e0,
       -.000464e0, .000413e0,-.000390e0, .000373e0, .000366e0, .000366e0,-.000360e0,
       -.000355e0, .000354e0, .000329e0, .000328e0, .000319e0, .000302e0, .000279e0,
       -.000274e0,-.000272e0, .000248e0,-.000225e0, .000224e0,-.000223e0,-.000216e0,
        .000211e0, .000209e0, .000194e0, .000185e0,-.000174e0,-.000171e0, .000159e0,
        .000131e0, .000127e0, .000120e0, .000118e0, .000117e0, .000108e0, .000107e0,
        .000105e0,-.000102e0, .000102e0, .000099e0,-.000096e0, .000095e0,-.000089e0,
       -.000085e0,-.000084e0,-.000081e0,-.000077e0,-.000072e0,-.000067e0, .000066e0,
        .000064e0, .000063e0, .000063e0, .000063e0, .000062e0, .000062e0,-.000060e0,
        .000056e0, .000053e0, .000051e0, .000050e0, .368645e0,-.262232e0,-.121995e0,
       -.050208e0, .050031e0,-.049470e0, .020620e0, .020613e0, .011279e0,-.009530e0,
       -.009469e0,-.008012e0, .007414e0,-.007300e0, .007227e0,-.007131e0,-.006644e0,
        .005249e0, .004137e0, .004087e0, .003944e0, .003943e0, .003420e0, .003418e0,
        .002885e0, .002884e0, .002160e0,-.001936e0, .001934e0,-.001798e0, .001690e0,
        .001689e0, .001516e0, .001514e0,-.001511e0, .001383e0, .001372e0, .001371e0,
       -.001253e0,-.001075e0, .001020e0, .000901e0, .000865e0,-.000794e0, .000788e0,
        .000782e0,-.000747e0,-.000745e0, .000670e0,-.000603e0,-.000597e0, .000542e0,
        .000542e0,-.000541e0,-.000469e0,-.000440e0, .000438e0, .000422e0, .000410e0,
       -.000374e0,-.000365e0, .000345e0, .000335e0,-.000321e0,-.000319e0, .000307e0,
        .000291e0, .000290e0,-.000289e0, .000286e0, .000275e0, .000271e0, .000263e0,
       -.000245e0, .000225e0, .000225e0, .000221e0,-.000202e0,-.000200e0,-.000199e0,
        .000192e0, .000183e0, .000183e0, .000183e0,-.000170e0, .000169e0, .000168e0,
        .000162e0, .000149e0,-.000147e0,-.000141e0, .000138e0, .000136e0, .000136e0,
        .000127e0, .000127e0,-.000126e0,-.000121e0,-.000121e0, .000117e0,-.000116e0,
       -.000114e0,-.000114e0,-.000114e0, .000114e0, .000113e0, .000109e0, .000108e0,
        .000106e0,-.000106e0,-.000106e0, .000105e0, .000104e0,-.000103e0,-.000100e0,
       -.000100e0,-.000100e0, .000099e0,-.000098e0, .000093e0, .000093e0, .000090e0,
       -.000088e0, .000083e0,-.000083e0,-.000082e0,-.000081e0,-.000079e0,-.000077e0,
       -.000075e0,-.000075e0,-.000075e0, .000071e0, .000071e0,-.000071e0, .000068e0,
        .000068e0, .000065e0, .000065e0, .000064e0, .000064e0, .000064e0,-.000064e0,
       -.000060e0, .000056e0, .000056e0, .000053e0, .000053e0, .000053e0,-.000053e0,
        .000053e0, .000053e0, .000052e0, .000050e0,-.066607e0,-.035184e0,-.030988e0,
        .027929e0,-.027616e0,-.012753e0,-.006728e0,-.005837e0,-.005286e0,-.004921e0,
       -.002884e0,-.002583e0,-.002422e0, .002310e0, .002283e0,-.002037e0, .001883e0,
       -.001811e0,-.001687e0,-.001004e0,-.000925e0,-.000844e0, .000766e0, .000766e0,
       -.000700e0,-.000495e0,-.000492e0, .000491e0, .000483e0, .000437e0,-.000416e0,
       -.000384e0, .000374e0,-.000312e0,-.000288e0,-.000273e0, .000259e0, .000245e0,
       -.000232e0, .000229e0,-.000216e0, .000206e0,-.000204e0,-.000202e0, .000200e0,
        .000195e0,-.000190e0, .000187e0, .000180e0,-.000179e0, .000170e0, .000153e0,
       -.000137e0,-.000119e0,-.000119e0,-.000112e0,-.000110e0,-.000110e0, .000107e0,
       -.000095e0,-.000095e0,-.000091e0,-.000090e0,-.000081e0,-.000079e0,-.000079e0,
        .000077e0,-.000073e0, .000069e0,-.000067e0,-.000066e0, .000065e0, .000064e0,
       -.000062e0, .000060e0, .000059e0,-.000056e0, .000055e0,-.000051
    };
    
    // Initialize variables.
    int k   ( 0 );
    int nlp ( 0 );
    int ndi ( 0 );
    int nsd ( 0 );
    
    for (int ll=0;ll<nin;ll++) {
        int ii;
        
        // See if Doodson numbers match
        for (int kk=0;kk<nt;kk++) {
            ii = 0;
        
            for (int i=0;i<6;i++) {
                ii += std::abs ( idd[kk][i]-idtin[ll][i] );
            }
        
            // If you have a match, put line into array [5]
            if ( (!ii) && k<ncon ) {
                
                rl[k] = ampin[ll] * cos (dtr*phin[ll]) / std::abs (tamp[kk]);
                aim[k]= ampin[ll] * sin (dtr*phin[ll]) / std::abs (tamp[kk]);
                /*+------------------------------------------------------------
                * Now have real and imaginary parts of admittance, scaled by 
                * Cartwright-Edden amplitude. Admittance phase is whatever was 
                * used in the original expression. (Usually phase is given 
                * relative to some reference, but amplitude is in absolute 
                * units). Next get frequency.
                *------------------------------------------------------------*/
                iers2010::hisp::tdfrph (idd[kk],itm,fr,pr);
                rf[k] = fr;
                k++;
            }
        }
    }

    /*+---------------------------------------------------------------------
    * Done going through constituents; there are k of them.
    * Have specified admittance at a number of points. Sort these by frequency
    * and separate diurnal and semidiurnal, recopying admittances to get them
    * in order using Shell Sort.
    *----------------------------------------------------------------------*/
    iers2010::hisp::shells (rf,key,k);
    
    for (int i=0;i<k;i++) {
        if (rf[i] < 0.5e0)
            nlp+= 1;
        if ( (rf[i]<1.5e0) && (rf[i]>0.5e0) )
            ndi += 1;
        if ( (rf[i]<2.5e0) && (rf[i]>1.5e0) )
            nsd += 1;
        scr[i] = rl[key[i]];
    }
    
    for (int i=0;i<k;i++) {
        rl[i]  = scr[i];
        scr[i] = aim[key[i]];
    }
    
    for (int i=0;i<k;i++)
        aim[i] = scr[i];

    /*+---------------------------------------------------------------------
    * now set up splines (8 cases - four species, each real and imaginary)
    * We have to allow for the case when there are no constituent amplitudes
    * for the long-period tides.
    *----------------------------------------------------------------------*/
    if (nlp) {
        iers2010::hisp::spline (nlp,rf,rl,zdr,scr);
    }
    if (nlp) {
        iers2010::hisp::spline (nlp,rf,aim,zdi,scr);
    }

    iers2010::hisp::spline (ndi,rf+nlp,rl+nlp,dr,scr);
    iers2010::hisp::spline (ndi,rf+nlp,aim+nlp,di,scr);
    iers2010::hisp::spline (nsd,rf+nlp+ndi,rl+nlp+ndi,sdr,scr);
    iers2010::hisp::spline (nsd,rf+nlp+ndi,aim+nlp+ndi,sdi,scr);
    
    // Evaluate all harmonics using the interpolated admittance
    int j = 0;
    for (int i=0;i<nt;i++) {
        if ( idd[i][0] + nlp ) {
            iers2010::hisp::tdfrph (idd[i],itm,f[j],p[j]);
            //  Compute phase corrections to equilibrium tide using 
            //  function 'eval'
            if (idd[i][0] == 0) {
                p[j] += 180.0e0;
            } else if (idd[i][0] == 1) {
                p[j] += 90.0e0;
            }
            sf = f[j];
            if (idd[i][0] == 0) {
                iers2010::hisp::eval (sf,nlp,rf,rl,zdr,re);
                iers2010::hisp::eval (sf,nlp,rf,aim,zdi,am);
            } else if (idd[i][0] == 1) {
                iers2010::hisp::eval (sf,ndi,rf+nlp,rl+nlp,dr,re);
                iers2010::hisp::eval (sf,ndi,rf+nlp,aim+nlp,di,am);
            } else if (idd[i][0] == 2) {
                iers2010::hisp::eval (sf,nsd,rf+nlp+ndi,rl+nlp+ndi,sdr,re);
                iers2010::hisp::eval (sf,nsd,rf+nlp+ndi,aim+nlp+ndi,sdi,am);
            }
            amp[j] = tamp[i]*sqrt (re*re+am*am);
            p[j] += atan2(am,re) / dtr;
        
            if (p[j] > 180)
                p[j] = p[j]-360.0e0;
        
            j++;
        
        }
    }

    nout = j-1;

    // Finished
    return 0;
 }
