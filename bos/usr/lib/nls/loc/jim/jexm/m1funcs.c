static char sccsid[] = "@(#)95	1.2  src/bos/usr/lib/nls/loc/jim/jexm/m1funcs.c, libKJI, bos411, 9428A410j 6/6/91 11:21:39";

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
 *  m1functions.
 *		m1allcand(kjsvpt)
 *		m1bs(kjsvpt)
 *		m1char(kjsvpt,c)
 *		m1conv(kjsvpt)
 *		m1convsw(kjsvpt)
 *		m1call(kjsvpt)
 *		m1cbs(kjsvpt)
 *		m1cchar(kjsvpt)
 *		m1cdel(kjsvpt)
 *		m1chkc(kjsvpt)
 *		m1nop(kjsvpt)
 *		m1curleft(kjsvpt)
 *		m1curright(kjsvpt)
 *		m1curup(kjsvpt)
 *		m1del(kjsvpt)
 *		m1diagoff(kjsvpt)
 *		m1diagon(kjsvpt)
 *		m1enter(kjsvpt)
 *		m1ereof(kjsvpt)
 *		m1erinput(kjsvpt)
 *		m1insert(kjsvpt)
 *		m1kjnum(kjsvpt)
 *		m1noconv(kjsvpt)
 *		m1precand(kjsvpt)
 *		m1reset(kjsvpt)
 *		m1yomi(kjsvpt)
 *		m1setcur(kjsvpt)
 *
 *	02/26/88 m1functions are put into one source file. m1functions defined
 *           below are same to version 5.
 *  03/22/88 m1noconv() and m1yomi() is corrected not to confirm in EDE state.
 *	06/28/88 version 6.0. A new function m1setcur() is added to support
 *			 the kjcrst().
 */

#include <exmdefs.h>
#include <exmctrl.h>

void	m1allcand(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/

	switch(phase2->state){
	case CYI :
		inpr2(PALLCAND)
		phase2->moncall = YES;
		if(mkjcblk->axuse1 == USE){
			phase2->prevstate = phase2->state;/* save previous state		*/
			phase2->nextstate = NCS;		/* set next state				*/
			phase2->axuse1 = USE;			/* aux area is used				*/
			phase2->state = ACI;			/* state changes to ACI			*/
		}
		else
			phase2->beep = ON;
		break;

	case CNV :
		inpr2(PALLCAND)
		phase2->moncall = YES;
		if(mkjcblk->axuse1 == USE){
			phase2->prevstate = CNV;		/* save previous state			*/
			phase2->nextstate = CNV;		/* save current state			*/
			phase2->axuse1 = USE;			/* aux area is used				*/
			phase2->state = ACI;			/* state changes to ACI			*/
		}
		else
			phase2->beep = ON;
		break;

	case EDM :
		if (chilited(mkjcblk)) {
			if (ciskanji(mkjcblk)) {
				inpr2(PALLCAND)
				phase2->moncall = YES;
				if (mkjcblk->axuse1 == USE){
					phase2->prevstate = EDM;/* save previous state			*/
					phase2->nextstate = EDM;/* save current state			*/
					phase2->axuse1 = USE;	/* aux area is used				*/
					phase2->state = ACI;	/* state changes to ACI			*/
				}
				else
					phase2->beep = ON;
			}
			else if (cisyomi(mkjcblk)) {
				inpr2(PALLCAND)
				phase2->moncall = YES;
				if (mkjcblk->axuse1 == USE){
					phase2->prevstate = EDM;/* save previous state			*/
					phase2->nextstate = NCS;/* set next state				*/
					phase2->axuse1 = USE;	/* aux area is used				*/
					phase2->state = ACI;	/* state changes to ACI			*/
				}
				else
					phase2->beep = ON;
			}
			else
				phase2->beep = ON;
		}
		else if (philited(mkjcblk)) {
			if (piskanji(mkjcblk)) {
				inpr2(PALLCAND)
				phase2->moncall = YES;
				if (mkjcblk->axuse1 == USE){
					phase2->prevstate = EDM;/* save previous state			*/
					phase2->nextstate = EDM;/* save current state			*/
					phase2->axuse1 = USE;	/* aux area is used				*/
					phase2->state = ACI;	/* state changes to ACI			*/
				}
				else
					phase2->beep = ON;
			}
			else if (pisyomi(mkjcblk)) {
				inpr2(PALLCAND)
				phase2->moncall = YES;
				if (mkjcblk->axuse1 == USE){
					phase2->prevstate = EDM;/* save previous state			*/
					phase2->nextstate = NCS;/* set next state				*/
					phase2->axuse1 = USE;	/* aux area is used				*/
					phase2->state = ACI;	/* state changes to ACI			*/
				}
				else
					phase2->beep = ON;
			}
			else
				phase2->beep = ON;
		}
		else
			phase2->beep = ON;
		break;

	case EDC :
		if (philited(mkjcblk)) {
			if (piskanji(mkjcblk)) {
				inpr2(PALLCAND)
				phase2->moncall = YES;
				if (mkjcblk->axuse1 == USE){
					phase2->prevstate = EDC;/* save previous state			*/
					phase2->nextstate = EDC;/* save current state			*/
					phase2->axuse1 = USE;	/* aux area is used				*/
					phase2->state = ACI;	/* state changes to ACI			*/
				}
				else
					phase2->beep = ON;
			}
			else if (pisyomi(mkjcblk)) {
				inpr2(PALLCAND)
				phase2->moncall = YES;
				if (mkjcblk->axuse1 == USE){
					phase2->prevstate = EDC;/* save previous state			*/
					phase2->nextstate = NCS;/* set next state				*/
					phase2->axuse1 = USE;	/* aux area is used				*/
					phase2->state = ACI;	/* state changes to ACI			*/
				}
				else
					phase2->beep = ON;
			}
			else
				phase2->beep = ON;
		}
		else
			phase2->beep = ON;
		break;

	default	:
		phase2->beep = ON;					/* beep							*/
		break;
	}
	return;
}

