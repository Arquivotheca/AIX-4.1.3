static char sccsid[] = "@(#)93	1.11  src/bos/usr/bin/whereis/whereis.c, cmdscan, bos411, 9428A410j 6/10/94 09:21:26";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27, 71
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
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 * $RCSfile: whereis.c,v $ $Revision: 2.10.2.3 $ (OSF) $Date: 92/02/18 20:47:57 $
 */

#define _ILS_MACROS

#include <sys/param.h>
#include <sys/dir.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <locale.h>

#include "whereis_msg.h" 
nl_catd         catd;
#define MSGSTR(n,s) catgets(catd, MS_WHEREIS, n, s) 

int findin();
int find();
int findv();
int lookman();
int lookbin();
int looksrc();
int lookup();
static void zerof();
static void getlist();

static char *bindirs[] = {
	"/etc",
	"/usr/bin",
	"/usr/sbin",
	"/usr/games",
	"/usr/ucb",
	"/usr/usg",
	"/usr/lib",
	"/usr/local",
	"/usr/new",
	"/usr/old",
	"/usr/hosts",
	"/usr/include",
	"/sbin",
	"/usr/lbin",
	"/usr/lbin/spell",
	"/usr/ccs/lib",
	"/usr/ccs/bin",
	0
};
static char *mandirs[] = {
	"/usr/share/man/man1",
	"/usr/share/man/man2",
	"/usr/share/man/man3",
	"/usr/share/man/man4",
	"/usr/share/man/man5",
	"/usr/share/man/man6",
	"/usr/share/man/man7",
	"/usr/share/man/man8",
	"/usr/share/man/manl",
	"/usr/share/man/mann",
	"/usr/share/man/mano",
	"/usr/share/man/cat1",
	"/usr/share/man/cat2",
	"/usr/share/man/cat3",
	"/usr/share/man/cat4",
	"/usr/share/man/cat5",
	"/usr/share/man/cat6",
	"/usr/share/man/cat7",
	"/usr/share/man/cat8",
	"/usr/share/man/catl",
	"/usr/share/man/catn",
	"/usr/share/man/cato",
	0
};
static char *srcdirs[]  = {
	"/usr/src/bin",
	"/usr/src/usr.bin",
	"/usr/src/etc",
	"/usr/src/ucb",
	"/usr/src/usg",
	"/usr/src/games",
	"/usr/src/usr.lib",
	"/usr/src/lib",
	"/usr/src/local",
	"/usr/src/new",
	"/usr/src/old",
	"/usr/src/include",
	"/usr/src/lib/libc/gen",
	"/usr/src/lib/libc/stdio",
	"/usr/src/lib/libc/sys",
	"/usr/src/lib/libc/net/common",
	"/usr/src/lib/libc/net/inet",
	"/usr/src/lib/libc/net/misc",
	"/usr/src/sbin",
	"/usr/src/hosts",
	0
};

char	sflag = 1;
char	bflag = 1;
char	mflag = 1;
char	**Sflag;
int	Scnt;
char	**Bflag;
int	Bcnt;
char	**Mflag;
int	Mcnt;
char	uflag;
int	count;
int	print;


/*
 *  NAME:  whereis <name>
 *
 *  FUNCTION:  
 * 	look for source, documentation and binaries
 *	      
 */

main(argc, argv)
	int argc;
	char *argv[];
{

	int status = 0;

	(void) setlocale (LC_ALL, "");
	catd = catopen(MF_WHEREIS, NL_CAT_LOCALE);
	argc--, argv++;
	if (argc == 0) {
usage:
		fprintf(stderr, MSGSTR(USAGE, "whereis [ -sbmu ] [ -SBM dir ... -f ] name...\n"));
		exit(1);
	}
	do
		if (argv[0][0] == '-') {
			register char *cp = argv[0] + 1;
			while (*cp) switch (*cp++) {

			case 'f':
				break;

			case 'S':
				getlist(&argc, &argv, &Sflag, &Scnt);
				break;

			case 'B':
				getlist(&argc, &argv, &Bflag, &Bcnt);
				break;

			case 'M':
				getlist(&argc, &argv, &Mflag, &Mcnt);
				break;

			case 's':
				zerof();
				sflag++;
				continue;

			case 'u':
				uflag++;
				continue;

			case 'b':
				zerof();
				bflag++;
				continue;

			case 'm':
				zerof();
				mflag++;
				continue;

			default:
				goto usage;
			}
			argv++;
		} else
			status |= lookup(*argv++);
	while (--argc > 0);
exit(status);
}


/*
 *  NAME:  getlist
 *
 *  FUNCTION:  	If the -B -S -M option is specified, this routine
 *		is called and an alaternate search path will be defined.
 *		This search path is used to locate the specifed file.
 *
 *  RETURN VALUE:  none, path parameter modified.
 *
 */

static void
getlist(argcp, argvp, flagp, cntp)
	char ***argvp;
	int *argcp;
	char ***flagp;
	int *cntp;
{

	(*argvp)++;
	*flagp = *argvp;
	*cntp = 0;
	for ((*argcp)--; *argcp > 0 && (*argvp)[0][0] != '-'; (*argcp)--)
		(*cntp)++, (*argvp)++;
	(*argcp)++;
	(*argvp)--;
}

