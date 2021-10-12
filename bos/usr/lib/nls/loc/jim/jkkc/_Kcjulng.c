static char sccsid[] = "@(#)17	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjulng.c, libKJI, bos411, 9428A410j 6/4/91 12:56:52";
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
 * MODULE NAME:       _Kcjulng
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
short  _Kcjulng(z_kcbptr,
                z_hinpos,               /* z_morlen is removed from inp */
                z_flag, 
                z_morpos,               /* obtain the length of yomi in */
                z_type,                 /* buff from MSB bits           */
                z_jkjptr,
                z_yomipt,
                z_yomiln)

struct KCB *z_kcbptr;                   /* kcb pointer                  */
uschar z_hinpos;                        /* JLE hinsi code               */
uschar z_flag[];                        /* JLE dflag                    */
uschar z_morpos;                        /* JLE mora position            */
uschar z_type;                          /* JLE type                     */
struct JKJ *z_jkjptr;                   /* JKJ pointer                  */
short  z_yomipt;                        /* yomi position in buff        */
short  z_yomiln;                        /* yomi length                  */

{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
extern  struct RETCGET _Kccget();
extern  short          _Kcicmpt();
extern  short          _Kcicode();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcjle.h"
#include   "_Kcjkj.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_INVALID   0x00

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETCGET   z_rcget;            /* return code str for _Kccget  */
   short            z_i1;               /* work loop counter            */
   uschar           *z_ymioff;          /* work to point yomi in buff   */
   uschar           z_yomi;             /* curr 7bit code(work)         */
   uschar           z_mora;             /* mora code                    */
   uschar           z_moraym[MAXMCE];   /* mora-yomi area               */
   short            z_moraln;           /* valid mora-yomi length       */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address'ty to kcb  */

/*----------------------------------------------------------------------*
 *      OBTAIN NEW JTE ENTRY
 *----------------------------------------------------------------------*/
   z_rcget = _Kccget(&kcb.jlhchh);
   if(z_rcget.rc == GET_EMPTY)
   {
      return( JLEOVER );
   }
   else
   {
      if((z_rcget.rc != GET_TOP_MID )&&
         (z_rcget.rc != GET_LAST    ))
         return( UERROR );
   }

/*----------------------------------------------------------------------*
 *      CONVERT 7 BIT CODE TO MORA CODE 
 *----------------------------------------------------------------------*/
   jleptr1 = (struct JLE *)z_rcget.cheptr;
                                        /* move returned value to base  */
   z_mora = Z_INVALID;
   z_moraln = 0;
   z_ymioff =(uschar *)( kcb.udeude + z_yomipt);

   for ( z_i1 = 0; z_i1 < z_yomiln; z_i1++ )
   {                                    /* as long as yomi continue     */
      z_yomi = *(z_ymioff + z_i1);
 
      switch(z_yomi)
      {
         case Y_XYA:                    /* small ya                     */
         case Y_XYU:                    /* small yu                     */
         case Y_XYO:                    /* small yo                     */
            if ( z_moraln != 0 )
               z_mora = _Kcicmpt( z_moraym[z_moraln - 1], z_yomi);
 
            if (z_mora == Z_INVALID)
            {
               z_mora = _Kcicode(z_yomi);

               if ( z_mora == Z_INVALID )
                  return( UERROR );

               z_moraln++;
            }

            z_moraym[z_moraln - 1] = z_mora;
               break;

         case Y_DKT:                    /* dakuten                      */
         case Y_HDK:                    /* han-dakuten                  */
            if ( z_moraln != 0 )
               z_mora = _Kcicmpt( z_moraym[z_moraln - 1], z_yomi);

            if ( z_mora == Z_INVALID )
               return( UERROR );
 
            z_moraym[z_moraln - 1] = z_mora;
            break;

         case Y_ESC:                    /* escape 0x1d                  */
            if ( ( kcb.mode == MODALPHA ) || ( kcb.mode == MODALKAN ) )
            {
               z_moraym[z_moraln] = 0x00;/* move mora to mora-yomi area  */
               z_moraln ++;              /* count up mora-yomi length    */
            }
            else
               return( UERROR );
            break;

         default :
            z_mora = _Kcicode(z_yomi);  /* call handakuon process       */

            if ( z_mora == Z_INVALID )
                 return( UERROR );

            z_moraym[z_moraln] = z_mora;/* move mora to mora-yomi area  */
            z_moraln ++;                /* count up mora-yomi length    */
            break;
      }
   }                                    /* end of for loop              */

   if ( ( z_moraln <= 3 ) | ( z_moraln >= MAXMCE ) )
      return( UERROR );

/*----------------------------------------------------------------------*
 *      MOVE LONG-WORD ENTRY
 *----------------------------------------------------------------------*/
   jle.hinpos = z_hinpos;
   jle.dflag[0] = z_flag[0];
   jle.dflag[1] = z_flag[1];
   jle.stap      = z_morpos;
   jle.dtype     = z_type;
   jle.jkjaddr   = z_jkjptr;
 
   jle.len       = z_moraln;            /* set yomi length              */

                                        /* set mora_yomi in long_word   */
                                        /* entry                        */
                                        /* move from the 4th yomi in    */
                                        /* buff until the end           */

   for(z_i1=0;z_i1 <= (z_moraln - 3);z_i1 ++)
   {
                                        /* skip 1st 3bytes,start        */
                                        /* from the 4th yomi and move   */
      jle.yomi[z_i1] = z_moraym[z_i1 + 3];
   }

   return( SUCCESS );
}
