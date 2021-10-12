/* @(#)73	1.3  src/bos/usr/lib/nls/loc/jim/jed/jedIntern.c, libKJI, bos411, 9428A410j 6/6/91 11:01:19 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <im.h>
#include "imP.h"
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

/*
 *	get_state()
 *	this determines the current internal processing state
 */
int	get_state(FEPCB *fepcb)
{
	KCB	*kcb = fepcb->kcb;

	if (fepcb->imode.ind3 == KP_SUPPRESSEDMODE)
		return PROCESSOFF;
	if (fepcb->axconvsw == ON)
		return CONVSW;
	if (fepcb->auxuse == USE) {
		if (kjauxkind(kcb) == AX_ALLCAN)
			return ALLCANDS;
		if (kjauxkind(kcb) == AX_KJCODE) {
			if (kcb->lastch > 0)
				return ACTKNJ;
			else
				return INACTKNJ;
		}
        }
	else if (kcb->lastch > 0)
		return ACTIVE;
	if (rkcbufempty(kcb))
		return INACTIVE;
	return ACTIVATING;
}

/*
 *	chimode()
 *	change input mode
 */
void	chimode(KCB *kcb, char shift1, char shift2, char shift3)
{
	if (shift1 != kcb->shift1) {
		if (shift1 == ALPHANUM)
			kcb->code = PALPHANUM;
		else if (shift1 == KATAKANA)
			kcb->code = PKATAKANA;
		else /* HIRAGANA */
			kcb->code = PHIRAGANA;
		kcb->type = TYPE2;
		exkjinpr(kcb);
	}
	if (shift2 != kcb->shift2) {
		kcb->code = PRKC;
		kcb->type = TYPE2;
		exkjinpr(kcb);
	}
	if (shift3 != kcb->shift3) {
		kcb->code = PSGLDBL;
		kcb->type = TYPE2;
		exkjinpr(kcb);
	}
}

/*
 *	set_imode()
 */
void	set_imode(FEPCB *fepcb)
{
	/************************************************/
	/* both JIMED and Kanji Monitor maintains shift */
	/* state information, synchronize information of*/
	/* Monitor to one of JIMED below                */
	/************************************************/
	chimode(fepcb->kcb, fepcb->shift[0], fepcb->shift[1], fepcb->shift[2]);
}

/*
 *	set_indicator()
 */
void	set_indicator(FEPCB *fepcb)
{
	/*****************************************************/
	/* update input mode (referred as indicator sometime */
	/* according to current shift informatino of JIMED   */
	/*****************************************************/
	if (fepcb->shift[0] == KATAKANA)
		fepcb->imode.ind0 = KP_KATAKANA;
	else if (fepcb->shift[0] == HIRAGANA)
		fepcb->imode.ind0 = KP_HIRAGANA;
	else /* ALPHANUM */
		fepcb->imode.ind0 = KP_ALPHANUM;

	if (fepcb->shift[1] == RKC_ON)
		fepcb->imode.ind2 = KP_ROMAJI_ON;
	else /* RKC_OFF */
		fepcb->imode.ind2 = KP_ROMAJI_OFF;

	if (fepcb->shift[2] == DOUBLE)
		fepcb->imode.ind1 = KP_DOUBLE;
	else /* SINGLE */
		fepcb->imode.ind1 = KP_SINGLE;
}

/*
 *	placestr()/placechar()
 */

#define	ALLOC_UNIT	512
#ifdef	REALLOC
#undef	REALLOC
#endif
#define	REALLOC(s, l)	((s) ? realloc(s, l) : malloc(l))
void	placestr(IMBuffer *imb, unsigned char *str, int len)
{
	if (imb->len + len > imb->siz) {
		imb->siz = ((imb->len + len) / ALLOC_UNIT + 1) * ALLOC_UNIT;
		imb->data = (unsigned char *)REALLOC(imb->data, imb->siz);
	}
	memcpy(&imb->data[imb->len], str, len);
	imb->len += len;
}

void	placechar(IMBuffer *imb, unsigned char c)
{
	if (imb->len >= imb->siz) {
		imb->siz += ALLOC_UNIT;
		imb->data = (unsigned char *)REALLOC(imb->data, imb->siz);
	}
	imb->data[imb->len++] = c;
}
