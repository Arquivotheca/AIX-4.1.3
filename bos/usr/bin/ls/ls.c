static char sccsid[] = "@(#)45	1.21.1.37  src/bos/usr/bin/ls/ls.c, cmdfiles, bos41J, 9523A_all 6/1/95 21:12:41";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: main, pdirectory, pem, pentry, pmode, ls_select, column,
 *            new_line, readdirs, gstat, makename, getname, cache_hit,
 *            add_cache, compar, pprintf, savestr
 *
 * ORIGINS: 3, 10, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*	Copyright (c) 1984 AT&T	*/
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 */
/*	  All Rights Reserved  	*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/* static char sccsid[] = "ls.c	1.24"; */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * char copyright[] =
 *  " Copyright (c) 1980 Regents of the University of California.\n\
 * All rights reserved.\n";
 * static char sccsid[] = "ls.c	5.9 (Berkeley) 10/22/87";
 *
 * ls.c	1.16  com/cmd/fs/progs,3.1,9021 4/10/90 09:39:27
 */
#define _ILS_MACROS
#include	<stdlib.h>
#include	<time.h>
#include	<sys/types.h>
#include 	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<stdio.h>
#include 	<ctype.h>
#include	<dirent.h>
#include	<pwd.h>
#include	<grp.h>
#include	<locale.h>
#include	<sys/ioctl.h>
#include	<nl_types.h>

#include        "ls_msg.h"

#define ISARG   0x8000 /* this bit equals 1 in lflags of structure lbuf 
                        *  if *namep is to be used;
                        */

#define	NAME_UID	0	/* pass to getname for user name */
#define NAME_GID	1	/*  "   "     "     "  group  "  */

#define MSGSTR(num,str) catgets(catd,MS_LS,num,str)  /*MSG*/

struct	lbuf	{
	union	{			 /* used for filename in a directory */
		char	lname[FILENAME_MAX+1];
		char	*namep;          /* for name in ls-command; */

	} ln;
	char	ltype;  	/* filetype */
	ino_t 	lnum;		/* inode number of file */
	mode_t	lflags; 	/* 0777 bits used as r,w,x permissions */
	char	lemode;		/* extended attributes: ACL, TP or TCB */
	short	lnl;    	/* number of links to file */
	uid_t	luid;   	/* local uid */
	gid_t	lgid;   	/* local gid */
	off_t	lsize;  	/* filesize or major/minor dev numbers */
	time_t	lmtime;
	char	*flinkto;	/* symbolic link value */
	mode_t	flinktype;	/* filetype (mode) of obj ref-d by symlink */
	ulong_t	lblkbytes;	/* number of allocated blocks */
};

struct dchain {
	char *dc_name;		/* path name */
	struct dchain *dc_next;	/* next directory in the chain */
};

static struct dchain *dfirst;	/* start of the dir chain */
static struct dchain *cdfirst;	/* start of the durrent dir chain */
static struct dchain *dtemp;	/* temporary - used for linking */
static char *curdir;		/* the current directory */

static int	nfiles = 0;	/* number of flist entries in current use */
static int	nargs = 0;	/* number of flist entries used for arguments */
static int	maxfils = 0;	/* number of flist/lbuf entries allocated */
static int	maxn = 0;	/* number of flist entries with lbufs assigned */
static int	quantn = 1024;	/* allocation growth quantum */

static struct	lbuf	*nxtlbf;	/* pointer to next lbuf to be assigned */
static struct	lbuf	**flist;	/* pointer to list of lbuf pointers */
static struct	lbuf	*gstat();
static char		*savestr();

static ulong_t		readdirs();
static int     aflg, bflg, cflg, dflg, fflg, gflg, iflg, lflg, mflg;
static int     nflg, oflg, pflg, qflg, sflg, tflg, uflg, xflg;
static int	Aflg, Cflg, Fflg, Rflg;
static int	eflg=0;		/* Display the mode */
static int	rflg = 1;   /* initialized to 1 for special use in compar() */
static int     Lflg = 0;   /* Determine if Lflg was used in cmd. line.     */
static int	use_lstat = FALSE;  /* Default is to follow symlinks by calling */
			    /*  stat() instead of lstat().              */

static int	terminal;	/* indicates whether output device is a terminal */
static int     flags;
static int	err = 0;	/* Contains return code */

static char	*dmark;	/* Used if -p option active. Contains "/" or NULL. */
		/* or if -F option active: "/" or "@" or "=" or "*" or NULL. */

static int	statreq;    /* is > 0 if any of sflg, (n)lflg, tflg are on */

static char	*dotp = ".";
static char	*makename();
static char	tbufu[16], tbufg[16];   /* assumed 15=max. length of user/group name*/

