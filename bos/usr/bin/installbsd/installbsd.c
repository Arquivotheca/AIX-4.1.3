static char sccsid[] = "@(#)48	1.3  src/bos/usr/bin/installbsd/installbsd.c, cmdfiles, bos411, 9428A410j 5/27/94 11:04:53";
/*
 *   COMPONENT_NAME: CMDFILES
 *
 *   FUNCTIONS: CLR
 *		MSGSTR
 *		bad
 *		copy
 *		defined
 *		error
 *		getmode
 *		install
 *		main
 *		setmode
 *		strip
 *		usage
 *		
 *
 *   ORIGINS: 26,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 *   ALL RIGHTS RESERVED 
 *
 *   OSF/1 1.1
 *
 *   Copyright (c) 1987 Regents of the University of California.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms are permitted provided
 *   that: (1) source distributions retain this entire copyright notice and
 *   comment, and (2) distributions including binaries display the following
 *   acknowledgement:  ``This product includes software developed by the
 *   University of California, Berkeley and its contributors'' in the
 *   documentation or other materials provided with the distribution and in
 *   all advertising materials mentioning features or use of this software.
 *   Neither the name of the University nor the names of its contributors may
 *   be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 *   WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 *   xinstall.c  5.24 (Berkeley) 7/1/90
 *
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <paths.h>
#include <locale.h>
#include "installbsd_msg.h"

#define _PATH_STRIP "/usr/bin/strip"
#define MSGSTR(num, str)	catgets(catd, MS_INSTALLBSD, num, str)
#define STAFF	"staff"

static nl_catd catd;
static struct passwd *pp;
static struct group *gp;
static int docopy, dostrip, mode = 0755;
static char *group, *owner, pathbuf[MAXPATHLEN];

void install(char *from_name, char *to_name, int isdir);
void copy(int from_fd, char *from_name, int to_fd, char *to_name);
void strip(char *to_name);
void error(char *s);
void bad(char *fname);
void usage(void);
mode_t getmode(mode_t *set, mode_t omode);
mode_t *setmode(register char *p);

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct stat from_sb, to_sb;
	mode_t *set;
	int ch, no_target;
	char *to_name;

	setlocale(LC_ALL, "");
	catd = catopen(MF_INSTALLBSD, NL_CAT_LOCALE);

	group = STAFF;
	while ((ch = getopt(argc, argv, "cg:m:o:s")) != EOF)
		switch((char)ch) {
		case 'c':
			docopy = 1;
			break;
		case 'g':
			group = optarg;
			break;
		case 'm':
			if (!(set = setmode(optarg))) {
				(void)fprintf(stderr,
			    MSGSTR(M_MODE, "installbsd: invalid file mode.\n"));
				exit(1);
			}
			mode = getmode(set, 0);
			break;
		case 'o':
			owner = optarg;
			break;
		case 's':
			dostrip = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc < 2)
		usage();

	/* get group and owner id's */
	if (!(gp = getgrnam(group))) {
		fprintf(stderr, MSGSTR(M_GROUP, "installbsd: unknown group %s.\n"), group);
		exit(1);
	}
	if (owner && !(pp = getpwnam(owner))) {
		fprintf(stderr, MSGSTR(M_OWNER, "installbsd: unknown user %s.\n"), owner);
		exit(1);
	}

	no_target = stat(to_name = argv[argc - 1], &to_sb);
	if (!no_target && (to_sb.st_mode & S_IFMT) == S_IFDIR) {
		for (; *argv != to_name; ++argv)
			install(*argv, to_name, 1);
		exit(0);
	}

	/* can't do file1 file2 directory/file */
	if (argc != 2)
		usage();

	if (!no_target) {
		if (stat(*argv, &from_sb)) {
			fprintf(stderr, MSGSTR(M_FIND, "installbsd: can't find %s.\n"), *argv);
			exit(1);
		}
		if ((to_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, MSGSTR(M_REGULAR, "installbsd: %s isn't a regular file.\n"), to_name);
			exit(1);
		}
		if (to_sb.st_dev == from_sb.st_dev && to_sb.st_ino == from_sb.st_ino) {
			struct stat tmp_sb;
			(void)lstat(to_name,&tmp_sb);
			if ((tmp_sb.st_mode & S_IFMT) !=  S_IFLNK) {
				fprintf(stderr, MSGSTR(M_SAME, "installbsd: %s and %s are the same file.\n"), *argv, to_name);
				exit(1);
				}
		}
		/* unlink now... avoid ETXTBSY errors later */
		(void)unlink(to_name);
	}
	install(*argv, to_name, 0);
	exit(0);
}

