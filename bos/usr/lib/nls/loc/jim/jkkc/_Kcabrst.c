static char sccsid[] = "@(#)99	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcabrst.c, libKJI, bos411, 9428A410j 6/4/91 10:06:17";
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
 * MODULE NAME:       _Kcabrst
 *
 * DESCRIPTIVE NAME:  SET RYAKU-SHO SEISHO
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS) : success
 *                    0x7FFF(UERROR ) : Fatal error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short _Kcabrst(z_kcbptr,z_stoff,z_endoff)

struct  KCB   *z_kcbptr;
short          z_stoff;                 /* offset of start point        */
short          z_endoff;                /* offset of end   point        */
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short   z_buf;                       /* point of usr dict buffer     */
   short   z_len;                       /* length of seisho             */
   uschar *z_seibuf;                    /* point of seisho buffer       */

/*----------------------------------------------------------------------*
 *       START OF PROCESS                                                 
 *----------------------------------------------------------------------*/ 
   kcbptr1  =  z_kcbptr;
   seiptr1  =  kcb.seiaddr;
   z_seibuf = &sei.kj[0][0];             /* set initial seisho address  */
   z_len = 2;
                                         /* points seisho buffer        */
   for(z_buf=z_stoff;z_buf < z_endoff;z_buf++,z_seibuf++)
   {
      *z_seibuf = *(kcb.udeude + z_buf);
      z_len++;
      if ( z_len > 255)
      {
         return( UERROR );
      }
   }
   sei.ll[0] = z_len / 256 ;            /* set seisho length            */
   sei.ll[1] = z_len % 256 ;
   return( SUCCESS );
}
