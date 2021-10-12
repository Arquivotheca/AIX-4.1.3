static char sccsid[] = "@(#)96	1.2  src/bos/usr/lib/nls/loc/jim/jexm/m2cmpmv.c, libKJI, bos411, 9428A410j 6/6/91 11:22:06";

/*
 * COMPONENT_NAME :	Japanese Input Method - Ex Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  ExMon version 6.3		11/11/88
 *	m2cmpmv(srcstr,srcflg,srcoff,dststr,dstflg,dstoff,i,schg)
 *		compare and move.
 *
 *	m2cmp(srcstr,srcflg,srcoff,dststr,dstflg,dstoff,i,echg)
 *		compare.
 *
 *  11/18/87 first write.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

void	m2cmpmv(srcstr,srcflg,srcoff,dststr,dstflg,dstoff,i,schg)
char	*srcstr;							/* ptr to source string			*/
char	*srcflg;							/* ptr to source strflg			*/
int		srcoff;								/* offset in source string		*/
char	*dststr;							/* ptr to destination string	*/
char	*dstflg;							/* ptr to destination strflg	*/
int		dstoff;								/* offset in destination string	*/
int		*i;									/* ptr to index					*/
int		*schg;								/* ptr to end of changed area	*/
{
	if ((srcflg[srcoff] & KJMASK) == KJ1st) {
		if (dststr[dstoff] == srcstr[srcoff]			&&
			dstflg[dstoff] == srcflg[srcoff]			&&
			dststr[dstoff + 1] == srcstr[srcoff + 1]	&&
			dstflg[dstoff + 1] == srcflg[srcoff + 1]	)
			*i += 2;
		else {
			*schg = *i;
			dststr[dstoff] = srcstr[srcoff];
			dstflg[dstoff] = srcflg[srcoff];
			dststr[dstoff + 1] = srcstr[srcoff + 1];
			dstflg[dstoff + 1] = srcflg[srcoff + 1];
			*i += 2;
		}
	}
	else {
		if (dststr[dstoff] == srcstr[srcoff]	&&
			dstflg[dstoff] == srcflg[srcoff]	)
			*i += 1;
		else {
			*schg = *i;
			dststr[dstoff] = srcstr[srcoff];
			dstflg[dstoff] = srcflg[srcoff];
			*i += 1;
		}
	}
}


void	m2cmp(srcstr,srcflg,srcoff,dststr,dstflg,dstoff,i,echg)
char	*srcstr;							/* ptr to source string			*/
char	*srcflg;							/* ptr to source strflg			*/
int		srcoff;								/* offset in source string		*/
char	*dststr;							/* ptr to destination string	*/
char	*dstflg;							/* ptr to destination strflg	*/
int		dstoff;								/* offset in destination string	*/
int		*i;									/* ptr to index					*/
int		*echg;								/* ptr to end of changed area	*/
{
	if ((srcflg[srcoff] & KJMASK) == KJ2nd) {
		if (dststr[dstoff] == srcstr[srcoff]			&&
			dstflg[dstoff] == srcflg[srcoff]			&&
			dststr[dstoff - 1] == srcstr[srcoff - 1]	&&
			dstflg[dstoff - 1] == srcflg[srcoff - 1]	)
			*i -= 2;
		else
			*echg = *i + 1;
	}
	else{
		if (dststr[dstoff] == srcstr[srcoff]	&&
			dstflg[dstoff] == srcflg[srcoff]	)
			*i -= 1;
		else
			*echg = *i + 1;
	}
}
