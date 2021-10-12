static char sccsid[] = "@(#)54	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcnmtch.c, libKJI, bos411, 9428A410j 6/4/91 15:15:49";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcnmtch
 *
 * DESCRIPTIVE NAME:  GET POINTER OF BTE WHICH HAS CURRENT SEISHO
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x00ff (Z_EXIST):     requested BTE exists
 *                    0x01ff (Z_NOT_EXIST): requested BTE doesn't exist
 *                    0x0108 (WSPOVER): work space overflows
 *                    0x7fff (UERROR):  unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
struct RETNMTCH _Kcnmtch(z_kcbptr,z_hykptr,z_hyklen)

struct KCB      *z_kcbptr;              /* pointer of KCB               */
unsigned char   *z_hykptr;              /* pointer of hyouki            */
short           z_hyklen;               /* length of hyouki             */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCBKNJ   _Kccbknj();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcbte.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_EXIST       0x00ff
#define   Z_NOT_EXIST   0x01ff

#define   Z_OVER        0x02ff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETNMTCH   z_ret;             /* return code save area        */
   struct RETCBKNJ   z_retk;            /* return code for _Kccbknj     */
   uschar            z_buf[SEIMAX];     /* work area of hyouki          */
   short             z_i;               /* counter                      */
   short             z_fg;              /* flg                          */
   uschar            z_endfg;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

/*----------------------------------------------------------------------*
 *      GET BTE WHICH HAS THE TARGET SEISHO                             *
 *----------------------------------------------------------------------*/
   /*----------------   CHECK THE NUMBER OF CANDIDATES   ---------------*/
   if (CACTV(kcb.bthchh) == 0 )
   {
      z_ret.bteno = 0;
      z_ret.rc = (short)Z_NOT_EXIST;
      z_ret.bteptr = NULL;
      return(z_ret);
   }

   FOR_FWD ( kcb.bthchh, bteptr1, z_endfg )
   {
                                        /* get hyouki of target BTE     */
      LST_FWD( bteptr1, z_endfg );

      z_retk = _Kccbknj( z_kcbptr, bteptr1, z_buf, SEIMAX );
      if ( z_retk.rc != SUCCESS )
      {
         z_ret.bteno = 0;
         z_ret.rc = (short)UERROR;
         z_ret.bteptr = NULL;
         return(z_ret);
      }
                                        /* compare hyouki of target BTE */
                                        /*   with hyouki of current BTE */

      if ( z_hyklen == z_retk.kjno * 2 )
      {
         for( z_i = 0, z_fg = 0; z_hyklen > z_i; z_i++ )
         {
            if ( z_hykptr[z_i] != z_buf[z_i] )
            {                           /* break if hyouki are different*/
               z_fg = 1;
               break;
            }
         }

         /*-----------------   both hyouki are same   ------------------*/
         if ( z_fg == 0 )
         {
            z_ret.rc = (short)Z_EXIST;
            z_ret.bteptr = bteptr1;
            z_ret.bteno = 0;
            return( z_ret );
         }
      }
   }

   /*--------   BTE which has the target hyouki is not found   ---------*/ 
   z_ret.bteno = 0;
   z_ret.rc = (short)Z_NOT_EXIST;
   z_ret.bteptr = NULL;
   return(z_ret);
}
