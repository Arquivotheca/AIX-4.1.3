static char sccsid[] = "@(#)21	1.14  src/bos/usr/bin/csh/exec.c, cmdcsh, bos411, 9428A410j 11/12/92 13:32:35";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: doexec pexerr texec execash xechoit dohash dounhash hashstat 
 *            hashname
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1992
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
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include "pathnames.h"

/*
 * System level search and execute of a command.
 * We look in each directory for the specified command name.
 * If the name contains a '/' then we execute only the full path name.
 * If there is no search path then we execute only full path names.
 * 
 * As we search for the command we note the first non-trivial error
 * message for presentation to the user.  This allows us often
 * to show that a file has the wrong mode/no access when the file
 * is not in the last component of the search path, so we must
 * go on after first detecting the error.
 */

uchar_t	*exerr;			/* Execution error message */
uchar_t	*expath;		/* Path for exerr */

/*
 * Xhash is an array of HSHSIZ uchar_ts, which are used to hash execs.
 * If it is allocated, then to tell whether ``name'' is (possibly)
 * present in the i'th component of the variable path, you look at
 * the i'th bit of xhash[hash("name")].  This is setup automatically
 * after .login is executed, and recomputed whenever ``path'' is
 * changed.
 * The two part hash function is designed to let texec() call the
 * more expensive hashname() only once and the simple hash() several
 * times (once for each path component checked).
 * Byte size is assumed to be 8.
 */
#define	HSHSIZ		8192			/* 1k bytes */
#define HSHMASK		(HSHSIZ - 1)
#define HSHMUL		243
char	xhash[HSHSIZ / 8];
#define hash(a, b)	((a) * HSHMUL + (b) & HSHMASK)
#define bit(h, b)	((h)[(b) >> 3] & 1 << ((b) & 7))	/* bit test */
#define bis(h, b)	((h)[(b) >> 3] |= 1 << ((b) & 7))	/* bit set */

int	havhash;
static int	hits, misses;

/* Dummy search path for just absolute search when no path */
static uchar_t *justabs[] = { (uchar_t *)"", 0 };

static void			pexerr(void);
static void			texec(uchar_t *,uchar_t **);
static int			hashname(uchar_t *);

void
doexec(struct command *t)
{
	uchar_t *sav;
	register uchar_t *dp, **pv, **av;
	register struct varent *v;
	bool slash = (any('/', t->t_dcom[0]) != NULL);
	int hashval, hashval1, i;
	uchar_t *blk[2];

	/*
	 * Glob the command name.  If this does anything, then we
	 * will execute the command only relative to ".".  One special
	 * case: if there is no PATH, then we execute only commands
	 * which start with '/'.
	 */
	sav = t->t_dcom[0];
	dp = globone(t->t_dcom[0]);
	exerr = 0; expath = t->t_dcom[0] = dp;
	xfree(sav);
	v = adrof("path");
	if (v == 0 && expath[0] != '/')
		pexerr();
	slash |= gflag;

	/*
	 * Glob the argument list, if necessary.
	 * Otherwise trim off the quote bits.
	 */
	gflag = 0; av = &t->t_dcom[1];
	rscan(av, tglob);
	if (gflag) {
		av = glob(av);
		if (av == 0)
			error(MSGSTR(M_NOMATCH, "No match"));
	}
	scan(av, trim);
	blk[0] = t->t_dcom[0];
	blk[1] = 0;
	av = blkspl(blk, av);

	xechoit(av);		/* Echo command if -x */
	closech();		/* Close random fd's */

	/*
	 * We must do this after any possible forking (like `foo`
	 * in glob) so that this shell can still do subprocesses.
	 */
	(void) sigsetmask(0L);

	/*
	 * If no path, no words in path, or a / in the filename
	 * then restrict the command search.
	 */
	if (v == 0 || v->vec[0] == 0 || slash)
		pv = justabs;
	else
{
#ifdef DEBUG
uchar_t **P;
#endif
		pv = v->vec;
#ifdef DEBUG
for (P=pv; P&&*P&&**P;P++)
	printf("doexec: pv=%s\n", *P);
#endif
}
	/* command name for postpending */
	sav = (uchar_t *)strspl((uchar_t *)"/", *av);
	if (havhash) {
		hashval = hashname(*av);
#ifdef DEBUG
printf("doexec: *av=%s, hashname(*av)=%d, xhash[hashname(*av)]=%d\n",
	*av, hashname(*av), xhash[hashname(*av)]);
#endif
}
	hits++;
	i = 0;
	do {
#ifdef DEBUG
printf("doexec: slash=%d, havhash=%d, *pv=%s, hashval=%d, i=%d\n",
        slash, havhash, *pv, hashval, i);
#endif

		if (!slash && pv[0][0] == '/' && havhash) { 
			hashval1 = hash(hashval, i);
			if (!bit(xhash, hashval1))
				goto cont;
		}
		if (pv[0][0] == 0 || EQ((char *)pv[0], "."))	/* don't make ./xxx */
{
#ifdef DEBUG
printf("doexec: *av=%s\n", *av);
#endif
			texec(*av, av);
}
		else {
			dp = (uchar_t *)strspl(*pv, sav);
			texec(dp, av);
			xfree(dp);
		}
cont:
		pv++;
		i++;
		misses++;
	} while (*pv);
	hits--;
	xfree(sav);
	xfree((uchar_t *)av);
	pexerr();
}