#define REPORT_BSIZE	UBSIZE 
				
#define BLOCK_SIZE	(sflg? 1024:UBSIZE) 	
#define BLOCKS(x)	( x.st_blocks * DEV_BSIZE / BLOCK_SIZE)

static time_t	year, now;

static int	num_cols;
static int	colwidth;
static int	filewidth;
static int	fixedwidth;
static int	curcol;
static int	len;
static int     compar();
/* int     compar(struct lbuf **pp1, struct lbuf **pp2); */
static int	mb_cur_max;
static int	first_is_a_dir;	/* is the first entry to print a directory? */

static nl_catd catd;   /* message catalog descriptor */

main(argc, argv)
int argc;
char *argv[];
{
	struct	winsize	win;		/* window size structure */
	int	amino, opterr=0;
	int	c;
	register struct lbuf *ep;
	struct	lbuf	lb;
	int	i, width;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_LS, NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;

	lb.lmtime = time((time_t *) NULL);
	year = lb.lmtime - 6L*30L*24L*60L*60L; /* 6 months ago */
	now = lb.lmtime + 60;
	Aflg = !getuid();

	if (isatty(1)) {
		terminal = Cflg = 1;
	} 
	while ((c=getopt(argc, argv, "1ARadCxmnleogrtucpFbqisfLN")) != EOF)
		switch(c) {
		case '1':
			Cflg = 0; 
			continue;
		case 'A':
			Aflg++; 
			continue;
		case 'R':
			Rflg++;
			statreq++;
			continue;
		case 'a':
			aflg++;
			continue;
		case 'd':
			dflg++;
			continue;
		case 'C':
			Cflg = 1;
			mflg = 0;
			lflg = 0;
			continue;
		case 'x':
			xflg = 1;
			Cflg = 1;
			lflg = 0;
			mflg = 0;
			continue;
		case 'm':
			Cflg = 0;
			mflg = 1;
			lflg = 0;
			continue;
		case 'N':
                        Lflg = 0;	/* see comments on symlinks below */
			use_lstat = TRUE;	/* Don't follow links */
                        continue;
		case 'n':
			nflg++;
		case 'l':
			mflg = 0;
			lflg++;		/* see comments on symlinks below */
			xflg = 0;
			use_lstat = TRUE;	/* Don't follow links */
			statreq++;
			continue;
		case 'e':
			eflg++;
			lflg++;
			use_lstat = TRUE;	/* Don't follow links */
			statreq++;
			continue;
		case 'o':
			oflg++;
			statreq++;
			continue;
		case 'g':
			gflg++;
			statreq++;
			continue;
		case 'r':
			rflg = -1;
			continue;
		case 't':
			tflg++;
			statreq++;
			continue;
		case 'u':
			uflg++;
			cflg = 0;
			continue;
		case 'c':
			cflg++;
			uflg = 0;
			continue;
		case 'p':
			pflg++;
			statreq++;
			continue;
		case 'F':
			Fflg++;
			statreq++;
			continue;
		case 'b':
			bflg = 1;
			qflg = 0;
			continue;
		case 'q':
			qflg = 1;
			bflg = 0;
			continue;
		case 'i':
			iflg++;
			statreq++;
			continue;
		case 's':
			sflg++;
			statreq++;
			continue;
		case 'f':
			fflg++;
			continue;
		case 'L':
                        Lflg++;		/* see comments on symlinks below */
			continue;
		case '?':
			opterr++;
			continue;
		}

	if (Lflg)
	 	use_lstat = FALSE;	/* Follow links--incase of "ls -lL" */

	/* Cases for symlink behavior: 					     */
	/*   ls -l	:	Don't follow link	:  use_lstat = TRUE  */
	/*   ls -Ll	:	Follow link		:  use_lstat = FALSE */
	/*   ls -L	:	Follow link		:  use_lstat = FALSE */
	/*   ls -N	:       Don't follow link	:  use lstat = TRUE  */
	/*   if NEITHER -l, -L, nor -N are used, default is to follow links  */
	/*      which means that use_lstat = FALSE.			     */
	/*   if -L and -N are both used, then the last one will be used.     */
	/*   if a traling slash is used (e.g. "ls -N link2file/") the link   */
	/*      will be followed regardless of any other options.   	     */

	if(opterr) {
		fprintf(stderr, MSGSTR(USAGE,
			"usage: ls [-1ACFLNRabcdefgilmnopqrstux] [File...]\n"));
		exit(2);
	}

	if (oflg && gflg) {		/* Behavior of -g & -o ==> AT&T SVR4  */
		oflg = gflg = 0;	/* -lgo & -go  =>  omit group & owner */
		lflg = 1;		/* and show in long format 	      */
		}
	else if (oflg || gflg) {
		lflg = 1;		/* (-o = -lo) & (-g = -gl)	      */
		}
	else if(lflg)
		oflg = gflg = 1;	/* -l  =>  show both group & owner    */

	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		rflg = 1;
		statreq = 0;
	}

	fixedwidth = 2;
	if (pflg || Fflg)
		fixedwidth++;
	if (iflg)
		fixedwidth += 6;
	if (sflg)
		fixedwidth += 5;

	if (lflg) {                             /* This is the way  */
		Cflg = mflg = 0;
	}

	if (Cflg || mflg) {
		char *clptr;
				/* assume 80 column output */
		num_cols = 80;
				/* check for user environment override */
		if ((clptr = getenv("COLUMNS")) != NULL)
			num_cols = atoi(clptr);
				/* otherwise check for tty window size */
		else if (terminal) {
			if ((ioctl(1, TIOCGWINSZ, &win) != -1) &&
					(win.ws_col != 0)) /* not undefined */
				num_cols = win.ws_col;
		}
				/* assume outrageous values are errors */
		if (num_cols < 20 || num_cols > 400)
			num_cols = 80;
	}

	/* allocate space for flist and the associated	*/
	/* data structures (lbufs)			*/
	maxfils = quantn;
	if ( (flist = (struct lbuf **)malloc(
			(size_t)(maxfils * sizeof(struct lbuf *)))) == NULL
		|| (nxtlbf = (struct lbuf *)malloc(
			(size_t)(quantn * sizeof(struct lbuf)))) == NULL) {
		fprintf(stderr, MSGSTR(NOMEMORY, "ls: out of memory\n"));
		exit(2);
	}
	if ((amino=(argc-optind))==0) { /* case when no names are given
					* in ls-command and current 
					* directory is to be used 
 					*/
		argv[optind] = dotp;
	}

	for (i=0; i < (amino ? amino : 1); i++) {
		if (Cflg || mflg) {
			width = (len = mbswidth(argv[optind], strlen(argv[optind]))) == -1 ? 1 : len;
			if (width > filewidth)
				filewidth = width;
		}

		if ((ep = gstat((*argv[optind] ? argv[optind] : dotp), 1, NULL)) ==NULL)
		{
			err = 2;
			optind++;
			continue;
		}
		ep->ln.namep = (*argv[optind] ? argv[optind] : dotp);
		ep->lflags |= ISARG;
		optind++;
		nargs++;	/* count good arguments stored in flist */
	}
	colwidth = fixedwidth + filewidth;
	qsort( (void *)flist, (size_t)nargs, (size_t)sizeof(struct lbuf *), (int(*)()) compar);
	for (i=0; i<nargs; i++)
		if (flist[i]->ltype=='d' && dflg==0 || fflg)
			break;
	if (flist[0]->ltype=='d')
		first_is_a_dir = 1;
	pem(&flist[0],&flist[i], 0);
	for (; i<nargs; i++) {
		pdirectory(flist[i]->ln.namep, (amino>1), nargs);
		/* -R: print subdirectories found */
		while (dfirst || cdfirst) {
			/* Place direct subdirs on front in right order */
			while (cdfirst) {
				/* reverse cdfirst onto front of dfirst */
				dtemp = cdfirst;
				cdfirst = cdfirst -> dc_next;
				dtemp -> dc_next = dfirst;
				dfirst = dtemp;
			}
			/* take off first dir on dfirst & print it */
			dtemp = dfirst;
			dfirst = dfirst->dc_next;
			pdirectory (dtemp->dc_name, 1, nargs);
			free ((void *)dtemp->dc_name);
			free ((void *)dtemp);
		}
	}
	if (fclose(stdout) == EOF) {
		perror("ls");
		err = 1;
	}
	exit(err);
	/* NOTREACHED */
}

