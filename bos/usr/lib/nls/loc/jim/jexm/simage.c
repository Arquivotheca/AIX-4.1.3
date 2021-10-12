static char sccsid[] = "@(#)01	1.2  src/bos/usr/lib/nls/loc/jim/jexm/simage.c, libKJI, bos411, 9428A410j 6/6/91 11:23:18";

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
 *  simage(kjsvpt)
 *      argument
 *          KJSVPT  *kjsvpt;        pointer to KJSVPT
 *
 *		description
 *			This routine makes MSIMAGE and MHIMAGE based on MSTRING and
 *			MHLATST and updated SDTTBL.
 *
 *  himage(kjsvpt)
 *      argument
 *          KJSVPT  *kjsvpt;        pointer to KJSVPT
 *
 *		description
 *			This routine makes MHIMAGE based on MHLATST and SDTTBL
 *
 *  11/18/87 first write.
 *  09/26/88 fix a bug. overflow.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

simage(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	char	*mstring	= mkjcblk->string;	/* ptr to MSTRING				*/
	char	*mhlatst	= mkjcblk->hlatst;	/* ptr to MHLATST				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*msimage	= buffer->msimage;	/* ptr to MSIMAGE				*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	int		src,dst;						/* index						*/
	int		maxim		= buffer->maxim;	/* max len of MSIMAGE			*/

	/*
	 *	convert MSTRING to MSIMAGE
	 *	if DBCS then move 2 bytes from MSIMAGE. if SBCS then move 1 byte
	 *	from SDTTBL.
	 */

	for (src = dst = 0; src < phase2->lastsd; src++) {
		if (sdttbl[src] == DOUBLEF || sdttbl[src] == SPECIALF) {
			if (dst >= maxim - 1)			/* overflow break;				*/
				break;
			if (2 * src == mkjcblk->curcol)
				phase2->curim = dst;		/* calc curim					*/
			msimage[dst] = mstring[2 * src];
			mhimage[dst] = mhlatst[2 * src] | KJ1st;
			dst++;
			msimage[dst] = mstring[2 * src + 1];
			mhimage[dst] = mhlatst[2 * src + 1] | KJ2nd;
			dst++;
		}
		else {
			if (dst >= maxim)				/* overflow break;				*/
				break;
			if (2 * src == mkjcblk->curcol)
				phase2->curim = dst;		/* calc curim					*/
			msimage[dst] = sdttbl[src];
			mhimage[dst] = mhlatst[2 * src] | JISCII;
			dst++;
		}
	}		/* end of for */
	if (2 * src <= mkjcblk->curcol)			/* the case cursor is at end of	*/
		phase2->curim = dst;				/* MSTRING						*/
	phase2->lastim = dst;					/* last position in MSIMAGE		*/
}


himage(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	char	*mhlatst	= mkjcblk->hlatst;	/* ptr to MHLATST				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	int		src,dst;						/* index						*/
	int		maxim		= buffer->maxim;	/* max len of MSIMAGE			*/

	/*
	 *	convert MHLATST to MHIMAGE
	 *	if DBCS then move 2 bytes from MSIMAGE. if SBCS then move 1 byte
	 *	from SDTTBL.
	 */

	for (src = dst = 0; src < phase2->lastsd; src++) {
		if (sdttbl[src] == DOUBLEF || sdttbl[src] == SPECIALF) {
			if (dst >= maxim - 1)			/* overflow break;				*/
				break;
			if (2 * src == mkjcblk->curcol)
				phase2->curim = dst;		/* calc curim					*/
			mhimage[dst++] = mhlatst[2 * src] | KJ1st;
			mhimage[dst++] = mhlatst[2 * src + 1] | KJ2nd;
		}
		else {
			if (dst >= maxim)				/* overflow break;				*/
				break;
			if (2 * src == mkjcblk->curcol)
				phase2->curim = dst;		/* calc curim					*/
			mhimage[dst++] = mhlatst[2 * src] | JISCII;
		}
	}		/* end of for */
	if (2 * src <= mkjcblk->curcol)			/* the case cursor is at end of	*/
		phase2->curim = dst;				/* MSTRING						*/
}
