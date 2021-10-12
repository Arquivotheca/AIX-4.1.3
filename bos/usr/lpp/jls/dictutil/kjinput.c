static char sccsid[] = "@(#)77  1.3  src/bos/usr/lpp/jls/dictutil/kjinput.c, cmdKJI, bos411, 9428A410j 8/27/91 12:18:08";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
 */


#if defined(CNVSTOC) || defined(PARMADD)
#include <stdio.h>
#if defined(CNVEVT)
#include <iconv.h>
#endif

#include "kucode.h"
#include "kje.h"

#if defined(CNVEVT)
extern  iconv_t icnvfd;         /* Input Conversion File Discripter     */
extern  iconv_t ocnvfd;         /* Output Conversion File Discripter    */
extern  int     cnvflg;         /* Conversion Flag                      */
#endif

int kjinput( data, msg )

char    *data;                  /* Event Get Parameter            (i/o) */
char    *msg;                   /* Event Get Message                (i) */
{
char    sbsp[10];               /* Single Byte Character (Space)        */
char    dbsp[10];               /* Duble Byte Character (Space)         */
size_t  ocd;
char    *pt = (char *)NULL;     /* Work Pointer                         */
int     len;                    /* Data Lenght                          */
int     i;                      /* Loop Counter                         */

	memset(sbsp,'\0',10);
	memset(dbsp,'\0',10);
	sbsp[0] = U_1SPACE;
	if (cnvflg == U_SJIS) {
		dbsp[0] = U_2SPACEH;
		dbsp[1] = U_2SPACEL;
	} else {
		dbsp[0] = E_2SPACEH;
		dbsp[1] = E_2SPACEL;
	}

	for ( ;; ) {
		printf("%s",msg);
		gets(data);

		pt = data;
		len = strlen(pt);
		for ( i = 0; i < len; i++ ) {
			if ( *(pt+i) == sbsp[0] ) {
				continue;
			} else if ( *(pt+i)   == dbsp[0] &&
				    *(pt+i+1) == dbsp[1] ) {
				i++;
				continue;
			} else {
				pt = &data[i];
				break;
			};
		};
		if ( len > 0 &&
		     i < len ) {
			break;
		};
	};

	return( 0 );

}   
#endif
