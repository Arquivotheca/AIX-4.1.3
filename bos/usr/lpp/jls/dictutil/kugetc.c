static char sccsid[] = "@(#)42	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kugetc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:26:46";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kugetc
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

/********************* START OF MODULE SPECIFICATIONS **********************  *
 * MODULE NAME:         kugetc
 *
 * DESCRIPTIVE NAME:    user dictionary sget character
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
 *  MODULE SIZE:        252 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kugetc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            int kugetc()
 *
 *  INPUT:              NA.
 *
 *  OUTPUT:             NA.
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
 *                              kugetkey
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
#include "kuke.h"
#include "kje.h"

/*
 * Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

#if defined(CNVEVT)
extern  int     cnvflg;         /* Conversion Type Code                 */
#endif

int kugetc()
{

#if defined(EXTCUR) 
extern  int     kugetkey();
static  int     flag = 0;
static  uchar   code[4];
#if defined(CNVEVT)
size_t     ilen;
size_t     olen;
size_t     cnvlen;
char    cnvdt[U_MEMSIZ];
#endif

int     rc;

	if ( flag == 0 ) {

		rc = kugetkey( GET_CHAR );

		memcpy(code,&rc,sizeof(code));

/* --- */
		if ( (code[0] == 0xff) &&
		     (code[1] == 0xff) ) {
			rc   = 0;
			rc   = code[2];
			flag = 1;
/* --- */

		} else {
			flag = 0;
			memset(code,0x00,sizeof(code));
		};
	} else {

/* --- */
		rc   = 0;
		rc   = code[3];
		flag = 0;
/* --- */ 

		memset(code,0x00,sizeof(code));
	};

	return( rc );
#else
  int           kugetkey();

  return( kugetkey(GET_CHAR) );
#endif
}
