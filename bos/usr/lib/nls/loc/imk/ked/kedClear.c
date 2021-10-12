static char sccsid[] = "@(#)66	1.1  src/bos/usr/lib/nls/loc/imk/ked/kedClear.c, libkr, bos411, 9428A410j 5/25/92 15:41:40";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kedClear.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM ED  
 *
 *  Module:       kedClear.c
 *
 *  Description:  Clears the buffers and initializes 
 *                the internal contrl block 
 *
 *  Functions:    kedClear()
 *
 *  History:      
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <stdio.h>
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*	external reference
*-----------------------------------------------------------------------*/

extern  char    *memset() ;

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/

#define		FORWARD		1

int     kedClear( kimed )
KIMED   *kimed ;
{
	/*******************/
	/* local variables */
	/*******************/
	int i;
	char **toindexs, **toindexa;

	/*********************************************/
	/* clear echo buffers, string and attributes */
	/*********************************************/
	(void)memset(kimed->echobufs, NULL, kimed->echosize) ;
	(void)memset(kimed->echobufa, NULL, kimed->echosize) ;
	kimed->echochfg.chlenbytes = kimed->echoacsz;

	/**************************************************/
	/* clear aux buffers, both string and attributes  */
	/**************************************************/
	toindexs = kimed->auxbufs;
	toindexa = kimed->auxbufa;
	for(i = 0; i < kimed->auxsize.itemnum; i++) {
	    (void)memset(*toindexs++, NULL, kimed->auxsize.itemsize) ;
	    (void)memset(*toindexa++, NULL, kimed->auxsize.itemsize) ;
        }

	/********************************/
	/* update internal information  */
	/********************************/
	kimed->echoacsz  = 0 ;
	kimed->fixacsz   = 0 ;
	kimed->auxacsz.itemsize   = 0 ;
	kimed->auxacsz.itemnum   = 0 ;
	kimed->echochfg.flag  = ON ; /* chlenbyte is set above */
	kimed->echochfg.chtoppos  = 0 ;
	kimed->auxchfg   = ON ;
	kimed->indchfg   = ON ;
	kimed->auxuse    = AUXBUF_NOTUSED ;
	kimed->echocrps  = 0 ;
	kimed->auxcrps.colpos   = -1 ;
	kimed->auxcrps.rowpos   = -1 ;
	kimed->eccrpsch  = ON ;
	kimed->axcrpsch  = ON ;
	kimed->isbeep = OFF;
        kimed->echosvchlen = 0 ;
        kimed->echosvchsp = -1 ;
        kimed->auxformat = NULL ;
        kimed->candgetfg = FORWARD ;
        kimed->candsize = 0 ;
        kimed->candcrpos = 0 ;
        kimed->hgstate = HG_ST_FINAL ;
	for (i=0; i<=4; i++)
        {
              kimed->hg_status_buf[i].state = HG_ST_FINAL;
              kimed->hg_status_buf[i].cmpshg = NULL;
              kimed->hg_status_buf[i].cmplhg = NULL;
        }
        kimed->hg_status_ps = -1 ;
        switch(kimed->imode.basemode)
           {
              case  MD_HAN : kimed->interstate = ST_HAN  ;
                             break ;
              case  MD_ENG : kimed->interstate = ST_ENG  ;
                             break ;
              case  MD_JAMO: kimed->interstate = ST_JAMO ;
           }

	return KP_OK ;
} /* end of kedClear  */
