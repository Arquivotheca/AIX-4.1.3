static char sccsid[] = "@(#)16	1.7  src/bos/usr/bin/csh/dir.c, cmdcsh, bos412, 9443A412c 10/20/94 15:28:44";
/*
 * COMPONENT_NAME: CMDCSH c shell (csh) 
 *
 * FUNCTIONS: dinit dodirs dtildepr dochngd dfollow dopushd dfind dopopd dfree
 *            dcanon dnewcwd
 *
 * ORIGINS: 10,26,27,18,71 
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */
#include "sh.h"
#include "dir.h"

/*
 * Local global and static variables.
 */
struct	directory dhead;		/* "head" of loop */
int	printd;				/* force name to be printed */
static	uchar_t *fakev[] = {
				(uchar_t *)"dirs", 
				NOSTR 
				};

/*
 * dinit - initialize current working directory
 */
void
dinit(uchar_t *hp)
{
	register uchar_t *cp;
	register struct directory *dp;
	uchar_t path[PATH_MAX+1];

	if (loginsh && hp)
		cp = hp;
	else
		cp = (uchar_t *)getcwd((char *)path,sizeof(path));
	dp = (struct directory *)calloc(sizeof (struct directory), 1);
	dp->di_name = savestr(cp);
	dp->di_count = 0;
	dhead.di_next = dhead.di_prev = dp;
	dp->di_next = dp->di_prev = &dhead;
	printd = 0;
	dnewcwd(dp);
}

/*
 * dodirs - list all directories in directory loop
 */
void
dodirs(uchar_t **v)
{
	register struct directory *dp;
	bool lflag;
	uchar_t *hp = value("home");

	if (*hp == '\0')
		hp = NOSTR;
	if (*++v != NOSTR)
		if (EQ(*v, "-l") && *++v == NOSTR)
			lflag = 1;
		else
			error(MSGSTR(M_DIRS, "Usage: dirs [ -l ]"));
	else
		lflag = 0;
	dp = dcwd;
	do {
		if (dp == &dhead)
			continue;
		if (!lflag && hp != NOSTR) {
			dtildepr(hp, dp->di_name);
		} else
			printf("%s", dp->di_name);
		display_char(' ');
	} while ((dp = dp->di_prev) != dcwd);
	printf("\n");
}

void
dtildepr(uchar_t *home, uchar_t *dir)
{

	if (!EQ((char *)home, "/") && prefix(home, dir))
		printf("~%s", dir + strlen(home));
	else
		printf("%s", dir);
}

/*
 * dochngd - implement chdir command.
 */
void
dochngd(uchar_t **v)
{
	register uchar_t *cp;
	register struct directory *dp;

	printd = 0;
	if (*++v == NOSTR) {
		if ((cp = value("home")) == NOSTR || *cp == 0)
			bferr(MSGSTR(M_NOHOME, "No home directory"));
		if (chdir(cp) < 0)
			bferr(MSGSTR(M_NOCHANGE,
				"Can't change to home directory"));
		cp = savestr(cp);
	} else if ((dp = dfind(*v)) != 0) {
		printd = 1;
		if (chdir(dp->di_name) < 0)
			Perror((char *)dp->di_name);
		dcwd->di_prev->di_next = dcwd->di_next;
		dcwd->di_next->di_prev = dcwd->di_prev;
		goto flushcwd;
	} else
		cp = dfollow(*v);
	dp = (struct directory *)calloc(sizeof (struct directory), 1);
	dp->di_name = cp;
	dp->di_count = 0;
	dp->di_next = dcwd->di_next;
	dp->di_prev = dcwd->di_prev;
	dp->di_prev->di_next = dp;
	dp->di_next->di_prev = dp;
flushcwd:
	dfree(dcwd);
	dnewcwd(dp);
}

/*
 * dfollow - change to arg directory; fall back on cdpath if not valid
 */

extern int errno;

