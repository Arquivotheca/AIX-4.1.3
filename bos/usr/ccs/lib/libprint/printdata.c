static char sccsid[] = "@(#)32	1.1  src/bos/usr/ccs/lib/libprint/printdata.c, libprint, bos411, 9428A410j 9/30/89 15:39:04";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: printdata
 *
 * ORIGINS: 4,10,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


#include    <lprio.h>

#define SIZEACR     480
#define SIZEDOWN    (72 * 6)
#define SIZEBITS    (SIZEACR / 16 * SIZEDOWN)
short   bitarea[SIZEBITS];
int     printhpos = 0;
int     printvpos = 0;
int     printempty = 1;
float   obotx = 0.;
float   oboty = 0.;
float   botx = 0.;
float   boty = 0.;
float   scalex = 1.;
float   scaley = 1.;
int     scaleflag = 0;
struct      lprmode     lprmode;
int         lpmode;