/*
 * install --
 *	build a path name and install the file
 */
void
install(char *from_name, char *to_name, int isdir)
{
	struct stat from_sb;
	int devnull, from_fd, to_fd;
	char *C, *rindex();

	/* if try to install NULL file to a directory, fails */
	if (isdir || strcmp(from_name, _PATH_DEVNULL)) {
		if (stat(from_name, &from_sb)) {
			fprintf(stderr, MSGSTR(M_FIND, "installbsd: can't find %s.\n"), from_name);
			exit(1);
		}
		if ((from_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, MSGSTR(M_REGULAR, "installbsd: %s isn't a regular file.\n"), from_name);
			exit(1);
		}
		/* build the target path */
		if (isdir) {
			(void)sprintf(pathbuf, "%s/%s", to_name, (C = rindex(from_name, '/')) ? ++C : from_name);
			to_name = pathbuf;
		}
		devnull = 0;
	} else
		devnull = 1;

	/* unlink now... avoid ETXTBSY errors later */
	(void)unlink(to_name);

	/* create target */
	if ((to_fd = open(to_name, O_CREAT|O_WRONLY|O_TRUNC, 0600)) < 0) {
		error(to_name);
		exit(1);
	}
	if (!devnull) {
		if ((from_fd = open(from_name, O_RDONLY, 0)) < 0) {
			(void)unlink(to_name);
			error(from_name);
			exit(1);
		}
		copy(from_fd, from_name, to_fd, to_name);
		(void)close(from_fd);
		(void)close(to_fd);
	}
	if (dostrip)
		strip(to_name);
	/*
	 * set owner, group, mode for target; do the chown first,
	 * chown may lose the setuid bits.
	 */
	if (chown(to_name, owner ? pp->pw_uid : -1, gp->gr_gid) ||
	    chmod(to_name, mode)) {
		error(to_name);
		bad(to_name);
	}
	if (!docopy && !devnull && unlink(from_name)) {
		error(from_name);
		exit(1);
	}
}

/*
 * copy --
 *	copy from one file to another
 */
void
copy(int from_fd, char *from_name, int to_fd, char *to_name)
{
	register int n;
	char buf[MAXBSIZE];

	while ((n = read(from_fd, buf, sizeof(buf))) > 0)
		if (write(to_fd, buf, n) != n) {
			error(to_name);
			bad(to_name);
		}
	if (n == -1) {
		error(from_name);
		bad(to_name);
	}
}

/*
 * strip --
 *	use strip(1) to strip the target file
 */
void
strip(char *to_name)
{
	int status;

	switch (fork()) {
	case -1:
		error("fork");
		bad(to_name);
	case 0:
		execl(_PATH_STRIP, "strip", to_name, (char *)NULL);
		error(_PATH_STRIP);
		_exit(1);
	default:
		if (wait(&status) == -1 || status)
			bad(to_name);
	}
}

/*
 * error --
 *	print out an error message
 */
void
error(char *s)
{
	char *strerror();

	(void)fprintf(stderr, "installbsd: %s: %s\n", s, strerror(errno));
}

/*
 * bad --
 *	remove created target and die
 */
void
bad(char *fname)
{
	(void)unlink(fname);
	exit(1);
}

/*
 * usage --
 *	print a usage message and die
 */
void
usage(void)
{
	(void)fprintf(stderr,
MSGSTR(USAGE, "usage: install [-cs] [-g group] [-m mode] [-o owner] file1 file2;\n\tor file1 ... fileN directory\n"));
	exit(1);
}



#define	setbits	set[0]
#define	clrbits	set[1]
#define	Xbits	set[2]

mode_t
getmode(mode_t *set, mode_t omode)
{
	register mode_t newmode;

	newmode = omode & clrbits;
	newmode |= setbits;
	if (omode & (S_IFDIR|S_IXUSR|S_IXGRP|S_IXOTH))
		newmode |= Xbits;
	return(newmode);
}

