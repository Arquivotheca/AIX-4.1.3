static char sccsid[] = "@(#)63  1.40  src/bos/usr/bin/mv/mv.c, cmdfiles, bos412, 9446C 11/14/94 16:48:53";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: mv
 *
 * ORIGINS: 3, 18, 26, 27, 71
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
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 *
 * OSF/1 1.1
 *
 */

#include <acl.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include "mv_msg.h"
#include "pathnames.h"

static nl_catd	catd;

#define MSGSTR(Num,Str) catgets(catd,MS_MV,Num,Str)
#define RESP_LEN 25			/* arbitrary length for response */

static int fflg, iflg;
static char *pname;				/* ptr to calling command (mv or move) */

/*
 * NAME: {mv | move} [-i | -f] [--] f1 f2  or: {mv | move} [-i | -f] [--] f1 ... fn d1
 * 
 * FUNCTION: moves file(s) to the specified target.
 *
 * NOTES: 
 *        -f  if the destination path exists, do not prompt for confirmation.
 *        -i  if the destination path exists, prompt for confirmation.
 *            -f and -i are mutually exclusive.  The last one specified takes
 *            precedence.   
 *        --  denotes end of options to allow for file names beginning with
 *            a hyphen.   (documented for backward compatibility.  This use to be
 *            a single hyphen.  However, with the implementation of getopts, the single
 *            hyphen became obsolete.  The getopts dash is explicitly documented here for
 *            clarity with previous releases)
 */