/*
 * pdirectory: print the directory name, labelling it if title is
 * nonzero, using lp as the place to start reading in the dir.
 */
static pdirectory (name, title, lp)
char *name;
int title;
int lp;
{
	register struct dchain *dp;
	register struct lbuf *ap;
	register char *pname;
	register int j;
	ulong_t	tKblkbytes;
	filewidth = 0;
	curdir = name;
	if (title) {
		if (first_is_a_dir)
			first_is_a_dir = 0;
		else
			putc('\n', stdout);
		pprintf(name, ":");
		new_line();
	}
	nfiles = lp;
	tKblkbytes = readdirs(name);
	if (fflg==0)
		qsort((void *)&flist[lp], (size_t)(nfiles - lp), (size_t)sizeof(struct lbuf *), (int(*)()) compar);
	if (Rflg) for (j = nfiles - 1; j >= lp; j--) {
		ap = flist[j];
		if (ap->ltype == 'd' &&
				strcmp(ap->ln.lname, ".") &&
				strcmp(ap->ln.lname, "..")) {
			dp = (struct dchain *)calloc((size_t)1,(size_t)sizeof(struct dchain));
			if (dp == NULL)
				fprintf(stderr, 
					MSGSTR(NOMEMORY,"ls: out of memory\n"));
			pname = makename(curdir, ap->ln.lname);
			dp->dc_name = (char *)calloc((size_t)1,(size_t)(strlen(pname)+1));
			if (dp->dc_name == NULL) {
				fprintf(stderr, 
					MSGSTR(NOMEMORY,"ls: out of memory\n"));
				free((void *)dp);
			}
			else {
				strcpy(dp->dc_name, pname);
				dp -> dc_next = dfirst;
				dfirst = dp;
			}
		}
	}
	if (lflg || sflg)
		curcol += printf(MSGSTR(TOTAL, "total %ld"), tKblkbytes);
	pem(&flist[lp],&flist[nfiles],lflg||sflg);
}

