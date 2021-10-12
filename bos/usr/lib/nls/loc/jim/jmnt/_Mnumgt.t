/* @(#)33	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/_Mnumgt.t, libKJI, bos411, 9428A410j 6/6/91 14:19:24 */

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


/********************* START OF MODULE SPECIFICATIONS *********************
 *
 * MODULE NAME:         _Mnumgt.t
 *
 * DESCRIPTIVE NAME:    Get a number code from the kana code
 *                                   that it share the same key.
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
 * FUNCTION:            The conversion betueen the PC code
 *                                  to the number letter of PC kanji code.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Table
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA. ( Include in module kmnumget() )
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         NA.
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            #include _Mnumgt.t
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
 *      _Mnumgt.c  conversion table
 *                 single bytes code to double bytes number code.
 */

/* the conversion table for number code. */
static  uchar   t0[16][2] =
{
/* input code : 0x30 -- 0x3f */
{0x82,0x4f},{0x82,0x50},{0x82,0x51},{0x82,0x52}, /*   0 ,  1 ,  2 ,  3 , */
{0x82,0x53},{0x82,0x54},{0x82,0x55},{0x82,0x56}, /*   4 ,  5 ,  6 ,  7 , */
{0x82,0x57},{0x82,0x58},{0xff,0xff},{0xff,0xff}, /*   8 ,  9 , -- , -- , */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}  /*  -- , -- , -- , --   */
};

/* the conversion table for katakana sift code. */
static  uchar   t1[16*3][2] =
{
/* input code : 0xb0 -- 0xbf */
{0xff,0xff},{0x82,0x52},{0xff,0xff},{0x82,0x53}, /*  -- ,  a , -- ,  u , */
{0x82,0x54},{0x82,0x55},{0xff,0xff},{0xff,0xff}, /*   e ,  o , -- , -- , */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}, /*  -- , -- , -- , -- , */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}, /*  -- , -- , -- , -- , */

/* input code : 0xc0 -- 0xcf */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}, /*  -- , -- , -- , -- , */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0x82,0x50}, /*  -- , -- , -- , nu , */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}, /*  -- , -- , -- , -- , */
{0x82,0x51},{0xff,0xff},{0xff,0xff},{0xff,0xff}, /*  hu , -- , -- , -- , */

/* input code : 0xd0 -- 0xdf */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}, /*  -- , -- , -- , -- , */
{0x82,0x56},{0x82,0x57},{0x82,0x58},{0xff,0xff}, /*  ya , yu , yo , -- , */
{0xff,0xff},{0xff,0xff},{0xff,0xff},{0xff,0xff}, /*  -- , -- , -- , -- , */
{0x82,0x4f},{0xff,0xff},{0xff,0xff},{0xff,0xff}  /*  wa , -- , -- , --   */
};