void	m1bs(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	int		src,dst;						/* index						*/
	int		botim = phase2->topim;
	extern	void	rs2();					/* reconfigure SDTTBL			*/

	if (phase2->repins == REPLACE)
		botim += phase2->lastim;

	switch (phase2->state) {
	case NCS :
		if (phase2->curis > 0) {
			if (kjsvpt->normalbs || phase2->repins == INSERT) {
				src = dst = phase2->curis;
				dst--;
				if ((ihlatst[dst] & KJMASK) == KJ2nd)
					dst--;
				phase2->curis = dst;
				for (; src < phase2->lastis; src++, dst++) {
					istring[dst] = istring[src];
					ihlatst[dst] = ihlatst[src];
				}
				phase2->lastis = dst;
				for (; dst < src; dst++)
					istring[dst] = ihlatst[dst] = 0;
			}
			else {
				phase2->curis--;
				istring[phase2->curis] = (char)SPACE;
				if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd) {
					ihlatst[phase2->curis] = JISCII;
					phase2->curis--;
					istring[phase2->curis] = (char)SPACE;
				}
				ihlatst[phase2->curis] = JISCII;
			}
		}
		else								/* phase2->curis == 0			*/
			phase2->beep = ON;
		break;

	case CYI :
		inpr2(PBACKSPACE)
		phase2->moncall = YES;
		rs2(kjsvpt);
		simage(kjsvpt);
		if (mkjcblk->cnvsts == FINISHED)
			confirm(kjsvpt);
		else								/* !!! to be deleted these 2	*/
			phase2->state = EDC;			/* lines if the Monitor is		*/
											/* updated on HKC				*/
		break;

	case EDE :								/* curis == botim or 0			*/
		if (phase2->curis == botim) {
			inpr2(PBACKSPACE)
			phase2->moncall = YES;
			rs2(kjsvpt);
			simage(kjsvpt);
			if (mkjcblk->cnvsts == FINISHED) {
				phase2->curis = -1;
				confirm(kjsvpt);
			}
			else {
				phase2->curis = phase2->topim;
				if (phase2->repins == REPLACE)
					phase2->curis += phase2->lastim;
				for (src = 0; src < phase2->lastim; src++)
					mhimage[src] &= ~REVERSE;
				phase2->state = EDE;
			}
		}
		else
			phase2->beep = ON;
		break;

	case EDC :
		inpr2(PBACKSPACE)
		phase2->moncall = YES;
		rs2(kjsvpt);
		simage(kjsvpt);
		if (mkjcblk->cnvsts == FINISHED)
			confirm(kjsvpt);
		else if (!philited(mkjcblk)) {
			phase2->curis = phase2->topim;
			if (phase2->repins == REPLACE)
				phase2->curis += phase2->lastim;
			phase2->state = EDE;
		}
		break;

	case EDM :
		if (phase2->curim == 0)				/* if curim = 0 then topim = 0	*/
			phase2->beep = ON;
		else {
			inpr2(PBACKSPACE)
			phase2->moncall = YES;
			rs2(kjsvpt);
			simage(kjsvpt);
			if (mkjcblk->cnvsts == FINISHED)
				confirm(kjsvpt);
		}
		break;

	case KNI :
		inpr2(PBACKSPACE)
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		break;

	default :
		phase2->beep = ON;					/* beep							*/
		break;

	}
	return;
}