/*
 * pem: print 'em.  Print a list of files (e.g. a directory) bounded
 * by slp and lp.
 */
static pem(slp, lp, tot_flag)
	register struct lbuf **slp, **lp;
	int tot_flag;
{
	int ncols, nrows, row, col;
	register struct lbuf **ep;

	if (Cflg || mflg)
		ncols = num_cols / colwidth;
	else
		ncols = 1;

	if (ncols <= 1 || mflg || xflg || !Cflg) {
		for (ep = slp; ep < lp; ep++)
			pentry(*ep);
		new_line();
		return;
	}
	/* otherwise print -C columns */
	if (tot_flag)
		slp--;
	nrows = (lp - slp - 1) / ncols + 1;
	for (row = 0; row < nrows; row++) {
		col = (row == 0 && tot_flag);
		for (; col < ncols; col++) {
			ep = slp + (nrows * col) + row;
			if (ep < lp)
				pentry(*ep);
		}
		new_line();
	}
}

static pentry(ap)  /* print one output entry;
            *  if uid/gid is not found in the appropriate
            *  file (passwd/group), then print uid/gid instead of 
            *  user/group name;
            */
struct lbuf *ap;
{
	register struct lbuf *p;
	struct tm *timestr;
	char timebuf [NLTBMAX];
	static char *lc_time;
	static int datewid = 0, ldatewid, sdatewid;
	p = ap;
	column();
	if (iflg)
		if (mflg && !lflg)
			curcol += printf("%u ", p->lnum);
		else
			curcol += printf("%5u ", p->lnum);
	if (sflg)
		curcol += printf( (mflg && !lflg) ? "%ld " : "%4ld " ,
				p->lblkbytes);
				/* p->lblkbytes / REPORT_BSIZE); */
	if (lflg) {
		putchar(p->ltype);
		curcol++;
		pmode(p->lflags);
		if (eflg) {
			putchar(p->lemode);
			curcol++;
		}
		curcol += printf("%4u ", p->lnl);
		if (oflg)
			   if(!nflg && getname(p->luid, tbufu, NAME_UID)==0)
				curcol += printf("%-8.8s ", tbufu);
			else
				curcol += printf("%-8u ", p->luid);
		if (gflg)
			if(!nflg && getname(p->lgid, tbufg, NAME_GID)==0)
				curcol += printf("%-8.8s ", tbufg);
			else
				curcol += printf("%-8u ", p->lgid);
		if (p->ltype=='b' || p->ltype=='c')
			curcol += printf("%3d,%3d", major((int)p->lsize), minor((int)p->lsize));
		else
			curcol += printf("%7lu", p->lsize);
		timestr = localtime (&p->lmtime);
		/* determine width of date column if not known */
		if (!datewid) {
			/* get what LC_TIME is set to */
			lc_time = setlocale(LC_TIME, 0);
			/* For a non-POSIX locale,
			 * use the old locale-specific format strings.
			 */
			if ((lc_time != NULL) && strcmp(lc_time,"C")
					      && strcmp(lc_time,"POSIX")) {
				sdatewid = strftime (timebuf, (size_t)NLTBMAX,
					"%sD %sT", timestr);
				ldatewid = strftime (timebuf, (size_t)NLTBMAX,
					"%sD %Y", timestr);
			} else { /* POSIX locale, use default format strings */
				sdatewid = strftime (timebuf, (size_t)NLTBMAX,
					"%b %e %sT", timestr);
				ldatewid = strftime (timebuf, (size_t)NLTBMAX,
					"%b %e  %Y", timestr);
			}
			datewid = sdatewid > ldatewid ? sdatewid : ldatewid;
		}
		/* Again, use old locale-specific format strings
		 * for non-POSIX and default strings for POSIX.
		 */
		if ((p->lmtime < year) || (p->lmtime > now)) {
			if ((lc_time != NULL) && strcmp(lc_time,"C")
					      && strcmp(lc_time,"POSIX"))
				strftime (timebuf, (size_t)NLTBMAX, "%sD %Y", timestr);
			else
				strftime (timebuf, (size_t)NLTBMAX, "%b %e %Y", timestr);
			curcol += printf (" %*.*s ", -datewid, ldatewid, timebuf);
		}
		else {
			if ((lc_time != NULL) && strcmp(lc_time,"C")
					      && strcmp(lc_time,"POSIX"))
				strftime (timebuf, (size_t)NLTBMAX, "%sD %sT", timestr);
			else
				strftime (timebuf, (size_t)NLTBMAX, "%b %e %sT", timestr);
			curcol += printf (" %*.*s ", -datewid, sdatewid, timebuf);
		}
	}
	dmark = "";
	switch(p->ltype) {
	case 'd':	/* Directories */
			if (pflg || Fflg)
				dmark = "/";
			break;
	case 'l':	/* Symbolic links */
			if (Fflg)
				dmark = "@";
			break;
	case 's':	/* Sockets */
			if (Fflg) 
				dmark = "=";
			break;
	case 'p':       /* Fifos */
			if (Fflg) 
				dmark = "|";
			break;
	default:	/* Executable files */
			if (Fflg && (p->lflags & 0111))
				dmark = "*";
			break;
	}
	if (dmark[0] != '\0' && (!qflg && !bflg))
		curcol++;
	if (p->lflags & ISARG) {
		if (qflg || bflg)
			pprintf(p->ln.namep,dmark);
                else {
                        printf("%s%s",p->ln.namep,dmark);
                        curcol += (len = mbswidth(p->ln.namep,strlen(p->ln.namep))) == -1 ? 1 : len;
                        }
	}
	else {
		if (qflg || bflg)
			pprintf(p->ln.lname,dmark);
		else {
			printf("%s%s",p->ln.lname,dmark);
			curcol += (len = mbswidth(p->ln.lname, strlen(p->ln.lname))) == -1 ? 1 : len;
			}
	}
	if ((eflg || lflg) && p->flinkto) {
		if (Fflg) {
			dmark = "";
			switch (p->flinktype & S_IFMT) {
			case S_IFDIR:
					dmark =  "/";
					break;
			case S_IFSOCK:
					dmark = "=";
					break;
			default:
					if (p->flinktype & 0111)
						dmark = "*";
					break;
			}
			if (dmark[0] != '\0' && (!qflg && !bflg))
				curcol++;
		}
		fputs(" -> ", stdout);
		curcol += 4;
		if (qflg || bflg)
			pprintf(p->flinkto, dmark);
		else {
			printf("%s%s", p->flinkto, dmark);
			curcol += (len = mbswidth(p->flinkto, strlen(p->flinkto))) == -1 ? 1 : len;
			}
		free((void *)p->flinkto);
        }
   
}

