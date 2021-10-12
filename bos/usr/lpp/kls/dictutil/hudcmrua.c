static char sccsid[] = "@(#)90	1.1  src/bos/usr/lpp/kls/dictutil/hudcmrua.c, cmdkr, bos411, 9428A410j 5/25/92 14:40:59";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudcmrua.c
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
 *  Module:       hudcmrua.c
 *
 *  Description:  Inserts a key and candidate into MRU area.
 *
 *  Functions:    hudcmrua()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*		Include Header.						*/
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package. */
#include <memory.h>     /* Memory Package        */
#include "hut.h"        /* Utility Define File   */

/*----------------------------------------------------------------------*/
/*			Begining of hudcmrua.				*/
/*		Inserts our key and candidate into MRU area.		*/
/*----------------------------------------------------------------------*/
void hudcmrua(udpnt, key, keylen, cand, candlen)
 uchar   *udpnt;   /* user dictionary buffer pointer */
 uchar   *key  ;   /* key data                       */
 short  keylen;    /* key length                     */
 uchar   *cand ;   /* candidate data                 */
 short  candlen;   /* candidate length               */
{
 char    *calloc()        ;
 void    free()           ;
 short   step_no          ;  /* step number            */
 short   mru_len          ;  /* MRU length             */
 short   insp             ;  /* copy position          */
 short   total_len        ;  /* total length           */
 short   ptr1, ptr2, ptr3 ;  /* MRU area check pointer */
 short   lastcflag;	     /* Flag, Is last candidate*/
 uchar	 l_keydata  [U_KEY_MX];
 uchar	 l_canddata [U_CAN_MX];
 uchar   *dummy1           ;  /* dummy buffer           */
 char    *mru_dat          ;  /* MRU data area          */

/* if error occured , break out  */
 for( step_no = NULL ; step_no <= U_STEPNO ; step_no++)
 {
        switch(step_no)    /* select process by step number */
        {
                case U_STEP0 : 
			 /*****************/
		 	 /*		  */
			 /* MRU data read */
		 	 /*		  */
			 /*****************/
                         total_len = keylen + candlen ;
                         mru_len = getmrulen( udpnt ) ;
                         break ;

                case U_STEP1 : 
			 /******************/
			 /*		   */
		  	 /* MRU area check */
			 /*		   */
			 /******************/
                         /* allocate MRU data area */
                         mru_dat = calloc((mru_len + total_len), sizeof(uchar)) ;
                         memcpy(mru_dat, udpnt, mru_len) ;
                         if((total_len + mru_len) > U_MRU_A)
                         {
                           for(ptr1 = U_STSLEN+U_MRULEN; ptr1 < (U_MRU_A - total_len);)
                           {
                             ptr3 = ptr1 ;
			     ptr1 += nxtkeylen(mru_dat, ptr1, U_MRU_A);
			     ptr1 += nxtcandlen(mru_dat, ptr1, &lastcflag, U_MRU_A);
                           }
                           /* delete the oldest Key and Cand */
                           mru_len = ptr3 ;
                         }
                         break ;

                case U_STEP2 : 
			 /***********************/
			 /*			*/ 
			 /* Converts the codes. */
			 /*			*/ 
			 /***********************/
			 memcpy(l_keydata, key, (int)keylen);
			 mkrbnk(l_keydata, keylen);
                         memcpy(l_canddata, cand, (int)candlen) ;
			 mkrbnlastc(l_canddata, candlen);
                         break ;

                case U_STEP3 : 
			 /****************/
			 /*		 */
			 /* entry insert */
			 /*		 */
			 /****************/
                         humvch(mru_dat,    /* area move  */
                                (int)U_TPOSI,
                                (int)(mru_len - sizeof(short)),
                                (int)U_BACKWD,
                                total_len,
                                (int)FALSE,
                                dummy1,
                                NULL,
                                NULL ) ;
                         mru_len += total_len ; /* plus MRU length */
			 setmrulen(mru_dat, mru_len);
                         insp = (U_STSLEN+U_MRULEN) ;   /* set top position */
                         /* copy key data */
                         memcpy((mru_dat + insp), l_keydata, (int)keylen) ;
                         /* copy cand data */
                         memcpy((mru_dat + insp + keylen), l_canddata, (int)candlen) ;
                         break ;

                case U_STEP4 : 
			 /******************/
			 /*		   */
		 	 /* MRU data write */
			 /*		   */
			 /******************/
                         memcpy(udpnt, mru_dat, (int)mru_len) ;
                         free(mru_dat) ; /* free allocated area */
                         break ;
        }
 }
 return ; 
}
/*----------------------------------------------------------------------*/
/*		End of hudcmrua.					*/
/*----------------------------------------------------------------------*/
