/* @(#)70	1.3  src/bos/usr/lib/nls/loc/jim/jed/jedFix.c, libKJI, bos411, 9428A410j 6/6/91 11:00:43 */
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
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int	jedFix(fepcb, imb)
FEPCB	*fepcb;
{
	extern  void    set_imode();
	KCB     *kcb = fepcb->kcb;
	int     i;
	char    **toindexs, **toindexa;

	/***********************/
	/* create fixed string */
	/***********************/
	placestr(imb, kcb->string, kcb->lastch);

	/**************************************************/
	/* clear echo buffers, both string and attributes */
	/**************************************************/
	(void)memset(fepcb->echobufs, 0, fepcb->echosize);
	(void)memset(fepcb->echobufa, 0, fepcb->echosize);

	/*************************************************/
	/* clear aux buffers, both string and attributes */
	/*************************************************/
	toindexs = fepcb->auxbufs;
	toindexa = fepcb->auxbufa;
	for(i = 0; i < fepcb->auxsize.itemnum; i++) {
	    (void)memset(*toindexs++, 0, fepcb->auxsize.itemsize);
	    (void)memset(*toindexa++, 0, fepcb->auxsize.itemnum);
	}
	/**************************************************/
	/* set what part of echo data changed by this fix */
	/**************************************************/
	fepcb->echochfg.chlenbytes  = fepcb->kcb->chlen;

	/************************/
	/* clear Monitor buffer */
	/************************/
	(void)exkjclr(fepcb->kcb);

	/************************************/
	/* we really need following ?       */
	/* which derived from original code */
	/************************************/
	set_imode(fepcb);

	/*******************************/
	/* update internal information */
	/*******************************/
	fepcb->echoacsz = 0;
	fepcb->auxacsz.itemsize = 0;
	fepcb->auxacsz.itemnum = 0;
	fepcb->echochfg.flag  = ON;
	fepcb->echochfg.chtoppos  = 0;
	fepcb->auxchfg = ON;
	fepcb->indchfg = ON;
	fepcb->auxuse = NOTUSE;
	fepcb->axconvsw = OFF;
	fepcb->echocrps = 0;
	fepcb->auxcrps.colpos = -1;
	fepcb->auxcrps.rowpos = -1;
	fepcb->eccrpsch = ON;
	fepcb->axcrpsch = ON;
	fepcb->isbeep = OFF;

	return KP_OK;
}
