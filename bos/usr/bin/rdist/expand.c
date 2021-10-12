static char sccsid[] = "@(#)99	1.8  src/bos/usr/bin/rdist/expand.c, cmdarch, bos411, 9428A410j 4/25/94 20:20:35";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
static char sccsid[] = "(#)expand.c	5.2 (Berkeley) 3/28/86";
#endif not lint
*/

#include <nl_types.h>
#include <fnmatch.h>
#include "rdist_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_RDIST,N,S)
nl_catd catd;

#include "defs.h"

#define	GAVSIZ	NCARGS / 6
#define LC '{'
#define RC '}'

int	which;		/* bit mask of types to expand */
int	eargc;		/* expanded arg count */
char	**eargv;	/* expanded arg vectors */
char	*path;
char	*pathp;
char	*lastpathp;
int	nleft;
int	mb_cur_max = 0;

char	*entp;
char	**sortbase;

char	*findch();
char	*unquote();
static int	argcmp();

#define sort()	qsort((char *)sortbase, &eargv[eargc] - sortbase, \
		      sizeof(*sortbase), argcmp), sortbase = &eargv[eargc]

#define PUTSTR(a,b)                                     \
	{                                               \
	int fclen;                                      \
	fclen = mblen(((char *)b),(mb_cur_max));        \
	do                                              \
		*a++ = *b++;                            \
	while (--fclen > 0);                            \
}

/*
 * Take a list of names and expand any macros, etc.
 * wh = E_VARS if expanding variables.
 * wh = E_SHELL if expanding shell characters.
 * wh = E_TILDE if expanding `~'.
 * or any of these or'ed together.
 *
 * Major portions of this were snarfed from csh/sh.glob.c.
 */
struct namelist *
expand(list, wh)
	struct namelist *list;
	int wh;
{
	register struct namelist *nl, *prev;
	register int n;
	char pathbuf[BUFSIZ];
	char *argvbuf[GAVSIZ];

	if (debug) {
		printf("expand(%x, %d)\nlist = ", list, wh);
		prnames(list);
	}

	if (mb_cur_max == 0)
		mb_cur_max = __getmbcurmax();
	if (wh == 0) {
		for (nl = list; nl != NULL; nl = nl->n_next)
			unquote(nl->n_name);
		return(list);
	}

	which = wh;
	path = pathbuf;
	lastpathp = &path[sizeof pathbuf - 2];
	eargc = 0;
	eargv = sortbase = argvbuf;
	*eargv = 0;
	nleft = NCARGS - 4;
	/*
	 * Walk the name list and expand names into eargv[];
	 */
	for (nl = list; nl != NULL; nl = nl->n_next)
		expstr(nl->n_name);
	/*
	 * Take expanded list of names from eargv[] and build a new list.
	 */
	list = prev = NULL;
	for (n = 0; n < eargc; n++) {
		nl = makenl(NULL);
		nl->n_name = eargv[n];
		if (prev == NULL)
			list = prev = nl;
		else {
			prev->n_next = nl;
			prev = nl;
		}
	}
	if (debug) {
		printf("expanded list = ");
		prnames(list);
	}
	return(list);
}