/* print various r,w,x permissions 
 */
static pmode(aflag)
mode_t aflag;
{
        /* these arrays are declared static to allow initializations */
	static int	m0[] = { 1, S_IREAD>>0, 'r', '-' };
	static int	m1[] = { 1, S_IWRITE>>0, 'w', '-' };
	static int	m2[] = 
		{ 3, S_ISUID|S_IEXEC, 's', S_IEXEC, 'x', S_ISUID, 'S', '-' };
	static int	m3[] = { 1, S_IREAD>>3, 'r', '-' };
	static int	m4[] = { 1, S_IWRITE>>3, 'w', '-' };
	static int	m5[] = { 3, S_ISGID|(S_IEXEC>>3),'s', 
					S_IEXEC>>3,'x', S_ISGID,'S', '-'};
	static int	m6[] = { 1, S_IREAD>>6, 'r', '-' };
	static int	m7[] = { 1, S_IWRITE>>6, 'w', '-' };
	static int	m8[] = { 3, S_ISVTX|(S_IEXEC>>6),'t', 
					S_IEXEC>>6,'x', S_ISVTX,'T', '-'};
        static int  *m[] = { m0, m1, m2, m3, m4, m5, m6, m7, m8};

	register int **mp;

	flags = aflag;
	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])];)
		ls_select(*mp++);
}

static ls_select(pairp)
register int *pairp;
{
	register int n;

	n = *pairp++;
	while (n-->0) {
		if((flags & *pairp) == *pairp) {
			pairp++;
			break;
		}else {
			pairp += 2;
		}
	}
	putchar(*pairp);
	curcol++;
}

