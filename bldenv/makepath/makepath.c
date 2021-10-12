static char sccsid[] = "@(#)01  1.2  src/bldenv/makepath/makepath.c, bldprocess, bos412, GOLDA411a 2/3/93 17:12:13";

/*
 *   COMPONENT_NAME: BOSBUILD
 *
 *   FUNCTIONS: defined
 *		fixpath
 *		main
 *		mkpath
 *		print_revision
 *		print_usage
 *		strdup
 *		
 *
 *   ORIGINS: 71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * @OSF_FREE_COPYRIGHT@
 * 
 * Copyright (c) 1990, 1991
 * Open Software Foundation, Inc.
 * 
 * Permission is hereby granted to use, copy, modify and freely distribute
 * the software in this file and its documentation for any purpose without
 * fee, provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.  Further, provided that the name of Open
 * Software Foundation, Inc. ("OSF") not be used in advertising or
 * publicity pertaining to distribution of the software without prior
 * written permission from OSF.  OSF makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */
/*
 * HISTORY
 * $Log: $
 * $EndLog$
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: makepath.c,v $ $Revision: 1.7 $ (OSF) $Date: 1991/12/05 20:45:35 $";
#endif
/*
 *  makepath - create intermediate directories along a path
 *
 *  makepath path ...
 *
 *  Create any directories missing in path.
 */
#include <sys/param.h>

#ifdef NO_SYS_LIMITS
#include <limits.h>
#else
#include <sys/limits.h>
#endif

#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

#ifndef _BLD
#include "ode/odedefs.h"
#endif

#ifndef	PATH_MAX
#define PATH_MAX	1024
#endif

#define	TRUE	1
#define	FALSE	0

static char *progname;		/* program name */

static
char *fixpath(pathname)
register char *pathname;
{
    register char *ls = NULL;
    register char *p = pathname;

    *p = *pathname;
    while (*pathname != '\0') {
	pathname++;
	while (*p == '/' && *pathname == '/')
	    pathname++;
	*++p = *pathname;
	if (*p == '/')
	    ls = p;
    }
    return(ls);
}

#ifdef _BLD
#ifdef NO_STRDUP
char *malloc();

char *strdup(p)
char *p;
{
	register char *q;
	register int l;

	q = malloc(l = strlen(p) + 1);
	if (q != 0)
		bcopy(p, q, l);
	return(q);
}
#endif
#endif

static
mkpath(pathname, trace)
char *pathname;
int trace;
{
    char *base;
    struct stat st;
    int ch, ididit;

    if (pathname == NULL) {
	fprintf(stderr, "%s: NULL path argument\n", progname);
	return(1);
    }

    pathname = (char *)strdup(pathname);
    if (pathname == NULL)
	fprintf(stderr, "%s: strdup failed\n", progname);
    base = fixpath(pathname);

    if (base == NULL || base == pathname) {
	fprintf(stderr, "%s: %s must have an imbedded '/' character\n",
		progname, pathname);
	return(1);
    }
    *base = '\0';
    base = pathname;
    if (*base == '/')
	base++;
    if (*base == '\0') {
	fprintf(stderr, "%s: illegal pathname %s\n", progname, pathname);
	return(1);
    }
    for (;;) {
	/* find end of this component */
	while (*base != '\0' && *base != '/')
	    base++;

	/* create path so far, if necessary */
	ch = *base;
	*base = '\0';
	if (stat(pathname, &st) < 0) {
	    if (mkdir(pathname, 0777) < 0) {
		if (errno != EEXIST) {
		    fprintf(stderr, "%s: unable to create directory %s: %s\n",
			    progname, pathname, strerror(errno));
		    return(1);
		}
		ididit = FALSE;
	    } else
		ididit = TRUE;
	    if (stat(pathname, &st) < 0) {
		fprintf(stderr, "%s: unable to stat directory %s: %s\n",
			progname, pathname, strerror(errno));
		return(1);
	    }
	    if (ididit && trace)
		fprintf(stderr, "%s: created directory\n", pathname);
	} else if ((st.st_mode&S_IFMT) != S_IFDIR) {
	    fprintf(stderr, "%s: %s is not a directory (mode %#o)\n",
		    progname, pathname, (st.st_mode&S_IFMT));
	    return(1);
	}
	if (ch == '\0')
	    break;
	*base++ = ch;
    }
    return(0);
}

main(argc, argv)
    int argc;
    char *argv[];
{
    int quiet = FALSE;
    int errors = 0;

    if (argc > 0) {
	if ((progname = (char *)rindex(argv[0], '/')) == NULL)
	    progname = argv[0];
	else
	    progname++;
    } else
	progname = "makepath";
    argc--;
    argv++;

    if (argc > 0 && strcmp(argv[0], "-version") == 0)
    {
	print_revision();
	exit(0);
    }

    if (argc > 0 && strcmp(argv[0], "-quiet") == 0) {
	quiet = TRUE;
	argc--;
	argv++;
    }

    if (argc == 0) {
	print_usage();
	exit(1);
    }

    if (strcmp(argv[0], "-") == 0) {	/* read stdin */
	char *pathname, *endp, *ptr;
	char buffer[PATH_MAX];

	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
	    ptr = buffer;
	    while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	    pathname = ptr;
	    while (*ptr && *ptr != '\n' && *ptr != ' ' && *ptr != '\t')
		ptr++;
	    endp = ptr;
	    while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	    if (*ptr && *ptr != '\n') {
		fprintf(stderr, "%s: bad pathname: %s\n", progname, buffer);
		continue;
	    }
	    *endp = 0;
	    if (*pathname == 0)
		continue;
	    errors |= mkpath(pathname, !quiet);
	}
    } else {
	while (argc > 0) {
	    errors |= mkpath(argv[0], !quiet);
	    argc--;
	    argv++;
	}
    }
    exit(errors);
}


/* show the revision of this program */
print_revision()
{
    printf("%s $Revision: 1.7 $ $Date: 1991/12/05 20:45:35 $\n", progname);
}


/* show invocation options */
print_usage()
{
    printf("usage: %s [ -version | -quiet ] path ...\n", progname);
}