expstr(s)
	char *s;
{
	register char *cp, *cp1;
	register struct namelist *tp;
	char *tail;
	char buf[BUFSIZ];
	int savec, oeargc;
	extern char homedir[];

	if (s == NULL || *s == '\0')
		return;

	pathp = path;
	*pathp = '\0';
	if ((which & E_VARS) && (cp = findch(s, '$')) != NULL) {
		*cp++ = '\0';
		if (*cp == '\0') {
			yyerror(MSGSTR(NODOLLAR, "no variable name after '$'"));
			return;
		}
		if (*cp == LC) {
			cp++;
			if ((tail = findch(cp, RC)) == NULL) {
				yyerror(MSGSTR(UNMATCH, "unmatched '{'"));
				return;
			}
			*tail++ = savec = '\0';
			if (*cp == '\0') {
				yyerror(MSGSTR(NODOLLAR, "no variable name after '$'"));
				return;
			}
		} else {
			tail = cp + 1;
			savec = *tail;
			*tail = '\0';
		}
		tp = lookup(cp, NULL, 0);
		if (savec != '\0')
			*tail = savec;
		if (tp != NULL) {
			for (; tp != NULL; tp = tp->n_next) {
				sprintf(buf, "%s%s%s", s, tp->n_name, tail);
				expstr(buf);
			}
			return;
		}
		sprintf(buf, "%s%s", s, tail);
		expstr(buf);
		return;
	}
	if ((which & ~E_VARS) == 0 || !strcmp(s, "{") || !strcmp(s, "{}")) {
		Cat(unquote(s), "");
		sort();
		return;
	}
	if (*s == '~') {
		cp = ++s;
		if (*cp == '\0' || *cp == '/') {
			if (which & E_TILDE)
				cp1 = homedir;
			else
				cp1 = "~";
		} else {
			cp1 = buf;
			*cp1++ = '~';
			do
				*cp1++ = *cp++;
			while (*cp && *cp != '/');
			*cp1 = '\0';
			s = cp;
			if (which & E_TILDE) {
				if (pw == NULL || !strcmp(pw->pw_name, buf+1)) {
					if ((pw = getpwnam(buf+1)) == NULL) {
						strcat(buf, MSGSTR(USERUNK,
							": unknown user name"));
						yyerror(buf+1);
						return;
					}
				}
				cp1 = pw->pw_dir;
			} else
				cp1 = buf;
		}
		for(cp = path; *cp++ = cp1++;)
			;
		pathp = cp - 1;
	}
	if (!(which & E_SHELL)) {
		Cat(path, unquote(s));
		sort();
		return;
	}
	oeargc = eargc;
	expsh(s);
	if (eargc == oeargc)
		Cat(unquote(s), "");		/* "nonomatch" is set */
	sort();
}

static
argcmp(a1, a2)
	char **a1, **a2;
{

	return (strcmp(*a1, *a2));
}

/*
 * If there are any Shell meta characters in the name,
 * expand into a list, after searching directory
 */
expsh(s)
	char *s;
{
	register char *cp;

	if (findch(s, LC) != NULL) {
		execbrc(s);
		return;
	}
	if ((cp = findch(s, '/')) != NULL) {
		if (cp > s) {
			*cp = '\0';
			matchdir(s, cp+1);
			*cp = '/';
		} else {
			addpath("/");
			expsh(cp+1);
		}
		return;
	}
	matchdir(s, NULL);
}

matchdir(pattern, rempath)
	char *pattern, *rempath;
{
	struct stat stb;
	register struct dirent *dp;
	char *spathp;
	DIR *dirp;

	if (*path)
		dirp = opendir(path);
	else
		dirp = opendir(".");
	if (dirp == NULL)
		goto patherr2;
	if (fstat(dirp->dd_fd, &stb) < 0)
		goto patherr1;
	if (!ISDIR(stb.st_mode)) {
		errno = ENOTDIR;
		goto patherr1;
	}
	while ((dp = readdir(dirp)) != NULL)
		if (fnmatch(pattern, dp->d_name, (FNM_PATHNAME|FNM_PERIOD)) == 0) {
			if (rempath) {
				spathp = pathp;
				addpath(dp->d_name);
				addpath("/");
				expsh(rempath);
				pathp = spathp;
			} else {
				Cat(path, dp->d_name);
			}
		}
	closedir(dirp);
	return;

patherr1:
	closedir(dirp);
patherr2:
	strcat(path, ": ");
	strcat(path, strerror(errno));
	yyerror(path);
}