static void
pexerr(void)
{

	/* Couldn't find it */
	setname(expath);
	/* xfree(expath); */
	if (exerr)
		bferr((char *)exerr);
	bferr(MSGSTR(M_NOTFOUND, "Command not found"));
}

/*
 * Execute command f, arg list t.
 * Record error message if not found.
 * Also do shell scripts here.
 */
static void
texec(uchar_t *f,uchar_t **t)
{
	register struct varent *v;
	register uchar_t **vp;
	uchar_t *lastsh[2];

#ifdef DEBUG
printf("texec: %s\n", f);
#endif

	execv(f, t);
	switch (errno) {

	case ENOEXEC:
		/*
		 * If there is an alias for shell, then
		 * put the words of the alias in front of the
		 * argument list replacing the command name.
		 * Note no interpretation of the words at this point.
		 */
		v = adrof1("shell", &aliases);
		if (v == 0) {
			register int ff = open((char *)f, O_RDONLY);
			char ch;
		
			vp = lastsh;
			vp[0] = adrof("shell") ? 
				value("shell") : (uchar_t *)_PATH_CSHELL;
			vp[1] = (uchar_t *)NULL;
			if (ff != -1 && read(ff, &ch, 1) == 1 && ch != '#')
				vp[0] = (uchar_t *)_PATH_BSHELL;
			(void)close(ff);
		} else
			vp = v->vec;
		t[0] = f;
		t = blkspl(vp, t);		/* Splice up the new arglst */
		f = *t;
		execv(f, t);
		xfree((uchar_t *)t);
		/* The sky is falling, the sky is falling! */
		/* FALL THROUGH */
	case ENOMEM:
		Perror((char *)f);
		break;

	case E2BIG:			/* arg list too big, it'll never work */
		Perror((char *)t[0]);	/* use basename of command */
		break;

	case ENOENT:
		break;

	default:
		if (exerr == 0) {
			exerr = (uchar_t *)strerror(errno);
			expath = savestr(f);
		}
	}
}

void
execash(int t, struct command *kp)
{
	struct sigvec nsv;

	didcch++;
	nsv.sv_handler = parintr;
	nsv.sv_mask = 0;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
	(void)sigvec(SIGQUIT, &nsv, (struct sigvec *)NULL);
	 /* if doexec loses, screw */
	nsv.sv_handler = parterm;
	(void)sigvec(SIGTERM, &nsv, (struct sigvec *)NULL);
	lshift(kp->t_dcom, 1);
	exiterr++;
#ifdef DEBUG
printf("debug: execash calls doexec\n");
#endif
	doexec(kp);
	/*NOTREACHED*/
}

void
xechoit(uchar_t **t)
{

	if (adrof("echo")) {
		flush();
		haderr = 1;
		blkpr(t);
		printf("\n");
		haderr = 0;
	}
}

/*VARARGS0*//*ARGSUSED*/
void
dohash(void)
{
	struct stat stb;
	DIR *dirp;
	register struct dirent *dp;
	register int cnt;
	int i = 0;
	struct varent *v = adrof("path");
	uchar_t **pv;
	int hashval;

	havhash = 1;
	for (cnt = 0; cnt < sizeof xhash; cnt++)
		xhash[cnt] = 0;
	if (v == 0)
		return;
	for (pv = v->vec; *pv; pv++, i++) {
		if (pv[0][0] != '/')
			continue;
		dirp = opendir((char *)*pv);
		if (dirp == NULL)
			continue;
		while ((dp = readdir(dirp)) != NULL) {
			if (dp->d_ino == 0)
				continue;
			if (dp->d_name[0] == '.' &&
			    (dp->d_name[1] == '\0' ||
			     dp->d_name[1] == '.' && dp->d_name[2] == '\0'))
				continue;
			hashval = hash(hashname((uchar_t*)dp->d_name), i);
			bis(xhash, hashval);
		}
		closedir(dirp);
	}
}

void
dounhash(void)
{

	havhash = 0;
}

void
hashstat(void)
{

	if (hits+misses)
		printf(MSGSTR(M_HITMISS,"%d hits, %d misses, %d%%\n"),
			hits, misses, 100 * hits / (hits + misses));
}

static int
hashname(uchar_t *cp)
{
	register long h = 0;

	while (*cp)
		h = hash(h, *cp++);
	return ((int) h);
}
