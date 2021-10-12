static char sccsid[] = "@(#)58  1.19  src/bos/usr/bin/lastcomm/lastcomm.c, cmdstat, bos41J, 9508A 2/10/95 15:42:36";
/*
 * COMPONENT_NAME: (CMDSTAT) Displays Info. about the last commands executed
 *
 * FUNCTIONS: lastcomm
 *
 * ORIGINS: 3, 9, 26, 27
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
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#define _ILS_MACROS
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/acct.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <utmp.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <locale.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>


/*
 * Must be a multiple of sizeof(struct acct) = 36.
 */
#define	READSIZE	540
#define	TIMEBUF		80
#define LSZ		12	/* sizeof line name */


#include "lastcomm_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LASTCOMM,n,s) 

static struct	acct buf[READSIZE / sizeof(struct acct)];

float	expand();
char	*flagbits();
char	*uidtonam();
char	*devtolin();
dev_t	lintodev();
static char	timbuf[TIMEBUF];

#define ACCTFILE	"/var/adm/pacct"

#define dbtob(n)	((sizeof (struct acct))*(n-1))
#define	fldoff(str, fld)	((int)&(((struct str *)0)->fld))
#define	fldsiz(str, fld)	(sizeof(((struct str *)0)->fld))
#define	strbase(str, ptr, fld)	((struct str *)((char *)(ptr)-fldoff(str, fld)))

#define PRECISION	0.01	/* Precision for rounding double values */

main(int argc, char *argv[])
{
	register int bn, cc;
	register struct acct *acp;
	int fd;
	struct stat sb;
	int lastblock;

	(void) setlocale (LC_ALL,"");

	if ((fd = open(ACCTFILE, O_RDONLY)) < 0) {
		perror(ACCTFILE);
		exit(1);
	}

	catd = catopen(MF_LASTCOMM,NL_CAT_LOCALE);

	fstat(fd, &sb);
	lastblock=sb.st_size/sizeof(struct acct); /* number of structures */
	for (bn = lastblock; bn > 0; bn--) {
		lseek(fd, (long)dbtob(bn), SEEK_SET);
		cc = read(fd,(char *)buf,(unsigned)sizeof(struct acct));
		if (cc < 0) {
			perror(MSGSTR(READ, "read")); /*MSG*/
			break;
		}
		acp = buf + (cc / sizeof(buf[0])) - 1;
		for (; acp >= buf; acp--) {
			register char *cp;
			register wchar_t *wc;
			register size_t nn;
			double x;
			struct tm *tm;

			if (acp->ac_comm[0] == '\0')
				strcpy(acp->ac_comm, "?");

			cp = &acp->ac_comm[0];
			nn = strlen(cp) + 1;
			wc = (wchar_t *)malloc(nn * sizeof(wchar_t));
			mbstowcs(wc, cp, nn);
			while (wc < (wchar_t *)&acp->ac_comm[fldsiz(acct, ac_comm)] && *wc){
				if (!iswprint(*wc))
					*wc = '?';
				wc++;
			}
			wcstombs(cp, wc, nn);

			if (argc > 1 && !ok(argc, argv, acp))
				continue;
			x = (double) expand(acp->ac_utime) + (double) expand(acp->ac_stime);

			tm = localtime(&acp->ac_btime);
			strftime(timbuf, TIMEBUF, "%a %b %d %sT", tm);
				
			printf("%-*.*s %s %-*s %-*s %6.2f secs %.18s\n",
				fldsiz(acct, ac_comm), fldsiz(acct, ac_comm),
				acp->ac_comm,
				flagbits(acp->ac_flag),
				fldsiz(utmp, ut_name), uidtonam(acp->ac_uid),
				fldsiz(utmp, ut_line), devtolin(acp->ac_tty),
				(x <= PRECISION) ? PRECISION : x,
				timbuf);
		}
	}
	exit(0);
}

/*
 *  NAME:  expand
 *
 *  FUNCTION:  Convert the number of ticks stored as a floating point
 *		to the number stored as an integer.
 *	      
 *  RETURN VALUE:  	 number of seconds.
 */

static float
expand(unsigned t)
{
	register long nt;
	float e;

	nt = t&017777;
	t >>= 13;
	while (t!=0) {
		t--;
		nt <<= 3;
	}
	e = (float) (nt/AHZ) + (((float)(nt%AHZ))/AHZ);
	return(e);
}

