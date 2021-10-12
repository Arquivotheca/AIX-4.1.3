static char sccsid[] = "@(#)00	1.2  src/bos/usr/lib/nls/loc/jim/jexm/rs.c, libKJI, bos411, 9428A410j 6/6/91 11:23:03";

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
 *  rs1(kjsvpt)
 *      argument
 *          KJSVPT  *kjsvpt;        pointer to KJSVPT
 *
 *		description
 *			This routine reconfigures SDTTBL based on MSTRING.
 *			assum that number of special character in MSTRING does not
 *			change before and after Monitor call.
 *
 *  rs2(kjsvpt)
 *      argument
 *          KJSVPT  *kjsvpt;        pointer to KJSVPT
 *
 *		description
 *			This routine reconfigures SDTTBL based on curcol in MSTRING.
 *			This routine is used for DEL and BS.
 *
 *  rs3(kjsvpt,c)
 *      argument
 *          KJSVPT  *kjsvpt;        pointer to KJSVPT
 *			char	c;				input char
 *
 *		description
 *			This routine reconfigures SDTTBL.
 *			This routine is used for single character input or special char
 *			input.
 *
 *  11/18/87 first write.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

void	rs1(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	/*
	 *	decleration of temp variables
	 */

	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	char	*mstring	= mkjcblk->string;	/* ptr to MSTRING				*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	char	*sdtwork	= buffer->sdtwork;	/* ptr to SDTWORK				*/
	int		src,dst,ref;					/* index						*/

	/*
	 *	move Single char and Special flag in SDTTBL to SDTWORK
	 */

	for (src = dst = 0; src < phase2->lastsd; src++)
		if (sdttbl[src] != DOUBLEF)			/* if not Double then move		*/
			sdtwork[dst++] = sdttbl[src];
	for (src = dst = ref = 0; src < mkjcblk->lastch; src += 2, dst++)
		if (mstring[src] == SCHARH && mstring[src + 1] == SCHARL)
			sdttbl[dst] = sdtwork[ref++];	/* if SPECIAL char then move	*/
											/* from SDTWORK					*/
		else
			sdttbl[dst] = DOUBLEF;			/* if not then store DOUBLE		*/
											/* flag							*/
	for (src = dst; src < phase2->lastsd; src++)
		sdttbl[src] = 0;					/* clear the garbage			*/
	phase2->lastsd = dst;					/* calc lastsd					*/
}


void	rs2(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	/*
	 *	decleration of temp variables
	 */

	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	int		i;								/* index						*/

	/*
	 *	delete an entry at cursor position in SDTTBL
	 */

	for (i = mkjcblk->curcol / 2 + 1; i < phase2->lastsd; i++)
		sdttbl[i - 1] = sdttbl[i];			/* delete one char in SDTTBL	*/
	sdttbl[--phase2->lastsd] = 0;
}


void	rs3(kjsvpt,c)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
char	c;									/* input character				*/
{
	/*
	 *	decleration of temp variables
	 */

	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	int		i;								/* index						*/
	int		cursd		= mkjcblk->curcol / 2;	/* curpos in SDTTBL			*/

	/*
	 *	insert a single char or a special flag at curpos in SDTTBL
	 */

	for (i = phase2->lastsd++; i >= cursd; i--)
		sdttbl[i] = sdttbl[i - 1];
	sdttbl[i] = c;							/* insert one char in SDTTBL	*/
}
