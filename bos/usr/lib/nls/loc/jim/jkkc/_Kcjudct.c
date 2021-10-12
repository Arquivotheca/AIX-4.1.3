static char sccsid[] = "@(#)12	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjudct.c, libKJI, bos411, 9428A410j 7/23/92 03:15:40";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcjudct
 *
 * DESCRIPTIVE NAME:  USER DICTIONARY LOOKING UP.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
 *                    0x7fff(UERROR):     unpredictable error
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
short _Kcjudct(z_kcbptr,z_strpos,z_length)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
unsigned char z_strpos;                 /* offset of 1st MCE            */
unsigned char z_length;                 /* length of string(mora code)  */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETJUHSH   _Kcjuhsh();
   extern short             _Kcjuunq();
   extern short             _Kczread();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcuxe.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_NOTFND     0x08ff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short             z_rc;              /* return code save area        */
   short             z_rc2;             /* return code save area        */
   struct RETJUHSH   z_ret;             /* rerative record number       */
   short             z_k;               /* loop counter                 */


   kcbptr1 = z_kcbptr;
/*----------------------------------------------------------------------*
 *      YOU SHOULD LOCK AND UNLOCK "INDEX AND DATA OF USER DICTIONARY"
 *      IN CORRECT SECUENCE.
 *      BUT YOU DON'T HAVE TO DO IT IN THIS VERSION,
 *      BEACAUSE USER DICTIONARY IS LOADED ON MEMORY NOW.
 *      IF YOU WANT TO DO IT, YOU SHOULD MAKE "_Kczlock()" & "_Kczunlk()"
 *      AND CHANGE "_Kczread()".
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *      LOCK "INDEX OF USERDICTIONARY" IN FUTURE
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *       READ THE RECORD CORRESPONDED TO RRN FROM TARGET FILE
 *----------------------------------------------------------------------*/
   z_rc = _Kczread(z_kcbptr,UX,0 , EXCLUSIVE, 0 );

   if ( z_rc != SUCCESS )
   {
      return( z_rc );
   }
   uxeptr1 = kcb.uxeuxe;

/*----------------------------------------------------------------------*
 *       SERACH INDEX TO GET RRN (Rerative Record Number)
 *----------------------------------------------------------------------*/
   z_ret = _Kcjuhsh(z_kcbptr,z_strpos,z_length);

   if ( z_ret.rc != SUCCESS)
   {
      if ( z_ret.rc == Z_NOTFND)
         /*-------------------------------------------------*
          *          UNLOCK UX IN FUTURE
          *-------------------------------------------------*/
         return((short)SUCCESS);

      else
         /*-------------------------------------------------*
          *          UNLOCK UX IN FUTURE
          *-------------------------------------------------*/
         return( UERROR );
   }

/*----------------------------------------------------------------------*
 *      LOCK "INDEX OF USERDICTIONARY" IN FUTURE
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *       READ THE RECORD CORRESPONDED TO RRN FROM TARGET FILE 
 *----------------------------------------------------------------------*/
   for(z_k = 0;( z_k < 5 ) && ( z_ret.rrn[z_k] != -1 ); z_k++)
   {

      if ( z_ret.rrn[z_k] > (short)(uxe.har) )
      {
                       /*-------------------------------------------------*
                        *          UNLOCK UX IN FUTURE
                        *-------------------------------------------------*/
         return((short)UERROR);    /*  if system dictionary error  */
      }

      z_rc = _Kczread(z_kcbptr,UD,z_ret.rrn[z_k] , EXCLUSIVE, 0 );

      if ( z_rc != SUCCESS)
      {
         /*-------------------------------------------------*
          *          UNLOCK UX IN FUTURE
          *-------------------------------------------------*/
         return( z_rc );
      }

/*----------------------------------------------------------------------*
 *       SERACH BUFFER TO GET JIRITSU-GO
 *----------------------------------------------------------------------*/
      z_rc = _Kcjuunq(z_kcbptr,z_strpos,z_length);

      if ( z_rc != SUCCESS )
      {
         /*-------------------------------------------------*
          *          UNLOCK UD IN FUTURE
          *-------------------------------------------------*/
         /*-------------------------------------------------*
          *          UNLOCK UX IN FUTURE
          *-------------------------------------------------*/
          return( z_rc );
      }
   }
/*----------------------------------------------------------------------*
 *      UNLOCK "DATA OF USERDICTIONARY" IN FUTURE
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *      UNLOCK "INDEX OF USERDICTIONARY" IN FUTURE
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
