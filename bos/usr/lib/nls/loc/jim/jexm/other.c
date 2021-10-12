static char sccsid[] = "@(#)98	1.2  src/bos/usr/lib/nls/loc/jim/jexm/other.c, libKJI, bos411, 9428A410j 6/6/91 11:22:36";

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
 *	other functions.
 *
 *		int	kjauxkind(kjcblk)
 *		KJCBLK	*kjcblk;
 *
 *			return the kind of currently used aux.
 *				 0	All Candidates.
 *				 1	Kanji Number Input.
 *				 2	Conversion Mode Switch.
 *				 3	Diagnosis.
 *				-1	other case.
 *
 *		int	rkcbufempty(kjcblk)
 *		KJCBLK	*kjcblk;
 *
 *			return an information whether the RKC buffer is empty or not.
 *				TRUE	RKC buffer is empty.
 *				FALSE	RKC buffer is not empty.
 *
 *		int	rkcbufclear(kjcblk)
 *		KJCBLK	*kjcblk;
 *
 *			clear buffer for the RKC.
 *
 *		int	normalbs(kjcblk, flag)
 *		KJCBLK	*kjcblk;
 *		int		flag;
 *
 *			appearence of the backspaceing.
 *
 *		int	exsetcmode(kjcblk, cmode)
 *		KJCBLK	*kjcblk;
 *		int		cmode;			// conversion mode.
 *
 *			set conversion mode.
 *
 *  11/23/87 version 4.
 *  09/20/88 add exsetcmode().
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

kjauxkind(kjcblk)
KJCBLK	*kjcblk;
{
	int		ret;

	switch(kjcblk->kjsvpt->phase2.state){
	case	ACI:
		ret = 0;
		break;
	case	KNI:
		ret = 1;
		break;
	case	CMS:
		ret = 2;
		break;
	case	DIA:
		ret = 3;
		break;
	default:
		ret = -1;
		break;
	}
	return ret;
}


rkcbufempty(kjcblk)
KJCBLK	*kjcblk;
{
	if(kjcblk->kjsvpt->phase1.rslen == 0)
		return TRUE;
	else
		return	FALSE;
}


rkcbufclear(kjcblk)
KJCBLK	*kjcblk;
{
	extern	void	rkcinit();

	rkcinit(&kjcblk->kjsvpt->phase1);
	return TRUE;
}


normalbs(kjcblk, flag)
KJCBLK	*kjcblk;
int		flag;
{
		return(kjcblk->kjsvpt->normalbs = (flag != FALSE));
}


exsetcmode(kjcblk, cmode)
KJCBLK	*kjcblk;
int		cmode;
{
	KJCBLK	*mkjcblk = kjcblk->kjsvpt->mkjcblk;

	if(cmode < LOOKAHEAD || WORD < cmode)
		return -1;
	exkjclr(kjcblk);
	mkjcblk->type = TYPE2;
	mkjcblk->code = PCONVSW;
	_Jinpr(mkjcblk);
	mkjcblk->type = TYPE1;
	mkjcblk->code = '1' + cmode;
	_Jinpr(mkjcblk);
	mkjcblk->type = TYPE2;
	mkjcblk->code = PENTER;
	_Jinpr(mkjcblk);
	exkjclr(kjcblk);
	return cmode;
}
