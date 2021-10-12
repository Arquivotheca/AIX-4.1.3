/* @(#)28	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/_Mlfrtc.t, libKJI, bos411, 9428A410j 6/6/91 14:18:26 */

/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         _Mlfrtc.t
 *
 * DESCRIPTIVE NAME:    Table of character class code.
 *
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Conversion table  a DBCS code
 *                                     to the character class code.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Table
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         NA.
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            #include _Mlfrtc.t
 *
 *  INPUT:              NA.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              NA.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      _Mlfrtc.c  conversion table double byte code to one bytes code.
 *               ( convert a DBCS code to character class code. )
 */

/*
 * The conversion table(1) for hiragana code and katakana code.
 *   ( convert DBCS code to character class code.)
 */

static  uchar   dbcstbl[2][M_TKNJN] =

{
/* dbcstbl[0] : 00 -- 0f ( DBCS CODE : 8140 -- 814f )          */
/* 0        1        2        3        4        5        6        7   */
M_HIRA  ,M_DOKU  ,M_KUTEN ,M_DOKU  ,M_KUTEN ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_KUTEN ,M_KUTEN ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : 10 -- 1f ( DBCS CODE : 8150 -- 815f )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_DELM  ,M_SYM4  ,

/* dbcstbl[0] : 20 -- 2f ( DBCS CODE : 8160 -- 816f )          */
M_DELM  ,M_SYM4  ,M_DELM  ,M_SYM4  ,M_SYM4  ,M_DELM  ,M_DELM  ,M_DELM  ,
M_DELM  ,M_LEFTP ,M_RIGHTP,M_LEFTP ,M_RIGHTP,M_LEFTP ,M_RIGHTP,M_LEFTP ,

/* dbcstbl[0] : 30 -- 3f ( DBCS CODE : 8170 -- 817f )          */
M_RIGHTP,M_LEFTP,M_RIGHTP,M_LEFTP ,M_RIGHTP,M_LEFTP ,M_RIGHTP,M_LEFTP,
M_RIGHTP,M_LEFTP,M_RIGHTP,M_SYM1  ,M_SYM1  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : 40 -- 4f ( DBCS CODE : 8180 -- 818f )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM2  ,

/* dbcstbl[0] : 50 -- 5f ( DBCS CODE : 8190 -- 819f )          */
M_SYM2  ,M_SYM4  ,M_SYM2  ,M_SYM3  ,M_SYM2  ,M_SYM4  ,M_SYM4  ,M_SYM2  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : 60 -- 6f ( DBCS CODE : 81a0 -- 81af )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : 70 -- 7f ( DBCS CODE : 81b0 -- 81bf )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : 80 -- 8f ( DBCS CODE : 81c0 -- 81cf )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : 90 -- 9f ( DBCS CODE : 81d0 -- 81df )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : a0 -- af ( DBCS CODE : 81e0 -- 81ef )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[0] : b0 -- bc ( DBCS CODE : 81f0 -- 81fc )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4,

/*
   the conversion table(2) for hiragana code and katakana cod.
       ( convert DBCS code to character class code.)
*/
/* dbcstbl[1] : 00 -- 0f ( DBCS CODE : 8240 -- 824f )          */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_NUM   ,

/* dbcstbl[1] : 10 -- 1f ( DBCS CODE : 8250 -- 825f )          */
M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,
M_NUM   ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[1] : 20 -- 2f ( DBCS CODE : 8260 -- 826f )          */
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,

/* dbcstbl[1] : 30 -- 3f ( DBCS CODE : 8270 -- 827f )          */
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* dbcstbl[1] : 40 -- 4f ( DBCS CODE : 8280 -- 828f )          */
M_SYM4  ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,

/* dbcstbl[1] : 50 -- 5f ( DBCS CODE : 8290 -- 829f )          */
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_HIRA  ,

/* dbcstbl[1] : 60 -- 6f ( DBCS CODE : 82a0 -- 82af )          */
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,

/* dbcstbl[1] : 70 -- 7f ( DBCS CODE : 82b0 -- 82bf )          */
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,

/* dbcstbl[1] : 80 -- 8f ( DBCS CODE : 82c0 -- 82cf )          */
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,

/* dbcstbl[1] : 90 -- 9f ( DBCS CODE : 82d0 -- 82df )          */
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,

/* dbcstbl[1] : a0 -- af ( DBCS CODE : 82e0 -- 82ef )          */
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,
M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,M_HIRA  ,

/* dbcstbl[1] : b0 -- bc ( DBCS CODE : 82f0 -- 82fc )          */
M_HIRA  ,M_HIRA  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4
};

/*
      the conversion table(3) for a byte code.
          ( convert a byte to character class code. )
*/

static  uchar   anktbl[C_HIBYTE] =

{
/* anktbl : 00 -- 0f   */
/* 0        1        2        3        4        5        6        7   */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* anktbl : 10 -- 1f   */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* anktbl : 20 -- 2f   */
M_SYM4  ,M_KUTEN ,M_DELM  ,M_SYM2  ,M_SYM2  ,M_SYM3  ,M_SYM4  ,M_DELM  ,
M_LEFTP ,M_RIGHTP,M_SYM4  ,M_SYM1  ,M_DOKU  ,M_SYM1  ,M_KUTEN ,M_DELM  ,

/* anktbl : 30 -- 3f   */
M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,M_NUM   ,
M_NUM   ,M_NUM   ,M_SYM4  ,M_SYM4  ,M_LEFTP ,M_SYM4  ,M_RIGHTP,M_KUTEN ,

/* anktbl : 40 -- 4f   */
M_SYM4  ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,

/* anktbl : 50 -- 5f   */
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_LEFTP ,M_SYM2  ,M_RIGHTP,M_SYM4  ,M_SYM4  ,

/* anktbl : 60 -- 6f   */
M_SYM4  ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,

/* anktbl : 70 -- 7f   */
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,M_ALPHA ,
M_ALPHA ,M_ALPHA ,M_ALPHA ,M_LEFTP ,M_SYM4  ,M_RIGHTP,M_SYM4  ,M_SYM4  ,

/* anktbl : 80 -- 8f   */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* anktbl : 90 -- 9f   */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* anktbl : a0 -- af   */
M_SYM4  ,M_KUTEN ,M_LEFTP ,M_RIGHTP,M_DOKU  ,M_SYM4  ,M_KANA  ,M_KANA  ,
M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,

/* anktbl : b0 -- bf   */
M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,
M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,

/* anktbl : c0 -- cf   */
M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,
M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,

/* anktbl : d0 -- df   */
M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,
M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,M_KANA  ,

/* anktbl : e0 -- ef   */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,

/* anktbl : f0 -- ff   */
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,M_SYM4  ,
};

