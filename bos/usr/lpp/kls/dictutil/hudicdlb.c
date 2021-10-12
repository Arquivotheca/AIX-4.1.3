static char sccsid[] = "@(#)03	1.1  src/bos/usr/lpp/kls/dictutil/hudicdlb.c, cmdkr, bos411, 9428A410j 5/25/92 14:43:09";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicdlb.c
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
 *  Module:       hudicdlb.c
 *
 *  Description:  User Dictionary Buffer Delete Status Code Set
 *
 *  Functions:    hudicdlb()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.                        */
#include "hut.h"        /* Dictionary Utility Header. 			*/

/*----------------------------------------------------------------------*/
/*                      Begining of hudicdlb.                           */
/*----------------------------------------------------------------------*/
void   hudicdlb ( dmappt, fldno )
UDMS    *dmappt;        /* Pointer to Display Map                       */
short    fldno;         /* Field No to be Delete                        */
{
    UDCS     *pt;            /* Pointer to Current Dictonary Buffer     */
    UDCS     *w_pt;          /* Pointer to Work Dictonary Buffer        */
    short     stat;          /* Field Status                            */
    uchar    l_keydata[U_KEY_MX]; /* Key Data String Save Area          */
    short    keylen;              /* Key Data String Length Save Area   */

    /*****************************/
    /*				 */
    /* Data Get from Display Map */
    /*				 */
    /*****************************/
    stat = (short)dmappt->fld[fldno].fstat;    
    pt   = dmappt->fld[fldno].dbufpt;  
    /***************************/
    /*			       */
    /* Cand Field Data Process */
    /*			       */
    /***************************/
    if ( stat == (short)U_CANDF )
        {                 
        if ( pt->status == (uchar)U_S_KEYA )     
           pt->status = (uchar)U_S_ADEL;
        else
           pt->status = (uchar)U_S_CAND;
        }
     /**************************/
     /*			       */
     /* Key Field Data Process */
     /*			       */
     /**************************/
     else                                     /* Key Field Process          	*/
        {                                     /* Input Key Field Data Save  	*/
        keylen = (short)pt->keylen;                   
 	memcpy(l_keydata, pt->key, (int)keylen);
        if ( pt->status == (uchar)U_S_KEYA )       /* Input Key Status Set 	*/
           pt->status = (uchar)U_S_ADEL;           /* Set Delete Status Code 	*/
        else
           pt->status = (uchar)U_S_KEYD;           /* Set Delete Status Code 	*/
        if ( pt->nx_pos != NULL )
           for ( w_pt = pt->nx_pos; w_pt != NULL; w_pt = w_pt->nx_pos )
              {                       /* Search for Same Key for Next 		*/
              if ( keylen != (short)w_pt->keylen ||   /* Check Key Data 	*/
                   memcmp ( l_keydata, w_pt->key, (int)keylen ) != NULL )
                 break;                       /* Defference Key Search End  	*/
              if ( w_pt->status != (uchar)U_S_KEYD   /* Check Status Code       */
               &&  w_pt->status != (uchar)U_S_CAND  
	       &&  w_pt->status != (uchar)U_S_ADEL )
                 if ( w_pt->status == (uchar)U_S_KEYA )
                    w_pt->status = (uchar)U_S_ADEL;  /* Set Delete Status Code  */
                 else
                    w_pt->status = (uchar)U_S_KEYD;  /* Set Delete Status Code  */
              }
        if ( pt->pr_pos != NULL )
           for ( w_pt = pt->pr_pos; w_pt != NULL; w_pt = w_pt->pr_pos )
              {                       /* Search for Same Key for Previous   	*/
              if ( keylen != (short)w_pt->keylen ||   /* Check Key Data         */
                   memcmp ( l_keydata, w_pt->key, (int)keylen ) != NULL )
                 break;                       /* Defference Key Search End  	*/
              if ( w_pt->status != (uchar)U_S_KEYD   /* Check Status Code       */
               &&  w_pt->status != (uchar)U_S_CAND  
	       &&  w_pt->status != (uchar)U_S_ADEL )
                 if ( w_pt->status == (uchar)U_S_KEYA )
                    w_pt->status = (uchar)U_S_ADEL;  /* Set Delete Status Code  */
                 else
                    w_pt->status = (uchar)U_S_KEYD;  /* Set Delete Status Code  */
              }
        }
    return;
}
/*----------------------------------------------------------------------*/
/*                      End of hudicdlb.                                */
/*----------------------------------------------------------------------*/
