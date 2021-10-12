static char sccsid [] = "@(#)06 1.12  src/bos/usr/bin/mkdir/mkdir.c, cmdfiles, bos412, 9446C 11/14/94 16:48:49";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: mkdir
 *
 * ORIGINS: 3, 18, 27
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
 * static char rcsid[] = "RCSfile: mkdir.c,v Revision: 2.6  (OSF) Date: 90/10/07 16:44:07
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <locale.h>
#include "mkdir_msg.h"
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_MKDIR,Num,Str)
#define CHMOD "/usr/bin/chmod"

static mode_t  mask;		/* mode mask */
static int	status = 0;	/* return code from mkdir */
static int	mflag = 0;	/* mode flag */
static int	pflag = 0;	/* create path flag */
static int	level = 0;
static char	*modestr;
static char	s[PATH_MAX];

/*
 * NAME: mkdir [-p] [-m mode] Directory ... 
 * 
 * FUNCTION: makes the specified directories.
 *
 * NOTES:
 *	  -m mode 	Sets the file permission bits of the
 *			newly created directory to the specified mode.
 *			If the -m option is not specified, rwx is assumed.
 *
 *	  -p		Creates any missing intermediate directories 
 *			in the pathname.
 */

main(int argc, char **argv)
{
	int c;
	char *dirp;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_MKDIR, NL_CAT_LOCALE);

	/* ignore interrupts */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	while ((c=getopt(argc,argv,"pm:")) != EOF) {
		switch(c) {
		case 'p':
			++pflag;
			break;
		case 'm':
			if (mflag) {
				(void)fprintf (stderr, MSGSTR(NOMULT, "mkdir: cannot have multiple -%c options.\n"), c);
				exit(2);
			}
			++mflag;
			modestr = optarg;
			break;
		default:
			usage();
		}
	} 

	if (!argv[optind]) /* check for insufficient no. of arguments */
		usage();

	/* Obtain the file mode creation mask by
	 * calling umask.  umask sets the file creation
	 * mode to the specified parameter and returns 
	 * the previous mask.  umask must then be called a
	 * second time to reset the mask to its original value.
	 * The file mode creation mask is needed because it may
	 * affect file/directory creation modes
	 */
	mask = umask(0);
	(void)umask(mask);

	/* process the remaining arguments 
	 * (considered to be the file names)
	 */
	for (c = optind; c < argc; c++) {
		level = 0;
		if (pflag) { /* Create intermediate path components */
			if(mkdirp(argv[c]) < 0) {
				status = 2;
				continue;
			}	
		}	/* end pflag */
		/* Create target directory */
		if(Mkdir(argv[c]) < 0)
			status = 2;
		else if (mflag) {
			level++;
			sprintf(s, "%s %s %s", CHMOD, modestr, (char *)argv[c]);
			if (system(s)) {
				for (;dirp = argv[c], level > 0; level--) {
					rmdir(dirp);
					if ((dirp = strrchr(dirp, '/')) != NULL)
						*dirp = '\0';
				}
				exit(2);
			}
		}
	}	/* end processing of file arguments */
	exit(status);
}

/*
 * NAME: usage
 *
 * FUNCTION: print usage message and exit.
 *
 */
static usage()
{
	(void) fprintf (stderr, MSGSTR(USAGE, "Usage: mkdir [-p] [-m mode] Directory ... \n"));
	exit(1);
}

/*
 * NAME: Mkdir
 *
 * FUNCTION: Calls mkdir with a pathname and a mode. 
 *
 * RETURN VALUES: -1 on error, 0 for successful
 */
static int
Mkdir(unsigned char *d)
{
	if (mkdir((char *)d, 0777) == 0)
		return(0);
	/*
	* For EACCES, ENOENT and ENOTDIR errors, print the name
	* of the parent of the target directory instead of the
	* target directory.
	*/
	if (errno == EACCES || errno == ENOENT || errno == ENOTDIR) {
		unsigned char *slash;
		if ((slash = strrchr(d, '/')) != (unsigned char *)NULL) {
			if (slash == d)
			      slash++;
			*slash = '\0';
		} else {
			d[0] = '.';
			d[1] = '\0';
		}
		(void)fprintf(stderr,
		MSGSTR(NOACCESS, "mkdir: cannot access directory %s.\n"), d);
	} else {
		/* if pflag and directory exists, do not report error */
		if (pflag && errno == EEXIST) {
			struct stat buf;
			if (stat((char *)d,&buf) == 0 && S_ISDIR(buf.st_mode))
				return(0);
		}
		(void)fprintf(stderr, 
		MSGSTR(NOMAKE, "mkdir: cannot create %s.\n"), d);
	}
	perror((char *)d);
	return(-1);
}

/*
 * NAME: mkdirp
 *
 * FUNCTION: creates intermediate path directories if the -p
 *           option is specified.  All intermediate directories
 *           are created with the umask permissions. If the umask
 *           prevents the user wx bits from being set, chmod is
 *           called to ensure that those mode bits are set so that
 *           following path directories can be created. If any
 *           intermediate directory already exists, it is silently
 *           ignored. The final directory will be created with the
 *           mode as specified by -m or the default mode of 0777.
 *
 * RETURN VALUES: -1 on error, 0 for successful
 */
static int
mkdirp(unsigned char *dir)
{
	unsigned char *dirp;
	struct stat buf;

	/*
	 * Skip any leading '/' characters
	 */
	for (dirp = dir; *dirp == '/'; dirp++)
		continue;
	/*
	 * For each component of the path, make sure the component
	 * exists.  If it doesn't exist, create it.
	 */
	while ((dirp = strchr(dirp, '/')) != (unsigned char *)NULL) {
		*dirp = '\0';
		/* Attempt to stat the directory before trying to create it.
		 * This should alleviate the problem of trying to create
		 * existing directories in a read-only filesystem which
		 * return with EROFS instead of EEXIST.
		 */
		if (stat(dir, &buf) != 0) {
			if (mkdir((char *)dir, 0777) != 0) {
				(void)fprintf(stderr, MSGSTR(NOMAKE, "Cannot create %s.\n"), dir);
				perror((char *) dir);
				return (-1);
			}
			/*  If this directory did not already exist AND
			 *  the umask prevented the user wx bits from being set,
			 *  then chmod it to set at least u+wx so the next
			 *  one can be created.
			 */
			level++;
			if (mask & 0300) {
				sprintf(s, "%s u+wx %s", CHMOD, dir);
				system(s);
			}
		}
		for (*dirp++ = '/'; *dirp == '/'; dirp++)
			continue;
	}
	return(0);
}
