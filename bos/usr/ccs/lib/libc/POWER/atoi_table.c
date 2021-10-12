static char sccsid[] = "@(#)41	1.3  src/bos/usr/ccs/lib/libc/POWER/atoi_table.c, libccnv, bos411, 9428A410j 6/8/93 14:18:40";
/*
 * COMPONENT_NAME: LIBCCNV 
 *
 * FUNCTIONS: none.  static table __ctype
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "atoi_local.h"

/* 
 * this table is used in atoi, atol, strtol, and strtoul.
 * It serves the following functions:  it identifies 
 * characters which are digits (in any radix up to 36),
 * and provides the 'value' of the character.
 * This is considerably faster than a construct such as
 *       while(*nptr >= '0' && *nptr <= '9')
 * or
 *       #define DIGIT(x) (isdigit((int)x)? ((x)-'0'): (10+tolower((int)x)-'a'))
 * There is no risk in this approach for any single byte
 * code page, since all of these characters are below 0x7F
 * (the ascii invariant region).
 *
 * it also identifies white space characters, they have the
 * value "SP" (which is defined in atoi_local.h).  This
 * replaces the call to isspace.  This is not
 * risk free, since the hard-code values are guaranteed
 * to be white space in all single-byte code sets, the 
 * list is not guaranteed to be exhaustive.  However, at
 * the time of 3.2 release there are code sets with other
 * white space characters, so with the agreement of the
 * ILS architect, we are keeping these hard-coded.
 *
 * all other character have the value 99
 */


int __ctype[] = { 

/*	 0	 1	 2	 3	 4	 5	 6	 7  */

/* 0*/	99,	99,	99,	99,	99,	99,	99,	99,
/* 10*/	99,	SP,   	SP,   	SP,   	SP,   	SP,   	99,	99,
/* 20*/	99,	99,	99,	99,	99,	99,	99,	99,
/* 30*/	99,	99,	99,	99,	99,	99,	99,	99,
/* 40*/	SP,	99,	99,	99,	99,	99,	99,	99,
/* 50*/	99,	99,	99,	99,	99,	99,	99,	99,
/* 60*/	 0,   	 1,   	 2,   	 3,   	 4,   	 5,   	 6,   	 7,   
/* 70*/	 8,   	 9,   	99,	99,	99,	99,	99,	99,
/*100*/	99,	10,   	11,   	12,   	13,   	14,   	15,   	16,
/*110*/	17,	18,	19,	20,	21,	22,	23,	24,
/*120*/	25,	26,	27,	28,	29,	30,	31,	32,
/*130*/	33,	34,	35,	99,	99,	99,	99,	99,
/*140*/ 99,     10,     11,     12,     13,     14,     15,     16,
/*150*/ 17,     18,     19,     20,     21,     22,     23,     24,
/*160*/ 25,     26,     27,     28,     29,     30,     31,     32,
/*170*/ 33,     34,     35,     99,     99,     99,     99,     99,
/*200*/	99,	99,	99,	99,	99,	99,	99, 	99,
/*210*/	99,	99,	99,	99,	99,	99,	99,	99,
/*220*/	99,	99,	99,	99,	99,	99,	99,	99,
/*230*/	99,	99,	99,	99,	99,	99,	99,	99,
/*240*/	99,	99,   	99,   	99,   	99,   	99,   	99,   	99,
/*250*/	99,	99,	99,	99,	99,	99,	99,	99,
/*260*/	99,	99,	99,	99,	99,	99,	99,	99,
/*270*/	99,	99,	99,	99,	99,	99,	99,	99,
/*300*/	99,	99,	99,	99,	99,	99,	99, 	99,
/*310*/	99,	99,	99,	99,	99,	99,	99,	99,
/*320*/	99,	99,	99,	99,	99,	99,	99,	99,
/*330*/	99,	99,	99,	99,	99,	99,	99,	99,
/*340*/	99,	99,   	99,   	99,   	99,   	99,   	99,   	99,
/*350*/	99,	99,	99,	99,	99,	99,	99,	99,
/*360*/	99,	99,	99,	99,	99,	99,	99,	99,
/*370*/	99,	99,	99,	99,	99,	99,	99,	99,
/*400*/	99,	99,	99,	99,	99,	99,	99, 	99
};


