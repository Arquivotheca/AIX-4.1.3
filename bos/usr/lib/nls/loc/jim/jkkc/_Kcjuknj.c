static char sccsid[] = "@(#)16	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjuknj.c, libKJI, bos411, 9428A410j 6/4/91 12:56:40";
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
 * MODULE NAME:       _Kcjuknj
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1204 (JKJOVER): JKJ table overflow
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
struct RETJUKNJ _Kcjuknj(z_kcbptr,z_offset)

struct  KCB   *z_kcbptr;
short   z_offset;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCGET _Kccget();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcjkj.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETJUKNJ   z_ret;

   struct RETCGET    z_hyoki;
   uschar            *z_bufptr;         /* address to point jictionary  */
                                        /*  buffer                      */
   short             z_newoff;          /* return value of new offset   */
                                        /* to point the nxtentry        */
   uschar            z_upper;           /* work area for the upper byte */
   uschar            z_lower;           /* work area for the lower byte */
   struct JKJ        *z_jkjp1;          /* work address pointer         */
                                        /*              of jkj entry    */
/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                   /* establish address'ty to mcb */

   z_hyoki = _Kccget(&kcb.jkhchh);       /* get a JKJ entry by _Kccget  */

   if(z_hyoki.rc == GET_EMPTY)
   {
      z_ret.rc = JKJOVER;
      return( z_ret );
   }
   else
   {
      if((z_hyoki.rc != GET_TOP_MID )&&
         (z_hyoki.rc != GET_LAST    ))
      {
         z_ret.rc = UERROR;
         return(z_ret);
      }
   }

/*----------------------------------------------------------------------*
 *      ESTABLISH ADDRESSABULITY TO JKJ ENTRY
 *----------------------------------------------------------------------*/
   z_ret.jkjptr = (struct JKJ *)z_hyoki.cheptr;
                                        /* set the 1st jkj kanji pointer*/
                                        /*    in return code            */
   z_bufptr = (uschar *)(kcb.udeude + z_offset);
                                        /* points kanji in buffer       */
/*----------------------------------------------------------------------*
 *      LOOP WHILE MSB OFF
 *----------------------------------------------------------------------*/
   for( ; ((*z_bufptr) & 0x80) == 0; z_bufptr += 2 )
                                        /* the 1st bit of the upper byte*/
                                        /* of dbcs(2byte code) is off   */
                                        /* except the last code         */
   {
      z_jkjp1  = (struct JKJ *)z_hyoki.cheptr;
                                        /* set the jkj kanji pointer    */
                                        /* into a work pointer          */

      z_upper = *z_bufptr;              /* point the upper byte         */
      (*z_jkjp1).kj[0] = ((z_upper & 0x20)<<1) | z_upper | 0x80 ;
                                        /* decode the upper byte        */

      z_lower = *(z_bufptr + 1);        /* point the lower byte         */
      (*z_jkjp1).kj[1] = z_lower;
                                        /* move the lower to JKJ        */

      z_hyoki = _Kccget(&kcb.jkhchh);   /* get a JKJ entry by _Kccget   */
                                        /* for the nxt entry            */
      if(z_hyoki.rc == GET_EMPTY)
      {
         z_ret.rc = JKJOVER;
         return( z_ret );
      }
      else
      {
         if((z_hyoki.rc != GET_TOP_MID )&&
            (z_hyoki.rc != GET_LAST    ))
         {
            z_ret.rc = UERROR;
            return(z_ret);
         }
      }
   }

/*----------------------------------------------------------------------*
 *      MSB ON ( THE LAST DBCS )
 *----------------------------------------------------------------------*/
   z_jkjp1  = (struct JKJ *)z_hyoki.cheptr;
                                        /* set the jkj kanji pointer    */
                                        /* into a work pointer          */
   z_upper = *z_bufptr;                 /* point the upper byte         */
   (*z_jkjp1).kj[0] = ((z_upper & 0x20)<<1) | z_upper & 0x7F ;
                                        /* decode the upper byte        */

   z_lower = *(z_bufptr + 1);           /* point the lower byte         */
   (*z_jkjp1).kj[1] = z_lower;
                                        /* move the lower to JKJ        */
   z_newoff = z_bufptr + 2 - kcb.udeude;
                                        /* get new offset past kanji_hyk*/

   z_ret.newoff = z_newoff;             /* set new_offset               */
   z_ret.rc = SUCCESS;                  /* set normal return code       */
   return(z_ret);
}
