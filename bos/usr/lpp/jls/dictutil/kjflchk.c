static char sccsid[] = "@(#)76  1.3  src/bos/usr/lpp/jls/dictutil/kjflchk.c, cmdKJI, bos411, 9428A410j 8/27/91 12:18:00";
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


#if defined(CNVSTOC)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "kje.h"

#define PR(XX)  XX;

int kjflchk( usrdic, udir, ubas, ubak )

char    *usrdic;        /* User Dictionary                          (i) */
char    *udir;          /* Directory Name                           (o) */
char    *ubas;          /* Base Dictionary Name                     (o) */
char    *ubak;          /* Back Dictionary Name                     (o) */
{
struct  stat    sbuf;           /* File Status                          */
char    fname[U_MEMSIZ];        /* Temporary File Name                  */
char    fwork[U_MEMSIZ];        /* Work File Name                       */
char    rmfile[U_MEMSIZ];       /* Remove File Name                     */
char    *pwd = NULL;            /* Current Directory                    */
int     len;                    /* User Dictionary Path Length          */
int     i;                      /* Loop Counter                         */
int     rc;                     /* Return Value                         */

do {

	/*
	 * Check User Dictionary File Access
	 */
	errno = 0;
	rc = access(usrdic,U_MSKAWT);
	if ( errno == ENOENT ) {
		(void)printf(CU_MSGNUD, usrdic);
		rc = 1;
		break;
	};
	if ( rc != 0 ) {
		(void)printf(CU_MSGWER, usrdic);
		rc = 2;
		break;
	};

	/*
	 * Check Dictonary File Access
	 */
	rc = access(usrdic,U_MSKARD);
	if ( rc != 0 ) {
		(void)printf(CU_MSGRER, usrdic);
		rc = 3;
		break;
	};

	/*
	 * Get File Name Position
	 */
	errno = 0;
	len = strlen(usrdic);
	for ( i = (len-1); i >= 0; i-- ) {
		if ( strncmp(&usrdic[i],U_SLASH,1) == 0 ) {
			break;
		};
	};

	i = U_MAX(i,0);
	if ( i == 0 ) {
		strncpy(udir,U_CURENT,1);
		i--;
	} else {
		strncpy(udir,usrdic,i);
	};
/*
#if defined(TMPDIR)
	strncpy(udir,U_TMPDIR,4);
	udir[4] = '\0';
#endif
*/
	strncpy(ubas,(usrdic+i+1),(len-i));
	strncpy(ubak,ubas,10);

	if ( strlen(ubak) > U_FLENMX ) {
		(void)printf(CU_MSGLOV, usrdic);
		rc = 4;
		break;
	};

	sprintf(fname,"%s%s%s",udir,U_SLASH,ubas);
	sprintf(fwork,"%s%s%s%s",udir,U_SLASH,ubas,U_BAK);

	if ( strcmp(fname,fwork) == 0 ) {
		(void)printf(CU_MSGLOV, usrdic);
		rc = 5;
		break;
	};

	sprintf(fwork,"%s%s%s%s",udir,U_SLASH,ubak,U_TMP);
	if ( strcmp(fname,fwork) == 0 ) {
		(void)printf(CU_MSGLOV, usrdic);
		rc = 6;
		break;
	};
#if !defined(TMPDIR)
	rc = access(U_CURDIR,U_MSKAWT);
	if ( rc != 0 ) {
		pwd = getenv(U_PWD);
		(void)printf(CU_MSGNDW, pwd);
		rc = 7;
		break;
	};
#endif
	rc = access(udir,U_MSKAWT);
	if ( rc != 0 ) {
		(void)printf(CU_MSGNDW, udir);
		rc = 8;
		break;
	};

	memset(rmfile,'\0',U_MEMSIZ);
	memset(fname,'\0',U_MEMSIZ);
	sprintf(fname,"%s%s%s%s",udir,U_SLASH,ubak,U_BAK);
	rc = access(fname,U_MSKAWT);
	if ( errno != ENOENT ) {
		if ( rc != 0 ) {
			(void)printf(CU_MSGFWE, fname);
			rc = 9;
			break;
		};
	};
	errno = 0;
	memset(fname,'\0',U_MEMSIZ);
	sprintf(fname,"%s%s",ubak,U_TMP);
	rc = access(fname,U_MSKAWT);
	if ( errno != ENOENT ) {
		if ( rc == 0 ) {
			sprintf(rmfile,"%s%s",U_RMFILE,fname);
			system( rmfile );
		} else {
			(void)printf(CU_MSGFWE, fname);
			rc = 10;
			break;
		};
	};

	errno = 0;
	stat( usrdic, &sbuf );
	if ( sbuf.st_size < U_MINBYT ||
	     sbuf.st_size > U_MAXBYT ) {
		(void)printf(CU_MSGROT, usrdic);
		rc = 12;
		break;
	};

	if ( (sbuf.st_size % U_URDC1K) != 0 ) {
		(void)printf(CU_MSGROT, usrdic);
		rc = 13;
		break;
	};

	errno = 0;
	rc    = 0;

} while( 0 );

	return( rc );

}
#endif
