static char sccsid[] = "@(#)58	1.2  src/bos/usr/bin/uucp/subdir.c, cmduucp, bos411, 9428A410j 6/16/90 00:00:54";

#include "uucp.h"
/* 
 * COMPONENT_NAME: CMDUUCP subdir.c
 * 
 * FUNCTIONS: subchdir, subfile 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*LINTLIBRARY*/

/*
 * By Tom Truscott, March 1983
 *
 * Prefix table.
 * If a prefix is "abc", for example,
 * then any file Spool/abc... is mapped to Spool/abc/abc... .
 * The first prefix found is used, so D.foo should preceed D. in table.
 *
 * Each prefix must be a subdirectory of Spool, owned by uucp!
 * Remember: use cron to uuclean these directories daily,
 * and check them manually every now and then.  Beware complacency!
 */

#define	DLocalX		"X."
#define	DLocal		"D."

static char *dprefix[] = {
	DLocalX,	/* Outbound 'xqt' request files */
	DLocal,		/* Outbound data files */
	"D.",		/* Other "D." files (remember the "."!) */
	"C.",		/* "C." subdirectory */
	"X.",		/* "X." subdirectory */
	"TM.",		/* Temporaries for inbound files */
	0
};


/*
 * filename mapping kludges to put uucp work files in other directories.
 */

#define	BUFLEN	50

static	char fn1[BUFLEN], fn2[BUFLEN];	/* remapped filename areas */
static	int	inspool;		/* true iff working dir is Spool */

/*
 * return (possibly) remapped string s
 */
char *
subfile(as)
char *as;
{
	register char *s, **p;
	register int n;
	static char *tptr = NULL;

	/* Alternate buffers so "link(subfile(a), subfile(b))" works */
	if (tptr != fn1)
		tptr = fn1;
	else
		tptr = fn2;

	s = as;
	tptr[0] = '\0';

	/* if s begins with Spool/, copy that to tptr and advance s */
	if (strncmp(s, Spool, n = strlen(Spool)) == 0 && s[n] == '/') {
		if (!inspool) {
			strcpy(tptr, Spool);
			strcat(tptr, "/");
		}
		s += n + 1;
	}
	else
		if (!inspool)
			return as;

	/* look for first prefix which matches, and make subdirectory */
	for (p = &dprefix[0]; *p; p++) {
		if (strncmp(s, *p, n = strlen(*p))==0 && s[n] && s[n] != '/') {
			strcat(tptr, *p);
			strcat(tptr, "/");
			strcat(tptr, s);
			return tptr;
		}
	}
	return as;
}

/*
 * save away filename
 */
subchdir(s)
register char *s;
{
	inspool = (strcmp(s, Spool) == 0);
	return chdir(s);
}
