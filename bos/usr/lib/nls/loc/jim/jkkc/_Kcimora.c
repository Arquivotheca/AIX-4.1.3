static char sccsid[] = "@(#)86	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcimora.c, libKJI, bos411, 9428A410j 6/4/91 12:52:12";
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
 * MODULE NAME:       _Kcimora
 *
 * DESCRIPTIVE NAME:  TRANSLATE FROM 7 BIT CODE TO THE MORA CODE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS): success
 *                    0x7fff(UERROR):  unpredictable error
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
short   _Kcimora( z_kcbptr, z_state )

struct  KCB     *z_kcbptr;              /* pointer of KCB               */
short   z_state;                        /* states of changes            */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern  uschar  _Kcicmpt();          /* change daku-on code          */
   extern  uschar  _Kcicode();          /* change ordinary code         */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcymi.h"
#include   "_Kcyce.h"
#include   "_Kcype.h"
#include   "_Kcmce.h"
#include   "_Kcgpw.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_ADD1   0x01ff
#define   Z_PART   0x02ff

#define   Z_XYA    0x61
#define   Z_XYU    0x63
#define   Z_XYO    0x65
#define   Z_DAK    0x72
#define   Z_HDK    0x73

#define   Z_INVALID  0x00

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_count;                /* counter                      */
   short        z_endlp;                /* loop-end                     */
   short        z_yminm;                /* Number of yomi.              */
   uschar       z_code;                 /* mora code save area          */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* initialize kcbptr1           */
   gpwptr1 = kcb.gpwgpe;                /* initialize gpwptr1           */
   ymiptr1 = kcb.ymiaddr;               /* initialize ymiptr1           */

/*----------------------------------------------------------------------*
 *      SET VALUE BY CASE OF Z_STATE   
 *----------------------------------------------------------------------*/
   switch( z_state )
   {
      case Z_ADD1: yceptr1 = kcb.ychyce + kcb.ychacyce;
                   ypeptr1 = kcb.yphype + kcb.yphacype;
                   mceptr1 = kcb.mchmce + kcb.mchacmce;
                   mceptr2 = mceptr1 - 1;
                                        /* add one character            */
                   z_count = kcb.ymill1 - 1;
                                        /* new.ymill1 = old.ymill1 + 1  */
                   z_endlp = kcb.ymill1;
                   break;

      case Z_PART: mceptr1 = kcb.mchmce + ( kcb.mchacmce - 1 );
                   mceptr2 = mceptr1 - 1;
                   if( kcb.mchacmce > 1)
                      z_yminm = mce.yceaddr - mce2.yceaddr;
                   else
                      z_yminm = mce.yceaddr - kcb.ychyce + 1;
                   ypeptr1 = kcb.yphype + ( kcb.yphacype - z_yminm );
                   yceptr1 = kcb.ychyce + ( kcb.ychacyce - z_yminm );
                   kcb.mchacmce-- ;      /* change last one character   */
                   kcb.ychacyce-=z_yminm;/* new.ymill1 = old.ymill1 !!  */
                   kcb.yphacype-=z_yminm;
                   z_count = kcb.ymill1 - z_yminm;
                   z_endlp = kcb.ymill1;
                   break;

      default:                          /* purge all information        */
                   yceptr1 = kcb.ychyce ; /* initialize yceptr1         */
                   ypeptr1 = kcb.yphype ; /* initialize ypeptr1         */
                   mceptr1 = kcb.mchmce ; /* initialize mceptr1         */
                   mceptr2 = kcb.mchmce ; /* initialize mceptr2         */
                   kcb.mchacmce = 0;      /* initialize No. of MCEs     */
                   kcb.ychacyce = 0;      /* initialize No. of YCEs     */
                   kcb.yphacype = 0;      /* initialize No. of YPEs     */
                   z_count = 0;           /* set counter                */
                   z_endlp = kcb.ymill1;
                   break;
   }

/*----------------------------------------------------------------------*
 *      CHECK THE INPUT CODE
 *----------------------------------------------------------------------*/
   do
   {
      switch( ymi.yomi[ z_count ] )
      {
         case Z_XYA:
         case Z_XYU:
         case Z_XYO:
                     if ( z_count != 0 )
                        z_code = _Kcicmpt( mce2.code, ymi.yomi[z_count] );
                     else
                        z_code = Z_INVALID;

                     switch( z_code )
                     {
                        case Z_INVALID:
                                mce.code = _Kcicode( ymi.yomi[z_count] );
                                if ( mce.code == Z_INVALID )
                                   return( UERROR );
                                kcb.mchacmce++;
                                break;
                        default:        /* success                      */
                                mce2.code = z_code;
                                mceptr1--;/* set previous MCE structure */
                                break;
                     }
                     break;

         case Z_DAK: 
         case Z_HDK: 
                     if ( z_count == 0 )
                        return( UERROR );

                     mce2.code = _Kcicmpt( mce2.code, ymi.yomi[z_count] );

                     if ( mce2.code == Z_INVALID )
                        return( UERROR );

                     mceptr1--;         /* set previous MCE structure   */
                     break;

         default   : mce.code = _Kcicode( ymi.yomi[ z_count ] );
                     if ( mce.code == Z_INVALID )
                        return( UERROR );
                     kcb.mchacmce++;
                     break;             /* check handaku-on             */
      }                                 /* set ordinary mora code       */

/*----------------------------------------------------------------------*
 *      SET YPE AND YPZ   
 *----------------------------------------------------------------------*/
      z_count++;

      if( z_count <= gpw.accfirm )
      {
         ype.prev = ymi.yomi[ z_count-1 ];
         ypeptr1++;
         kcb.yphacype++;
      }
      else
      {
         ype.prev = ymi.yomi[ z_count-1 ];
         yce.yomi = ymi.yomi[ z_count-1 ];
         ypeptr1++;
         kcb.yphacype++;
         mce.yceaddr = yceptr1;
         yceptr1++;
         kcb.ychacyce++;
      }

/*----------------------------------------------------------------------*
 *      IF ERROR OCCURRED, RETRY PROCESSING
 *----------------------------------------------------------------------*/
      if(( mce.code == 0x00 )&&( kcb.ymill2 > 0 )&&( z_endlp < kcb.ymill2 ))
      {
         yceptr1 = kcb.ychyce ;
         ypeptr1 = kcb.yphype ;
         mceptr1 = kcb.mchmce ;

         kcb.mchacmce = 0;
         z_count = 0;
         z_endlp = kcb.ymill2;          /* set loop-end No.             */
      }

      mceptr2 = mceptr1;                /* set previous MCE pointer     */
      mceptr1++;                        /* set next MCE structure       */
   }
   while( z_count <  z_endlp );         /* check loop-end               */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
