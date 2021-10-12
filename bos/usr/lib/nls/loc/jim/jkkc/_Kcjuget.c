static char sccsid[] = "@(#)13	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjuget.c, libKJI, bos411, 9428A410j 6/4/91 16:55:39";
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
 * MODULE NAME:       _Kcjuget
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
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
short _Kcjuget(z_kcbptr,z_tbtype,z_strd,z_endd,
                                       z_strpos,z_length,z_stry,z_ylen)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
short         z_tbtype;                 /* type of table                */
short         z_strd;                   /* offset of top of data buf    */
short         z_endd;                   /* offset of last of data buf   */
unsigned char z_strpos;                 /* offset of 1st MCE            */
unsigned char z_length;                 /* length of MCE(yomi data)     */
short         z_stry;                   /* offset of mora code in buf   */
short         z_ylen;                   /* offset of mora code in buf   */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETJUGT1 _Kcjugt1();   /* Look up about One Entry      */
   extern short           _Kcabrst();   /* Set RYAKU-SHO SEISHO         */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETJUGT1 z_rjugt1;    /* Define Area for Return of _Kcjugt1   */
   short           z_rabrst;    /* Define Area for Return of _Kcabrst   */

/*----------------------------------------------------------------------*
 *      SET JIRITSU-GO OR LONG WORD TABLE
 *----------------------------------------------------------------------*/
   kcbptr1  = z_kcbptr;

   z_rjugt1.newoff = z_strd + (short)1;    /* set initialization offset    */

   if(kcb.mode != MODRYAKU)
   {
      while ( z_rjugt1.newoff < z_endd )
      {
         z_rjugt1
            = _Kcjugt1(z_kcbptr,z_tbtype,z_rjugt1.newoff,
                                z_strpos,z_length,z_stry,z_ylen);
         if ( z_rjugt1.rc  != SUCCESS )
            return( z_rjugt1.rc );
      }
   }
   else
   {
      z_rabrst = _Kcabrst(z_kcbptr,z_rjugt1.newoff,z_endd);
                                        /* set seisho of ryaku-sho      */
      if ( z_rabrst != SUCCESS )
      {
         return(z_rabrst);
      }
   }
   return( SUCCESS );
}
