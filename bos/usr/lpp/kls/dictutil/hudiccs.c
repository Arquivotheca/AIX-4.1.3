static char sccsid[] = "@(#)02	1.1  src/bos/usr/lpp/kls/dictutil/hudiccs.c, cmdkr, bos411, 9428A410j 5/25/92 14:42:59";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudiccs.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudiccs.c
 *
 *  Description:  Checks System Dictionary.
 *
 *  Functions:    hudiccs()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      
#include "hut.h"       

/*----------------------------------------------------------------------*/
/*                      Begining of hudcread.                           */
/*	Finds wheather there is our key candidate in Systerm Dictionary */
/*----------------------------------------------------------------------*/
void hudiccs ( sdcbpt, keylen, key, candlen, cand, srcflg )
SDCB    *sdcbpt;         		/* Pointer to System Dictionry */
short   keylen;        
uchar   *key;      
short   candlen;  
uchar   *cand;   
short  *srcflg;        					/* Return Code. */
{
extern  void    hudcread();     	/* Dictionary Pointer Search    */
short  cbix;
short  d_p,
	d_candlen,
	lastcflg,
	rc,
	rrn;
uchar	l_keydata[U_KEY_MX],
	l_canddata[U_CAN_MX],
	*dicdata,
	*dicindex;

	/***********************/
	/*		       */
	/* Return code setting */
	/*		       */
	/***********************/
        *srcflg = 0;

	/****************************************/
    	/*					*/
	/* Local Copy                           */
	/* Inorder to compare system dictionary */
    	/*					*/
	/****************************************/
	memcpy(l_keydata, key, (int)keylen);
	mkrbnk(l_keydata, keylen);
	memcpy(l_canddata, cand, (int)candlen);
	mkrbnc(l_canddata, candlen);

	/******************************************/
	/*					  */
	/* Gets the Systerm Dictionary IndexBlock */
	/*					  */
	/******************************************/
        (void)hudcread(sdcbpt, (short)1, (short)0);
        dicindex = (uchar*)sdcbpt->rdptr;
 	rrn = findrrn(dicindex, key, keylen, SYSDICT);	
	if (rrn < 2) return;
	
	/******************************************/
	/*					  */
	/* Gets the Systerm Dictionary DataBlock  */
	/*					  */
	/******************************************/
	(void)hudcread(sdcbpt, (short)2, (short)rrn);
	dicdata = (uchar*)sdcbpt->rdptr;
/****
	cbix = findcblock(dicdata, l_keydata, keylen, SYSDICT);
******/
	cbix = findcblock(dicdata, key, keylen, SYSDICT);
	if (cbix < 0) return;
	/************************/
	/*			*/
	/* Search the candidate */
	/*			*/
	/************************/
	d_p = cbix;
	do {
	   d_candlen = nxtcandlen(dicdata, d_p, &lastcflg, U_REC_L);
	   if (lastcflg == U_FON) {
		/*****************************************/
		/*					 */
	 	/* In ordef to compare, convert the code */
		/*					 */
		/*****************************************/
		mkrbnlastc(l_canddata, candlen);
	   }
	   rc = hudstrcmp(l_canddata, candlen, dicdata+d_p, d_candlen);
	   if (rc == 0) {
		*srcflg = 1;
		return;
	   } 
	   d_p += d_candlen;
	} while(lastcflg == U_FOF);
	return;
}
/*----------------------------------------------------------------------*/
/*                      End of hudcread.                                */
/*----------------------------------------------------------------------*/
