static char sccsid[] = "@(#)97	1.2  src/bos/usr/lib/nls/loc/jim/jexm/misc.c, libKJI, bos411, 9428A410j 6/6/91 11:22:20";

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
 *	miscellenious functions
 *
 *	cisyomi()		Whether the current postion is Yomi or not.
 *					Return TRUE if the current position is Yomi.
 *					Otherwise return FALSE.
 *
 *	pisyomi()		Whether the previous postion is Yomi or not.
 *					Return TRUE if the previous position is Yomi.
 *					Otherwise return FALSE.
 *
 *	ciskanji()		Whether the current postion is Kanji or not.
 *					Return TRUE if the current position is Kanji.
 *					Otherwise return FALSE.
 *
 *	piskanji()		Whether the previous postion is Kanji or not.
 *					Return TRUE if the previous position is Kanji.
 *					Otherwise return FALSE.
 *
 *	cisother()		Whether the current postion is Other or not.
 *					Return TRUE if the current position is Other.
 *					Otherwise return FALSE.
 *
 *	pisother()		Whether the previous postion is Other or not.
 *					Return TRUE if the previous position is Other.
 *					Otherwise return FALSE.
 *
 *	chilited()		Whether the current postion is highlighted or not.
 *					Return TRUE if the current position is highlighted.
 *					Otherwise return FALSE.
 *
 *	philited()		Whether the previous postion is highlighted or not.
 *					Return TRUE if the previous position is highlighted.
 *					Otherwise return FALSE.
 *
 *	aisyomi()		Whether all convertable objects are yomi or not.
 *					Return TRUE if all yomi. Otherwise return FALSE.
 *
 * 11/18/87 first write.
 * 11/20/87 version 4.
 * 11/20/87 rename isyomi() and previsyomi() to cisyomi() and pisyomi().
 * 11/20/87 add cisother(), pisother(), ciskanji() and piskanji().
 * 11/20/87 add chilited() and philited().
 * 02/23/88 version 5.
 * 02/23/88 add aisyomi().
 */

#include <exmkmisa.h>
#include <exmkcb.h>
#include <exmdefs.h>

#define REVMASK	1

cisyomi(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	char	*kjcvmap = kcb->kjsvpt->kjcvmap;/* ptr to KJCVMAP				*/
	short	curcol  = kcb->curcol;			/* cursor position				*/
	short	convpos = kcb->kjsvpt->convpos;	/* converting object position	*/
	short	convlen = kcb->kjsvpt->convlen;	/* length of converting object	*/

	curcol -= convpos;
	if (curcol < 0 || convlen <= curcol)
		return FALSE;
	if (kjcvmap[curcol + 1] >> 4)			/* current position is Yomi?	*/
		return FALSE;
	else
		return TRUE;
}

pisyomi(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	char	*kjcvmap = kcb->kjsvpt->kjcvmap;/* ptr to KJCVMAP				*/
	short	curcol  = kcb->curcol - 2;		/* previous position			*/
	short	convpos = kcb->kjsvpt->convpos;	/* converting object position	*/
	short	convlen = kcb->kjsvpt->convlen;	/* length of converting object	*/

	curcol -= convpos;
	if (curcol < 0 || convlen <= curcol)
		return FALSE;
	if (kjcvmap[curcol + 1] >> 4)			/* previous position is Yomi?	*/
		return FALSE;
	else
		return TRUE;
}


ciskanji(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	char	*kjcvmap = kcb->kjsvpt->kjcvmap;/* ptr to KJCVMAP				*/
	short	curcol  = kcb->curcol;			/* cursor position				*/
	short	convpos = kcb->kjsvpt->convpos;	/* converting object position	*/
	short	convlen = kcb->kjsvpt->convlen;	/* length of converting object	*/

	curcol -= convpos;
	if (curcol < 0 || convlen <= curcol)
		return FALSE;
	if (kjcvmap[curcol + 1] >> 4 == 1 || kjcvmap[curcol + 1] >> 4 == 2)
		return TRUE;
	else
		return FALSE;
}

piskanji(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	char	*kjcvmap = kcb->kjsvpt->kjcvmap;/* ptr to KJCVMAP				*/
	short	curcol  = kcb->curcol - 2;		/* previous position			*/
	short	convpos = kcb->kjsvpt->convpos;	/* converting object position	*/
	short	convlen = kcb->kjsvpt->convlen;	/* length of converting object	*/

	curcol -= convpos;
	if (curcol < 0 || convlen <= curcol)
		return FALSE;
	if (kjcvmap[curcol + 1] >> 4 == 1 || kjcvmap[curcol + 1] >> 4 == 2)
		return TRUE;
	else
		return FALSE;
}


cisother(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	char	*kjcvmap = kcb->kjsvpt->kjcvmap;/* ptr to KJCVMAP				*/
	short	curcol  = kcb->curcol;			/* cursor position				*/
	short	convpos = kcb->kjsvpt->convpos;	/* converting object position	*/
	short	convlen = kcb->kjsvpt->convlen;	/* length of converting object	*/

	curcol -= convpos;
	if (curcol < 0 || convlen <= curcol)
		return TRUE;
	if (kjcvmap[curcol + 1] >> 4 == 3)		/* current position is Other ?	*/
		return TRUE;
	else
		return FALSE;
}


pisother(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	char	*kjcvmap = kcb->kjsvpt->kjcvmap;/* ptr to KJCVMAP				*/
	short	curcol  = kcb->curcol - 2;		/* previous position			*/
	short	convpos = kcb->kjsvpt->convpos;	/* converting object position	*/
	short	convlen = kcb->kjsvpt->convlen;	/* length of converting object	*/

	curcol -= convpos;
	if (curcol < 0 || convlen <= curcol)
		return TRUE;
	if (kjcvmap[curcol + 1] >> 4 == 3)		/* previous position is Other ?	*/
		return TRUE;
	else
		return FALSE;
}


chilited(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	short	curcol  = kcb->curcol;			/* current position				*/

	return (kcb->hlatst[curcol] & REVMASK);
}


philited(kcb)
KCB		*kcb;								/* ptr to KCB					*/
{
	short	curcol  = kcb->curcol - 2;		/* previous position			*/

	return (kcb->hlatst[curcol] & REVMASK);
}

aisyomi(kcb)
KCB		*kcb;
{
	char	*kjcvmap = kcb->kjsvpt->kjcvmap;/* ptr to KJCVMAP				*/
	short	convlen = kcb->kjsvpt->convlen;	/* length of converting object	*/
	int		i;

	for (i = 1; i < convlen && !(kjcvmap[i] >> 4); i += 2);
	if (i < convlen)
		return FALSE;
	return TRUE;
}
