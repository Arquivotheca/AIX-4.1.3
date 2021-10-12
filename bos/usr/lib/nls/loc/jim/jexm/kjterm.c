static char sccsid[] = "@(#)93	1.2  src/bos/usr/lib/nls/loc/jim/jexm/kjterm.c, libKJI, bos411, 9428A410j 6/6/91 11:21:06";

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
 *	exkjterm(kjcblk)
 *	KJCBLK	*kjcblk;						ptr to KJCBLK
 *
 *	terminate the ExMon.
 *
 *  11/18/87 first write.
 *  11/20/87 version 4.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

exkjterm(kjcblk)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
{
	/*
	 *	temporary variable
	 */

	KJSVPT	*kjsvpt  = kjcblk->kjsvpt;		/* ptr to KJSVPT				*/
	KJCBLK	*mkjcblk = kjsvpt->mkjcblk;		/* ptr to MKJCBLK				*/
	int		i;								/* loop counter					*/
	BUFFER	*buffer  = &kjsvpt->buffer;		/* ptr to BUFFER				*/
	char	*string  = kjcblk->string;		/* ptr to STRING				*/
	char	*hlatst  = kjcblk->hlatst;		/* ptr to HLATST				*/
	char	*istring = buffer->istring;		/* ptr to ISTRING				*/
	char	*ihlatst = buffer->ihlatst;		/* ptr to IHLATST				*/
	PHASE1	*phase1  = &kjsvpt->phase1;		/* ptr to PHASE1 structure		*/
	PHASE2	*phase2  = &kjsvpt->phase2;		/* ptr to PHASE2 structure		*/
	int		monret;							/* Monitor return code			*/
	int		schg;							/* start pos of changed area	*/
	int		echg;							/* end pos of changed area		*/
	int		imax;							/* loop limit					*/
	int		lastch;							/* save last char position		*/

	/*
	 *	decleration of external functions
	 */

	extern	void	m2cmp();				/* compare character			*/
	extern	void	m2cmpmv();				/* compare and move character	*/

	/*
	 *	check whether the ExMon is already opened or not
	 */

	if (kjcblk == 0 || kjcblk->kjsvpt == 0)
		return EXMONNOTOPEN;				/* ERROR return					*/

	/*
	 *	check whether the ExMon is already initialized or not
	 */

	if (kjsvpt->initflg == NOTINITIALIZED)
		return EXMONNOTINIT;				/* ERROR return					*/

	/*
	 *	ISTRING confirmation
	 */

	if (kjcblk->axuse1 == USE)
		inpr2(PRESET)
	confirm(kjsvpt);
	lastch = phase2->lastis;
	if (lastch > buffer->actcol) {			/* if overflow, trunc the rest	*/
		while (--lastch >= buffer->actcol)
			istring[lastch] = ihlatst[lastch] = 0;
		if ((ihlatst[lastch] & KJMASK) == KJ1st)
			istring[lastch] = ihlatst[lastch] = 0;
		else
			lastch++;
		phase2->lastis = lastch;
		if (phase2->curis > lastch)
			phase2->curis = lastch;
	}

	/*
	 *	move ISTRING and IHLATST to STRING and HLATST
	 */

	if (kjcblk->lastch > lastch) {
		echg = kjcblk->lastch;
		imax = lastch;
		for (i = imax; i < kjcblk->lastch; i++)
			string[i] = hlatst[i] = 0;
	}
	else if (kjcblk->lastch < lastch) {
		echg = lastch;
		imax = lastch;
	}
	else {
		echg = 0;
		for (i = lastch - 1; i >= 0 && echg == 0; )
			m2cmp(istring,ihlatst,i,string,hlatst,i,&i,&echg);
		imax = echg;
	}
	for (i = 0, schg = imax; i < imax; ) {
		if (schg == imax)
			m2cmpmv(istring,ihlatst,i,string,hlatst,i,&i,&schg);
		else {					/* chg area already found		*/
			string[i] = istring[i];
			hlatst[i] = ihlatst[i];
			i += 1;
		}
	}
	kjcblk->curcol = phase2->curis;
	if ((hlatst[kjcblk->curcol] & KJMASK) == KJ1st)
		kjcblk->curlen = CURATD;
	else
		kjcblk->curlen = CURATS;
	kjcblk->chpos = schg;
	kjcblk->chlen = echg - schg;
	kjcblk->lastch = lastch;

	/*
	 *	clear and set PHAES1 and PHASE2 structure
	 */

	for (i = 0; i < sizeof(PHASE1); i++)
		((char *)phase1)[i] = NULL;
	for (i = 0; i < sizeof(PHASE2); i++)
		((char *)phase2)[i] = NULL;
	phase2->state = NCS;
	phase2->topim = phase2->curim = -1;

	/*
	 *	clear ISTRING and IHLATST.
	 */

	for (i = 0; i < buffer->maxis; i++)
		istring[i] = ihlatst[i] = NULL;

	/*
	 *	set other parameters
	 */

	kjsvpt->initflg = TERMINATED;

	inpr2(PALPHANUM)						/* let the Monitor AN mode		*/
	kjcblk->shift1 = ALPHANUM;

	kjcblk->cnvsts = FINISHED;
	kjcblk->beep = OFF;
	kjcblk->axuse1 = NOTUSE;

	/*
	 *	terminate the Monitor
	 */

	monret = _Jterm(mkjcblk);
	if (monret != 0) return monret;			/* ERROR return					*/

	return	EXMONNOERR;
}
