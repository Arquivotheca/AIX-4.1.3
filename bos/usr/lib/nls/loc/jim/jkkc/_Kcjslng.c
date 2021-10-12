static char sccsid[] = "@(#)06	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjslng.c, libKJI, bos411, 9428A410j 6/4/91 12:55:12";
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
 * MODULE NAME:       _Kcjslng
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
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
short  _Kcjslng(z_kcbptr,
                z_hinpos,               /* z_morlen is removed from inp */
                z_flag,
                z_morpos,               /* obtain the length of yomi in */
                z_type,                 /* buff from MSB bits           */
                z_jkjptr,
                z_yomipt)

struct KCB *z_kcbptr;                   /* kcb pointer                  */
uschar z_hinpos;                        /* JLE hinsi code               */
uschar z_flag[];                        /* JLE dflag                    */
uschar z_morpos;                        /* JLE mora position            */
uschar z_type;                          /* JLE type                     */
struct JKJ *z_jkjptr;                   /* JKJ pointer                  */
short  z_yomipt;                        /* yomi position in buff        */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern  struct RETCGET _Kccget();

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcjle.h"
#include   "_Kcjkj.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETCGET   z_retcget;          /* return code str for _Kccget  */
   short   z_i1;                        /* work loop counter            */
   uschar  *z_ymioff;                   /* work to point yomi in buff   */
   short   z_c1;                        /* work counter                 */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;

/*----------------------------------------------------------------------*
 *      GET NEW JLE ENTRY
 *----------------------------------------------------------------------*/
   z_retcget = _Kccget(&kcb.jlhchh);

   if ( z_retcget.rc == GET_EMPTY )
      return( JLEOVER );
   else if ( ( z_retcget.rc != GET_TOP_MID )
          && ( z_retcget.rc != GET_LAST ) )
      return( UERROR );

/*----------------------------------------------------------------------*
 *      MOVE INTO LONG-WORD ENTRY
 *----------------------------------------------------------------------*/
   jleptr1 = (struct JLE *)z_retcget.cheptr;
   jle.hinpos = z_hinpos;
   jle.dflag[0] = z_flag[0];
   jle.dflag[1] = z_flag[1];
   jle.stap      = z_morpos;
   jle.dtype     = z_type;
   jle.jkjaddr   = z_jkjptr;
                                        /* determine the length of yomi */
                                        /* from MSB bits                */
                                        /* starts MSB OFF               */
                                        /* ends MSB ON                  */
   z_ymioff = kcb.sdesde + z_yomipt;
   for ( z_c1 = 0; ( *(z_ymioff + z_c1) & 0x80 ) == 0; z_c1++ )
   {
      if(z_c1 >= 12)                    /* detect error if the length   */
      {                                 /* exceeds 12 bytes             */
         return( UERROR );
      }
   }

   jle.len       = z_c1 + 1 + 2;        /* set original yomi length     */
                                        /* set mora_yomi in long_word   */
                                        /* entry                        */
                                        /* move from the 2nd yomi in    */
                                        /* buff until the end           */
   for(z_i1=0;z_i1 <= (z_c1 -1);z_i1 ++)
   {
                                        /* skip the 1st yomi ,start     */
                                        /* from the 2nd yomi and move   */
      jle.yomi[z_i1] = z_ymioff[z_i1+1];
   }

   jle.yomi[z_i1-1] = jle.yomi[z_i1-1] & 0x7f;

   return( SUCCESS );
}
