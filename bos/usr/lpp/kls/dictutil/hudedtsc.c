static char sccsid[] = "@(#)96	1.1  src/bos/usr/lpp/kls/dictutil/hudedtsc.c, cmdkr, bos411, 9428A410j 5/25/92 14:41:58";
/*
 * COMPONENT_NAME :	(KRDU) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudedtsc.c
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
 *  Module:       hudedtsc.c
 *
 *  Description:  User Dictionary Data Area Search
 *
 *  Functions:    hudedtsc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*              Include Header.                                         */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package. */
#include "hut.h"        /* Utility File.         */

/*----------------------------------------------------------------------*/
/*              Begining of hudedtsc.					*/
/*   Finds whearher there is our key and candidate aleady exist.	*/
/*----------------------------------------------------------------------*/
int  hudedtsc ( dicdata, key, keylen, cand, candlen, datapos, datapos1, firstfg, t_candlen)
uchar  *dicdata;  /* Pointer to RRN                                     */
uchar  *key;      /* Pointer to Input Key  Data                         */
short   keylen;   /* Input Key  Data Length                             */
uchar  *cand;     /* Pointer to Candidate Data                          */
short   candlen;  /* Input Candidate Data Length                        */
short  *datapos;  /* Pointer to Data Position 0                         */
short  *datapos1; /* Pointer to Data Position 1                         */
short  *firstfg;
short  *t_candlen;
{
  int	 rc;						/* Return Code  */
  uchar  l_keydata [U_KEY_MX],
  	 l_canddata[U_CAN_MX];
  short d_len,		/* Active Data Block Length 			*/
	 d_keylen,	/* Key Data Length of data block 		*/
 	 d_candlen,	/* Candidate length of Data block 		*/
 	 d_cbsz,	/* Candidates Block Size of data block 		*/
	 prelen,	/* Previous Entry Length 			*/
	 lastcflg,	/* Is Last Candidates of a Key 			*/
	 length,	/* Length of compare data 			*/ 
	 ret;	  	/* Comparision Result 				*/
  

  /*  1.1
   *      Initialize
   */
   *datapos  = NULL;         /* Clear Relative Byte Address -1  */
   *datapos1 = NULL;         /* Clear Relative Byte Address -2  */
   *firstfg = U_FOF;
   *t_candlen = keylen ;

  /* 
   * Modify Comments: 
   * src	
   *	If index Block's update required, then lastflg is on. 
   *    The key of Index Block is the last key of Data Block.
   * our
   *	If Index Block's update required, then firstflg is on.
   *	The key of Index Block is the first key of Data Block.
   */

  /**********************/
  /* Local Copy         */ 
  /* Input Parameter is */
  /* the KS code without*/
  /* modified           */
  /**********************/
   memcpy(l_keydata, key, keylen);
   mkrbnk(l_keydata, keylen);
   memcpy(l_canddata, cand, candlen);
   mkrbnc(l_canddata, candlen);
                             
   d_len = getrl(dicdata);
   if ( d_len <= NULL ) 
   {
	return(IUFAIL);     
   }
   for ( *datapos1 = U_RLLEN; *datapos1 < d_len; *datapos1 += prelen )
   {
      d_keylen = nxtkeylen(dicdata, *datapos1, U_REC_L);
      length = ( d_keylen < keylen ) ? d_keylen  : keylen;
      /***********************/
      /*                     */
      /* In order to Compare */
      /*                     */
      /***********************/
      makeksstr(&l_keydata[0], keylen);
      makeksstr(&dicdata[*datapos1], d_keylen);

      rc = memcmp(&l_keydata[0], (char *)&dicdata[*datapos1], (int)length);

      /***********************/
      /*                     */
      /* Restore the Code    */
      /*                     */
      /***********************/
      mkrbnk(&l_keydata[0], keylen);
      mkrbnk(&dicdata[*datapos1], d_keylen);

      *datapos = *datapos1 + d_keylen;   
      d_cbsz = nxtkoffset(dicdata, *datapos, U_REC_L);
      /*************************/
      /*		       */
      /* Search Candidate Data */
      /*		       */
      /*************************/
      if ( rc == NULL  &&  keylen == d_keylen )
      {
         while ( 1 ) {
            d_candlen = nxtcandlen(dicdata, *datapos, &lastcflg, U_REC_L);
            if (lastcflg == U_FON) {
		memcpy(l_canddata, cand, candlen);
		mkrbnlastc(l_canddata, candlen);
	    }
            ret = hudstrcmp(l_canddata, candlen, 
 	                   (uchar*)&dicdata[*datapos], d_candlen);
            if (ret == 0) {
                *datapos -= d_candlen;
		while(lastcflg != U_FON)
		{
            	      d_candlen = nxtcandlen(dicdata, *datapos, &lastcflg, U_REC_L);
		      *t_candlen += d_candlen ;
		}
                return (U_KEYCAN);
            }
            *datapos += d_candlen;
	    *t_candlen += d_candlen;
            if (lastcflg == U_FON) break;
         }
         return (U_NOCAN);
      }
      if ( rc < 0 || ( rc == 0  &&  keylen < d_keylen  ) )
      {
         if ( *datapos1 == U_RLLEN )
            *datapos = U_RLLEN;    
         else
         {
            *datapos   = *datapos1; 
            *datapos1 -= prelen;     
         }
	 if (*datapos == U_RLLEN) *firstfg = U_FON;
         return(U_NOKEY);             
      }
      prelen = d_keylen + d_cbsz ;     
   }
   *datapos  = d_len;                   
   *datapos1 = d_len - d_keylen - d_cbsz; 
   return(U_NOKEY);                      
}
/*----------------------------------------------------------------------*/
/*              	End of hudedtsc.				*/
/*----------------------------------------------------------------------*/