void	m1char(kjsvpt,c)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
char	c;									/* input character				*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	int		i;								/* generall loop counter		*/
	extern	void	rs1();					/* reconfigure SDTTBL			*/
	extern	void	rs3();					/* reconfigure SDTTBL			*/


	switch (phase2->state) {
	case NCS :
		if (phase2->shift3 == SINGLE) {		/* input char is Single			*/
			if (phase2->repins == INSERT) {	/* insert mode					*/
				for (i = phase2->lastis; i > phase2->curis; i--) {
					istring[i] = istring[i - 1];
					ihlatst[i] = ihlatst[i - 1];
				}
				istring[phase2->curis] = c;	/* insert character c			*/
				ihlatst[phase2->curis] = JISCII;
				phase2->curis++;
				phase2->lastis++;
			}
			else {							/* replace mode					*/
				istring[phase2->curis] = c;	/* replace character c			*/
				ihlatst[phase2->curis] = JISCII;
				if (phase2->curis == phase2->lastis) {
											/* if cursor at the end of		*/
					phase2->curis++;		/* ISTRING, then length of		*/
					phase2->lastis++;		/* ISTRING increases.			*/
				}
				else {						/* cursor not at the end of it.	*/
					phase2->curis++;
					if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd) {
											/* if replaced char is KJ1st,	*/
											/* then replace a corresponding	*/
											/* KJ2nd with SPACE				*/
						istring[phase2->curis] = SPACE;
						ihlatst[phase2->curis] = JISCII;
					}
				}
			}
		}
		else {								/* input char is Double			*/
			if(c == 0xde) {					/* char is 'tenten'				*/
				if(phase2->curis > 1 &&
					(ihlatst[phase2->curis - 1] & KJMASK) == KJ2nd)
					if(checktenten(&istring[phase2->curis - 2])) {
						istring[phase2->curis - 1]++;
						break;
					}
					else if(istring[phase2->curis - 2] == 0x83 &&
						istring[phase2->curis - 1] == 0x45) {
						istring[phase2->curis - 1] = 0x94;
						break;
					}
			}
			else if(c == 0xdf) {			/* char is 'maru'				*/
				if(phase2->curis > 1 &&
					(ihlatst[phase2->curis - 1] & KJMASK) == KJ2nd &&
					checkmaru(&istring[phase2->curis - 2])) {
						istring[phase2->curis - 1] += 2;
						break;
				}
			}
			inpr1(c)						/* invoke the Monitor			*/
			phase2->moncall = YES;
			if(c == SCHARS)					/* special character			*/
				sdttbl[0] = SPECIALF;		/* special flag					*/
			else							/* note special character		*/
				sdttbl[0] = DOUBLEF;		/* double flag					*/
			phase2->lastsd = 1;
			phase2->topim = phase2->curis;
			phase2->curis = -1;
			simage(kjsvpt);					/* make MSIMAGE					*/
			if (mkjcblk->cnvsts == FINISHED)
				confirm(kjsvpt);
			else
				phase2->state = CYI;
		}
		break;

	case CYI or EDM or EDC :
		if (phase2->shift3 == SINGLE)
			inpr1(SCHARS)
		else
			inpr1(c)
		phase2->moncall = YES;
		if (phase2->shift3 == SINGLE)
			rs3(kjsvpt,c);					/* reconfigure SDTTBL			*/
		else if (c == SCHARS)
			rs3(kjsvpt,SPECIALF);			/* reconfigure SDTTBL			*/
		else
			rs1(kjsvpt);					/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		if (mkjcblk->cnvsts == FINISHED)	/* if Monitor conv is finished,	*/
			confirm(kjsvpt);				/* then confirm.				*/
		break;

	case EDE :								/* curis == botim & pisyomi		*/
		if (phase2->shift3 == SINGLE)
			inpr1(SCHARS)
		else
			inpr1(c)
		phase2->moncall = YES;
		if (phase2->shift3 == SINGLE)
			rs3(kjsvpt,c);					/* reconfigure SDTTBL			*/
		else if (c == SCHARS)
			rs3(kjsvpt, SPECIALF);			/* reconfigure SDTTBL			*/
		else
			rs1(kjsvpt);					/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		if (mkjcblk->cnvsts == FINISHED) {	/* if Monitor conv is finished,	*/
			phase2->curis = -1;				/* then confirm.				*/
			confirm(kjsvpt);				/* state becomes into NCS.		*/
		}
		else if (philited(mkjcblk)) {
			phase2->curis = -1;
			phase2->state = EDC;
		}
		else {
			phase2->curis = phase2->topim;
			if (phase2->repins == REPLACE)
				phase2->curis += phase2->lastim;
		}
		break;

	case ACI :
		inpr1(c)							/* invoke the Monitor			*/
		phase2->moncall = YES;
		if(mkjcblk->axuse1 == USE)			/* if invalid char comes, then	*/
			phase2->beep = ON;				/* beep.						*/
		else {								/* select and end				*/
			rs1(kjsvpt);					/* reconfigure SDTTBL			*/
			simage(kjsvpt);					/* update MSIMAGE				*/
			phase2->state = phase2->nextstate;/* becomes pre-ACI state		*/
			if (phase2->nextstate == NCS)
				confirm(kjsvpt);
			phase2->axuse1 = NOTUSE;		/* hide aux1					*/
		}
		break;

	case KNI :
		inpr1(c)							/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)			/* if invalid char comes, then	*/
			phase2->beep = ON;				/* beep.						*/
		break;

	case CMS :
		if ('1' <= c && c <= '4' ||
			c == KATAKANA_NU || c == KATAKANA_HU ||
			c == KATAKANA_A  || c == KATAKANA_U) {
			inpr1(c)						/* invoke the Monitor			*/
			phase2->moncall = YES;
			if (mkjcblk->beep == ON)		/* if invalid char comes, then	*/
				phase2->beep = ON;			/* beep.						*/
		}
		else
			phase2->beep = ON;
		break;

	case DIA :
		inpr1(c)							/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == USE)			/* if invalid char comes, then	*/
			phase2->beep = ON;				/* beep.						*/
		else {								/* NOT USE means end of these	*/
			phase2->axuse1 = NOTUSE;		/* state.						*/
			phase2->state = NCS;
		}
		break;

	case OVF :
		phase2->beep = ON;					/* beep							*/
		break;

	default	:								/* default never occurs			*/
		break;
	}
	return;
}

