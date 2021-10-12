static char sccsid[] = "@(#)71	1.5  src/bos/usr/lpp/jls/dictutil/kjcnvets.c, cmdKJI, bos411, 9428A410j 3/23/94 04:37:23";
/*
 *   COMPONENT_NAME: cmdKJI
 *
 *   FUNCTIONS: User Dictionary Utility for Japanese
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
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
#if defined(CNVEVT)
#include <stdio.h>
#include <iconv.h>
#include <string.h>

#include "kje.h"

extern  iconv_t icnvfd;         /* Input Conversion File Discripter     */
extern  int     cnvflg;         /* Conversion Type Code                 */

int kjcnvets( in, ibyte, out, obyte )

char    *in;                    /* EUC Code                         (i) */
size_t  *ibyte;                 /* EUC Code Byte                    (i) */
char    *out;                   /* Shift Jis Code                   (o) */
size_t  *obyte;                 /* Shift Jis Code Byte            (i/o) */
{
    int     	rc = 0;		/* Return Value                         */
    int		i;		/* Counter value			*/
    size_t      obyte_bk;       /* Backup of obyte                      */

    do {

	memset(out,'\0',*obyte);

	/*
	 * Check Conversion Code
	 */
	if ( cnvflg != U_EUC ) {
		strncpy(out,in,*ibyte); /* Move Output Code             */
		*obyte = *ibyte;        /* Move Byte                    */
		break;
	};

	/*
	 * Check Input Data Byte Range
	 */
	if ( *ibyte <= 0 ) {
		*obyte = (size_t)0;     /* Move 0 Byte                  */
		break;
	};

	/*
	 * Code Conversion Subroutine Call
	 */
	icnvfd = iconv_open( U_SJISCD, U_EUCCD );   /* euc to sjis */
	if ( icnvfd == (iconv_t) -1 ) {
		printf(U_MSGCNV);
		break;
	};

        for ( i=0; i<*ibyte; i++ ) {
            if ( in[i] == 0x00 )
                break;
        }
	if ( i < *ibyte )
            *ibyte = i;
        obyte_bk = *obyte;

	rc = iconv( icnvfd, &in, ibyte, &out, obyte );    
	iconv_close( icnvfd );

	if ( *obyte > 0 )
            out[ obyte_bk - *obyte ] = 0x00;

	/*
	 * Coversion Code Return Code Check
	 */
	if ( rc == -1 ) {
	    rc = 0;
	    break;
	};

    } while ( 0 );

    return( rc );
}
#endif
