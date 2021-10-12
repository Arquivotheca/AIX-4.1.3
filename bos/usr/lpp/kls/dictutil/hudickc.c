static char sccsid[] = "@(#)07	1.1  src/bos/usr/lpp/kls/dictutil/hudickc.c, cmdkr, bos411, 9428A410j 5/25/92 14:43:50";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudickc.c
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
 *  Module:       hudickc.c
 *
 *  Description:  Check if key code is valid.
 *
 *  Functions:    hudickc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      Include File.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include "hut.h"        /* Dictionary Utility Define File.                   */

void    hudickc( key, keylen, mflag )

uchar   *key;      /* key string.                   */
short   keylen;    /* key string length.            */
short   *mflag;    /* key type flag.                */
{

        int     i, j;		/* Loop Counter.                */
        int	code;
        uchar   eflg;		/* error flag.           */
        uchar   hflg;		/* hangeul code flag.           */
        uchar   nflg;		/* numeric code flag.           */

        /*  1.
         *      Initialize the parameter area.
         */

	eflg = C_SWOFF;
	hflg = C_SWOFF;
	nflg = C_SWOFF;

	j = 0;

        while ( keylen > 0 )  {
		code = key[0] << 8;
		code |= key[1];
		key += C_DBCS;
		keylen -= C_DBCS;

		if (U_CHK_HL <= code && code <= U_CHK_HH)
		{
			hflg = C_SWON;
			continue;
                }
		else if (U_CHK_NL <= code && code <= U_CHK_NH)
		{
			nflg = C_SWON;
			continue;
		}
                else
		{
                        eflg = C_SWON;   /*  Set error flag    */
			break;
                }
        }  

	if (eflg == C_SWON)
		*mflag = U_INVALD;
	else if (hflg == C_SWON && nflg == C_SWON)
		*mflag = U_HNMIX;
	else if (hflg == C_SWON)
		*mflag = U_HANGEUL;
	else if (nflg == C_SWON)
		*mflag = U_NUMERIC;

        return;
}