execbrc(p)
	char *p;
{
	char restbuf[BUFSIZ + 2];
	register char *pe, *pm, *pl;
	int brclev = 0;
	int n;
	char *lm, savec, *spathp;

	lm = restbuf;
	while(*p != '{') {
		if (*p == '\\')
			*lm++ = *p++;
		PUTSTR(lm,p);
	}
	for (pe = ++p; *pe; pe++)
		switch (*pe) {

		case '{':
			brclev++;
			continue;

		case '}':
			if (brclev == 0)
				goto pend;
			brclev--;
			continue;

		case '[':
			for (pe++; *pe && *pe != ']'; pe++) {
				if (*pe == '\\')
					pe++;
				n = mblen((char *)pe, mb_cur_max);
				if (n > 1)
					pe += n - 1;
			}
			if (!*pe)
				yyerror(MSGSTR(MISSBRK, "Missing ']'"));
			continue;
		case '\\':
			pe++;
			/* fall through */
		default:
			n = mblen((char *)pe, mb_cur_max);
			if (n > 1)
				pe += n - 1;
		}
pend:
	if (brclev || !*pe) {
		yyerror(MSGSTR(MISSBRC, "Missing '}'"));
		return (0);
	}
	for (pl = pm = p; pm <= pe; pm++)
		switch (*pm) {

		case '{':
			brclev++;
			continue;

		case '}':
			if (brclev) {
				brclev--;
				continue;
			}
			goto doit;

		case '\\':
			pm++;
			if (*pm != ',') {
				n = mblen((char *)pm, mb_cur_max);
				if ( n > 1)
					pm += n - 1;
				continue;
			}
			pm--;
		case ',':
			if (brclev)
				continue;
doit:
			savec = *pm;
			*pm = 0;
			strcpy(lm, pl);
			strcat(restbuf, pe + 1);
			*pm = savec;
			spathp = pathp;
			expsh(restbuf);
			pathp = spathp;
			*pathp = 0;
			sort();
			if (savec == '\\') pm++;
			pl = pm + 1;
			continue;

		case '[':
			for (pm++; *pm && *pm != ']'; pm++) {
				if (*pm == '\\')
					pm++;
				n = mblen((char *)pm, mb_cur_max);
				if (n > 1)
					pm += n - 1;
			}
			if (!*pm)
				yyerror(MSGSTR(MISSBRK, "Missing ']'"));
			continue;
		default:
			n = mblen((char *)pm, mb_cur_max);
			if (n > 1)
				pm += n - 1;
		}
	return (0);
}

Cat(s1, s2)
	register char *s1, *s2;
{
	int len = strlen(s1) + strlen(s2) + 1;
	register char *s;

	nleft -= len;
	if (nleft <= 0 || ++eargc >= GAVSIZ)
		yyerror(MSGSTR(ARGTOO, "Arguments too long"));
	eargv[eargc] = 0;
	eargv[eargc - 1] = s = malloc(len);
	if (s == NULL)
		fatal(MSGSTR(NOMEM, "ran out of memory\n"));

	strcpy(s, s1);
	strcat(s, s2);
}

char *
unquote(s)
	register char *s;
{
	register char *ns = s;
	char *rs = s;

	while (*s) {
		if (*s == '\\')
			s++;
		PUTSTR(ns,s);
	}
	*ns = '\0';
	return rs;
}

addpath(str)
	register char *str;
{
	/*
	 * Copy str to pathp, including NULL char.  Do not
	 * increment pathp beyond NULL char.
	 */
	while(*pathp = *str++)
		if (++pathp >= lastpathp)
			yyerror(MSGSTR(PATHTOO, "Pathname too long"));
}

/*
 * findch -- Find character ch in string str and ignore escaped chars.
 */
char *
findch(str, ch)
	register char *str, ch;
{
	int n;
	for(; *str; str++) {
		if (*str == ch)
			return str;
		if (*str == '\\')
			str++;
		if ((n = mblen((char *)str, mb_cur_max)) > 1)
			str += n - 1;
	}
	return NULL;
}

/*
 * Expand file names beginning with `~' into the
 * user's home directory path name. Return a pointer in buf to the
 * part corresponding to `file'.
 */
char *
exptilde(buf, file)
	char buf[];
	register char *file;
{
	register char *s1, *s2, *s3;
	extern char homedir[];

	if (*file != '~') {
		strcpy(buf, file);
		return(buf);
	}
	if (*++file == '\0') {
		s2 = homedir;
		s3 = NULL;
	} else if (*file == '/') {
		s2 = homedir;
		s3 = file;
	} else {
		s3 = file;
		while (*s3 && *s3 != '/')
			s3++;
		if (*s3 == '/')
			*s3 = '\0';
		else
			s3 = NULL;
		if (pw == NULL || strcmp(pw->pw_name, file) != 0) {
			if ((pw = getpwnam(file)) == NULL) {
				error(MSGSTR(USERUNK1, "%s: unknown user name\n"), file);
				if (s3 != NULL)
					*s3 = '/';
				return(NULL);
			}
		}
		if (s3 != NULL)
			*s3 = '/';
		s2 = pw->pw_dir;
	}
	for (s1 = buf; *s1++ = *s2++; )
		;
	s2 = --s1;
	if (s3 != NULL) {
		s2++;
		while (*s1++ = *s3++)
			;
	}
	return(s2);
}
