static char sccsid[] = "@(#)39	1.5.1.1  src/bos/usr/lpp/jls/dictutil/kudisply.c, cmdKJI, bos411, 9428A410j 7/23/92 01:25:48";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudisply
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

 /******************** START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudisply
 *
 * DESCRIPTIVE NAME:    user dictionary combain handler
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
 *  MODULE SIZE:        10332 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudisply
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            int kudisply( udcb, line, col, data, data_len )
 *
 *  INPUT:              udcb            : pointer to UDCB
 *                      line            : display line
 *                      col             : display column
 *                      data            : pointer to display data
 *                      data_len        : display data length
 *
 *  OUTPUT:
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
 *                              strlen
 *                              printf
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
#include <stdio.h>
#include <string.h>
#include "kut.h"
#include "kje.h"

#if defined(CNVEVT)
extern  int     cnvflg;         /* Conversion Type Code                 */
#endif

/*
 *   Copyright Identify.
 */
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989          ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM ";

int kudisply( udcb, line, col, data, data_len )

UDCB    *udcb;          /* User Dictionary Contrpl Block Pointer        */
short   line;           /* Line Number                                  */
short   col;            /* Col Number                                   */
char    *data;          /* Data                                         */
short   data_len;       /* Data Length                                  */
{

    char    format[80];   /* Print Format Area                    	*/
    char    disarea[256]; /* Echo Data                            	*/
    int     drlen;        /* Echo Data Length                     	*/
    int     i;            /* Loop Counter                         	*/
    int     rc = 0;       /* Return Value                         	*/

    if ( data_len <= 0 )
	return( rc );
    if ( (line < 0)          ||
    	(udcb->ymax < line) ||
	(col < 0)           ||
	((U_MAXCOL+udcb->xoff) < col) ) {
	rc =  -1;
    }

    data_len = MIN(data_len,(U_MAXCOL+udcb->xoff-col));
    drlen = strlen(data);
    memcpy(disarea,data,data_len);
    if ( data_len > drlen ) {
	for( i = drlen; i < data_len; i++ ) {
	    disarea[i] = 0x20;
	}
    }
    disarea[data_len] = '\0';

    sprintf(format,"%%B.%ds",data_len);    
    CURSOR_MOVE(line,col);
    printf(format,disarea);

    return( rc );

}