/*
 * column: get to the beginning of the next column.
 */
static column()
{

	if (curcol == 0)
		return;
	if (mflg) {
		putc(',', stdout);
		curcol++;
		if (curcol + colwidth + 2 > num_cols) {
			putc('\n', stdout);
			curcol = 0;
			return;
		}
		putc(' ', stdout);
		curcol++;
		return;
	}
	if (Cflg == 0) {
		putc('\n', stdout);
		curcol = 0;
		return;
	}
	if ((curcol / colwidth + 2) * colwidth > num_cols) {
		putc('\n', stdout);
		curcol = 0;
		return;
	}
	do {
		putc(' ', stdout);
		curcol++;
	} while (curcol % colwidth);
}

static new_line()
{
	if (curcol) {
		putc('\n',stdout);
		curcol = 0;
	}
}

/* read each filename in directory dir and store its
 *  status in flist[nfiles] 
 *  use makename() to form pathname dir/filename;
 */
static ulong_t
readdirs(dir)
char *dir;
{
	ulong_t	tblkbytes;
	struct dirent *dentry;
	register struct lbuf *ep;
	register int width;
	DIR	*dirf;

	if ((dirf = opendir(dir)) == NULL) {
		fputs("ls: ",stdout);
		fflush(stdout);
		perror(dir);
		err = 2;
		return( (ulong_t)NULL ); 
	}
        else {
          	tblkbytes = 0;
          	for(;;) {
          		if ( (dentry = readdir(dirf)) == NULL)
          			break;  /* end of directory */
			/* check for directory items '.', '..', 
                         *  and items without valid inode-number;
                         */
          		if (dentry->d_ino==0
          			 || aflg==0 && dentry->d_name[0]=='.' 
          			 && ( Aflg == 0 || dentry->d_name[1]=='\0' || 
				      dentry->d_name[1]=='.' &&
          			      dentry->d_name[2]=='\0') 
			   )
          			continue;

			if (Cflg || mflg) {
				width = (len = mbswidth(dentry->d_name,strlen(dentry->d_name))) == -1 ? 1 : len;  
				if (width > filewidth)
					filewidth = width;
			}
          		ep = gstat(makename(dir,dentry->d_name),0,&tblkbytes);
          		if (ep==NULL)
          			continue;
                        else {
			     if (!statreq)                      /*ptm 24874*/
				 ep->lnum = dentry->d_ino;
          		         strcpy(ep->ln.lname, dentry->d_name);
                        }
          	}
          	closedir(dirf);
		colwidth = fixedwidth + filewidth;
	}
	return (tblkbytes);
}

/* get status of file and recomputes tblkbytes (number of blocks
 * times bytes per block = number of bytes in allocated blocks);
 * argfl = 1 if file is a name in ls-command and  = 0
 * for filename in a directory whose name is an
 * argument in the command;
 * stores a pointer in flist[nfiles] and
 * returns that pointer;
 * returns NULL if failed;
 */
