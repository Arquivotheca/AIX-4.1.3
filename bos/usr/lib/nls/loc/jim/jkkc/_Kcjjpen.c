static char sccsid[] = "@(#)93	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjjpen.c, libKJI, bos411, 9428A410j 6/4/91 12:53:25";
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
 * MODULE NAME:       _Kcjjpen
 *
 * DESCRIPTIVE NAME:  CALCULATE JIRITSU-GO PENALTY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       jiritsu-go penalty
 *                    0x7fff(UERROR): unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/*----------------------------------------------------------------------* 
 *      INCLUDE TABLES                                                    
 *----------------------------------------------------------------------*/
#include   "_Kcjpn.t"                   /* jiritsu-go penalty table     */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kcjjpen( z_jteptr, z_hinsi )

struct JTE      *z_jteptr;              /* get address of JTE           */ 
short    z_hinsi;                       /* hinsi code                   */
{ 
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcjte.h"
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short      z_jpnlty;                 /* define jiritsugo penalty area*/
   short      z_jfreq;                  /* define jiritsuge freq area   */
   short      z_jtp;                    /* define jiritsugo dtype area  */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   jteptr1 = z_jteptr;                  /* set base address of JTE      */ 

/*----------------------------------------------------------------------*
 *       CALCULATE JIRITSUGO PENALTY BY TYPE & FREQUENCY
 *----------------------------------------------------------------------*/
   z_jfreq = jte.dflag[0] & 0x07;       /* pick up frequency from dflag */

   switch( jte.dtype )                  /* branch by type               */
   {
      case TP_IPPAN:                    /* general word                 */
      case TP_SETTO:                    /* prefix                       */
      case TP_OSETTO:                   /* prefix 'o'                   */
      case TP_GSETTO:                   /* prefix 'go'                  */
      case TP_SETSUBI:                  /* suffix                       */
      case TP_SUUCHI:                   /* numeric                      */
      case TP_NSETTO:                   /* prefix for numeric           */
      case TP_JOSUSHI:                  /* suffix for numeric           */
      case TP_KOYUU:                    /* proper noun                  */
      case TP_PSETTO:                   /* prefix for proper noun       */
      case TP_PSETSUBI:                 /* suffix for proper noun       */
      case TP_TOUTEN:                   /* touten                       */
      case TP_KUTEN:                    /* kuten                        */
      case TP_CONVKEY:                  /* conversion key               */
         z_jtp = jte.dtype;
         z_jpnlty = (short)jpnlty[z_jtp][z_jfreq];
         break;

      case TP_USERDCT:                  /* words in user-dictionary     */
         z_jtp = TP_IPPAN;
         z_jpnlty = (short)jpnlty[z_jtp][z_jfreq];
         break;

      default: 
         return((short)UERROR);
   }                                    /* switch(jte.dflag)            */ 

/*----------------------------------------------------------------------*
 *       PENALTY ADJUSTMENT FOR WORDS OF SPECIAL DFLAG 
 *----------------------------------------------------------------------*/
   if( ( z_jtp == TP_IPPAN ) && ( ( jte.dflag[1] & PRE_GOKI_REQ ) != 0 ) )
      z_jpnlty += 10;

   if ( ( (z_jtp == TP_SETTO) || (z_jtp == TP_SETSUBI) )
     && ( (jte.dflag[1] & (SAHEN_REQ | KEIDOU_REQ | MEISHI_REQ)) != 0))
      z_jpnlty += 3;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return(z_jpnlty);  
}
