/* @(#)48	1.4  src/bos/usr/lib/methods/common/cfgdebug.h, cfgmethods, bos411, 9428A410j 6/15/91 16:21:01 */
#ifndef _H_CFGDEBUG
#define _H_CFGDEBUG
/*********************************************************************
 *
 * COMPONENT_NAME: CFGMETHODS
 *
 * ORIGINS : 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * Unpublished Work
 * All Rights Reserved
 *
 * RESTRICTED RIGHTS LEGEND
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ***********************************************************************/

/* DEBUGGING AIDS: */

#ifdef CFGDEBUG
#include <stdio.h>
#define DEBUG_0(A)			{fprintf(stderr,A);fflush(stderr);}
#define DEBUG_1(A,B)			{fprintf(stderr,A,B);fflush(stderr);}
#define DEBUG_2(A,B,C)			{fprintf(stderr,A,B,C);fflush(stderr);}
#define DEBUG_3(A,B,C,D)		{fprintf(stderr,A,B,C,D);fflush(stderr);}
#define DEBUG_4(A,B,C,D,E)		{fprintf(stderr,A,B,C,D,E);fflush(stderr);}
#define DEBUG_5(A,B,C,D,E,F)		{fprintf(stderr,A,B,C,D,E,F);fflush(stderr);}
#define DEBUG_6(A,B,C,D,E,F,G)		{fprintf(stderr,A,B,C,D,E,F,G);fflush(stderr);}
#define DEBUGELSE			else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUGELSE
#endif
#endif /* _H_CFGDEBUG */
