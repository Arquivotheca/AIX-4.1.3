static char sccsid[] = "@(#)04	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjsjrt.c, libKJI, bos411, 9428A410j 7/23/92 03:14:39";
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
 * MODULE NAME:       _Kcjsjrt
 *
 * DESCRIPTIVE NAME:  CREATING JIRITSU-GO TABLE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
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
short  _Kcjsjrt(z_kcbptr,z_hinpos,z_flag,
                z_morap,z_moraln,z_type,z_kjptr)

struct  KCB     *z_kcbptr;              /* get address of KCB           */
uschar  z_hinpos;                       /* set jrt-go hinshi            */
uschar  z_flag[];                       /* set jrt-go flag              */
uschar  z_morap;                        /* set jrt-go start mora pointer*/
uschar  z_moraln;                       /* set jrt-go mora length       */
uschar  z_type;                         /* set jrt-go type              */
struct  JKJ     *z_kjptr;               /* set jrt-go kanji pointer     */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCGET  _Kccget();
   extern short           _Kccswpj();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcjte.h"
#include   "_Kcmce.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETCGET  z_rccget;            /* define ret. code area of cget*/
   short           z_rcswpj;            /* define ret. code area ofcswpj*/

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */

   z_rccget = _Kccget(&kcb.jthchh);     /* get one of chain element     */

   if ( z_rccget.rc == GET_EMPTY )
   {
       z_rcswpj = _Kccswpj(kcbptr1,(short)(z_morap+z_moraln-1));
       if ( z_rcswpj == UERROR )
          return( UERROR );

       z_rccget = _Kccget(&kcb.jthchh); /* retry                */
       if ( z_rccget.rc == GET_EMPTY )
          return( JTEOVER );            /* jiretsu-go table over flow   */
       else if ( ( z_rccget.rc != GET_TOP_MID ) 
              && ( z_rccget.rc != GET_LAST ) )
          return( UERROR );
   }
   else if ( ( z_rccget.rc != GET_TOP_MID ) && ( z_rccget.rc != GET_LAST ) )
      return( UERROR );

   jteptr1 = (struct JTE *)z_rccget.cheptr;
                                        /* set jiritsugo element pointer*/
   jte.hinpos   = z_hinpos  ;           /* set jrt-go hin. pos table tag*/
   jte.dflag[0] = z_flag[0];            /* set jrt-go flag              */
   jte.dflag[1] = z_flag[1];            /* set jrt-go flag              */
   jte.stap     = z_morap;              /* set jrt-go start mora pointer*/
   jte.len      = z_moraln;             /* set jrt-go mora length       */
   jte.dtype    = z_type;               /* set jrt-go type              */
   jte.jkjaddr  = z_kjptr;              /* set jrt-go kanji pointer     */
   jte.usage    = 0;                    /* clear jrt-go usage           */
   mceptr1 = kcb.mchmce + z_morap;
   if(mce.maxln < z_moraln)
      mce.maxln = z_moraln;
#ifdef DBG_MSG
printf(" hinpo=%2x  flag[0][1]=%2x,%2x  moralen=%2x  type=%2x\n",
	 z_hinpos, z_flag[0], z_flag[1], z_moraln, z_type );
#endif
   return( SUCCESS );
}
