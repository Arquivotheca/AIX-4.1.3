static char sccsid[] = "@(#)23	1.2.1.2  src/bos/usr/lib/nls/loc/jim/jkkc/_Kckcan2.c, libKJI, bos411, 9428A410j 5/28/93 08:05:56";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kckcan2
 *
 * DESCRIPTIVE NAME:  MAKE 2ND CANDIDATE OF KANSUJI CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x7fff (UERROR) : unpredictable error
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
struct KJTBL
{
    unsigned char kj[2];
};

struct U_KJTBL
{
    unsigned char len;
    unsigned char kj[8];
};

static struct KJTBL kjn1[10] = {
/* 0-4 */	{0x81,0x5a},{0x88,0xea},{0x93,0xf1},{0x8e,0x4f},{0x8e,0x6c},
/* 5-9 */	{0x8c,0xdc},{0x98,0x5a},{0x8e,0xb5},{0x94,0xaa},{0x8b,0xe3}
};

static  struct KJTBL kjun[6] = {{0x81,0x40},    /* dummy                */      
                                {0x96,0x9c},    /* MAN  (10,000)        */      
                                {0x89,0xad},    /* OKU (100.000)        */
                                {0x92,0x9b},    /* CYO                  */
                                {0x8b,0x9e},    /* KYO                  */
                                {0x9a,0xb4}};   /* GAI                  */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short  _Kckcan2(z_kcbptr, z_strpos, z_length)
struct  KCB  *z_kcbptr;
uschar        z_strpos;                 /* pointer of 1st MCE           */
uschar        z_length;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern   struct RETCGET   _Kccget();
   extern   short            _Kcjsjrt();
   extern   short            _Kckunit();

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjkj.h"   /* JKJ  Code table Entry (JKJ)                  */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_END        0x01ff
#define   Z_CONTINUE   0x02ff
#define   Z_HIN_DUMMY  127

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short            z_i;                /* counter                      */
   short            z_j;                /* counter                      */
   short            z_topflg;           /* flag for JKJ                 */
   short            z_rkunit;           /* return code from _Kckunit()  */
   short            z_rjsjrt;           /* return code from _Kcjsjrt()  */
   struct RETCGET   z_rcget;            /* return code from _Kccget()   */
   short            z_unit;
   short            z_rem;
   struct KJTBL     z_outbuf[7];
   uschar           z_flag[2];

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address   to kcb   */

/*----------------------------------------------------------------------*
 *      DEVIDE YCEs BY 4
 *----------------------------------------------------------------------*/
   z_unit = (  z_length  - 1 ) / 4;
   z_rem  = (( z_length  - 1 ) % 4 ) + 1;
   if ( z_unit >= 6 )
      return( UERROR );

/*----------------------------------------------------------------------*
 *      GET & SET JKJs
 *----------------------------------------------------------------------*/
   jkjptr1 = NULL;
   z_topflg = ON;

   for( z_i = z_unit, mceptr1 = kcb.mchmce + z_strpos;
	z_i >= 0; z_i--, mceptr1 += z_rem, z_rem = 4 ) {
      /*---------------  Translate 4 digits to 1 number  ---------------*/
      z_rkunit = _Kckunit( mceptr1, z_outbuf, z_rem ); 
  
      /*-------------------  Set the number in JKJ  --------------------*/
      for ( z_j = 0; z_j < z_rkunit; z_j++ ) {
         z_rcget = _Kccget(&kcb.jkhchh);
         if(z_rcget.rc == GET_EMPTY) {
            return( JKJOVER );
         }
         else {
            if((z_rcget.rc != GET_TOP_MID ) && (z_rcget.rc != GET_LAST ))
               return( UERROR );
         }
         jkjptr1 = (struct JKJ *)z_rcget.cheptr;
         if ( ( z_topflg == ON ) && ( z_j == 0 ) ) {
            jkjptr2 = jkjptr1;
            z_topflg = OFF;
         }
         jkj.kj[0] = z_outbuf[z_j].kj[0];
         jkj.kj[1] = z_outbuf[z_j].kj[1];
      }
      /*--------------------  Set the unit in JKJ  ---------------------*/
      if ( ( z_rkunit != 0 ) && ( z_i != 0 ) ) {
         z_rcget = _Kccget(&kcb.jkhchh);
         if(z_rcget.rc == GET_EMPTY) {
            return( JKJOVER );
         }
         else {
            if((z_rcget.rc != GET_TOP_MID ) && (z_rcget.rc != GET_LAST))
               return( UERROR );
         }
         jkjptr1 = (struct JKJ *)z_rcget.cheptr;
         jkj.kj[0] = kjun[z_i].kj[0];
         jkj.kj[1] = kjun[z_i].kj[1];
      }
   }

   if ( jkjptr1 == NULL ) {
      z_rcget = _Kccget(&kcb.jkhchh);
      if(z_rcget.rc == GET_EMPTY) {
         return( JKJOVER );
      }
      else {
         if((z_rcget.rc != GET_TOP_MID ) && (z_rcget.rc != GET_LAST))
            return( UERROR );
      }
      jkjptr2 = (struct JKJ *)z_rcget.cheptr;
      jkj2.kj[0] = kjn1[0].kj[0] & 0x7f;
      jkj2.kj[1] = kjn1[0].kj[1];
   }
   else
      jkj.kj[0] &= 0x7F;                /* last JKJ entry               */
   
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
   return( z_rjsjrt );
}