static struct lbuf *
gstat(file, argfl, tblkbytes)
char *file;
ulong_t	*tblkbytes;
{
	struct stat statb, statb1;
	register struct lbuf *rep;
	static int nomocore;
	int status;

	if (nomocore)
		return(NULL);
	else if (nfiles >= maxfils) { 
		/* all flist/lbuf pair assigned files 
		   time to get some more space */
		maxfils += quantn;
		if ( (flist=(struct lbuf **)realloc((void *)flist,
			(size_t)(maxfils * sizeof(struct lbuf *)))) == NULL
			|| (nxtlbf = (struct lbuf *)malloc(
			(size_t)(quantn * sizeof(struct lbuf)))) == NULL) {
			fprintf(stderr, MSGSTR(NOMEMORY,"ls: out of memory\n"));
			nomocore = 1;
			return(NULL);
		}
	}

/* nfiles is reset to nargs for each directory
 * that is given as an argument maxn is checked
 * to prevent the assignment of an lbuf to a flist entry
 * that already has one assigned.
 */
	if(nfiles >= maxn) {
		rep = nxtlbf++;
		flist[nfiles++] = rep;
		maxn = nfiles;
	}else {
		rep = flist[nfiles++];
	}
	rep->lflags = 0;
	if (argfl || statreq) {
	        status = (Lflg&&!use_lstat)? stat(file, &statb) : lstat(file, &statb);
		if (status < 0 && (!Lflg || lstat(file, &statb) < 0))
                {
                    if(errno==EACCES)
                        fprintf(stderr,MSGSTR(NOPERM, "%s: No permission\n"),
                                file);
                    else
			fprintf(stderr, MSGSTR(NOTFOUND, "%s not found\n"),
				file, file);
	            nfiles--;
		    return(NULL);
		}

		rep->lnum = statb.st_ino;
		rep->lsize = statb.st_size;
		rep->lblkbytes = BLOCKS(statb);        /* used for block cnt */
		rep->flinkto = NULL;
		switch(statb.st_mode & S_IFMT) {

	            	case S_IFDIR:
				rep->ltype = 'd';
	            		break;

	            	case S_IFBLK:
				rep->ltype = 'b';
				rep->lsize = statb.st_rdev;
	            		break;

	            	case S_IFCHR:
				rep->ltype = 'c';
				rep->lsize = statb.st_rdev;
	            		break;

			case S_IFSOCK:
				rep->ltype = 's'; 
				rep->lsize = 0; 
				break;

			case S_IFLNK:
				rep->ltype = 'l';
                        	if (use_lstat || Lflg) {
					char	buf[BUFSIZ];
					int	cc;
                                	cc = readlink(file, buf, BUFSIZ);
                                	if (cc >= 0) {
                                        	buf[cc] = 0;
                                        	rep->flinkto = savestr(buf);
                               	 	}
					if (Fflg && (stat(file, &statb1) >= 0))
						rep->flinktype = statb1.st_mode;
                        	} else if (argfl
				    && stat(file, &statb1) >= 0
				    && (statb1.st_mode & S_IFMT) == S_IFDIR) {
					statb = statb1;
					rep->ltype = 'd';
					rep->lsize = statb.st_size;
					rep->lblkbytes = BLOCKS(statb);
				}
				break;

	            	case S_IFIFO:
				rep->ltype = 'p';
                 		break;
                        default:
				rep->ltype = '-';
                 }
#ifdef DEBUG
	printf	( "after stat( ino, size, mode, uid, gid): %d %d %o %d %d\n",
		statb.st_ino, statb.st_size,
		statb.st_mode, statb.st_uid,
		statb.st_gid
 		);
#endif
		rep->lflags = statb.st_mode & ~S_IFMT;
                                   /* mask ISARG and other file-type bits */
		if ((statb.st_mode&S_IXACL) ||
		    (statb.st_mode&S_ITCB) || (statb.st_mode&S_ITP)) 
			rep->lemode = '+';
		else rep->lemode = '-'; 

		rep->luid = statb.st_uid;
		rep->lgid = statb.st_gid;
		rep->lnl = statb.st_nlink;
          	if(uflg)
			rep->lmtime = statb.st_atime;
          	else if (cflg)
			rep->lmtime = statb.st_ctime;
          	else
			rep->lmtime = statb.st_mtime;
		if (tblkbytes != NULL)
			*tblkbytes += rep->lblkbytes; /* is blocks not bytes */
#ifdef DEBUG
	printf( "type, mode, inum, size, uid, gid): '%c' %o %d %d %d %d\n",
		rep->ltype, rep->lflags, rep->lnum, rep->lsize, rep->luid, rep->lgid
	);
#endif

	}
        return(rep);
}


/* returns pathname of the form dir/file;
 *  dir is a null-terminated string;
 */
static char *
makename(dir, file) 
char *dir, *file;
{
	register char *dp, *fp;
	register int i;
	static	int dflen = 0;		/* length of malloc'd dfile */
	static	char *dfile = NULL;

	if ((i = strlen(dir) + strlen(file) + 2) > dflen) {
		if (dfile != NULL)
			free((void *)dfile);
		if ((dfile = malloc((size_t)i)) == NULL) {
			fprintf(stderr,MSGSTR(NOMEMORY,"ls: out of memory\n"));
			exit(1);
		}
		dflen = i;
	}
	dp = dfile;
	fp = dir;
	while (*fp)
		*dp++ = *fp++;
	if (dp > dfile && *(dp - 1) != '/')
		*dp++ = '/';
	fp = file;
	strcpy(dp,fp);
	return(dfile);
}

/* get name from passwd/group file for a given uid/gid
 * and store it in buf; Use a LRU cache system to cut down on
 * accesses to the passwd and group files.
 */
struct idcache {
	uid_t 	id;
	char	*name;
	struct idcache *forward;
	struct idcache *back;
};

static struct idcache *uid_head, *gid_head;
static int	uid_cachesize = 0, gid_cachesize = 0;
#define MAX_CACHE	255
static getname(uid, buf, type)
unsigned uid;
int type;
char buf[];
{
	char *ptr, *cache_hit();
	struct passwd *pwd;
	struct group  *grp;

	switch (type) {
	case NAME_UID:	if ((ptr = cache_hit(&uid_head, uid)) == NULL) {
				if ((pwd = getpwuid((uid_t)uid)) == NULL) 
					return(1);
				ptr = pwd->pw_name;
				add_cache(&uid_head, uid, ptr, &uid_cachesize);
			}
			break;

	case NAME_GID:	if ((ptr = cache_hit(&gid_head, uid)) == NULL) {
				if ((grp = getgrgid((uid_t)uid)) == NULL) 
					return(1);
				ptr = grp->gr_name;
				add_cache(&gid_head, uid, ptr, &gid_cachesize);
			}
			break;
	default: 	return(1);
	}
	strcpy(buf, ptr);
	return(0);
}