static	checktenten(p)
char	*p;
{
	int	c1 = p[0];							/* 1st byte of DBCS				*/
	int	c2 = p[1];							/* 2nd byte of DBCS				*/
#define	T	TRUE
#define	F	FALSE
	static	char	tententable[] = {
		T, F, T, F, T, F, T, F, T, F,			/* 'ka' line				*/
		T, F, T, F, T, F, T, F, T, F,			/* 'sa' line				*/
		T, F, T, F, F, T, F, T, F, T, F,		/* 'ta' line				*/
		F, F, F, F, F,							/* 'na' line				*/
		T, F, F, T, F, F, T, F, F, T, F, F, T};	/* 'ha' line				*/
#undef	T
#undef	F

	if(c1 == 0x82)
		c2 -= 0xa9;
	else if(c1 == 0x83)
		c2 -= 0x4a;
	else
		return FALSE;
	if(0 <= c2 && c2 <= sizeof(tententable))
		return(tententable[c2]);
	return FALSE;
}

static	checkmaru(p)
char	*p;
{
	int	c1 = p[0];							/* 1st byte of DBCS				*/
	int	c2 = p[1];							/* 2nd byte of DBCS				*/

	if(c1 == 0x82)
		c2 -= 0xcd;
	else if(c1 == 0x83)
		c2 -= 0x6e;
	else
		return FALSE;
	if(0 <= c2 && c2 <= 12)
		return(!(c2 % 3));
	return FALSE;
}

void	m1conv(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	extern	void	rs1();					/* reconfigure SDTTBL			*/


	switch (phase2->state) {
	case CYI or CNV :
		inpr2(PCONVERT)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		if (phase2->curim == phase2->lastim)
			if (pisother(mkjcblk)) {
				phase2->curis = phase2->topim;
				if (phase2->repins == REPLACE)
					phase2->curis += phase2->lastim;
				phase2->state = EDE;
			}
			else if (pisyomi(mkjcblk))
				phase2->state = EDC;
			else
				phase2->state = CNV;
		else
			phase2->state = EDM;
		break;

	case EDM :
		inpr2(PCONVERT)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		break;

	case EDC :
		inpr2(PCONVERT)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		if (phase2->curim == phase2->lastim)
			if (pisother(mkjcblk)){
				phase2->curis = phase2->topim;
				if (phase2->repins == REPLACE)
					phase2->curis += phase2->lastim;
				phase2->state = EDE;
			}
			else
				phase2->state = EDC;
		else
			phase2->state = EDM;
		break;

	case ACI :
		inpr2(PCONVERT)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		break;

	default :
		phase2->beep = ON;					/* beep							*/
		break;
	}
	return;
}

void	m1convsw(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/


	if (phase2->state == NCS) {
		inpr2(PCONVSW)
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == USE) {
			phase2->axuse1 = USE;			/* aux area is used.			*/
			phase2->state = CMS;			/* state changes to CMS			*/
		}
		else
			phase2->beep = ON;
	}
	else
		phase2->beep = ON;					/* beep							*/
	return;
}

void	m1call(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	PHASE2	*phase2	= &kjsvpt->phase2;		/* ptr to phase2 struct			*/


	switch (phase2->state) {
	case NCS or ACI or KNI or CMS or DIA or OVF :
		/* no action taken				*/
		break;

	default :
		confirm(kjsvpt);					/* confirmation					*/
		break;
	}
	return;
}


void	m1cbs(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk = kjsvpt->mkjcblk;		/* ptr to MKJCBLK				*/
	PHASE2	*phase2	= &kjsvpt->phase2;		/* ptr to phase2 struct			*/
	int		botim = phase2->topim;


	if (phase2->repins == REPLACE)
		botim += phase2->lastim;

	switch (phase2->state) {
	case CNV or HK1 or HK2 :
		confirm(kjsvpt);					/* confirmation					*/
		break;

	case EDE :
		if ((phase2->curis != botim || !pisyomi(mkjcblk)) && phase2->curis)
			confirm(kjsvpt);
		break;

	case EDM :
		if (phase2->curim == 0) {
			if (phase2->topim > 0)
				confirm(kjsvpt);
		}
		else if (cisother(mkjcblk) || !pisyomi(mkjcblk))
			confirm(kjsvpt);
		break;
		
	case EDC :
		if (!pisyomi(mkjcblk))
			confirm(kjsvpt);
		break;

	default	:								/* no action taken				*/
		break;
	}
	return;
}


