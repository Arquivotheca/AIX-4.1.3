static char sccsid[] = "@(#)81	1.2  src/bos/usr/lib/nls/loc/jim/jexm/confirm.c, libKJI, bos411, 9428A410j 6/6/91 11:18:40";

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
 *
 *  confirm(kjsvpt)
 *      argument
 *          KJSVPT  *kjsvpt;        pointer to KJSVPT
 *
 *		description
 *			This routine makes ISTRING confirm.
 *			Replace or insert MSIMAGE to ISTRING
 *			and clear SDTTBL, MSIMAGE and MHIMAGE.
 *
 *  11/18/87 first write.
 *  11/20/87 version 4.
 *  11/23/87 fix a bug.
 */

#include <exmdefs.h>
#include <exmctrl.h>

confirm(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	/*
	 *	decleration of temp variables
	 */

	int		i, len, src;					/* general loop counter			*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	char	*msimage	= buffer->msimage;	/* ptr to MSIMAGE				*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	char	*sdtwork	= buffer->sdtwork;	/* ptr to SDTWORK				*/

	/*
	 *	move the MSIMAGE into the ISTRING
	 */

	if (phase2->lastim > 0) {				/* is there any char in MSIMAGE?*/
		if (phase2->repins == INSERT) {		/* if replace mode				*/
			len = MIN(phase2->lastis - phase2->topim,
				buffer->maxis - phase2->topim - phase2->lastim);
			for (src = phase2->topim + len - 1, i = 0; i < len; i++, src--) {
				istring[src + phase2->lastim] = istring[src];
				ihlatst[src + phase2->lastim] = ihlatst[src];
			}								/* move ISTRING after topmstr	*/
			phase2->lastis += phase2->lastim;
			if (phase2->lastis > buffer->maxis)
				phase2->lastis = buffer->maxis;
			if (phase2->curis >= phase2->topim)
				phase2->curis += phase2->lastim;
		}
		len = MIN(phase2->lastim, buffer->maxis - phase2->topim);
		for (i = 0; i < len; i++) {
			istring[phase2->topim + i] = msimage[i];
			ihlatst[phase2->topim + i] = mhimage[i] & KJMASK;
		}
		if ((ihlatst[buffer->maxis - 1] & KJMASK) == KJ1st) {
			istring[buffer->maxis - 1] = (char)SPACE;
			ihlatst[buffer->maxis - 1] = JISCII;
		}
		phase2->lastis = MAX(phase2->topim + len, phase2->lastis);
		if (phase2->lastis < buffer->maxis &&
			(ihlatst[phase2->topim + len] & KJMASK) == KJ2nd) {
				istring[phase2->topim + len] = (char)SPACE;
				ihlatst[phase2->topim + len] = JISCII;
		}
		if (phase2->curis == -1)
			phase2->curis = phase2->topim + phase2->curim;
		if (phase2->curis > phase2->lastis)
			phase2->curis = phase2->lastis;
	}
	else if (phase2->topim >= 0)
			phase2->curis = phase2->topim;

	/*
	 *	clear Monitor
	 */

	_Jclr(kjsvpt->mkjcblk);

	/*
	 *	clear SDTTBL, SDTWORK, MSIMAGE and MHIMAGE
	 */

	for (i = 0; i < buffer->maxsd; i++)
		sdttbl[i] = sdtwork[i] = 0;			/* clear SDTTBL and SDTWORK		*/
	for (i = 0; i < buffer->maxim; i++)
		msimage[i] = mhimage[i] = 0;		/* clear MSIMAGE and MHIMAGE	*/

	/*
	 *	reset PHASE2 structure
	 */

	phase2->lastsd = 0;						/* last position in	SDTTBL		*/
	phase2->lastim = 0;						/* last position in	MSIMAGE		*/
	phase2->curim = phase2->topim = -1;		/* curim and topim				*/
	phase2->axuse1	= NOTUSE;				/* clear AUXarea 1 use flag		*/
	phase2->moncall = YES;
	phase2->state = NCS;
}
