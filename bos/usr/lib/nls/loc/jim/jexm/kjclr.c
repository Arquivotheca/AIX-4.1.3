static char sccsid[] = "@(#)88	1.2  src/bos/usr/lib/nls/loc/jim/jexm/kjclr.c, libKJI, bos411, 9428A410j 6/6/91 11:19:56";

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
 *  ExMon version 6.4		11/11/88
 *	exkjclr(kjcblk)
 *	KJCBLK	*kjcblk						ptr to KJCBLK
 *		The ExMon clear rouinte.
 *
 *	11/18/87 first write.
 *  09/20/88 add exkjclr2().   Clear without RKC.
 */

#include <exmdefs.h>
#include <exmctrl.h>

/*
 *	clear all.
 */

exkjclr(kjcblk)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
{
	return netexkjclr(kjcblk, TRUE);
}

/*
 *	clear without rkc.
 */

exkjclr2(kjcblk)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
{
	return netexkjclr(kjcblk, FALSE);
}

static	netexkjclr(kjcblk, rkc)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
int	rkc;									/* rkc buf clear if true.		*/
{
	/*
	 *	definition of temporary variables
	 */

	KJSVPT	*kjsvpt  = kjcblk->kjsvpt;		/* ptr to KJSVPT structure		*/
	PHASE1	*phase1  = &kjsvpt->phase1;		/* ptr to PHASE1 structure		*/
	PHASE2	*phase2  = &kjsvpt->phase2;		/* ptr to PHASE2 structure		*/
	KJCBLK	*mkjcblk = kjsvpt->mkjcblk;		/* ptr to MKJCBLK structure		*/
	BUFFER	*buffer  = &kjsvpt->buffer;		/* ptr to BUFFER structure		*/
	int		i;								/* loop counter					*/
	char	*string  = kjcblk->string;		/* ptr to STRING				*/
	char	*hlatst  = kjcblk->hlatst;		/* ptr to HLATST				*/
	char	*istring = buffer->istring;		/* ptr to ISTRING				*/
	char	*ihlatst = buffer->ihlatst;		/* ptr to IHLATST				*/
	char	*msimage = buffer->msimage;		/* ptr to MSIMAGE				*/
	char	*mhimage = buffer->mhimage;		/* ptr to MHIMAGE				*/
	char	*sdttbl  = buffer->sdttbl;		/* ptr to SDTTBL				*/
	char	*sdtwork = buffer->sdtwork;		/* ptr to SDTWORK				*/
	int		rslen;
	char	rest[P1BUFSIZE];

	/*
	 *	check whether the ExMon is already opened or not
	 */

	if(kjcblk == 0 || kjcblk->kjsvpt == 0)
		return EXMONNOTOPEN;				/* ERROR return					*/

	/*
	 *	check whether the ExMon is already initialized or not
	 */

	if(kjsvpt->initflg == NOTINITIALIZED)
		return EXMONNOTINIT;				/* ERROR return					*/

	/*
	 *	if AUX area is in use, then terminate it
	 */

	if(phase2->axuse1 == USE)				/* send PRESET to Monitor		*/
		inpr2(PRESET)

	/*
	 *	clear the Monitor
	 */

	_Jclr(mkjcblk);

	/*
	 *	clear buffers in Ex. Monitor
	 */

	for(i = 0;i < kjsvpt->extinf.maxstc;i++)/* clear STRING and HLATST		*/
		string[i] = hlatst[i] = 0;
	for(i = 0;i < buffer->maxis;i++)		/* clear ISTRING and IHLATST	*/
		istring[i] = ihlatst[i] = 0;
	for(i = 0;i < buffer->maxim;i++)		/* clear MSIMAGE and MHIMAGE	*/
		msimage[i] = mhimage[i] = 0;
	for(i = 0;i < buffer->maxsd;i++)		/* clear SDTTBL and SDTWORK		*/
		sdttbl[i]  = sdtwork[i] = 0;

	/*
	 *	clear contents of PHASE1 and PHASE2 structure
	 */

	if(!rkc) {
		rslen = phase1->rslen;
		for(i = 0;i < rslen;i++)
			rest[i] = phase1->rest[i];
	}
	for(i = 0;i < sizeof(PHASE1);i++)
		((char *)phase1)[i] = NULL;
	if(!rkc) {
		phase1->rslen = rslen;
		for(i = 0;i < rslen;i++)
			phase1->rest[i] = rest[i];
	}
	for(i = 0;i < sizeof(PHASE2);i++)
		((char *)phase2)[i] = NULL;
	phase2->state = NCS;
	phase2->topim = phase2->curim = -1;

	/*
	 *	clear contents of KJCBLK
	 */

	kjcblk->curcol = kjcblk->currow =  0;
	kjcblk->cura1c = kjcblk->cura1r = -1;
	kjcblk->chpos = 0;
	kjcblk->chlen = kjcblk->lastch;
	kjcblk->lastch = 0;
	kjcblk->axuse1 = NOTUSE;
	kjcblk->indlen = 0;
	kjcblk->type = kjcblk->code = 0;

	inpr2(PALPHANUM)							/* make Monitor in AN mode	*/

	kjcblk->shift = 0;							/* clear the shift chg flag */
	if(kjcblk->shift1 != ALPHANUM){				/* reset input mode			*/
		kjcblk->shift1 = ALPHANUM;				/* Alpha_Numeric mode		*/
		kjcblk->shift |= SHIFT1;
	}
	if(kjcblk->shift2 != RKC_OFF){				/* RKC_off mode				*/
		kjcblk->shift2 = RKC_OFF;
		kjcblk->shift |= SHIFT2;
	}
	if(kjcblk->shift3 != SINGLE){				/* Single character mode	*/
		kjcblk->shift3 = SINGLE;
		kjcblk->shift |= SHIFT3;
	}
	
	kjcblk->curlen = CURATS;
	kjcblk->cnvsts = FINISHED;
	kjcblk->beep = OFF;
	kjcblk->discrd = DISCARD;

	kjsvpt->initflg = INITIALIZED;

	return EXMONNOERR;
}
