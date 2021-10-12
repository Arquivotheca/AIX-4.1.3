static char sccsid[] = "@(#)10	1.1  src/bos/usr/lpp/kls/dictutil/hudicueh.c, cmdkr, bos411, 9428A410j 5/25/92 14:44:24";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicueh.c
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
/************************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudicueh.c
 *
 *  Description:  User Dictionary End Update Handler
 *
 *  Functions:    hudicueh()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ************************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.   			*/
#include "hut.h"        /* user dictionary utility 			*/

/*----------------------------------------------------------------------*/
/*                      Begining of hudicueh.                           */
/*----------------------------------------------------------------------*/
int     hudicueh ( frstpos, lastpos, udcbptr, errpos )
UDCS    *frstpos;       /* Pointer to First Dictionary Buffer                */
UDCS    *lastpos;       /* Pointer to Last Dictionary Buffer                 */
UDCB    *udcbptr;       /* Pointer to User Dictionary Control Block          */
UDCS   **errpos;        /* Pointer to Pointer to Error Dictionary Buffer     */
{
    void    hudicdlc();     /* User dictionary Data Delete Function          */
    int     hudicadp();     /* User Dictionary Data Regist Function          */
    void    hudcmrua();     /* User Dictionary MRU Area Regist Function      */
    int     rc;             /* Return Code.                                  */
    UDCS    *pt;  
    short   mode,  
            keylen, 
            candlen; 
    uchar   *keydata ,
	    *canddata;

    rc  =  IUSUCC;

    /* User Dictionary Delete */
    hudicdlc( lastpos, udcbptr );

    /* User dictionary addition */
    for ( pt = frstpos ; pt != NULL ; pt = pt->nx_pos )
        if ( pt->status == U_S_KEYA || pt->status == U_S_CANU )
            {
            mode       = U_REGIST;    
            keylen     = (short)pt->keylen;     
            candlen    = (short)pt->candlen;      
 	    keydata    = pt->key;
	    canddata   = pt->cand;
            rc = hudicadp( mode, keydata, keylen, canddata, candlen, udcbptr );
            if ( rc != IUSUCC )
                *errpos = pt;    
            if ( rc != IUSUCC  &&  rc != UDOVFDLE )
                break;          
            if ( rc == IUSUCC )
               {
               hudcmrua ( udcbptr->dcptr, keydata, keylen, canddata, candlen );
               }
            }
    return( rc );
}
/*----------------------------------------------------------------------*/
/*                      End of hudicueh.                                */
/*----------------------------------------------------------------------*/
