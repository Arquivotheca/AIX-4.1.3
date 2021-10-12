/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: fixpath
 *		makepath
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: makepath.c,v $
 * Revision 1.9.5.2  1993/04/27  21:27:07  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  21:26:44  damon]
 *
 * Revision 1.9.5.1  1993/04/27  21:27:06  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.9.2.4  1992/12/03  17:21:11  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:23  damon]
 * 
 * Revision 1.9.2.3  1992/12/02  20:26:19  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:22  damon]
 * 
 * Revision 1.9.2.2  1992/09/24  19:01:45  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:21:58  gm]
 * 
 * Revision 1.9  1991/12/05  21:05:15  devrcs
 * 	Correct copyright; clean up lint input.
 * 	[91/01/08  12:14:10  randyb]
 * 
 * Revision 1.7  90/10/07  20:03:36  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:30  gm]
 * 
 * Revision 1.6  90/08/09  14:23:17  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:47:15  gm]
 * 
 * Revision 1.5  90/07/17  12:36:46  devrcs
 * 	More changes for gcc.
 * 	[90/07/04  22:30:48  gm]
 * 
 * Revision 1.4  90/06/29  14:38:51  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:20  gm]
 * 
 * Revision 1.3  90/04/27  19:31:53  devrcs
 * 	Changed default mode for mkdir from 0755 to 0777.
 * 	[90/04/15            gm]
 * 
 * Revision 1.2  90/01/02  19:27:03  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:14:57  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * Revision 2.2  88/12/13  13:51:54  gm0w
 * 	Created from makepath program.
 * 	[88/12/13            gm0w]
 * 
 * $EndLog$
 */
/*
 *  makepath - create intermediate directories along a path
 *
 *  makepath(path, refpath, trace, showerrs)
 *  char *path, *refpath;
 *  int trace, showerrs;
 *
 *  Create any directories missing in path.  If refpath is non-null,
 *  then it must have a non-null trailing subpath that is common to
 *  path.  The entire path will be created if refpath is given and
 *  refpath is the path of a directory, otherwise only the components
 *  preceding the final component will be created for path.  If trace
 *  is non-zero, diagnostics will be printed to stderr.  If showerrs
 *  is non-zero error messages will be printed to stderr.
 */

#ifndef lint
static char sccsid[] = "@(#)90  1.1  src/bldenv/sbtools/libode/makepath.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:22";
#endif /* not lint */

#include <string.h>
#include <unistd.h>
#include <ode/util.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

static
char *fixpath( register char *fpath, char *buf )
{
    register char *ls = NULL;
    register char *p = buf;

    *p = *fpath;
    while (*fpath != '\0') {
	fpath++;
	while (*p == '/' && *fpath == '/')
	    fpath++;
	*++p = *fpath;
	if (*p == '/')
	    ls = p;
    }
    return(ls);
}

