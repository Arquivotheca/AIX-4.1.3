static char sccsid[] = "@(#)89	1.2  src/bos/usr/lib/nls/loc/jim/jexm/kjcrst.c, libKJI, bos411, 9428A410j 6/6/91 11:20:08";

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
 *	exkjcrst(kjcblk)
 *		argument
 *			KJCBLK	*kjcblk;		pointer to KJCBLK
 *
 *		The ExMon input processing routine.
 *
 *  06/28/88 version 6. first write.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

exkjcrst(kjcblk)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
{
	/*
	 *	check whether kjcblk is valid or not.
	 */

	if (kjcblk == 0 || kjcblk->kjsvpt == 0)
		return EXMONNOTOPEN;				/* ERROR return					*/
	if (kjcblk->kjsvpt->initflg == NOTINITIALIZED)
		return EXMONNOTINIT;				/* ERROR return					*/

	kjcblk->type = TYPE2;
	kjcblk->code = PSETCUR;
	return exkjinpr(kjcblk);
}
