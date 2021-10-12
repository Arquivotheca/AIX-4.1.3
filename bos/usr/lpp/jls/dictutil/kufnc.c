static char sccsid[] = "@(#)40	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kufnc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:26:07";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kufnc
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

/********************* START OF MODULE SPECIFICATIONS ***********************
 *
 * MODULE NAME:         kufnc
 *
 * DESCRIPTIVE NAME:    check input file name
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        609 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kugetc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            int kufnc(fname)
 *
 *  INPUT:              fname   : pointer to file name
 *
 *  OUTPUT:             fname   : pointer to file name
 *
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              NA.
 *                      Standard Liblary.
 *                              strcpy
 *                              strlen
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
#include "kut.h"
#include <stdio.h>
/*#include <memory.h>*/

/*
 * Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989          ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM ";

int kufnc(fname)
char    *fname;         /* file name    */
{
    char        wname[1024];    /* work name    */
    int         data_len;       /* length of data       */
    int         i;              /* work variable of integer     */

    /* set of data length       */
    data_len = strlen(fname);
    if(data_len < 1) {
      return(IUFAIL);
    };

    /* find blank code from top to last */
    i = 0;
    while(TRUE) {
      if( (*(fname+i) == U_2SPACEH) && (*(fname+i+1) == U_2SPACEL) ) {
	/* DBCS blank code    */
	i = i + 2;
	continue;
      } else if(*(fname+i) == U_1SPACE) {
	/* SBCS blank code    */
	i++;
	continue;
      };
      break;
    };

    /* erase blank data */
    strcpy(wname,(char *)(fname+i));
    strcpy(fname,wname);

    /* find blank code from top to last */
    i = 0;
    while(*(fname+i) != NULL) {
      if( (*(fname+i) == U_2SPACEH) && (*(fname+i+1) == U_2SPACEL) ) {
	/* DBCS blank code    */
	return(IUFAIL);
      } else if(*(fname+i) == U_1SPACE) {
	/* SBCS blank code    */
	return(IUFAIL);
      };
      i++;
    };

    /* check of last character  */
    data_len = strlen(fname) - 1;
    if(*(fname+data_len) == '/') {
      return(IUFAIL);
    };


    return(IUSUCC);
}

