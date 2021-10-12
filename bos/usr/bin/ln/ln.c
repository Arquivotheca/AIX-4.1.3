static char sccsid[] = "@(#)13	1.6.1.10  src/bos/usr/bin/ln/ln.c, cmdfiles, bos412, 9446C 11/14/94 16:48:41";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: ln
 *
 * ORIGINS: 3, 18, 26, 27
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 *
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 */

#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include "ln_msg.h"

static nl_catd	catd;

#define MSGSTR(Num,Str) catgets(catd,MS_LN,Num,Str)

static int 	linkit(char *, char *, struct stat *);
static void	usage(void);

static int		isdir = 0;			/* Is target a directory? */
static static int	fflag = 0;			/* force flag */
static int	sflag = 0;			/* symbolic, not hard, link */

/*
 * NAME: ln [-fs] source [target] or ln [-fs] src1 ... srcN directory
 * 
 * FUNCTION: links files
 *
 * NOTES:   -f  forces existing destination pathnames to be 
 *              removed to allow the link
 *          -s  creates symbolic links instead of hard links
 *              (can span file systems)
 */

main(int argc, char **argv)
{
	struct stat statbuf, lstatbuf, *tbuf = NULL;
	char *target;
	int ch, i, exitval = 0;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_LN, NL_CAT_LOCALE);

	while ((ch = getopt(argc, argv, "fs")) != EOF) {
		switch((char)ch) {
			case 'f':
				fflag = 1;
				break;
			case 's':
				sflag = 1;
				break;
			default:
				usage();
		}
	}

	argv += optind;
	argc -= optind;

	if (argc <= 0) {
		usage();
		exit(2);
	}

	/* "ln <src>" is an IBM extension to the standard, and */
	/* has the same effect as the command "ln <src> .".    */

	if (argc == 1)
		target = ".";
	else
		target = argv[--argc];

	/* argc is now equal to the number of sources. */

	if (lstat(target, &lstatbuf) >= 0) {
		tbuf = &lstatbuf;
		if ((stat(target, &statbuf) >= 0) && S_ISDIR(statbuf.st_mode)) {
			tbuf = &statbuf;
			isdir = 1;
		}
	}

	if ((argc > 1) && !isdir) /* ln src1 ... srcN directory */
		usage();

	for(i=0; i<argc; i++)
		exitval |= linkit(argv[i], target, tbuf);

	exit(exitval);
}

/*
 * NAME: linkit
 *
 * FUNCTION: links files and directories
 *
 * RETURNS: 0 if successful or 1 on error
 */

static int
linkit(char *source, char *target, struct stat *tbuf)
{
	int    rc;
	struct stat sbuf, statbuf;
	char   path[PATH_MAX];
	char  *t_path;

	if (!sflag) {
		/* The source file must exist and cannot be */
		/* a directory if hard links are specified. */
		if (stat(source, &sbuf) == -1) {
			fputs("ln: ",stderr);
			perror(source);
			return(1);
		}
		if (S_ISDIR(sbuf.st_mode)) {
			(void) fprintf(stderr, MSGSTR(SISDIR,
			    "ln: cannot hard link directory %s\n"), source);
			return(1);
		}
	}

	if (isdir) {	/* target is a directory */
		/* get the base name for the destination path */
		if ((t_path = strrchr(source, '/')) == (char *) NULL)
			t_path = source;
		else
			t_path++;
		(void) sprintf(path, "%s/%s", target, t_path);
		/* set target to be the destination path */
		target = path;
		tbuf = NULL;
		if (stat(target, &statbuf) >= 0)  {
			tbuf = &statbuf;
			if (S_ISDIR(tbuf->st_mode)) {
				(void) fprintf(stderr, MSGSTR(ISDIR,
				    "ln: %s is a directory.  (cannot unlink)\n"), target);
				return(1);
			}
		}
	}

	/* if tbuf is not NULL, then the target exists */
	if (tbuf) {
		if (!fflag) {
			(void) fprintf(stderr, MSGSTR(FOPT,
			    "ln: %s exists.  Specify -f to remove.\n"), target, target);
			return(1);
		}
		if (!sflag &&
		    (sbuf.st_dev == tbuf->st_dev) &&
		    (sbuf.st_ino == tbuf->st_ino)) {
			(void) fprintf(stderr, MSGSTR(IDENTICAL,
			    "ln: %s and %s are identical.\n"), source, target);
			return(1);
		}
		if (unlink(target) < 0) {
			(void) fprintf(stderr, MSGSTR(CANTUNLINK,
			    "ln: cannot unlink %s.\n"), target);
			return(1);
		}
	}

	if (sflag)
		rc = symlink(source, target);
	else
		rc = link(source, target);

	if (rc < 0) {
		switch (errno)  {
		    case (EXDEV):
				(void) fprintf(stderr, MSGSTR(DIFFFS,
				    "ln: %s and %s are located on different file systems.\n"),
				    source, target);
				break;
		    case (EACCES):
				(void) fprintf(stderr, MSGSTR(NOPERM,
				    "ln: insufficient permission for %s.\n"),
				    target);
				break;
		    default:
				perror("ln");
		}
		return(1);
	}

	return(0);
}

/*
 * NAME: usage
 *
 * FUNCTION: prints usage message
 */

static void
usage(void)
{
	(void) fprintf(stderr,MSGSTR(USAGE, 
		"Usage: ln [-fs] src [target]\n   or: ln [-fs] src1 ... srcN directory\n"));
	exit(1);
}