/*
 *  NAME:  flagbits
 *
 *  FUNCTION:  Fill in flag array with symbols of particular flags set.
 *	      
 *  RETURN VALUE:  	 Pointer to the flag array is returned.
 */

static char *
flagbits(register int f)
{
	register int i = 0;
	static char flags[20];

#define BIT(flag, ch)	flags[i++] = (f & flag) ? ch : ' '
	BIT(ASU, 'S');		/* cmd executed by super-user */
	BIT(AFORK, 'F');	/* cmd ran after a fork, but w/o exec */
	BIT(ACOMPAT, 'C');	/* cmd ran in PDP-11 compatibility mode */
	BIT(ACORE, 'D');	/* cmd terminated and generated core file */
	BIT(AXSIG, 'X');	/* cmd terminated with a signal */
	flags[i] = '\0';
	return (flags);
}

/*
 *  NAME:  ok
 *
 *  FUNCTION:  Check to see if given accounting record should be
 *		printed out.
 *	      
 *  RETURN VALUE:  	 0 if record is NOT to be printed out.
 *		
 */

static int
ok(register int argc, 
   register char *argv[], 
   register struct acct *acp)
{
	register int j;

	for (j = 1; j < argc; j++)
		if (strcmp(uidtonam(acp->ac_uid), argv[j]) &&
		    strcmp(devtolin(acp->ac_tty), argv[j]) &&
		    strncmp(acp->ac_comm, argv[j], fldsiz(acct, ac_comm)))
			break;
	return (j == argc);
}


/* This code was taken from the cmdacct uidtonam.c,
 * lintodev.c, and devtolin.c files.
 */

/***************************************************************************
 * Function: uidtonam()
 * convert uid to login name; interface to getpwuid that keeps up to USIZE1
 * names to avoid unnecessary accesses to passwd file
 * returns ptr to NSZ-byte name (not necessarily null-terminated)
 * returns ptr to "??" if cannot convert
 **************************************************************************/
#define NSZ     8       /* sizeof login name */
#define USIZE1	50
static	usize1;
static struct ulist {
	char	uname[NSZ];
	uid_t	uuid;
} ul[USIZE1];

char *
uidtonam(uid_t uid)
{
	register struct ulist *up;
	register struct passwd *pp;

	for (up = ul; up < &ul[usize1]; up++)
		if (uid == up->uuid)
			return(up->uname);
	setpwent();
	if ((pp = getpwuid(uid)) == NULL)
		return("??");
	else {
		if (usize1 < USIZE1) {
			up->uuid = uid;
			strncpy(up->uname, pp->pw_name, sizeof(up->uname));
			usize1++;
		}
		return(pp->pw_name);
	}
}

static	char	devtty[5+LSZ+1]	= "/dev/xxxxxxxx";
#define TSIZE1	300	/* # distinct names, for speed only */
static	tsize1 = 0;
static struct tlist {
	char	tname[LSZ + 4];	/* linename */
	dev_t	tdev;		/* device */
} tl[TSIZE1];
static char *devdir[] = { "/dev", "/dev/pts"};
static struct dirent *d;

dev_t
lintodev(char linename[LSZ])
{
	struct stat sb;
	strncpy(&devtty[5], linename, LSZ);
	if (stat(devtty, &sb) != -1 && (sb.st_mode&S_IFMT) == S_IFCHR)
		return((dev_t)sb.st_rdev);
	return((dev_t)-1);
}

char *
devtolin(dev_t device)
{
	register struct tlist *tp;
	register i;
	register char buf[LSZ + 4];
	DIR *fdev;

	if (device == NODEVICE)
		return("__");

	for (tp = tl; tp < &tl[tsize1]; tp++)
		if (device == tp->tdev)
			return(tp->tname);

	for (i = 0; i < 2; i++) {
		if ((fdev = opendir(devdir[i])) == NULL)
			return("??");
		while ( (d = readdir(fdev)) != NULL) {
			buf[0] = '\0';
			if (i > 0)
				strcpy(buf, "pts/");
			strcat(buf, d->d_name);
			if (lintodev(buf) == device) {
				if (tsize1 < TSIZE1) {
					tp->tdev = device;
					strncpy(tp->tname,buf,
						sizeof(tp->tname));
					tsize1++;
				}
				closedir(fdev);
				return(buf);
			}
		}
		closedir(fdev);
	}
	return("??");
}