/*
 * These next two tables are used by strtol, strtoul, strtoll, strtoull,
 * wcstol, wcstoul, wcstoll, wcstoull.  They hold the number of digits
 * that a number can have without overflow in either 32 or 64 bits.
 * The maximum number of significant base-nn "digits" before overflow.
 * Each entry is ceil(MAX_BITS*ln(2)/ln(base)); MAX_BITS = 32 or 64.
 * Each entry is smallest n such that base**n >= 2**MAX_BITS.
 */

const int __32[] = {
				 0,	/* 0 */
				 0,	/* 1 */
				 32,	/* 2: 32    */
				 21,	/* 3: 20.19 */
				 16,	/* 4: 16    */
				 14,	/* 5: 13.78 */
				 13,	/* 6: 12.38 */
				 12,	/* 7: 11.40 */
				 11,	/* 8: 10.67 */
				 11,	/* 9: 10.09 */
				 10,	/* 10: 9.63 */
				 10,	/* 11: 9.25 */
				 9,	/* 12: 8.93 */
				 9,	/* 13: 8.65 */
				 9,	/* 14: 8.40 */
				 9,	/* 15: 8.19 */
				 8,	/* 16: 8    */
				 8,	/* 17: 7.83 */
				 8,	/* 18: 7.67 */
				 8,	/* 19: 7.53 */
				 8,	/* 20: 7.40 */
				 8,	/* 21: 7.29 */
				 8,	/* 22: 7.18 */
				 8,	/* 23: 7.07 */
				 7,	/* 24: 6.98 */
				 7,	/* 25: 6.89 */
				 7,	/* 26: 6.81 */
				 7,	/* 27: 6.73 */
				 7,	/* 28: 6.66 */
				 7,	/* 29: 6.59 */
				 7,	/* 30: 6.52 */
				 7,	/* 31: 6.46 */
				 7,	/* 32: 6.40 */
				 7,	/* 33: 6.34 */
				 7,	/* 34: 6.29 */
				 7,	/* 35: 6.24 */
				 7	/* 36: 6.19 */
};

const int __64[] = {
                                 0,	/* 0 */
				 0,	/* 1 */
				 64,	/* 2: 64    */
				 41,	/* 3: 40.38 */
				 32,	/* 4: 32    */
				 28,	/* 5: 27.56 */
				 25,	/* 6: 24.76 */
				 23,	/* 7: 22.80 */
				 22,	/* 8: 21.33 */
				 21,	/* 9: 20.19 */
				 20,	/* 10: 19.27 */
				 19,	/* 11: 18.50 */
				 18,	/* 12: 17.85 */
				 18,	/* 13: 17.30 */
				 17,	/* 14: 16.81 */
				 17,	/* 15: 16.38 */
				 16,	/* 16: 16    */
				 16,	/* 17: 15.66 */
				 16,	/* 18: 15.35 */
				 16,	/* 19: 15.07 */
				 15,	/* 20: 14.81 */
				 15,	/* 21: 14.57 */
				 15,	/* 22: 14.35 */
				 15,	/* 23: 14.15 */
				 14,	/* 24: 13.96 */
				 14,	/* 25: 13.78 */
				 14,	/* 26: 13.62 */
				 14,	/* 27: 13.46 */
				 14,	/* 28: 13.31 */
				 14,	/* 29: 13.17 */
				 14,	/* 30: 13.04 */
				 13,	/* 31: 12.92 */
				 13,	/* 32: 12.80 */
				 13,	/* 33: 12.69 */
				 13,	/* 34: 12.58 */
				 13,	/* 35: 12.48 */
				 13	/* 36: 12.38 */
};
