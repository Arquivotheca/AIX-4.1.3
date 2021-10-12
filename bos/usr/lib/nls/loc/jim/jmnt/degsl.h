/* @(#)61	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/degsl.h, libKJI, bos411, 9428A410j 6/6/91 14:28:38 */

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


/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         degsl.h
 *
 * DESCRIPTIVE NAME:    GSL Interface Constant Values.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              DBCS  Editor  V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            NA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Macro.
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 * ENTRY POINT:         NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: NA.
 *
 * TABLES:              NA.
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
 *      Graphic Subroutine Language(GSL) Interface Define.
 */
#ifndef _kj_gsl
#define _kj_gsl
/*
 *      GSL gsevwt_ Event Type.
 *      exclude Event Type 0 is DBCS Editor Event Type.
 */
#define E_RTNTYP (      0     )/* deevwt_ Generate DBCS Event Type.       */
#define E_CODE   (      1     )/* GSL Event Type(Code Data).              */
#define E_FUNC   (      2     )/* GSL Event Type(Control  Sequence)       */
#define E_TIMEOT (     10     )/* GSL Event Type(Timeout).                */
#define E_MKEYST (     43     )/* Maximim Number of GSL Keystroke String. */
/*
 *      GSL gsqdsp_ Plotter Type Device ID.
 */
#define E_DEVIHC ( (' '<<24)+(' '<<16)+('H'<<8)+'C')
                               /* For a Printer or Plotter Device         */
#define E_5081DA (   0x0408   )/* IBM5080 Display Id.                     */
/*
 *      GSL gsgtat_ Interface Parameter.
 */
#define E_JFNTID (  0x80c0    )/* Japanese DBCS Character Set Font ID.    */
#define E_ALIGNH (      1     )/* Horizonal Alignment Normal.             */
#define E_ALIGNV (      1     )/* Vertical  Alignment Normal.             */
#define E_ATUNCH ( 0x80000000 )/* Attribute is unchanged.                 */
#define E_ATUNCI (     -1     )/* Attribute is unchanged.                 */
#define E_BASELN (      0     )/* 0 Degree or left to right in viwer's    */
                               /* terms.                                  */
#define E_EXPAN  ( 0x00000100 )/* No Text Expantion.                      */
#define E_FONT   ((char *)-1  )/* Unchanged font parameter.               */
#define E_FONTID (     -1     )/* Unchanged text font.                    */
#define E_PRE    (      1     )/* Character preceision.                   */
#define E_SPAC   (      0     )/* No Text Spacing.                        */
#define E_UPVCTX (      0     )/* Upper vector of text string(X-component)*/
#define E_UPVCTY (      1     )/* Upper vector of text string(Y-component)*/
/*
 *      GSL gsxblt_ Interface Parameter.
 */
#define E_LOGOPS (      3     )/* Logical Operation Mode 'Set'.           */
#define E_LGOPSD (     15     )/* Source Data Set Destination.            */
#define E_PWID   (     16     )/* Display X-coordinate alignment value.   */
/*
 *      GSL gslatt_ Interface Parameter.
 */
#define E_LSOLID (      0     )/* Solid Line Pattern.                     */
#define E_COUNCH (     -1     )/* Color Index Unchanged.                  */
/*
 *      GSL gslop_ Interface Parameter.
 */
#define E_LOPSET (      3     )/* Logical Operation Mode 'Set'.           */
/*
 *      GSL Cordinate & Color Interface Parameter.
 */
#define E_AREAX  (      0     )/* The display origin X-coordinate value.  */
#define E_AREAY  (      0     )/* The display origin Y-coordinate value.  */
#define E_MINCOL (      0     )/* Mimimum Number of Color Index.          */
#endif