void	m1cchar(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk = kjsvpt->mkjcblk;		/* ptr to MKJCBLK				*/
	PHASE2	*phase2	= &kjsvpt->phase2;		/* ptr to phase2 struct			*/
	int		botim = phase2->topim;


	if (phase2->repins == REPLACE)
		botim += phase2->lastim;

	switch (phase2->state) {
	case CNV or HK1 or HK2 :
		confirm(kjsvpt);					/* confirmation					*/
		break;

	case EDE :
		if (phase2->curis != botim || !pisyomi(mkjcblk))
			confirm(kjsvpt);
		break;

	case EDC :
		if (!pisyomi(mkjcblk))
			confirm(kjsvpt);
		break;

	case EDM :
		if (phase2->curim == 0 && !cisyomi(mkjcblk))
			confirm(kjsvpt);
		else if (cisother(mkjcblk) || !(cisyomi(mkjcblk) || pisyomi(mkjcblk)))
			confirm(kjsvpt);
		break;

	default	:								/* no action taken				*/
		break;
	}
	return;
}


void	m1cdel(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk = kjsvpt->mkjcblk;		/* ptr to MKJCBLK				*/
	PHASE2	*phase2	= &kjsvpt->phase2;		/* ptr to phase2 struct			*/
	int		botim = phase2->topim;


	if (phase2->repins == REPLACE)
		botim += phase2->lastim;

	switch (phase2->state) {
	case CYI or CNV or EDC :
		if (botim < phase2->lastis)
			confirm(kjsvpt);				/* confirmation					*/
		break;

	case EDE :
		if (phase2->curis < phase2->lastis )
			confirm(kjsvpt);				/* confirmation					*/
		break;

	case EDM :
		if (!cisyomi(mkjcblk))
			confirm(kjsvpt);				/* confirmation					*/
		break;

	case HK1 or HK2 :
		confirm(kjsvpt);					/* confirmation					*/
		break;

	default	:								/* no action taken				*/
		break;
	}
	return;
}


void	m1chkc(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	PHASE2	*phase2	= &kjsvpt->phase2;		/* ptr to phase2 struct			*/


	if (phase2->state == HK1 || phase2->state == HK2)
		confirm(kjsvpt);					/* confirmation					*/
	return;
}

/* ARGSUSED */
void	m1nop(kjsvpt)	KJSVPT	*kjsvpt;	{return; }

void	m1curleft(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	int		i;								/* loop counter					*/
	int		botim		= phase2->topim;


	if (phase2->repins == REPLACE)
		botim += phase2->lastim;

	switch (phase2->state) {
	case NCS :
		if (phase2->curis == 0)				/* curis at top of ISTRING		*/
			phase2->beep = ON;
		else {
			phase2->curis--;
			if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd)
				phase2->curis--;
		}
		break;

	case CYI or CNV or EDC :
		inpr2(PCURLEFT)
		phase2->moncall = YES;
		himage(kjsvpt);
		phase2->state = EDM;
		break;

	case EDE :
		if (phase2->curis == botim) {
			inpr2(PCURLEFT)
			phase2->moncall = YES;
			himage(kjsvpt);
			phase2->curis = -1;
			phase2->state = EDM;
		}
		else if (phase2->curis == 0)
			phase2->beep = ON;
		else {
			phase2->curis--;
			if (phase2->curis != botim &&
				(ihlatst[phase2->curis] & KJMASK) == KJ2nd)
				phase2->curis--;
		}
		break;

	case EDM :
		if (phase2->curim == 0)
			if (phase2->topim == 0)
				phase2->beep = ON;
			else {
				for (i = 0; i < phase2->lastim; i++)
					mhimage[i] &= ~REVERSE;	/* clear reverse highlight		*/
				phase2->curis = phase2->topim - 1;
				if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd)
					phase2->curis--;
				phase2->state = EDE;
			}
		else {
			inpr2(PCURLEFT)
			phase2->moncall = YES;
			himage(kjsvpt);
		}
		break;

	case KNI :
		inpr2(PCURLEFT)
		phase2->moncall = YES;
		if(mkjcblk->beep == ON)
			phase2->beep = ON;
		break;

	default :
		phase2->beep = ON;
		break;
	}
	return;
}

void	m1curright(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	int		i;								/* loop counter					*/
	int		botim		= phase2->topim;

	if (phase2->repins == REPLACE)
		botim += phase2->lastim;

	switch (phase2->state) {
	case NCS :
		if (phase2->curis == phase2->lastis)
			phase2->beep = ON;
		else {								/* curis < lastis				*/
			phase2->curis++;
			if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd)
				phase2->curis++;
		}
		break;

	case CYI or CNV or EDC :
		if (botim >= phase2->lastis)
			phase2->beep = ON;
		else {
			for(i = 0;i < phase2->lastim;i++)
				mhimage[i] &= ~REVERSE;		/* clear REVERSE Highlight		*/
			phase2->curis = botim + 1;
			if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd)
				phase2->curis++;
			phase2->state = EDE;
		}
		break;

	case EDE :
		if (phase2->curis >= phase2->lastis)
			phase2->beep = ON;
		else{
			phase2->curis++;
			if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd)
				phase2->curis++;
			if (phase2->curis == phase2->topim) {
				himage(kjsvpt);				/* restore reverse highlight	*/
				phase2->curis = -1;
				phase2->state = EDM;
			}
		}
		break;

	case EDM :
		inpr2(PCURRIGHT)
		phase2->moncall = YES;
		himage(kjsvpt);
		if (phase2->curim == phase2->lastim) {
			phase2->curis = botim;
			phase2->state = EDE;
		}
		break;

	case KNI :
		inpr2(PCURRIGHT)
		phase2->moncall = YES;
		if(mkjcblk->beep == ON)
			phase2->beep = ON;
		break;

	default :
		phase2->beep = ON;					/* beep							*/
		break;
	}
	return;
}

