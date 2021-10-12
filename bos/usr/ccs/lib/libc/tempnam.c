static char sccsid[] = "@(#)55	1.18  src/bos/usr/ccs/lib/libc/tempnam.c, libcio, bos412, 9446B 11/11/94 14:02:23";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: tempnam 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/access.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

static char *pcopy();
extern char *__pidnid();	/* see DStemp.c */
#define	PIDNIDLEN 9		/* _pidnid() generates a 9 char string */

/*                                                                    
 * FUNCTION: Returns temporary file name.
 *                                                                    
 * RETURN VALUE DESCRIPTION: char * filename, NULL on error.
 */  

char *
tempnam(
const char *dir,		/* use this directory please (if non-NULL) */
const char *pfx)		/* use this (if non-NULL) as filename prefix */
{
	register char *p, *tmp, *tdir;
	int x = 0;
	int sv_errno;
	struct stat sb;

	sv_errno = errno;

	if ((tdir = getenv("TMPDIR")) != NULL)
		if ((x = strlen(tdir)) > 0 && 
				access(tdir, W_ACC | X_ACC) == 0)
			goto OK;
	if ((tdir = dir) != NULL)
		if ((x = strlen(tdir)) > 0 && 
				access(tdir, W_ACC | X_ACC) == 0)
			goto OK;
	if ((tdir = P_tmpdir) != NULL)
		if ((x = strlen(tdir)) > 0 && 
				access(tdir, W_ACC | X_ACC) == 0)
			goto OK;
	if (access(tdir = "/tmp", W_ACC | X_ACC) != 0)
		return(NULL);
	else
		x = 4;	/* strlen("/tmp") */

OK:
	
	/* Need space for name of tmp directory, plus '/', plus prefix, 
	 * plus 6 X's to be replaced by mktemp(), plus NULL. 
	 */
	if ((p = (char *)malloc((size_t)(x + strlen(pfx) + 8))) == NULL)
		return(NULL);
	pcopy(p, tdir);
	(void)strcat(p, "/");

	if(pfx) {
		int pfxlen, charlen = mblen(pfx, MB_CUR_MAX);

		if (charlen < 1)
			charlen = 1;

		/* determine how many bytes of prefix, up to 5 bytes, to keep
		 * (do not truncate prefix mid-character) 
		 */
		for (pfxlen=0; pfxlen+charlen<=5; ) {
			pfxlen += charlen;
			charlen = mblen(pfx + pfxlen, MB_CUR_MAX);
			if (charlen < 1)
				charlen = 1;
		}

		*(p+strlen(p)+pfxlen) = '\0';
		(void)strncat(p, pfx, pfxlen);
	}

	tmp = p+strlen(p);
	do {		      /* mktemp() puts '\0' in p[0] if it fails.    */
		*p = *tdir;   /* If so, set it up again for next iteration. */
		strcpy(tmp, "XXXXXX");
		mktemp(p);    /* Will eventually find a unique name since   */
	} while (!*p);	      /*  mktemp() is partially based on time       */

	errno = sv_errno;
	return(p);
}

static char*
pcopy(space, arg)
char *space, *arg;
{
	char *p;

	if(arg) {
		(void)strcpy(space, arg);
		p = space-1+strlen(space);
		if(*p == '/')
			*p = '\0';
	}
	return(space);
}
