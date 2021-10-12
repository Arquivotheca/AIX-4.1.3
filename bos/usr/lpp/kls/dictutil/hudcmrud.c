static char sccsid[] = "@(#)91	1.1  src/bos/usr/lpp/kls/dictutil/hudcmrud.c, cmdkr, bos411, 9428A410j 5/25/92 14:41:09";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudcmrud.c
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
 *  Module:       hudcmrud.c
 *
 *  Description:  Deletes a Key and Candidate from MRU area.
 *
 *  Functions:    hudcmrud()
 *
 *  History:      5/22/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*              Include Header.                                         */
/*----------------------------------------------------------------------*/

#include <stdio.h>   	/* Standards I/O package */ 
#include <memory.h>     /* Memory Package 	 */
#include "hut.h"        /* Utility Define File   */

/*----------------------------------------------------------------------*/
/*              Begining of hudcmrud.                                   */
/*----------------------------------------------------------------------*/
void  hudcmrud( mode, key, keylen, cand, candlen, udcbptr )
short  mode    ;      /* Delete Mode ( Key Delete or Candidate Delete ) */
uchar   *key   ;     
short  keylen  ; 
uchar   *cand  ;  
short  candlen ; 
UDCB   *udcbptr ; 
{
   uchar   mrudata[U_MRU_A],       /* MRU Data Read Area */
	   l_keydata  [U_KEY_MX], 
	   l_canddata [U_CAN_MX],
           clrstr[U_REC_L];        /* Work Delete Entry */
   short   m_len,
	   m_p,
	   m_keylen,
	   lastcflg,
	   m_candlen;
   int     del_f;                  /* Delete Flag                  */
   int     dpos,                   /* Move Charcter Start Position */
           dlen,                   /* Move Charcter Length         */
           ddst;                   /* Move Charcter Distance       */

   /*********/
   /*	    */
   /* Init. */
   /*	    */
   /*********/
   memset(clrstr, 0xff, U_REC_L);
   del_f   = FALSE;                          

  /**************/
  /*		*/
  /* Local Copy */
  /*		*/
  /**************/
   memcpy(l_keydata, key, (int)keylen);
   mkrbnk(l_keydata, keylen);
   memcpy(l_canddata, cand, (int)candlen);
   mkrbnlastc(l_canddata, candlen);

   memcpy(mrudata, udcbptr->dcptr, U_MRU_A );
   m_len = getmrulen(mrudata); 

   /*******************/
   /*		      */
   /* Mru Area Search */
   /*		      */
   /*******************/
   for ( m_p = (U_STSLEN+U_MRULEN); m_p < m_len; m_p += (m_keylen+m_candlen) ) {
       m_keylen = nxtkeylen(mrudata, m_p, U_MRU_A);	       
       m_candlen = nxtcandlen (mrudata, m_p+m_keylen, &lastcflg, U_MRU_A);
       if ( m_keylen == keylen  &&
            memcmp( &mrudata[m_p], l_keydata, keylen ) == NULL )
	  /* Finds there is our key and candidate */
          if ( mode == U_S_KEYD ||
             ( m_candlen == candlen  &&
               memcmp(&mrudata[m_p+m_keylen], l_canddata, candlen) == NULL) )
             {
	     /* find s O.K */
             del_f = TRUE;                  
             dpos  = m_p + m_keylen + m_candlen;
             dlen  = m_len - dpos;  
             ddst  = m_keylen + m_candlen;     
             if ( m_len == dpos ) { 
		memset(mrudata+m_p, 0xff, (m_len-m_p));
 	     } else {      
                humvch ( mrudata, dpos, dlen, U_FORWD, ddst,
                    TRUE, clrstr, clrstr[0], ddst );
	     }
             m_len -= (m_keylen+m_candlen); 
             m_p -= (m_keylen+m_candlen);    
             }
   }   /* for */
   /* Deletes a key and candidate */
   if ( del_f == TRUE ) {
       setmrulen(mrudata, m_len); 
       memcpy ( udcbptr->dcptr, mrudata, U_MRU_A );
   }
   return;
}
/*----------------------------------------------------------------------*/
/*              End of hudcmrud.                                        */
/*----------------------------------------------------------------------*/
