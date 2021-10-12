static char sccsid[] = "@(#)69	1.1  src/bos/usr/lib/nls/loc/imk/ked/kedFix.c, libkr, bos411, 9428A410j 5/25/92 15:42:10";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kedFix.c
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
 *  Module:       kedFix.c
 *
 *  Description:  Makes character being manipulated fixed 
 *
 *  Functions:    kedFix()
 *               
 *  History:      
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <im.h>
#include <imP.h>
#include "kedconst.h"
#include "ked.h"
#define	FORWARD	1

/*-----------------------------------------------------------------------*
*        external reference
*-----------------------------------------------------------------------*/

extern  char    *memcpy(), *memset() ;

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int      kedFix( kimed, imbuffer )
KIMED    *kimed ;
IMBuffer *imbuffer;
{
	/*******************/
	/* local variables */
	/*******************/
	int     	i;
	char    	**toindexs, **toindexa;
	register int	len;

        /***********************/
        /* create fixed string */
        /***********************/
	len = kimed->echoacsz;
	placestr(imbuffer,kimed->echobufs,len);

	/*
	 * Sets echobufs change informations.
	 */
	kimed->echochfg.flag = ON;
	kimed->echochfg.chlenbytes = len;
	kimed->echochfg.chtoppos = 0;

	/**************************************************/
	/* clear echo buffers, both string and attributes */
	/**************************************************/
	(void)memset(kimed->echobufs, NULL, len) ;
	(void)memset(kimed->echobufa, NULL, len) ;

	/*************************************************/
	/* clear aux buffers, both string and attributes */
	/*************************************************/
	toindexs = kimed->auxbufs;
	toindexa = kimed->auxbufa;
	for(i = 0; i < kimed->auxsize.itemnum; i++) {
	    (void)memset(*toindexs++, NULL, kimed->auxsize.itemsize) ;
	    (void)memset(*toindexa++, NULL, kimed->auxsize.itemnum) ;
	}

	/*******************************/
	/* update internal information */
	/*******************************/
	kimed->echoacsz  	= 	0;
	kimed->echocrps  	= 	0 ;
	kimed->eccrpsch  	= 	ON ;
	kimed->isbeep  		= 	OFF ;
        kimed->echosvchsp       =       -1 ;
        kimed->echosvchlen      =       0 ;
        kimed->hgstate          =       HG_ST_FINAL ;
	for (i=0; i<=4; i++)
        {
            kimed->hg_status_buf[i].state = HG_ST_FINAL;
            kimed->hg_status_buf[i].cmpshg = NULL;
            kimed->hg_status_buf[i].cmplhg = NULL;
        }
        kimed->hg_status_ps = -1 ;
        kimed->auxformat        =       NULL ;
        kimed->candgetfg        =     FORWARD ;
        kimed->candsize         =       0  ;
        kimed->candcrpos        =       0  ;
        switch (kimed->imode.basemode)
            {
                  case   MD_HAN :  kimed->interstate = ST_HAN ;
                                   break ;
                  case   MD_ENG :  kimed->interstate = ST_ENG ;
                                   break ;
                  case   MD_JAMO:  kimed->interstate = ST_JAMO ;
            }
	return KP_OK ;
} /* end of kedFix */