uchar_t *
dfollow(uchar_t *cp)
{
	register uchar_t *dp;
	struct varent *c;
	
	cp = globone(cp);
	if (chdir(cp) >= 0)
		goto gotcha;

	/* check if a component of the path is not a directory (i.e. a file)
	   or has had search permission denied */
	if ((errno == ENOTDIR) || (errno == EACCES))
	  Perror_free((char *)cp);

	if (cp[0] != '/' && !prefix((uchar_t *)"./", cp) 
	    && !prefix((uchar_t *)"../", cp)
	    && (c = adrof("cdpath"))) {
		uchar_t **cdp;
		register uchar_t *p;

		for (cdp = c->vec; *cdp; cdp++) {
			uchar_t buf[PATH_MAX+1];
			unsigned int len = strlen(cdp) + strlen(cp) +1;
			if (len <= PATH_MAX) {
				strcpy(buf, *cdp);
				strcat(buf, "/");
				strcat(buf, cp);
				if (chdir(buf) == 0) {
					printd = 1;
					xfree(cp);
					cp = savestr(buf);
					goto gotcha;
				} else {
			   		/* check if a component of the path is
					   not a dir (i.e. a file) or has had 
					   search permission denied */
			   		if ((errno==ENOTDIR) || (errno==EACCES))
			     			Perror((char *)buf);
				}
			}
		}
	}
	if (adrof((char *)cp)) {
		dp = value((char *)cp);

		if (dp[0] == '/' || dp[0] == '.')
			if (chdir(dp) >= 0) {
				xfree(cp);
				cp = savestr(dp);
				printd = 1;
				goto gotcha;
			}
	}
	Perror_free((char *)cp);

gotcha:
        if (*cp != '/') {
                register uchar_t *p, *q;
                int cwdlen;

                /*
                 * All in the name of efficiency?
                 */
                for (p = dcwd->di_name; *p++;)
                        ;
                if ((cwdlen = p - dcwd->di_name - 1) == 1)      /* root */
                        cwdlen = 0;
                for (p = cp; *p++;)
                        ;
                dp = calloc((cwdlen + (p - cp) + 2),1);
                for (p = dp, q = dcwd->di_name; *p++ = *q++;)
                        ;
                if (cwdlen)
                        p[-1] = '/';
                else
                        p--;                    /* don't add a / after root */
                for (q = cp; *p++ = *q++;)
                        ;
                xfree(cp);
                cp = dp;
                dp += cwdlen;
        } else
                dp = cp;
        return(dcanon(cp, dp));

}

/*
 * dopushd - push new directory onto directory stack.
 *	with no arguments exchange top and second.
 *	with numeric argument (+n) bring it to top.
 */
void
dopushd(uchar_t **v)
{
	register struct directory *dp;

	printd = 1;
	if (*++v == NOSTR) {
		if ((dp = dcwd->di_prev) == &dhead)
			dp = dhead.di_prev;
		if (dp == dcwd)
			bferr(MSGSTR(M_NODIR, "No other directory"));
		if (chdir(dp->di_name) < 0)
			Perror((char *)dp->di_name);
		dp->di_prev->di_next = dp->di_next;
		dp->di_next->di_prev = dp->di_prev;
		dp->di_next = dcwd->di_next;
		dp->di_prev = dcwd;
		dcwd->di_next->di_prev = dp;
		dcwd->di_next = dp;
	} else if (dp = dfind(*v)) {
		if (chdir(dp->di_name) < 0)
			Perror((char *)dp->di_name);
	} else {
		register uchar_t *cp;

		cp = dfollow(*v);
		dp = (struct directory *)calloc(sizeof (struct directory), 1);
		dp->di_name = cp;
		dp->di_count = 0;
		dp->di_prev = dcwd;
		dp->di_next = dcwd->di_next;
		dcwd->di_next = dp;
		dp->di_next->di_prev = dp;
	}
	dnewcwd(dp);
}

/*
 * dfind - find a directory if specified by numeric (+n) argument
 */
struct directory *
dfind(uchar_t *cp)
{
	register struct directory *dp;
	register int i;
	register uchar_t *ep;

	if (*cp++ != '+')
		return (0);
	for (ep = cp; digit(*ep); ep++)
		continue;
	if (*ep)
		return (0);
	i = getn(cp);
	if (i <= 0)
		return (0);
	for (dp = dcwd; i != 0; i--) {
		if ((dp = dp->di_prev) == &dhead)
			dp = dp->di_prev;
		if (dp == dcwd)
			bferr(MSGSTR(M_STACK,"Directory stack not that deep"));
	}
	return (dp);
}

/*
 * dopopd - pop a directory out of the directory stack
 *	with a numeric argument just discard it.
 */
void
dopopd(uchar_t **v)
{
	register struct directory *dp, *p;

	printd = 1;
	if (*++v == NOSTR)
		dp = dcwd;
	else if ((dp = dfind(*v)) == 0)
		bferr(MSGSTR(M_BADDIR, "Bad directory"));
	if (dp->di_prev == &dhead && dp->di_next == &dhead)
		bferr(MSGSTR(M_EMPTY, "Directory stack empty"));
	if (dp == dcwd) {
		if ((p = dp->di_prev) == &dhead)
			p = dhead.di_prev;
		if (chdir(p->di_name) < 0)
			Perror((char *)p->di_name);
	}
	dp->di_prev->di_next = dp->di_next;
	dp->di_next->di_prev = dp->di_prev;
	if (dp == dcwd)
		dnewcwd(p);
	else
		dodirs(fakev);
	dfree(dp);
}

