/* @(#)82	1.3  src/bos/usr/ccs/lib/libm/POWER/atan.h, libm, bos411, 9428A410j 6/15/90 17:57:01 */
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: header file for atan
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
static long bound = 0x434d0297;
static double CCLVI = 256.0;
static double big2= 6.7553994410557440000e+15; /* 0x43380000, 0x00000000 */
static double pihh = 1.570796326794896619231322;
static double pihl = 6.1232339957367660e-17;
 
/* atan(x) = x + c[5] * x**3 + c[4] * x**5 + c[3] * x**7 +
	    c[2] * x**9 + c[1] * x**11 + c[0] * x**13
	with relative accuracy 6.2E-20 on [-1/16,1/16] 
*/

static double c13 =7.6018324085327799000e-02;  /* 0x3FB375EF, 0xD7D7DCBE */
static double c11 = -9.0904243427904791000e-02; /* 0xBFB74580, 0x2097294E */
static double c9 = 1.1111109821671356000e-01;  /* 0x3FBC71C6, 0xE5103CDD */
static double c7 = -1.4285714283952728000e-01; /* 0xBFC24924, 0x923F7566 */
static double c5 = 1.9999999999998854000e-01;  /* 0x3FC99999, 0x999997FD */
static double c3 = -3.3333333333333330000e-01; /* 0xBFD55555, 0x55555555 */
 
struct tdef { double p;
	      double f5;
	      double f4;
	      double f3;
	      double f2;
	      double f1;
	      double f0;
	    } *t;
