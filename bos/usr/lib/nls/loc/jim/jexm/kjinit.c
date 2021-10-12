static char sccsid[] = "@(#)90	1.2  src/bos/usr/lib/nls/loc/jim/jexm/kjinit.c, libKJI, bos411, 9428A410j 6/6/91 11:20:21";

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
 *	exkjinit(kjcblk)
 *	KJCBLK	*kjcblk;						ptr to KJCBLK
 *
 *	 Initialize the Monitor and ExMon.
 *	This routine can accepts initial string.
 *
 *  11/18/87 first write.
 *  11/20/87 version 4.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>
#include <exmkmisa.h>

exkjinit(kjcblk)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
{
	/*
	 *	temporary variable
	 */

	KJSVPT	*kjsvpt  = kjcblk->kjsvpt;		/* ptr to KJSVPT				*/
	BUFFER	*buffer  = &kjsvpt->buffer;		/* ptr to BUFFER				*/
	KJCBLK	*mkjcblk = kjsvpt->mkjcblk;		/* ptr to MKJCBLK				*/
	int		i;								/* loop counter					*/
	char	*string  = kjcblk->string;		/* ptr to STRING				*/
	char	*hlatst  = kjcblk->hlatst;		/* ptr to HLATST				*/
	char	*istring = buffer->istring;		/* ptr to ISTRING				*/
	char	*ihlatst = buffer->ihlatst;		/* ptr to IHLATST				*/
	PHASE1	*phase1  = &kjsvpt->phase1;		/* ptr to PHASE1 structure		*/
	PHASE2	*phase2  = &kjsvpt->phase2;		/* ptr to PHASE2 structure		*/
	int		monret;							/* Monitor return code			*/
	int		err;							/* error flag					*/

	/*
	 *	check whether the ExMon is already opened or not
	 */

	if (kjcblk == 0 || kjcblk->kjsvpt == 0)
		return EXMONNOTOPEN;				/* ERROR return					*/

	/*
	 *	check whether the ExMon is already initialized or not
	 */

	if (kjsvpt->initflg == INITIALIZED)
		return EXMONNOTTERM;				/* ERROR return					*/

	/*
	 *	validation on input parameter in KJCBLK
	 */

	if (kjcblk->actcol > kjsvpt->extinf.maxstc)
		return EXMONPARMERR;				/* ERROR return					*/
	buffer->actcol = kjcblk->actcol;		/* copy actcol into KJSVPT		*/
	kjcblk->actrow = 1;

	/*
	 *	scan the initial string and copy it to ISTRING
	 */

	for (i = 0, err = FALSE; !err; ) {
		if (i < buffer->actcol)
			if (kj1st(string[i]) && kj2nd(string[i + 1])) {
				istring[i]		= string[i];
				ihlatst[i]		= hlatst[i]		= KJ1st;
				istring[i + 1]	= string[i + 1];
				ihlatst[i + 1]	= hlatst[i + 1]	= KJ2nd;
				i += 2;
			}
			else if (jiscii(string[i])) {
				istring[i]		= string[i];
				ihlatst[i]		= hlatst[i]		= JISCII;
				i += 1;
			}
			else							/* invalid code detected		*/
				err = TRUE;
		else								/* i >= kjsvpt->actcol			*/
			err = TRUE;
	}
	kjcblk->lastch = i;
	for (i = kjcblk->lastch; i < kjsvpt->extinf.maxstc; i++)
		string[i] = hlatst[i] = 0;			/* clear rest of inited area	*/
	hlatst[kjsvpt->extinf.maxstc] = 0;

	kjcblk->currow = 0;
	kjcblk->curcol = MIN(kjcblk->curcol,kjcblk->lastch);
	if (hlatst[kjcblk->curcol] == KJ2nd)
		kjcblk->curcol--;

	/*
	 *	clear and set PHAES1 and PHASE2 structure
	 */

	for (i = 0; i < sizeof(PHASE1); i++)
		((char *)phase1)[i] = NULL;
	for (i = 0; i < sizeof(PHASE2); i++)
		((char *)phase2)[i] = NULL;
	phase2->state = NCS;
	phase2->curis = phase2->curst = kjcblk->curcol;
	phase2->lastis = phase2->lastst = kjcblk->lastch;
	phase2->topim = phase2->curim = -1;

	for (i = phase2->lastis; i < buffer->maxis; i++)
		istring[i] = ihlatst[i] = 0;		/* clear rest of inited area	*/

	/*
	 *	initialize the Monitor
	 */

	mkjcblk->actcol = 2 * buffer->actcol + MAXLENOFKJ + 2;
	mkjcblk->actrow = 1;
	mkjcblk->curcol = mkjcblk->currow = 0;
	mkjcblk->flatsd = ONLYDOUBLE;
	if (kjcblk->repins == INSERT)
		mkjcblk->repins = INSERT;
	else
		mkjcblk->repins = kjcblk->repins = REPLACE;
	mkjcblk->conv = kjcblk->conv;
	monret = _Jinit(mkjcblk);
	if (monret != 0) return monret;			/* ERROR return					*/

	/*
	 *	clear the monitor
	 */

	_Jclr(mkjcblk);

	/*
	 *	set other parameters
	 */

	buffer->string = string;				/* ptr to STRING				*/
	kjsvpt->initflg = INITIALIZED;			/* kjsvpt->initflg				*/
	kjsvpt->beep = ((KMISA *)mkjcblk->kjsvpt)->kmpf[0].beep;
											/* beep available flag			*/
	kjsvpt->normalbs = TRUE;

	while (mkjcblk->shift1 != ALPHANUM)
		inpr2(PALPHANUM)					/* force Monitor AN mode		*/
	kjcblk->shift1 = ALPHANUM;

	while (mkjcblk->shift2 != RKC_OFF)
		inpr2(PRKC)							/* force Monitor RKC off mode	*/
	kjcblk->shift2 = RKC_OFF;

	kjcblk->shift3 = SINGLE;				/* kjcblk->shift3				*/
	kjcblk->shift  = SHIFT1 | SHIFT2 | SHIFT3;
											/* kjcblk->shift				*/
	kjcblk->cnvsts = FINISHED;				/* kjcblk->cnvsts				*/
	kjcblk->axuse1 = NOTUSE;				/* kjcblk->axuse1				*/
	kjcblk->beep   = OFF;					/* kjcblk->beep					*/
	kjcblk->discrd = DISCARD;				/* kjcblk->discrd				*/
	kjcblk->chpos  = 0;						/* kjcblk->chpos				*/
	kjcblk->chlen  = kjcblk->lastch;		/* kjcblk->chlen				*/
	kjcblk->chpsa1 = 0;						/* kjcblk->chpsa1				*/
	kjcblk->chlna1 = 0;						/* kjcblk->chlna1				*/
	kjcblk->cura1c = -1;					/* kjcblk->cura1c				*/
	kjcblk->cura1r = -1;					/* kjcblk->cura1r				*/
	if (hlatst[kjcblk->curcol] == KJ1st)	/* kjcblk->curlen				*/
		kjcblk->curlen = CURATD;
	else
		kjcblk->curlen = CURATS;

	return	EXMONNOERR;
}
