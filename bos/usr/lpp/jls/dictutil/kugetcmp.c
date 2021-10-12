static char sccsid[] = "@(#)43	1.5.1.1  src/bos/usr/lpp/jls/dictutil/kugetcmp.c, cmdKJI, bos411, 9428A410j 7/23/92 01:27:05";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kugetcmp
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
 * MODULE NAME:         kugetcmp
 *
 * DESCRIPTIVE NAME:    get cursor move value
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
 *  MODULE SIZE:        372  Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kugetcmp
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kugetcmp ( udcbptr )
 *
 *  INPUT               *udcbptr: pointer to User Dictionary Control Block
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
#if defined(CNVEVT)
extern int cnvflg;
#define U_EUC_C1   (  0xa1  )
#define U_EUC_C2   (  0xfe  ) 
#include "kje.h"                        /* Kanji Utility Define File.   */
#endif 

int kugetcmp(getcode)
int     getcode;                /* check character code         */
{
  int   curmp = 1;

#if defined(CNVEVT)
   if( cnvflg == U_SJIS ){
#endif

       if(    ( (U_FST_C1 <= getcode) && (getcode <= U_FST_C2) )
           || ( (U_FST_C3 <= getcode) && (getcode <= U_FST_C4) )  ) {
         curmp = 2;   /* getcode is DBCS */
       };

#if defined(CNVEVT)
   };
   if( cnvflg == U_EUC ){
       if(    ( (U_EUC_C1 <= getcode) && (getcode <= U_EUC_C2) )  ) {
         curmp = 2;   /* getcode is DBCS */
       } else if( (getcode == 0x8e) || (getcode == 0x8f) ) {
         curmp = 2;   /* getcode is DBCS */
       };
   };
#endif

  return( curmp );

}