int
makepath( char *mpath, char *refpath, int trace, int showerrs)
{
    char pathbuf[MAXPATHLEN], refpathbuf[MAXPATHLEN];
    char linkbuf[MAXPATHLEN], linkbuf2[MAXPATHLEN];
    char *base, *refbase;
    char *slash, *refslash;
    struct stat statb, refstatb;
    int ch, cc, cc2;
    uid_t uid;

    uid = getuid();

    if (mpath == NULL) {
	if (showerrs)
	    fprintf ( stderr, "makepath: NULL path argument\n");
	return(1);
    }

    base = fixpath(mpath, pathbuf);

    if (refpath == NULL) {
	if (base == NULL || base == pathbuf) {
	    if (showerrs)
		fprintf ( stderr,
			"makepath: %s must have an imbedded '/' character\n",
			pathbuf);
	    return(1);
	}
	*base = '\0';
	base = pathbuf;
	if (*base == '/')
	    base++;
	if (*base == '\0') {
	    if (showerrs)
		fprintf ( stderr, "makepath: illegal pathname %s\n", pathbuf);
	    return(1);
	}
	for (;;) {
	    /* find end of this component */
	    while (*base != '\0' && *base != '/')
		base++;

	    /* create path so far, if necessary */
	    ch = *base; *base = '\0';
	    if (stat(pathbuf, &statb) < 0) {
		if (mkdir(pathbuf, 0777) < 0) {
		    if (showerrs)
			fprintf ( stderr,
				"makepath: unable to create directory %s: %s\n",
				pathbuf, strerror(errno));
		    return(1);
		}
		if (stat(pathbuf, &statb) < 0) {
		    if (showerrs)
			fprintf ( stderr,
				"makepath: unable to stat directory %s: %s\n",
				pathbuf, strerror(errno));
		    return(1);
		}
		if (trace)
		    fprintf ( stderr, "%s: created directory\n", pathbuf);
	    } else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
		if (showerrs)
		    fprintf ( stderr,
			    "makepath: %s is not a directory (mode %#o)\n",
			    pathbuf, (statb.st_mode&S_IFMT));
		return(1);
	    }
	    if (ch == '\0')
		break;
	    *base++ = ch;
	}
	return(0);
    }

    refbase = fixpath(refpath, refpathbuf);

    /* if not a directory path, strip final component */
    if (stat(refpathbuf, &refstatb) < 0 ||
	(refstatb.st_mode&S_IFMT) != S_IFDIR) {
	if (base == NULL || base == pathbuf) {
	    if (showerrs)
		fprintf ( stderr,
			"makepath: %s must have an imbedded '/' character\n",
			pathbuf);
	    return(1);
	}
	if (refbase == NULL || refbase == refpathbuf) {
	    if (showerrs)
		fprintf ( stderr,
			"makepath: %s must have an imbedded '/' character\n",
			refpathbuf);
	    return(1);
	}
	if (strcmp(base, refbase) != 0) {
	    if (showerrs)
		fprintf ( stderr,
			"makepath: %s and %s do not have common trailing components\n",
			base, refbase);
	    return(1);
	}
	*base = *refbase = '\0';
	if (stat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf ( stderr,
			"makepath: unable to stat reference directory %s: %s\n",
			refpathbuf, strerror(errno));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf ( stderr, "makepath: %s is not a directory (mode %#o)\n",
			refpathbuf, (refstatb.st_mode&S_IFMT));
	    return(1);
	}
    }

    /* find beginning of common part of path names */
    base = pathbuf + strlen(pathbuf);
    refbase = refpathbuf + strlen(refpathbuf);
    while (*base == *refbase) {
	if (base == pathbuf || refbase == refpathbuf)
	    break;
	base--; refbase--;
    }
    if (*base == *refbase && *base != '/') {
	if (base == pathbuf && *(refbase-1) == '/') {
	    base = pathbuf + strlen(pathbuf) + 2;
	    do {
		*base = *(base - 2);
	    } while (base-- > pathbuf + 2);
	    *(base-1) = '.';
	    *base = '/';
	    refbase--;
	}
	if (refbase == refpathbuf && *(base-1) == '/') {
	    refbase = refpathbuf + strlen(refpathbuf) + 2;
	    do {
		*refbase = *(refbase - 2);
	    } while (refbase-- > refpathbuf + 2);
	    *(refbase-1) = '.';
	    *refbase = '/';
	    base--;
	}
    }
    while (*base != '\0' && *base != '/') {
	base++; refbase++;
    }
    slash = base++;
    refslash = refbase++;
    ch = *slash; *slash = '\0';
    if (stat(pathbuf, &statb) < 0) {
	if (showerrs)
	    fprintf ( stderr,
		    "makepath: unable to stat target directory %s: %s\n",
		    pathbuf, strerror(errno));
	return(1);
    }
    if ((statb.st_mode&S_IFMT) != S_IFDIR) {
	if (showerrs)
	    fprintf ( stderr, "makepath: %s: invalid mode %#o\n",
		    pathbuf, (statb.st_mode&S_IFMT));
	return(1);
    }
    *slash = ch;

    /* check each component along common path and make them the same */
    while (ch != '\0') {
	/* find end of this component */
	while (*base != '\0' && *base != '/') {
	    base++; refbase++;
	}

	/* get stat information for source path */
	ch = *refbase; *refbase = '\0'; *base = '\0';
	if (stat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf ( stderr, "makepath: stat %s: %s\n",
			refpathbuf, strerror(errno));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf ( stderr, "makepath: %s: invalid mode %#o\n",
			refpathbuf, (refstatb.st_mode&S_IFMT));
	    return(1);
	}
	if (lstat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf ( stderr, "makepath: lstat %s: %s\n",
			refpathbuf, strerror(errno));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) == S_IFLNK) {
	    if ((cc = readlink(refpathbuf, linkbuf, sizeof(linkbuf)-1)) < 0) {
		if (showerrs)
		    fprintf ( stderr, "makepath: readlink %s: %s\n",
			    refpathbuf, strerror(errno));
		return(1);
	    }
	    if (cc > 0 && *linkbuf != '/') {
		*refbase = ch;
		linkbuf[cc] = '\0';
		if (lstat(pathbuf, &statb) < 0) {
		    if (symlink(linkbuf, pathbuf) < 0) {
			if (showerrs)
			    fprintf ( stderr, "makepath: symlink %s: %s\n",
				    pathbuf, strerror(errno));
			return(1);
		    }
		    if (trace)
			fprintf ( stderr, "%s: created symlink to %s\n",
				pathbuf, linkbuf);
		} else {
		    if ((statb.st_mode&S_IFMT) != S_IFLNK) {
			if (showerrs)
			    fprintf ( stderr, "makepath: %s: invalid mode %#o\n",
				    pathbuf, (statb.st_mode&S_IFMT));
			return(1);
		    }
		    cc2 = readlink(pathbuf, linkbuf2, sizeof(linkbuf2)-1);
		    if (cc2 < 0) {
			if (showerrs)
			    fprintf ( stderr, "makepath: readlink %s: %s\n",
				    pathbuf, strerror(errno));
			return(1);
		    }
		    if (cc != cc2 || memcmp(linkbuf, linkbuf2, cc) != 0) {
			if (showerrs)
			    fprintf ( stderr,
				    "makepath: symlinks %s and %s differ\n",
				    refpathbuf, pathbuf );
			return(1);
		    }
		}
		(void) strcpy(linkbuf+cc, refbase);
		(void) strcpy(refslash+1, linkbuf);
		(void) strcpy(slash+1, linkbuf);
		(void) fixpath(refslash, refslash);
		(void) fixpath(slash, slash);
		refbase = refslash+1;
		base = slash+1;
		ch = *refslash;
		continue;
	    }
	}

	/* create path so far, if necessary */
	if (lstat(pathbuf, &statb) < 0) {
	    if (mkdir(pathbuf, (int)(refstatb.st_mode&07777)) < 0) {
		if (showerrs)
		    fprintf ( stderr, "makepath: mkdir %s: %s\n",
			    pathbuf, strerror(errno));
		return(1);
	    }
	    if (stat(pathbuf, &statb) < 0) {
		if (showerrs)
		    fprintf ( stderr, "makepath: stat %s: %s\n",
			    pathbuf, strerror(errno));
		return(1);
	    }
	    if (trace)
		fprintf ( stderr, "%s: created directory\n", pathbuf);
	} else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf ( stderr, "makepath: %s: invalid mode %#o\n",
			pathbuf, (statb.st_mode&S_IFMT));
	    return(1);
	}

	if (uid == 0 && (statb.st_uid != refstatb.st_uid ||
			 statb.st_gid != refstatb.st_gid)) {
	    (void) chown(pathbuf, (int)refstatb.st_uid, (int)refstatb.st_gid);
	    if (trace)
		fprintf ( stderr, "%s: owner %d, group %d\n", pathbuf,
			refstatb.st_uid, refstatb.st_gid);
	}
	if ((statb.st_mode&07777) != (refstatb.st_mode&07777)) {
	    (void) chmod(pathbuf, (int)(refstatb.st_mode&07777));
	    if (trace)
		fprintf ( stderr, "%s: mode %#o\n", pathbuf,
			refstatb.st_mode&07777);
	}

	refslash = refbase;
	*refbase++ = ch;
	slash = base;
	*base++ = ch;
    }
    return(0);
}
