static char sccsid[] = "@(#)80  1.6  src/bos/usr/bin/del/del.c, cmdfiles, bos411, 9428A410j 1/12/94 17:56:00";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: del
 *
 * ORIGINS: 3, 26, 27
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
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/limits.h>	/* for PATH_MAX */
#include	<sys/stat.h>
#include	<dirent.h>
#include	<sys/access.h>
#include	<sys/mode.h>
#include	<sys/errno.h>
#include 	<ctype.h>


#include <locale.h>

#include <nl_types.h>
#include "del_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_DEL,Num,Str)

#define UNSET 2                 /* init value for YES/NO variable */
#define YES   1
#define NO    0

char pathname[PATH_MAX+1];

/*
 * NAME: del [-] file...
 *
 * FUNCTION: delete with confirmation
 *
 * NOTES:    The "-" option indicates that a confirmation
 *           prompt is to be issued for each file, not just
 *           for the group.
 */
 
main(argc,argv)
int argc;
char **argv;
{
    int rv = 2;

    (void) setlocale (LC_ALL,"");
    catd = catopen((char *)MF_DEL, NL_CAT_LOCALE);

    rv = del_main(argc,argv);
    exit(rv);
}

#define max(a,b) ((a) > (b) ? (a) : (b))

#define allargs  i=1; i<ac; i++

#define LINEWIDTH       78      /* width of filename printout */

int     longestname;            /* length of longest name */
int     numtodelete;            /* number left after bad args */
char    oneatatime;             /* was "-" given? */

/* from IN/standard.h */
#define EXITOK    ((unsigned char)(0))
#define EXITBAD   ((unsigned char)(254))

/*
 * NAME: del_main  
 *
 * FUNCTION: deletes files with confirmation
 */

del_main(ac, av)
register ac;
register char **av; 
{
    register i;
    register int xcode = 0;    /* assume exit code 0 */
    char cmd[4] = "del";

    /* confirm one at a time? */
    if (ac > 1 && av[1][0] == '-' && av[1][1] == (char) 0) {
	oneatatime++;
	av++;
	ac--;
    }

    if (ac == 1) {
	fprintf(stderr,MSGSTR(USAGE,"Usage: del [-] File...\n"));
	exit(EXITBAD);
    }

    /* check for del permission.  if none, clobber arg with 0. */
    numtodelete = ac - 1;
    for (allargs) 
	if (!candel(av[i])) {
	    av[i] = 0;
	    --numtodelete;
	    xcode = EXITBAD;
	}
	else  longestname = max(longestname, mbswidth(av[i], strlen(av[i])));
    if (numtodelete == 0)
	exit(EXITBAD);

    if (oneatatime || numtodelete == 1) {
	for (allargs)
	    if (av[i]) {
		printf(MSGSTR(DELETE_MSG1,"del: Remove %s?  "),av[i] );
		if(confirm())
		if (unlink(av[i]) == -1) {
        	    fprintf(stderr, MSGSTR(NOTREMOVED,"%1$s: Cannot remove %2$s.\n"), cmd, av[i]);
        	    perror("");
		    xcode = EXITBAD;
		}
	    }
    } else {
	dumpargs(ac, av);
	printf(MSGSTR(DELETE_MSG2,"del: Remove these files?  "));
	if (confirm())
	    for (allargs)
		if (av[i] && unlink(av[i]) == -1) {
        	    fprintf(stderr, MSGSTR(NOTREMOVED,"%1$s: Cannot remove %2$s.\n"), cmd, av[i]);
        	    perror("");
		    xcode = EXITBAD;
		}
    }
    exit(xcode);
}

/*
 * NAME: confirm
 *
 * FUNCTION: issues confirmation prompt for deletion of arg and
 *           returns YES or NO.  No response considered  to be
 *           YES.  Exits immediately on unexpected EOF.
 */
 
confirm()
{
	char ans[100];
	int	in;

	fgets(ans,100,stdin);
	
	/* normal check */
	/* yflag marks default case when YESSTR not set anywhere */

	if ((ans[0] == '\n')||(rpmatch(ans) == 1) ) {
		printf(MSGSTR(YES_MSG, "yes\n"));
		return YES;
	} else {
		printf(MSGSTR(NO_MSG, "no\n"));
		return NO;
	}

}

/*
 * NAME: candel
 *
 * FUNCTION: checks permissions for removal of file.  
 *           returns YES or NO.
 */
 
candel(file)
	char *file; {
	static dotaccess = UNSET;       /* write permission on "."? */
	char buffer[PATH_MAX];
	struct stat statb;
	register char *fp;

	if (stat(file, &statb) == -1) {
		fprintf(stderr, "del: %s: %s\n", file, strerror(errno));
		return NO;
	}
	if ((statb.st_mode & S_IFMT) == S_IFDIR) {
		fprintf(stderr,MSGSTR(DIRECTORY_MSG,"del: %s: Directory\n"), file);
		return NO;
	}

	/* if no slash, check "." access */
	for (fp=file; *fp && *fp != '/'; fp++)
		;
	if (!*fp) {
		if (dotaccess == UNSET)
			dotaccess = (access(".", W_OK) < 0? NO: YES);
		if (dotaccess == NO)
			fprintf(stderr,MSGSTR(PERM_MSG,"del: %s: Permission denied\n"),file);
		return dotaccess;
	}

	/* has a slash in it. */
	strcpy(buffer, file);
	if (access(dname(buffer), W_OK) < 0) {
		fprintf(stderr,MSGSTR(PERM_MSG,"del: %s: Permission denied\n"),file);
		return NO;
	}
	return YES;
}

/*
 * NAME: dumpargs
 *
 * FUNCTION: dumps all the args in a multicolumn list.  
 */
 
dumpargs(ac, av)
	char **av; {
	register numprinted;
	register numperline = 1;
	char format[PATH_MAX + 5];
	register int i;

	if (longestname < LINEWIDTH)
		numperline = LINEWIDTH / (longestname + 1);
	sprintf(format, "%%-%ds", longestname);
	numprinted = 0;

	for (allargs) {
		printf(format, av[i]);
		if (++numprinted % numperline && numprinted != numtodelete)
			putchar(' ');
		else    putchar('\n');
	}
}