main(argc, argv)
	int argc;
	char **argv;
{
	register int baselen, exitval, len;
	register char *p, *endp;
	struct stat sbuf;
	int ch;
	char path[MAXPATHLEN + 1];

        (void)setlocale(LC_ALL, "");
        catd = catopen(MF_MV, NL_CAT_LOCALE);

	/*
	 * determine the "called as" program name, mv or move.
	 */
	pname = (p = strrchr(*argv,'/')) ? ++p : *argv;

	while (((ch = getopt(argc, argv, "if")) != EOF))
		switch((char)ch) {
		case 'i':
			++iflg;
			fflg = 0;	/* reset any previously specified -f */
			break;
		case 'f':
			++fflg;
			iflg = 0;	/* reset any previously specified -i */
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	/*
	 * if stat fails on target, it doesn't exist (or can't be accessed
	 * by the user, doesn't matter which) try the move.  If target exists,
	 * and isn't a directory, try the move.  More than 2 arguments is an
	 * error.
	 */

	if (stat(argv[argc - 1], &sbuf) || !S_ISDIR(sbuf.st_mode)) {
		if (argc > 2)
			usage();
		exit(do_move(argv[0], argv[1]));
	}

	/* got a directory, move each file into it */

	(void)strcpy(path, argv[argc - 1]);
	baselen = strlen(path);
	endp = &path[baselen];
	*endp++ = '/';
	++baselen;
	for (exitval = 0; --argc; ++argv) {
		register length;

		length = strlen(*argv);
		while ((length > 1) && (argv[0][--length] == '/'))
			argv[0][length] = '\0';

		if ((p = strrchr(*argv, '/')) == NULL)
			p = *argv;
		else
			++p;
		if ((baselen + (len = strlen(p))) >= MAXPATHLEN)
			(void)fprintf(stderr,
			    MSGSTR(P2LNG, "%s: %s: destination pathname too long\n"), pname, *argv);
		else {
			bcopy(p, endp, len + 1);
			exitval |= do_move(*argv, path);
		}
	}
	exit(exitval);
}

/*
 * NAME: do_move
 *
 * FUNCTION: main move routine.  performs the
 *           move if possible.
 *
 * RETURNS: 0 if successful, 1 on error
 */
  
static int
do_move(from, to)
	char *from, *to;
{
	struct stat sbuf, sbuf2;
	int ask, ch;
	int rc = 0;
	
	/*
	 * Complain if files are identical.
	 */
	if (!lstat(from, &sbuf) && !lstat(to, &sbuf2) && 
	    (sbuf.st_dev == sbuf2.st_dev) && (sbuf.st_ino == sbuf2.st_ino)) {
		fprintf(stderr, MSGSTR(IDENTICLE, "%s: %s and %s are identical.\n"), pname, from, to);
		return(1);
	}


	/*
	 * Check access.  If interactive and file exists ask user if it
	 * should be replaced.  Otherwise if file exists but isn't writable
	 * make sure the user wants to clobber it.
	 */

	if (!fflg && !access(to, F_OK)) {
		ask = 0;
		if (iflg) {
			(void)fprintf(stderr, MSGSTR(REMOVE, "overwrite %s? "), to);
			ask = 1;
		}
		else if (access(to, W_OK) && !stat(to, &sbuf) && isatty((int)fileno(stdin))) {
			(void)fprintf(stderr, MSGSTR(REMOVE2, "override mode %o on %s? "),
			    sbuf.st_mode & 07777, to);
			ask = 1;
		}
		if (ask) {
			char response[RESP_LEN];

			if (gets(response) != NULL) {
				if (rpmatch(response) != 1)	/* if not affirmative, do not move */
					return(0);
			}
			else
				return(0);
		}
	}

	if (!rename(from, to)) {		/* rename successful */

		return(0);
	}

	if (errno != EXDEV) {
		(void)fprintf(stderr,
		    MSGSTR(RENAME, "%s: cannot rename %s to %s: \n%s\n"), pname, from, to, strerror(errno));
		return(1);
	}
	/*
	 * if rename fails, and it's a regular file, do the copy
	 * internally; otherwise, use cp and rm.
	 */
	if (lstat(from, &sbuf)) {
		(void)fprintf(stderr,
		    "%s: %s: %s\n", pname, from, strerror(errno));
		return(1);
	}
	rc = (S_ISREG(sbuf.st_mode) ?
	    fastcopy(from, to, &sbuf) : copy(from, to));

	return(rc);		/* return code from fastcopy or copy */
}

/*
 * NAME: fastcopy
 *
 * FUNCTION: performs an internal copy 
 *	     for regular files in the event
 *           the call to rename fails
 *
 * RETURNS: 0 if successful, 1 on error
 */

static int
fastcopy(from, to, sbp)
	char *from, *to;
	struct stat *sbp;
{
	struct timeval tval[2];
	static u_int blen;
	static char *bp;
	register int nread, from_fd, to_fd;
	int rc;
	struct stat tbuf;
	char *aclp = NULL;           /* pointer for acl_get/acl_put  */
	char *acl_fget();

	if ((from_fd = open(from, O_RDONLY, 0)) < 0) {
		(void)fprintf(stderr,
		    "%s: %s: %s\n", pname, from, strerror(errno));
		return(1);
	}

	/*
	 * If target file exists and is not a directory,
	 * remove target file first before creating.
	 */
	if (!stat(to, &tbuf) && !S_ISDIR(tbuf.st_mode)) {
		if (unlink(to) < 0) {
			(void)fprintf(stderr, "%s: %s: %s\n", pname, to, strerror(errno));
			(void)close(from_fd);
			return(1);
		}
	}

	/*
	 * Call open to create the target file.  The permissions will
	 * be set to that of the source file modified by the umask.  Hence,
	 * the need for the fchmod call below.
	 */
	if ((to_fd = open(to, O_WRONLY|O_CREAT|O_TRUNC, sbp->st_mode)) < 0) {
		(void)fprintf(stderr,
		    "%s: %s: %s\n", pname, to, strerror(errno));
		(void)close(from_fd);
		return(1);
	}
	if (!blen && !(bp = (char *) malloc(blen = sbp->st_blksize))) {
		(void)fprintf(stderr, MSGSTR(NOMEM, "%s: %s: out of memory.\n"), pname, from);
		return(1);
	}
	while ((nread = read(from_fd, bp, blen)) > 0)
		if (write(to_fd, bp, nread) != nread) {
			(void)fprintf(stderr, "%s: %s: %s\n",
			    pname, to, strerror(errno));
			goto err;
		}
	if (nread < 0) {
		(void)fprintf(stderr, "%s: %s: %s\n", pname, from, strerror(errno));
err:		(void)unlink(to);
		(void)close(from_fd);
		(void)close(to_fd);
		return(1);
	}

	rc = 0;

        /* obtain the source's access information */
        if ((aclp = acl_fget(from_fd)) == NULL) {
                rc=1;
        }

	/*
	 * call fchown and fchmod to duplicate the source's
	 * owner i.d., group i.d., and file mode.  Clear the
	 * setuid and setgid bits if fchown fails.
	 */
	if (fchown(to_fd, sbp->st_uid, sbp->st_gid)) { 	/* fail on fchown */
		rc = 1;
		sbp->st_mode &= ~(S_ISUID | S_ISGID);
	}
	if (fchmod(to_fd, sbp->st_mode)) 	/* fail on fchmod */
		rc = 1;

        /* set the source's access information */
        if ((aclp = acl_fput(to_fd, aclp, 0)) == -1) {
                rc=1;
        }

	if (rc) {
		(void)fprintf(stderr, MSGSTR(CANTDUP, "%s: %s: unable to duplicate owner and mode after move.\n"), pname, to);
	}

	free((void *) aclp); /* free the acl */
	(void)close(from_fd);
	/*
	 * If close fails, don't want an empty target file, so
	 * print diagnostic msg and unlink the target file.
	 */
	if (close(to_fd) < 0)
	{
		(void)fprintf(stderr, "%s: %s: %s\n",
			pname, to, strerror(errno));
		unlink(to);
		return(1);
	}

	tval[0].tv_sec = sbp->st_atime;
	tval[1].tv_sec = sbp->st_mtime;
	tval[0].tv_usec = tval[1].tv_usec = 0;
	(void)utimes(to, tval);
	if (unlink(from) == -1) {
		(void)fprintf(stderr,
			MSGSTR(CANTRM, "mv: cannot remove source file %s\n"), from);
		return(1);
	}
	return(0);
}

/*
 * NAME: copy
 *
 * FUNCTION: calls cp and rm to move special files.
 *
 * RETURNS: 0 if successful, 1 on error
 */

static int
copy(from, to)
	char *from, *to;
{
	int pid, status;

	if (!(pid = fork())) {
		execl(_PATH_CP, "mv", "-phR", from, to, 0);
		(void)fprintf(stderr, MSGSTR(NOEXEC, "%s: cannot exec %s.\n"), pname, _PATH_CP);
		_exit(1);
	}
	(void)waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status))
		return(1);
	if (!(pid = fork())) {
		execl(_PATH_RM, "mv", "-rf", from, 0);
		(void)fprintf(stderr, MSGSTR(NOEXEC, "%s: cannot exec %s.\n"), pname, _PATH_RM);
		_exit(1);
	}
	(void)waitpid(pid, &status, 0);
	return(!WIFEXITED(status) || WEXITSTATUS(status));
}

/*
 * NAME: usage
 *
 * FUNCTION: prints usage message
 */

static usage()
{
	(void)fprintf(stderr,
MSGSTR(USAGE, 
"Usage: %s [-i | -f] [--] src target\n   or: %s [-i | -f] [--] src1 ... srcN directory\n"), pname, pname);
	exit(1);
}