void	m1curup(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/


	switch (phase2->state) {
	case ACI or KNI or CMS or DIA or OVF :
		phase2->beep = ON;
		break;

	default :								/* no action is taken (NCS)		*/
		break;
	}
	return;
}

void	m1del(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	int		src,dst;						/* index						*/
	extern	void	rs2();					/* reconfigure SDTTBL			*/


	switch (phase2->state) {
	case NCS :
		if (phase2->curis < phase2->lastis) {
			src = dst = phase2->curis;
			src++;
			if ((ihlatst[phase2->curis] & KJMASK) == KJ1st)
				src++;
			for (; src < phase2->lastis; src++, dst++) {
				istring[dst] = istring[src];
				ihlatst[dst] = ihlatst[src];
			}
			phase2->lastis = dst;
			for (; dst < src; dst++)
				istring[dst] = ihlatst[dst] = 0;
		}
		else
			phase2->beep = ON;
		break;

	case EDM :
		inpr2(PDELETE)
		phase2->moncall = YES;
		rs2(kjsvpt);
		simage(kjsvpt);
		if (mkjcblk->cnvsts == FINISHED)
			confirm(kjsvpt);
		else if (phase2->curim == phase2->lastim)
			if (philited(mkjcblk))
				phase2->state = EDC;
			else {
				phase2->curis = phase2->topim;
				if (phase2->repins == REPLACE)
					phase2->curis += phase2->lastim;
				phase2->state = EDE;
			}
		break;

	case KNI :
		inpr2(PDELETE)
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		break;	

	default :
		phase2->beep = ON;					/* beep							*/
		break;
	}
	return;
}

void	m1diagoff(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/


	if (phase2->state == NCS) {
		inpr2(PDIAGNOS)
		inpr2(PCURDOWN)
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == USE) {
			phase2->axuse1 = USE;
			phase2->state = DIA;
		}
		else
			phase2->beep = ON;
	}
	else
		phase2->beep = ON;
	return;
}

void	m1diagon(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/


	if (phase2->state == NCS) {
		inpr2(PDIAGNOS)
		inpr2(PCURUP)
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == USE) {
			phase2->axuse1 = USE;
			phase2->state = DIA;
		}
		else
			phase2->beep = ON;
	}
	else
		phase2->beep = ON;
	return;
}

void	m1enter(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*mstring	= mkjcblk->string;	/* ptr to MSTRING				*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	int		mcurcol;						/* save the old curcol			*/
	int		i;								/* loop counter					*/


	switch (phase2->state) {
	case NCS :
		phase2->discrd = NOTDISCARD;
		break;

	case KNI :
		mcurcol = mkjcblk->curcol;
		inpr2(PENTER)
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == USE) {
			if (mkjcblk->curcol > mcurcol) {
				if (phase2->repins == INSERT) {
					for (i = phase2->lastis - 1; i >= phase2->curis; i--) {
						istring[i + 2] = istring[i];
						ihlatst[i + 2] = ihlatst[i];
					}
					istring[phase2->curis]		= mstring[mcurcol];
					ihlatst[phase2->curis]		= KJ1st;
					istring[phase2->curis + 1]	= mstring[mcurcol + 1];
					ihlatst[phase2->curis + 1]	= KJ2nd;
					phase2->curis += 2;
					phase2->lastis += 2;
				}
				else {						/* replace mode					*/
					istring[phase2->curis]		= mstring[mcurcol];
					ihlatst[phase2->curis]		= KJ1st;
					istring[phase2->curis + 1]	= mstring[mcurcol + 1];
					ihlatst[phase2->curis + 1]	= KJ2nd;
					phase2->curis += 2;
					if ((ihlatst[phase2->curis] & KJMASK) == KJ2nd) {
						istring[phase2->curis] = SPACE;
						ihlatst[phase2->curis] = JISCII;
					}
					if (phase2->curis > phase2->lastis)
						phase2->lastis = phase2->curis;
				}
				if (mcurcol >= mkjcblk->actcol - 2) {
					_Jclr(mkjcblk);
					inpr2(PKJNUM)
					phase2->moncall = YES;
				}
			}
			else
				phase2->beep = ON;
		}
		else {								/* KNI ended					*/
			_Jclr(mkjcblk);
			phase2->moncall = YES;
			phase2->axuse1 = NOTUSE;
			phase2->state = NCS;
		}
		break;

	case CMS :
		inpr2(PENTER)
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == USE)
			phase2->beep = ON;
		else {
			phase2->axuse1 = NOTUSE;
			phase2->state = NCS;
		}
		break;

	case ACI :
		inpr2(PENTER)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if(mkjcblk->axuse1 == USE)			/* if invalid char comes, then	*/
			phase2->beep = ON;				/* beep.						*/
		else{								/* select and end				*/
			phase2->axuse1 = NOTUSE;		/* hide aux1					*/
			phase2->state = phase2->prevstate;/* becomes pre-ACI state		*/
		}
		break;

	default :								/* DIA, OVF						*/
		phase2->beep = ON;					/* beep							*/
		break;
	}
	return;
}

