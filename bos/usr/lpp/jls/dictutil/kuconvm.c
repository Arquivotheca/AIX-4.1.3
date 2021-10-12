static char sccsid[] = "@(#)11	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kuconvm.c, cmdKJI, bos411, 9428A410j 7/23/92 00:58:23";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kuconvm
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         kuconvm
 *
 * DESCRIPTIVE NAME:    set conversion moode
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            user dictionary table handler
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        ???  Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kuconvm
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kuconvm ( x )
 *
 *  INPUT               x       : conversion mode
 *
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         UDSUCC  : sucess return
 *
 * EXIT-ERROR:          IUFAIL  : error
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Liblary.
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */
#include "kut.h"                        /* Kanji Utility Define File.   */

/*
 *      work define.
 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void  kuconvm( x )
short   x;              /* conversion mode      */
{
  static char   con[] ={0x1b,0x4a,0x30,0x50,NULL};   /* ESC J0P Kanji-on   */
  static char   coff[]={0x1b,0x4a,0x32,0x50,NULL};   /* ESC J2P Kanji-off  */

/**************************
 * As of 10/05/88, the "kjterm" terminal can use the conversion code.
 * However, the "jxterm" terminal can not use it.
 * Therefore, this routine is invalid now.
 * When X-KJ-11 products are issued, this routin will be valid.
 **************************

  if(x == U_FON) {
      fprintf(stdout,"%s",con);
      ** putp(con);     **
  } else {
      fprintf(stdout,"%s",coff);
      ** putp(coff);    **
  };

****************************/

  return;
}
