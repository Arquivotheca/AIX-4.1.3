static char sccsid[] = "@(#)43  1.1  src/bldenv/sbtools/libsb/makepath.c, bldprocess, bos412, GOLDA411a 4/29/93 12:21:44";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 *  School of Computer Science 
 *  Carnegie Mellon University 
 *  Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * ODE 2.1.1
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
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>

static
char *fixpath(path, buf)
register char *path;
char *buf;
{
    register char *ls = NULL;
    register char *p = buf;

    *p = *path;
    while (*path != '\0') {
	path++;
	while (*p == '/' && *path == '/')
	    path++;
	*++p = *path;
	if (*p == '/')
	    ls = p;
    }
    return(ls);
}

makepath(path, refpath, trace, showerrs)
char *path, *refpath;
int trace, showerrs;
{
    char pathbuf[MAXPATHLEN], refpathbuf[MAXPATHLEN];
    char linkbuf[MAXPATHLEN], linkbuf2[MAXPATHLEN];
    char *base, *refbase;
    char *slash, *refslash;
    struct stat statb, refstatb;
    int ch, cc, cc2;
    extern uid_t getuid();
    uid_t uid = getuid();

    if (path == NULL) {
	if (showerrs)
	    fprintf ( stderr, "makepath: NULL path argument\n");
	return(1);
    }

    base = fixpath(path, pathbuf);

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
				pathbuf, errmsg(-1));
		    return(1);
		}
		if (stat(pathbuf, &statb) < 0) {
		    if (showerrs)
			fprintf ( stderr,
				"%s: unable to stat directory %s: %s\n",
				pathbuf, errmsg(-1));
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
			"%s: unable to stat reference directory %s: %s\n",
			refpathbuf, errmsg(-1));
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
		    pathbuf, errmsg(-1));
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
			refpathbuf, errmsg(-1));
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
			refpathbuf, errmsg(-1));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) == S_IFLNK) {
	    if ((cc = readlink(refpathbuf, linkbuf, sizeof(linkbuf)-1)) < 0) {
		if (showerrs)
		    fprintf ( stderr, "makepath: readlink %s: %s\n",
			    refpathbuf, errmsg(-1));
		return(1);
	    }
	    if (cc > 0 && *linkbuf != '/') {
		*refbase = ch;
		linkbuf[cc] = '\0';
		if (lstat(pathbuf, &statb) < 0) {
		    if (symlink(linkbuf, pathbuf) < 0) {
			if (showerrs)
			    fprintf ( stderr, "makepath: symlink %s: %s\n",
				    pathbuf, errmsg(-1));
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
				    pathbuf, errmsg(-1));
			return(1);
		    }
		    if (cc != cc2 || bcmp(linkbuf, linkbuf2, cc) != 0) {
			if (showerrs)
			    fprintf ( stderr,
				    "makepath: symlinks %s and %s differ\n",
				    refpathbuf, pathbuf, errmsg(-1));
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
			    pathbuf, errmsg(-1));
		return(1);
	    }
	    if (stat(pathbuf, &statb) < 0) {
		if (showerrs)
		    fprintf ( stderr, "makepath: stat %s: %s\n",
			    pathbuf, errmsg(-1));
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