void	m1ereof(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	int		i;								/* index						*/


	if (phase2->state == NCS) {
		for (i = phase2->curis; i < phase2->lastis; i++)
			istring[i] = ihlatst[i] = 0;	/* clear ISTRING and IHLATST	*/
											/* after cursor position		*/
		phase2->lastis = phase2->curis;
	}
	else
		phase2->beep = ON;					/* invalid input then beep		*/

	return;
}


void	m1erinput(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	int		i;								/* index						*/


	if (phase2->state == NCS) {
		for (i = 0; i < phase2->lastis; i++)/* clear all ISTRING and		*/
			istring[i] = ihlatst[i] = 0;	/* IHLATST						*/
		phase2->lastis = phase2->curis = 0;
	}
	else
		phase2->beep = ON;					/* invalid input then beep		*/

	return;
}

void	m1insert(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/


	if (phase2->state == NCS) {
		inpr2(PINSERT)
		phase2->moncall = YES;
		if (phase2->repins == INSERT)		/* switch replace/insert mode	*/
			phase2->repins = REPLACE;
		else
			phase2->repins = INSERT;
	}
	else
		phase2->beep = ON;					/* beep							*/
	return;
}

void	m1kjnum(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/


	switch (phase2->state) {
	case NCS :
		inpr2(PKJNUM)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == USE) {
			phase2->axuse1 = USE;			/* aux area is used.			*/
			phase2->state = KNI;			/* state changes to KNI			*/
		}
		else
			phase2->beep = ON;
		break;

	case KNI :
		inpr2(PKJNUM)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		break;

	default :
		phase2->beep = ON;					/* beep							*/
		break;
	}
	return;
}

void	m1noconv(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	extern	void	rs1();					/* reconfigure SDTTBL			*/


	switch (phase2->state) {
	case CNV :
		inpr2(PNOCONV)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		phase2->state = EDM;
		break;

	case EDM :
		inpr2(PNOCONV)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		if (mkjcblk->cnvsts == FINISHED)
			confirm(kjsvpt);
		break;

	case EDE :
/*
		confirm(kjsvpt);
*/
		break;

	case EDC :
		if (aisyomi(mkjcblk)) {				/* if so, go to H/K conv		*/
			inpr2(PNOCONV)					/* invoke the Monitor			*/
			phase2->moncall = YES;
			rs1(kjsvpt);					/* reconfigure SDTTBL			*/
			simage(kjsvpt);					/* update MSIMAGE				*/
			phase2->prevstate = EDC;
			phase2->state = HK1;			/* state changes to HK1			*/
		}
		else {
			inpr2(PNOCONV)					/* invoke the Monitor			*/
			phase2->moncall = YES;
			if (mkjcblk->beep == ON)
				phase2->beep = ON;
			rs1(kjsvpt);					/* reconfigure SDTTBL			*/
			simage(kjsvpt);					/* update MSIMAGE				*/
			if (mkjcblk->cnvsts == FINISHED)
				confirm(kjsvpt);
			else if (phase2->curim == phase2->lastim && !philited(mkjcblk)) {
				phase2->curis = phase2->topim;
				if (phase2->repins == REPLACE)
					phase2->curis += phase2->lastim;
				phase2->state = EDE;
			}
			else
				phase2->state = EDM;
		}
		break;

	case CYI :
		inpr2(PNOCONV)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		phase2->prevstate = CYI;
		phase2->state = HK1;				/* state changes to HK1			*/
		break;

	case HK1 :
		inpr2(PNOCONV)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		phase2->state = HK2;				/* state changes to HK2			*/
		break;

	case HK2 :
		inpr2(PNOCONV)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		phase2->state = phase2->prevstate;	/* restore previous state		*/
		break;

	default	:
		phase2->beep = ON;
		break;
	}
	return;
}

void	m1precand(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	extern	void	rs1();					/* reconfigure SDTTBL			*/


	switch (phase2->state) {
	case CNV :
		inpr2(PPRECAND)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		break;

	case EDM :
		inpr2(PPRECAND)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		break;

	case EDC :
		inpr2(PPRECAND)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		if (phase2->curim < phase2->lastim)
			phase2->state = EDM;
		break;

	case ACI :
		inpr2(PPRECAND)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		break;

	default :
		phase2->beep = ON;					/* beep							*/
		break;
	}
	return;
}

