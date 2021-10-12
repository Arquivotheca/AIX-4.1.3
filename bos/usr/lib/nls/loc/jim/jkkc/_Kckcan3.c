static char sccsid[] = "@(#)24	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kckcan3.c, libKJI, bos411, 9428A410j 7/23/92 03:15:55";
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
 * MODULE NAME:       _Kckcan3
 *
 * DESCRIPTIVE NAME:  MAKE 3RD CANDIDATE OF KANSUJI CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x7fff (UERROR) :    unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
                                                                          
/*----------------------------------------------------------------------*
 *       COPYRIGHT
 *----------------------------------------------------------------------*/
#ifdef _AIX
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";
#endif

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
short  _Kckcan3( z_kcbptr, z_strpos, z_length )
struct   KCB   *z_kcbptr;
uschar          z_strpos;              /* pointer of 1st MCE           */
uschar	        z_length;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern   struct RETCGET   _Kccget();
   extern   short            _Kcjsjrt();

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcjte.h"   /* JTE  Code table Entry (JTE)                  */
#include   "_Kcjkj.h"   /* JKJ  Code table Entry (JKJ)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_HIN_DUMMY   127

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short            z_rc;
   short            z_rjsjrt;
   struct RETCGET   z_rcget;
   uschar           z_flag[2];
   struct JKJ       *z_jkjptr3;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address   to kcb   */

/*----------------------------------------------------------------------*
 *      GET & SET JKJs
 *----------------------------------------------------------------------*/
   CMOVL( kcb.jthchh, jteptr1 );

   for( jkjptr2 = jte.jkjaddr; ; CMOVF(kcb.jkhchh, jkjptr2 ) ) {
      /*----------------------  Get JKJ entries  -----------------------*/
      z_rcget = _Kccget(&kcb.jkhchh);
      if(z_rcget.rc == GET_EMPTY) {
         return( JKJOVER );
      }
      else {
         if((z_rcget.rc != GET_TOP_MID ) && (z_rcget.rc != GET_LAST))
            return( UERROR );
      }
      jkjptr1 = (struct JKJ *)z_rcget.cheptr;
      /*--------------------  Save 1st JKJ entry  ---------------------*/
      if ( jkjptr2 == jte.jkjaddr )
         z_jkjptr3 = jkjptr1;
      /*--------  Translate simple kansuji to complex kansuji  --------*/
      if((( jkj2.kj[0] | 0x80) == 0x88 ) && ( jkj2.kj[1] == 0xea )) {
                       jkj.kj[0] = 0x88; jkj.kj[1] = 0xeb; }
      else if((( jkj2.kj[0] | 0x80) == 0x93 ) && ( jkj2.kj[1] == 0xf1 )) {
                       jkj.kj[0] = 0x93; jkj.kj[1] = 0xf3; }
      else if((( jkj2.kj[0] | 0x80) == 0x8e ) && ( jkj2.kj[1] == 0x4f )) {
                       jkj.kj[0] = 0x8e; jkj.kj[1] = 0x51; }
      else if((( jkj2.kj[0] | 0x80) == 0x8f ) && ( jkj2.kj[1] == 0x5c )) {
                       jkj.kj[0] = 0x8f; jkj.kj[1] = 0x45; }
      else if((( jkj2.kj[0] | 0x80) == 0x96 ) && ( jkj2.kj[1] == 0x9c )) {
                       jkj.kj[0] = 0xe4; jkj.kj[1] = 0xdd; }
      else {
                       jkj.kj[0] = jkj2.kj[0]; jkj.kj[1] = jkj2.kj[1]; }

      /*-----------------------  LAST JKJ ENTRY  -----------------------*/
      if ( ( jkj2.kj[0] & 0x80 ) != 0x80 ) {
         jkj.kj[0] &= 0x7F;             /* last JKJ entry               */
         break;
      }
   }

/*----------------------------------------------------------------------*
 *      GET & SET JTE
 *----------------------------------------------------------------------*/

   z_flag [0] = NUM_FLAG0;
   z_flag [1] = NUM_FLAG1;
   z_rjsjrt = _Kcjsjrt( z_kcbptr,       /* pointer of KCB               */
                       NUM_HINSI,       /* hinshi                       */
                          z_flag,       /* dflag                        */
                        z_strpos,       /* mora position                */
                        z_length,       /* mora length                  */
                        NUM_TYPE,       /* type                         */
                       z_jkjptr3 );     /* pointer of first JKJ         */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rjsjrt );
}
