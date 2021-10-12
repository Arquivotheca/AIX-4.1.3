static char sccsid[] = "@(#)87	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jexm/kjclos.c, libKJI, bos411, 9428A410j 7/23/92 01:55:57";
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
 *	exkjclos(kjcblk)
 *	KJCBLK	*kjcblk;	ptr to KJCBLK
 *
 *	The ExMon close routine.
 *
 *	11/18/87 first write.
 *
 */


#include <exmdefs.h>
#include <exmctrl.h>

exkjclos(kjcblk)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
{
	/*
	 *	temporary variable
	 */

	KJSVPT	*kjsvpt = kjcblk->kjsvpt;		/* ptr to KJSVPT				*/
	BUFFER	*buffer = &kjsvpt->buffer;		/* ptr to BUFFER				*/
	char	*mstring;						/* temporaly ptr to MSTRING		*/
											/* save area					*/
	int		monret;							/* Monitor return code			*/

	/*
	 *	check whether the ExMon is already terminated or not
	 */

	if(kjsvpt->initflg == NOTTERMINATED)
		return EXMONNOTTERM;

	/*
	 *	Monitor close
	 */

	mstring = kjsvpt->mkjcblk->string;		/* save ptr to MSTRING			*/
	monret = _Jclos(kjsvpt->mkjcblk);		/* close the Monitor			*/
	if(monret != 0)							/* ERROR return					*/
		return monret;

	/*
	 *	deallocation of contorol blocks and buffers
	 */

	free(mstring);							/* MSTRING						*/
	free(buffer->sdttbl);					/* SDTTBL						*/
	free(buffer->sdtwork);					/* SDTWORK						*/
	free(buffer->msimage);					/* MSIMAGE						*/
	free(buffer->mhimage);					/* MHIMAGE						*/
	free(buffer->istring);					/* ISTRING						*/
	free(buffer->ihlatst);					/* IHLATST						*/
	free(kjcblk->hlatst);					/* HLATST						*/
	free(kjsvpt);							/* KJSVPT						*/
	kjcblk->kjsvpt = (KJSVPT *)NULL;		/* clear the ptr to KJSVPT		*/
	free(kjcblk);							/* KJCBLK						*/

	return	EXMONNOERR;
}