void	m1reset(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to PHASE2 struct			*/
	char	*string		= buffer->string;	/* ptr to STRING				*/
	char	*hlatst		= buffer->hlatst;	/* ptr to HLATST				*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	char	*msimage	= buffer->msimage;	/* ptr to MSIMAGE				*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	char	*sdttbl		= buffer->sdttbl;	/* ptr to SDTTBL				*/
	char	*sdtwork	= buffer->sdtwork;	/* ptr to SDTWORK				*/
	int		i;								/* loop counter					*/


	switch (phase2->state) {
	case ACI :
		inpr2(PRESET)
		phase2->moncall = YES;
		if (mkjcblk->axuse1 == NOTUSE) {
			phase2->axuse1 = NOTUSE;
			phase2->state = phase2->prevstate;
		}
		break;

	case KNI or CMS :
		inpr2(PRESET)
		if (mkjcblk->axuse1 == NOTUSE) {
			_Jclr(mkjcblk);
			phase2->axuse1 = NOTUSE;
			phase2->state = NCS;
		}
		phase2->moncall = YES;
		break;

	case DIA :
		if(mkjcblk->shift1 == ALPHANUM)
			inpr1('n')
		else
			inpr1(KATAKANA_MI)
		if (mkjcblk->axuse1 == NOTUSE) {
			_Jclr(mkjcblk);
			phase2->axuse1 = NOTUSE;
			phase2->state = NCS;
		}
		phase2->moncall = YES;
		break;

	case OVF :
		inpr2(PRESET)
		_Jclr(mkjcblk);

		for (i = 0; i < buffer->actcol; i++) {/* copy STRING to ISTRING		*/
			istring[i] = string[i];
			ihlatst[i] = hlatst[i] & KJMASK;
		}
		for (i = buffer->actcol; i < buffer->maxis; i++)
			istring[i] = ihlatst[i] = 0;	/* clear the rest of ISTRING	*/
		phase2->curis  = phase2->curst;
		phase2->lastis = phase2->lastst;

		for (i = 0; i < buffer->maxim; i++)	/* clear MSIMAGE				*/
			msimage[i] = mhimage[i] = 0;
		phase2->topim = phase2->curim = -1;
		phase2->lastim = 0;

		for (i = 0; i < buffer->maxsd; i++)	/* clear SDTTBL					*/
			sdttbl[i] = sdtwork[i] = 0;
		phase2->lastsd = 0;

		phase2->state  = NCS;
		phase2->axuse1 = NOTUSE;
		phase2->moncall = YES;
		break;

	default :								/* no action is taken.			*/
		break;
	}
	return;
}

void	m1setcur(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*hlatst		= buffer->hlatst;	/* ptr to HLATST				*/
	int		topim		= phase2->topim;
	int		lastim		= phase2->lastim;
	int		setcsc		= phase2->setcsc;

	if (setcsc < 0 || phase2->lastst < setcsc) {
		phase2->beep = ON;
		return;
	}
	if ((hlatst[setcsc] & KJMASK) == KJ2nd)
		setcsc--;
	if (setcsc == phase2->curst)
		return;
	if (phase2->state == NCS)
		phase2->curis = setcsc;
	else if (phase2->state == OVF)
		phase2->beep = ON;
	else {
		if (setcsc < phase2->curst) {
			do {
				m1curleft(kjsvpt);
				if (phase2->curis == -1)
					phase2->curst = topim + phase2->curim;
				else if (phase2->repins == INSERT && phase2->curis >= topim)
					phase2->curst = phase2->curis + lastim;
				else
					phase2->curst = phase2->curis;
			} while (setcsc < phase2->curst);
		}
		else {
			do {
				m1curright(kjsvpt);
				if (phase2->curis == -1)
					phase2->curst = topim + phase2->curim;
				else if (phase2->repins == INSERT && phase2->curis >= topim)
					phase2->curst = phase2->curis + lastim;
				else
					phase2->curst = phase2->curis;
			} while (setcsc > phase2->curst);
		}
	}
	return;
}

void	m1yomi(kjsvpt)
KJSVPT	*kjsvpt;							/* pointer to KJSVPT			*/
{
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	extern	void	rs1();					/* reconfigure SDTTBL			*/


	switch(phase2->state){
	case CNV :
		inpr2(PYOMI)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		phase2->state = EDM;
		break;

	case EDM :
		inpr2(PYOMI)						/* invoke the Monitor			*/
		phase2->moncall = YES;
		if (mkjcblk->beep == ON)
			phase2->beep = ON;
		rs1(kjsvpt);						/* reconfigure SDTTBL			*/
		simage(kjsvpt);						/* update MSIMAGE				*/
		if (mkjcblk->cnvsts == FINISHED)
			confirm(kjsvpt);
		break;

	case EDC :
		if (pisother(mkjcblk))
			confirm(kjsvpt);
		else {
			inpr2(PYOMI)					/* invoke the Monitor			*/
			phase2->moncall = YES;
			if (mkjcblk->beep == ON)
				phase2->beep = ON;
			rs1(kjsvpt);					/* reconfigure SDTTBL			*/
			simage(kjsvpt);					/* update MSIMAGE				*/
			if (mkjcblk->cnvsts == FINISHED)
				confirm(kjsvpt);
			else if (phase2->curim == phase2->lastim && !philited(mkjcblk)) {
				phase2->curis = phase2->topim;
				if (phase2->repins == REPLACE)
					phase2->curis += phase2->lastim;
				phase2->state = EDE;
			}
			else
				phase2->state = EDM;
		}
		break;

	case HK1 or HK2 :
		phase2->beep = ON;					/* beep on						*/
		/* drop through */

	case CYI :
		confirm(kjsvpt);
		break;

	case EDE :
		break;

	default :
		phase2->beep = ON;					/* beep on						*/
		break;
	}
	return;
}