/*
 *  NAME:  zerof
 *
 *  FUNCTION:  zero all the option flags
 *
 *  RETURN VALUE:  none
 *
 */
static void
zerof()
{

	if (sflag && bflag && mflag)
		sflag = bflag = mflag = 0;
}

/*
 *  NAME:  lookup
 *
 *  FUNCTION:   Take a filename, strip off path and suffix.  Call 
 *		look{bin,src,man} to search path and print out file
 *		location.
 *
 *  RETURN VALUE:  returns zero something was found, or one if nothing found
 *
 */

int
lookup(cp)
	register char *cp;
{
	register char *dp;
	int found = 1;		/* was anything found? 0=yes 1=no */

	for (dp = cp; *dp; dp++)
		continue;
	/*
	 * check for the s. prefix before checking
  	 * for suffixes 
	 */
	if ( (*cp == 's') && (*(cp+1) == '.') )
		cp+=2;
	for (; dp > cp; dp--) {
		if (*dp == '.') {
			*dp = 0;
			break;
		}
	}
	for (dp = cp; *dp; dp++)
		if (*dp == '/')
			cp = dp + 1;
	if (uflag) {
		print = 0;
		count = 0;
	} else
		print = 1;
again:
	if (print)
		printf("%s:", cp);
	if (sflag) {
		found *= looksrc(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (bflag) {
		found *= lookbin(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (mflag) {
		found *= lookman(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	if (print)
		printf("\n");
	return (found);
}

/*
 *  NAME:  looksrc
 *
 *  FUNCTION:  Check for the file in the Source directories
 *  RETURN VALUE:  returns zero something was found, or one if nothing found
 */

int
looksrc(cp)
	char *cp;
{
	if (Sflag == 0) {
		return(find(srcdirs, cp));
	} else
		return(findv(Sflag, Scnt, cp));
}

/*
 *  NAME:  lookbin
 *
 *  FUNCTION:  Check for the file in the Binary directories
 *  RETURN VALUE:  returns zero something was found, or one if nothing found
 */

int
lookbin(cp)
	char *cp;
{
	if (Bflag == 0)
		return(find(bindirs, cp));
	else
		return(findv(Bflag, Bcnt, cp));
}

/*
 *  NAME:  lookman
 *
 *  FUNCTION:  Check for the file in the Man page directories
 *  RETURN VALUE:  returns zero something was found, or one if nothing found
 */

int
lookman(cp)
	char *cp;
{
	if (Mflag == 0) {
		return(find(mandirs, cp));
	} else
		return(findv(Mflag, Mcnt, cp));
}

/*
 *  NAME:  findv
 *
 *  FUNCTION:  	Check for file using the given path.  This is an
 *		alternate path.
 *  RETURN VALUE:  returns zero something was found, or one if nothing found
 */

int
findv(dirv, dirc, cp)
	char **dirv;
	int dirc;
	char *cp;
{
	int found = 1;

	while (dirc > 0)
		found *= findin(*dirv++, cp), dirc--;
	return (found);
}

/*
 *  NAME:  find
 *
 *  FUNCTION:  	Check for file using the given path.  
 *  RETURN VALUE:  returns zero something was found, or one if nothing found
 */

int
find(dirs, cp)
	char **dirs;
	char *cp;
{
	int found = 1;

	while (*dirs)
                found *= findin(*dirs++, cp);
	return (found);
}

/*
 *  NAME:  findin
 *
 *  FUNCTION:  	Actually check to see if the given file exists (or a like file)
 *		in the directory given.  If so, print it out.
 *  RETURN VALUE:  returns zero something was found, or one if nothing found
 */

int
findin(dir, cp)
	char *dir, *cp;
{
	DIR *dirp;
	struct dirent *dp;
	int found = 1;

	dirp = opendir(dir);
	if (dirp == NULL)
		return (found);
	while ((dp = readdir(dirp)) != NULL) {
		if (itsit(cp, dp->d_name)) {
			count++;
			found = 0;
			if (print)
				printf(" %s/%s", dir, dp->d_name);
		}
	}
	closedir(dirp);
	return (found);
}

/*
 *  NAME:  itsit
 *
 *  FUNCTION:  	Returns 1 if the current file matches the current directory
 *		entry.  0 if no match.
 */


itsit(cp, dp)
	register char *cp, *dp;
{
	register int i = strlen(dp);

	if (dp[0] == 's' && dp[1] == '.' && itsit(cp, dp+2))
		return (1);
	while (*cp && *dp && *cp == *dp)
		cp++, dp++, i--;
	if (*cp == 0 && *dp == 0)
		return (1);
	while (isdigit((int)*dp))
		dp++;
	if (*cp == 0 && *dp++ == '.') {
		--i;
		while (i > 0 && *dp)
			if (--i, *dp++ == '.')
				return (*dp++ == 'C' && *dp++ == 0);
		return (1);
	}
	return (0);
}
