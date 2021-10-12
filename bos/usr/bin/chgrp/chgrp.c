static char sccsid[] = "@(#)18  1.15.1.6  src/bos/usr/bin/chgrp/chgrp.c, cmdsdac, bos411, 9428A410j 4/27/94 15:29:03";
/*
 * COMPONENT_NAME: (CMDSDAC) security:  access control commands
 *
 * FUNCTIONS: chgrp
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Usage:  chgrp gid file ...
 */
 
#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <dirent.h>
#include <locale.h>
#include "chgrp_msg.h" 
nl_catd  catd;   /* Message catalog file descriptor */
#define MSGSTR(num,str) catgets(catd,MS_chgrp,num,str)  /*MSG*/

extern char *optarg;
extern int optind;

int	status = 0;
int	hflag = 0, fflag = 0;

/* function prototypes */

int isnumber(char *);
int chownr(char *, gid_t);
void fatal(int, char *, char *);
void Perror (char *);

/*
 * NAME: chgrp
 *                                                                    
 * FUNCTION: Changes the group ownership of a file or directory.
 *     FLAGS: 
 *      -f   Suppresses all error messages except usage messages
 *      -h   Causes chgrp to act on the symlink itself, and NOT follow
 *           or access the file that the link references.
 *      -R   Causes chgrp to descend recursively through its directory 
 *           arguments, setting the specified group ID.  When symbolic links 
 *           are encountered, the file pointed to is changed, but the link is
 *           not traversed.
 *     RETURNS:
 *       0   If successful, i.e., all requested changes were made.
 *      >0   If an error occurred.
 */  

main(argc, argv)
int argc;
char *argv[];
{
	register int c;
	struct group *gr;
	struct stat stbuf;
	gid_t gid;
	int Rflag = 0;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_CHGRP, NL_CAT_LOCALE);

	if(argc < 3) {
		fprintf(stderr,MSGSTR(M_MSG_0,
			"%s: Usage: %s -fhR {groupname|gid} file ...\n"),argv[0],argv[0]);
		exit(4);
	}

	while ((c = getopt(argc,argv,"fhR")) != EOF) {
		switch(c) {
			case 'f':
				fflag++;	
				break;
			case 'h':
				hflag++;	
				break;
			case 'R':
				Rflag++;	
				break;
			case '?':
				fprintf(stderr,MSGSTR(M_MSG_0,
					"%s: Usage: %s -fhR {groupname|gid} file ...\n"),argv[0],argv[0]);
				exit(4);
				break;
		}
	}

	if((gr=getgrnam(argv[optind])) == NULL)  
		if(isnumber(argv[optind]))  
			gid = strtoul(argv[optind], NULL, 10);
		else {
			if (!fflag) 
				fprintf(stderr,MSGSTR(M_MSG_2,
					"%s: unknown group: %s\n") ,argv[0],argv[optind]);
			exit(4);
		}
	else
		gid = gr->gr_gid;
	
	optind++;
	for(; optind<argc; optind++) {
		/* do stat for directory arguments */
		if (lstat(argv[optind], &stbuf)) {
			status++;
			Perror(argv[optind]);
			continue;
		}
		if (Rflag && ((stbuf.st_mode & S_IFMT) == S_IFDIR)) {
			status += chownr(argv[optind], gid);
			continue;
		}
#ifndef _BLD
		if (hflag)
		{
			if (lchown(argv[optind], -1, gid ) < 0) {
				status ++;
				Perror(argv[optind]);
				continue;
			}
			continue;
		}
#endif
		if(chown(argv[optind],-1,gid) < 0)  {
			status++;
			Perror(argv[optind]);
			continue;
		}
	}
	exit(status);
}

/* 
 * NAME: isnumber
 * FUNCTION: is s a number
 * RETURN VALUES:   0 if not number
 *                  1 if number
 */

int
isnumber(char *s)
{
	register int c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}

/*
 * NAME: chownr
 * FUNCTION: recursively descend directory and change the group id of all
 *   files.
 * RETURNS:  integer representing number of unsuccessful attempts to
 *           change group ids
 */

int
chownr(char *dir, gid_t gid)
{
	DIR *dirp;
	struct dirent *dp;
	struct stat st;
	char savedir[1024];
	int ecode=0;

	if (getwd(savedir) == 0)
		fatal(255, "%s", savedir);
	/*
	 * Change what we are given before doing its contents.
	 */
	if (chown(dir, -1, gid) < 0 )  {
		ecode++;
		Perror(dir);
	}
	if (chdir(dir) < 0) {
		Perror(dir);
		return (1);
	}
	if ((dirp = opendir(".")) == NULL) {
		Perror(dir);
		return (1);
	}
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (dp->d_name[0] == '.' && (!dp->d_name[1] ||
		   dp->d_name[1] == '.' && !dp->d_name[2]))
		     continue;       /* read "." and ".." and in-betweens */

		if (lstat(dp->d_name, &st) < 0) {
			Perror(dp->d_name);
			ecode++;
			continue;
		}
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			ecode += chownr(dp->d_name, gid);
			continue;
		}
#ifndef _BLD
		if (hflag)
		{
			if (lchown(dp->d_name, -1, gid ) < 0) {
				ecode ++;
				Perror(dp->d_name);
				continue;
			}
			continue;
		}
#endif
		if (chown(dp->d_name, -1, gid) < 0 ) {
		     Perror(dp->d_name);
			ecode++;
		}
	}
	closedir(dirp);
	if (chdir(savedir) < 0)
		fatal(255,MSGSTR(M_MSG_4,"can't change back to %s"), savedir);
	return (ecode);
}

/*
 * NAME: fatal
 * FUNCTION:  displays fatal error messages to the user
 * RETURNS:  NONE
 */

void
fatal(int status, char *fmt, char *a)
{

	fprintf(stderr, "chgrp: ");
	fprintf(stderr, fmt, a);
	putc('\n', stderr);
	exit(status);
}

/*
 * NAME: Perror
 * FUNCTION: displays perror messages if error msg reporting is not
 *           suppressed (!fflag)
 * RETURNS:  NONE
 */

void
Perror(char *s)
{

	if (!fflag) {
		fprintf(stderr, "chgrp: ");
		perror(s);
	}
}