/*
 * dfree - free the directory (or keep it if it still has ref count)
 */
void
dfree(struct directory *dp)
{

	if (dp->di_count != 0)
		dp->di_next = dp->di_prev = 0;
	else {
		xfree(dp->di_name);
		xfree((uchar_t *)dp);
	}
}

/*
 * dcanon - canonicalize the pathname, removing excess ./ and ../ etc.
 *	we are of course assuming that the file system is standardly
 *	constructed (always have ..'s, directories have links)
 */
uchar_t *
dcanon(uchar_t *cp, uchar_t *p)
{
	register uchar_t *sp;
	register uchar_t *p1, *p2;		/* general purpose */
	bool slash;

	if (*cp != '/')
		abort();
	while (*p) {			/* for each component */
		sp = p;			/* save slash address */
		while (*++p == '/')	/* flush extra slashes */
			;
		if (p != ++sp)
			strcpy(sp, p);
		p = sp - 1;			/* save start of component */
		slash = 0;
		while (*++p)		/* find next slash or end of path */
			if (*p == '/') {
				slash = 1;
				*p = 0;
				break;
			}
		if (*sp == '\0')	/* if component is null */
			if (--sp == cp)	/* if path is one char (i.e. /) */
				break;
			else
				*sp = '\0';
		else if (sp[0] == '.' && sp[1] == 0) {
			if (slash) {
				strcpy(sp, ++p);
				p = --sp;
			} else if (--sp != cp)
				*sp = '\0';
		} else if (sp[0] == '.' && sp[1] == '.' && sp[2] == 0) {
			uchar_t link[PATH_MAX+1];
			int cc;
			uchar_t *newcp;

			/*
			 * We have something like "yyy/xxx/..", where "yyy"
			 * can be null or a path starting at /, and "xxx"
			 * is a single component.
			 * Before compressing "xxx/..", we want to expand
			 * "yyy/xxx", if it is a symbolic link.
			 */
			*--sp = 0;	/* form the pathname for readlink */
			if (sp != cp &&
			    (cc = readlink(cp, link, sizeof link)) >= 0) {
				link[cc] = '\0';
				if (slash)
					*p = '/';
				/*
				 * Point p to the '/' in "/..", and restore
				 * the '/'.
				 */
				*(p = sp) = '/';
				/*
				 * find length of p
				 */
				for (p1 = p; *p1++;)
					;
				if (*link != '/') {
					/*
					 * Relative path, expand it between
					 * the "yyy/" and the "/..".
					 * First, back sp up to the character
					 * past "yyy/".
					 */
					while (*--sp != '/')
						;
					sp++;
					*sp = 0;
					/*
					 * New length is
					 * "yyy/" + link + "/.." and rest
					 */
					p1 = newcp = malloc(
					(sp - cp) + cc + (p1 - p));

					/*
					 * Copy new path into newcp
					 */
					for (p2 = cp; *p1++ = *p2++;)
						;
					for (p1--, p2 = link; *p1++ = *p2++;)
						;
					for (p1--, p2 = p; *p1++ = *p2++;)
						;
					/*
					 * Restart canonicalization at
					 * expanded "/xxx".
					 */
					p = sp - cp - 1 + newcp;
				} else {
					/*
					 * New length is link + "/.." and rest
					 */
					p1 = newcp = malloc((cc + (p1 - p)));

					/*
					 * Copy new path into newcp
					 */
					for (p2 = link; *p1++ = *p2++;)
						;
					for (p1--, p2 = p; *p1++ = *p2++;)
						;
					/*
					 * Restart canonicalization at beginning
					 */
					p = newcp;
				}
				xfree(cp);
				cp = newcp;
				continue;	/* canonicalize the link */
			}
			*sp = '/';
			if (sp != cp)
				while (*--sp != '/')
					;
			if (slash) {
				for (p1 = sp + 1, p2 = p + 1; *p1++ = *p2++;)
					;
				p = sp;
			} else if (cp == sp)
				*++sp = '\0';
			else
				*sp = '\0';
		} else if (slash)
			*p = '/';
	}
	return cp;
}


/*
 * dnewcwd - make a new directory in the loop the current one
 */
void
dnewcwd(struct directory *dp)
{

	dcwd = dp;
	set_noglob("cwd", savestr(dcwd->di_name));
	if (printd)
		dodirs(fakev);
}