static char *
cache_hit(head, id)
struct idcache **head;
unsigned id;
{
	register struct idcache *p;
	for (p = *head; p != NULL; p = p->forward) {
		if (p->id == id) {
			if (p != *head) {
				/* take it out of the list */
				p->back->forward = p->forward;
				p->forward->back = p->back;
				/* Add it at the head */
				p->forward = *head;
				p->back = (*head)->back;
				(*head)->back->forward = p;
				(*head)->back = p;
				*head = p;
			}
			return(p->name);
		}
		if (p == (*head)->back)
			break;
	}
	return(NULL);
}

static add_cache(head, id, name, size)
struct  idcache **head;
unsigned id;
char *name;
int	*size;
{
	register struct idcache *ptr;

	if (*size < MAX_CACHE) {
		if ((ptr = (struct idcache *)malloc((size_t)sizeof(struct idcache)))
								== NULL) {
			fprintf(stderr, MSGSTR(ADDCACHE, 
				"add_cache: no more memory\n"));
			exit(1);
		}
	}
	else {
		ptr = (*head)->back;
		free((void *)ptr->name); /* free him we're going to malloc a new one */
	}

	/* fill in the struct */
	ptr->name = savestr(name);
	ptr->id = id;

	/* add to the list */
	if (*size < MAX_CACHE) {
		(*size)++; 
		if (*head == NULL) {
			*head = ptr;
			ptr->forward = *head;
			ptr->back = *head;
		}
		else {
			ptr->forward = *head;
			ptr->back = (*head)->back;
			(*head)->back->forward = ptr;
			(*head)->back = ptr;
			*head = ptr;
		}
	}
	else
		*head = ptr;
}

/* return >0 if item pointed by pp2 should appear first */
static compar(pp1, pp2)
struct lbuf **pp1;
struct lbuf **pp2;
{
	register struct lbuf *p1, *p2;

	p1 = *pp1;
	p2 = *pp2;
	if (dflg==0) {
/* compare two names in ls-command one of which is file
 *  and the other is a directory;
 *  this portion is not used for comparing files within
 *  a directory name of ls-command;
 */
		if (p1->lflags&ISARG && p1->ltype=='d')
		{
			if (!(p2->lflags&ISARG && p2->ltype=='d'))
				return(1);
                }
                else {
			if (p2->lflags&ISARG && p2->ltype=='d')
				return(-1);
		}
	}
	if (tflg) {
		if (p2->lmtime < p1->lmtime)
			return(-rflg);
		else if(p2->lmtime > p1->lmtime)
			return(rflg);
                /* if modification times are the same, then fall */
                /* thru and compare based on collation sequence */
	}
	return (rflg * strcoll (
	    p1->lflags&ISARG ? p1->ln.namep:p1->ln.lname,
	    p2->lflags&ISARG ? p2->ln.namep:p2->ln.lname));
}

static pprintf(s1,s2)
	char *s1, *s2;
{
	register char *s;
	register int   c;
	register int  cc;
	int z;
	char outc;
	wchar_t	wct;
	int i;

	for (s = s1, i = 0; i < 2; i++, s = s2) {
		while((c = mbtowc(&wct, s, mb_cur_max))>0) {
			if (! iswprint (wct)) {
				for(z=0; z<c; z++) { 
					if (qflg)
						outc = '?';
					else if (bflg) {
						curcol += 3;
						putc ('\\', stdout);
						cc = '0' + (*s>>6 & 07);
						putc (cc, stdout);
						cc = '0' + (*s>>3 & 07);
						putc (cc, stdout);
						outc = '0' + (*s & 07);
						}	/* if (bflg) 	    */
					putc(outc, stdout);
					curcol++;
					s++;
					}
				continue;
				}			/* if (! iswprint() */
			curcol += (len = mbswidth(s, c)) == -1 ? 1 : len;
			for(z=0; z<c; z++)	/* printable mb char */
				putc(*s++, stdout);
			}
		}
}

static char *
savestr(str)
        char *str;
{
        char *cp = malloc((size_t)(strlen(str) + 1));

        if (cp == NULL) {
                fprintf(stderr, MSGSTR(NOMEMORY, "ls: out of memory\n"));
                exit(1);
        }
        (void) strcpy(cp, str);
        return (cp);
}