#define	STANDARD_BITS	(S_ISUID|S_ISGID|S_IRWXU|S_IRWXG|S_IRWXO)
#define	CLR(a)		{ clrbits |= a; setbits &= ~(a); Xbits &= ~(a); }

/*
 * Sticky bit and save text bit are the same on OSF/1
 */
#ifndef S_ISTXT
#define S_ISTXT S_ISVTX
#endif

mode_t *
setmode(register char *p)
{
	register int perm, who;
	register char op;
	mode_t mask, *set;
	int permXbits;
	char *malloc();

	if (!*p)
		return(NULL);

	/*
	 * get a copy of the mask for the permissions that are mask
	 * relative.  Flip the bits, we want what's not set.
	 */
	(void)umask(mask = umask(0));
	mask = ~mask;

	if (!(set = (mode_t *)malloc((u_int)(sizeof(mode_t) * 3)))) {
		errno = ENOMEM;
		return(NULL);
	}

	setbits = clrbits = Xbits = 0;

	/*
	 * if an absolute number, get it and return; disallow non-octal
	 * digits or illegal bits.
	 */
	if (isdigit(*p)) {
		setbits = (mode_t)strtol(p, (char **)0, 8);
		clrbits = ~(STANDARD_BITS|S_ISTXT);
		Xbits = 0;
		while (*++p)
			if (*p < '0' || *p > '7')
				return(NULL);
		if (setbits & clrbits)
			return(NULL);
		return(set);
	}

	/*
	 * accumulate bits to add and subtract from each clause of
	 * the symbolic mode
	 */
	for (;;) {
		for (who = 0;; ++p) {
			switch (*p) {
			case 'a':
				who |= STANDARD_BITS;
				break;
			case 'u':
				who |= S_ISUID|S_IRWXU;
				break;
			case 'g':
				who |= S_ISGID|S_IRWXG;
				break;
			case 'o':
				who |= S_IRWXO;
				break;
			default:
				goto getop;
			}
		}

getop:		if ((op = *p++) != '+' && op != '-' && op != '=')
			return(NULL);

		who &= ~S_ISTXT;
		for (perm = 0;; ++p) {
			switch (*p) {
			case 'r':
				perm |= S_IRUSR|S_IRGRP|S_IROTH;
				break;
			case 's':
				/* if only "other" bits ignore set-id */
				if (who & ~S_IRWXO)
					perm |= S_ISUID|S_ISGID;
				break;
			case 't':
				/* if only "other" bits ignore sticky */
				if (who & ~S_IRWXO) {
					who |= S_ISTXT;
					perm |= S_ISTXT;
				}
				break;
			case 'w':
				perm |= S_IWUSR|S_IWGRP|S_IWOTH;
				break;
			case 'X':
				permXbits = S_IXUSR|S_IXGRP|S_IXOTH;
				break;
			case 'x':
				perm |= S_IXUSR|S_IXGRP|S_IXOTH;
				break;
			default:
				goto apply;
			}
		}

apply:		switch(op) {
		case '+':
			/*
			 * If no perm value, skip.  If no who value, use umask
			 * bits.  Don't bother clearing any bits, getmode
			 * clears first, then sets.
			 */
			if (perm || permXbits) {
				if (!who)
					who = mask;
				if (permXbits)
					Xbits |= who & permXbits;
				setbits |= who & perm;
			}
			break;
		case '-':
			/*
			 * If no perm value, skip.  If no who value, use
			 * owner, group, and other.
			 */
			if (perm) {
				if (!who)
					who = S_IRWXU|S_IRWXG|S_IRWXO;
				CLR(who & perm);
			}
			break;
		case '=':
			/*
			 * If no who value, clear all the bits.  Otherwise,
			 * clear the bits specified by who.
			 */
			if (!who) {
				CLR(STANDARD_BITS);
				who = mask;
			} else
				CLR(who);
			if (perm)
				setbits |= who & perm;
			break;
		}

		if (!*p)
			break;
		if (*p != ',')
			goto getop;
		++p;
	}
	clrbits = ~clrbits;
	return(set);
}
