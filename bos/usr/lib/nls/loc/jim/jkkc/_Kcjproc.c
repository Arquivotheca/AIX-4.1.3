static char sccsid[] = "@(#)99	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjproc.c, libKJI, bos411, 9428A410j 6/4/91 12:54:24";
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
 * MODULE NAME:       _Kcjproc
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE overflow
 *                    0x1204 (JKJOVER): JKJ overflow
 *                    0x1304 (JLEOVER): JLE overflow
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
short _Kcjproc(z_kcbptr,z_dctctl,z_mode)

struct KCB      *z_kcbptr;              /* get address of KCB           */
short  z_dctctl;                        /* dict. looking up cntrl info. */
short  z_mode;                          /* dict. looking up mode        */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETJCHEK _Kcjchek();   /* jiritsu-go check routine     */
   extern short           _Kcjdict();   /* jiritsu-go dictionary process*/

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_NOTLUP   0x00ff             /* don't proceed to dict lookup */
#define   Z_LUP      0x01ff             /* proceed to dict lookup       */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short               z_rc1;           /* declare retern code of jdict */
   uschar              z_stpos;         /* declare start                */
   struct   RETJCHEK   z_rjchek;        /* declare retern of jchek      */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */
   mceptr1 = kcb.mchmce;                /* set base address of MCE      */

/*----------------------------------------------------------------------*
 *      CHECK THE TAFGET OF LOOKING UP
 *----------------------------------------------------------------------*/
   z_rjchek = _Kcjchek(z_kcbptr,z_dctctl,z_mode);
                                        /* check looking up target      */
   if ( z_rjchek.rc == Z_NOTLUP ) 
      return( SUCCESS );
   else if ( z_rjchek.rc != Z_LUP ) 
      return( z_rjchek.rc );

/*----------------------------------------------------------------------*
 *      GET CANDIDATES 
 *----------------------------------------------------------------------*/
   for ( z_stpos=0; mce.jdok != 0; z_stpos++, mceptr1++ )
                                        /* loop all target              */
   {
      switch(mce.jdok)                  /* branch off by jdok           */
      {
         case JD_COMPLETE: break;       /* if look up complete          */

         case JD_LONG:                  /* if same mora in front        */
         case JD_READY:                 /* if target of looking up      */
            z_rc1 = _Kcjdict(z_kcbptr,z_stpos,(uschar)z_rjchek.endmora);
                                        /* call jiritsu-go dict. look up*/
            if ( z_rc1 != SUCCESS )
               return(z_rc1);
            break;

         default: return(UERROR);       /* something is wrong           */
      }
   }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
