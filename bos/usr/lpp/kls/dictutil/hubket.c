static char sccsid[] = "@(#)86	1.1  src/bos/usr/lpp/kls/dictutil/hubket.c, cmdkr, bos411, 9428A410j 5/25/92 14:40:19";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hubket.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hubket.c
 *
 *  Description:  display left and right bracket at given coordinate.
 *
 *  Functions:    hubket()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/
#include "hut.h"
#include <stdio.h>

int hubracket(line,col,dtlen)
short   line;   /* data display line    */
short   col;    /* data display column  */
short   dtlen;  /* data length  */
{
    int         rp,lp;
    static char *bracket[] =  { "[", "]" };

    lp = col - 1;
    rp = col + dtlen;
    CURSOR_MOVE(line,lp);
    fprintf(stdout,"%s",bracket[0]);
    CURSOR_MOVE(line,rp);
    fprintf(stdout,"%s",bracket[1]);
    return( IUSUCC );
}
