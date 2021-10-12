static char sccsid[] = "@(#)22	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kckcan1.c, libKJI, bos411, 9428A410j 7/23/92 03:15:47";
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
 * MODULE NAME:       _Kckcan1
 *
 * DESCRIPTIVE NAME:  MAKE 1ST CANDIDATE OF KANSUJI CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS):    success
 *                    0x1104 (JTEOVER):    JTE table overflow
 *                    0x1204 (JKJOVER):    JKJ table overflow
 *                    0x01ff (Z_END):      either piriod or comma exists
 *                    0x02ff (Z_CONTINUE): neither piriod nor comma exists
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

/*----------------------------------------------------------------------*
 *      STATIC TABLES
 *----------------------------------------------------------------------*/
struct KJTBL {
    unsigned char kj[2];
};

static struct KJTBL kjn1[10] = {
/* 0-4 */	{0x81,0x5a},{0x88,0xea},{0x93,0xf1},{0x8e,0x4f},{0x8e,0x6c},
/* 5-9 */	{0x8c,0xdc},{0x98,0x5a},{0x8e,0xb5},{0x94,0xaa},{0x8b,0xe3}
};

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short  _Kckcan1(z_kcbptr, z_strpos, z_length)
struct  KCB  *z_kcbptr;
uschar        z_strpos;                 /* pointer of 1st MCE           */
uschar	      z_length;
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
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcjkj.h"   /* JKJ  Code table Entry (JKJ)                  */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)  		*/

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_END        0x01ff
#define   Z_CONTINUE   0x02ff
#define   Z_HIN_DUMMY  127

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short            z_i;
   short            z_rc;
   short            z_rjsjrt;
   struct RETCGET   z_rcget;
   short            z_endflg;
   uschar           z_flag[2];
   struct JKJ       *z_jkjtmp;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address   to kcb   */

/*----------------------------------------------------------------------*
 *      INITIALIZE
 *----------------------------------------------------------------------*/
   z_rc = Z_CONTINUE;

/*----------------------------------------------------------------------*
 *      GET & SET JKJ
 *----------------------------------------------------------------------*/
   for( z_i = 0, mceptr1 = kcb.mchmce + z_strpos ; 
	z_i < z_length; z_i++, mceptr1++ ) {
      /*----------------------  GET JKJ ENTRY  -------------------------*/
      z_rcget = _Kccget(&kcb.jkhchh);
      if(z_rcget.rc == GET_EMPTY) {
         return( JKJOVER );
      }
      else {
         if((z_rcget.rc != GET_TOP_MID )&&
            (z_rcget.rc != GET_LAST    ))
            return( UERROR );
      }

      jkjptr1 = (struct JKJ *)z_rcget.cheptr;

      if ( z_i == 0 )                   /* save 1st pointer of JKJ      */
         jkjptr2 = jkjptr1;

      /*----------------------  SET JKJ  -------------------------------*/
      switch( mce.code ) {
         case M_APER :                  /* PERIOD                       */
            jkj.kj[0] = PERIOD_H_PCCODE;
            jkj.kj[1] = PERIOD_L_PCCODE;
            z_rc = Z_END;
            break;

         case M_ACOM :                  /* COMMA                        */
            jkj.kj[0] = COMMA_H_PCCODE;
            jkj.kj[1] = COMMA_L_PCCODE;
            z_rc = Z_END;
            break;

         case NUM_MORA_0 :              /*     0  (0xcf)                */
         case NUM_MORA_1 :              /*     1  (0xd0)                */
         case NUM_MORA_2 :              /*     2  (0xd1)                */
         case NUM_MORA_3 :              /*     3  (0xd2)                */
         case NUM_MORA_4 :              /*     4  (0xd3)                */
         case NUM_MORA_5 :              /*     5  (0xd4)                */
         case NUM_MORA_6 :              /*     6  (0xd5)                */
         case NUM_MORA_7 :              /*     7  (0xd6)                */
         case NUM_MORA_8 :              /*     8  (0xd7)                */
         case NUM_MORA_9 :              /*     9  (0xd8)                */
            jkj.kj[0] = kjn1[mce.code-NUM_MORA_0].kj[0];
            jkj.kj[1] = kjn1[mce.code-NUM_MORA_0].kj[1];
            break;
         default:                       /* INVALID CODE                 */
            z_rc = Z_END;
            FOR_BWD_MID( kcb.jkhchh, jkjptr1, z_endflg) {
               z_jkjtmp = jkjptr1;
               CMOVF(kcb.jkhchh,jkjptr1);
               _Kccfree( &kcb.jkhchh, z_jkjtmp );
               if ( z_jkjtmp == jkjptr2 )
                  z_endflg = ON;
            }
            return( z_rc );
      }
   }
   jkj.kj[0] &= 0x7F;                   /* last JKJ entry               */

/*----------------------------------------------------------------------*
 *      GET & SET JTE
 *----------------------------------------------------------------------*/

   z_flag[0] = NUM_FLAG0;
   z_flag[1] = NUM_FLAG1;

   z_rjsjrt = _Kcjsjrt( z_kcbptr,               /* pointer of KCB       */
                       NUM_HINSI,               /* hinshi               */
                          z_flag,               /* dflag                */
                        z_strpos,               /* mora position        */
                        z_length,               /* mora length          */
                        NUM_TYPE,               /* type                 */
                        jkjptr2 );              /* pointer of first JKJ */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   if ( z_rjsjrt  == SUCCESS )
      return( z_rc );
   else
      return( z_rjsjrt );               /* return error code            */
}
