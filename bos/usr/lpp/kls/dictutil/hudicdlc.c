static char sccsid[] = "@(#)04	1.1  src/bos/usr/lpp/kls/dictutil/hudicdlc.c, cmdkr, bos411, 9428A410j 5/25/92 14:43:18";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicdlc.c
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
 *  Module:       hudicdlc.c
 *
 *  Description:  Candidate User Dictionary Delete Control
 *
 *  Functions:    hudicdlc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.             		*/
#include <memory.h>     /*                                   		*/
#include "hut.h"        /* Candidate user dictionary utility 		*/

/*----------------------------------------------------------------------*/
/*                      Begining of hudicdlc.                           */
/*----------------------------------------------------------------------*/
void   hudicdlc ( lastpos, udcbptr )
UDCS    *lastpos;       /* Pointer Last Dictionary Buffer           	*/
UDCB    *udcbptr;       /* Pointer to User Dictionary Control Block 	*/
{
    void    hudicdlp();     
    void    hudcmrud();      
    UDCS    *pt;              
    short   mode,              
	    pos,
	    key_f,
            keylen,         
	    prev_keylen,
            candlen;            
    uchar   prev_keydata[U_KEY_MX],      /* Previous Key Data 		*/
            *canddata , 
	    *keydata  ;

    /*******************************/
    /*				   */
    /* User Dictionary Data Delete */
    /*				   */
    /*******************************/
    for ( pt = lastpos ; pt != NULL ; pt = pt->pr_pos )
      if ( pt->status == U_S_KEYD || pt->status == U_S_CAND
                                  || pt->status == U_S_CANU )
        {
        mode = (short)( pt->status == (uchar)U_S_KEYD ) ? U_S_KEYD : U_S_CAND;
        keylen     = (short)pt->keylen; 
	keydata    = pt->key;
        candlen    = (short)pt->candlen;
	canddata   = pt->cand;
        pos        = (short)pt->pos;     
        key_f      = TRUE;          
        if (mode == U_S_KEYD) {
          if ((prev_keylen == keylen) && 
	      (memcmp(prev_keydata, keydata, keylen) == NULL)) {
             key_f = FALSE;      
	  }
	  /***************************************/
	  /*					 */
          /* Current Key Delete Data String Save */
	  /*					 */
	  /***************************************/
          memcpy (prev_keydata, keydata, keylen);
	  prev_keylen = keylen;
        }
	/****************************************/
	/*					*/
	/* Deletes a key and it's a candidate.  */
	/*					*/
	/****************************************/
        if (key_f == TRUE) {
	  /**************************************/
	  /*					*/
	  /* Deletes a key's position candidate */
	  /*					*/
	  /**************************************/
          hudicdlp( mode, keydata, keylen, pos, udcbptr );
          if ( mode == U_S_KEYD )     
            { 
            hudcmrud( mode, keydata, keylen, canddata, candlen, udcbptr );
            }
        }
       }
    return;
}
/*----------------------------------------------------------------------*/
/*                      End of hudicdlc.                                */
/*----------------------------------------------------------------------*/
