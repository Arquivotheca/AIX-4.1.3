static char sccsid[] = "@(#)21  1.21.1.7  src/bos/usr/bin/chown/chown.c, cmdsdac, bos411, 9428A410j 4/7/94 17:06:31";
/*
 * COMPONENT_NAME:  (CMDSDAC) security: access control
 *
 * FUNCTIONS: chown    
 *
 * ORIGINS: 3 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#define _ILS_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include "chown_msg.h"
nl_catd catd;
#define	MSGSTR(Num, Str) catgets(catd,MS_CHOWN,Num,Str)

extern char *optarg;
extern int optind;

void Perror( char *);
int isnumber( char *);
int chownr( char *, uid_t, gid_t);
void parseug(char *, uid_t *, gid_t *);
static void cnv2posix(char *);

struct	passwd	*pwd;
struct	group	*grp;
struct	stat	stbuf;
uid_t	newuid;
gid_t   newgid;
int     hflag = 0, fflag = 0;
int	status = 0;

/*
 * NAME: chown [-fhR] uid file | directory ...
 * FUNCTION: changes the owner of the files to uid
 *    FLAGS
 *      -f   Suppresses all error reporting of messages,except Usage
 *	-h   Causes chown to act on the symlink itself, and NOT follow
 *	     or access the file that the link references.
 *      -R   Causes chown to descend recursively through its directory 
 *           arguments, setting the specified group ID.  When symbolic links 
 *           are encountered, the file pointed to is changed, but the link
 *           is NOT traversed.
 *
 * RETURNS:
 *     0 If successful
 *    >0 If an error occurred.
 */
main(argc, argv)
char *argv[];
int argc;
{
	register c;
	int     Rflag = 0;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_CHOWN,NL_CAT_LOCALE);


	if(argc < 3) {
		fprintf(stderr, 
MSGSTR(USAGE,"Usage: chown -fhR {username|uid}[:{groupname|gid}] file ...\n"));
							
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
				fprintf(stderr,
MSGSTR(USAGE,"Usage: chown -fhR {username|uid}[:{groupname|gid] file ...\n"));
							
				exit(4);
				break;
		}
	}

	/*
	 * check for BSDism of "user.group"
	 */
	parseug(argv[optind], &newuid, &newgid );

	optind++;

	for(; optind<argc; optind++) {
		/* do stat for directory arguments */
		if (lstat(argv[optind], &stbuf) < 0) {
			status ++; 
			Perror(argv[optind]);
			continue;
		}
		if (Rflag && ((stbuf.st_mode&S_IFMT) == S_IFDIR)) {
			status += chownr(argv[optind], newuid, newgid);
			continue;
		}
#ifndef _BLD
		if (hflag)
		{
			if (lchown(argv[optind], newuid, newgid )) {
				status ++; 
				Perror(argv[optind]);
				continue;
			}
			continue;
		}
#endif
		if (chown(argv[optind], newuid, newgid )) {
			status ++; 
			Perror(argv[optind]);
			continue;
		}
	}
	exit(status);
}

/*
 * NAME:
 * FUNCTION: is s a number
 */
int isnumber( char *s)
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}

/*
 * NAME: chownr
 * FUNCTION: recursively descend the directory and change the owner of each
 *  file
 */
int chownr( char *dir, uid_t uid, gid_t gid)
{
	DIR *dirp;
	struct dirent *dp;
	struct stat st;
	char savedir[MAXPATHLEN];
	int ecode = 0;
	extern char *getwd();

	if (getwd(savedir) == (char *)0) {
		if (!fflag)
			fprintf( stderr, "chown: %s\n", savedir);
		exit(255);
	}

	/*
	 * Change what we are given before doing its contents.
	 * 
	 * Philosophy here is that if it is an error that chown is
	 * encountering, we are incrementing ecode, if it's an
	 * error produced by chdir or opendir (external to chown)
	 * we return 1, instead of keeping a counter.  Assuming that this
	 * is for the purpose of knowing the number of times we
	 * had the correct conditions for doing the chown, but unable
	 * to carry out the mission.
	 */

	if (chown(dir, uid, gid ) < 0 )   {
	    	Perror(dir);
		ecode++;		
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
			continue;       /* read "." and ".." and in-betweens" */

		if (lstat(dp->d_name, &st) < 0) {
			ecode ++; 
			Perror(dp->d_name);
			continue;
		}
		if ((st.st_mode&S_IFMT) == S_IFDIR) {
			ecode += chownr(dp->d_name, uid, gid);
			continue;
		}

#ifndef _BLD
		if (hflag)
		{
			if (lchown(dp->d_name, uid, gid) < 0) {
				ecode ++; 
				Perror(dp->d_name);
				continue;
			}
			continue;
		}
#endif
		if (chown(dp->d_name, uid, gid) < 0) {
			ecode ++; 
			Perror(dp->d_name);
		}
	}
	closedir(dirp);
	if (chdir(savedir) < 0) {
		if (!fflag)
			fprintf( stderr, MSGSTR(NOBACK, 
				"chown: Cannot change directory back to %s.\n"), savedir);
		exit(255);
	}
	return (ecode);
}

/*
 * NAME: Perror
 * FUNCTION: display perror messages
 */
void Perror(char *s)
{

	if (!fflag) {
		fprintf(stderr, "chown: ");
		perror(s);
	}
	return;
}


/*
 * BSDism of being able to specify both user and group 
 */
void parseug(char *arg, uid_t *uidp, gid_t *gidp)
{
	char	*gidstr;

	cnv2posix(arg);	/* convert old AIX to new POSIX separator character */

	for (gidstr=arg; *gidstr; gidstr++)
		if (*gidstr == ':')
			break;

	if (*gidstr)
	{
		*gidstr = (char)NULL;
		if ((gidstr == arg)	      ||  /* if ':' at beginning */
		    (*++gidstr == (char)NULL) ||  /* or ':' at the end */
		    (strchr(gidstr, ':')))	  /* or multiple ':'s: error */
		{
				fprintf( stderr,  MSGSTR(USAGE, "Usage: chown -fhR {username|uid}[:{groupname|gid}] file ...\n")); 
			exit(4);
			
		}
	}
	else
		gidstr = NULL;


	if ((pwd=getpwnam(arg)) == NULL) 
	{
		if (isnumber(arg)) 
			*uidp =  (uid_t)strtoul(arg, (char **) 0, 10);
		else {
			if (!fflag)
				fprintf( stderr, MSGSTR(BADUID, "chown: %s is an unknown username. \n"), arg); 
			exit(4);
		}
	}
	else
		*uidp = pwd->pw_uid;


	if (gidstr)
	{
		if ((grp=getgrnam(gidstr)) == NULL) 
		{
			if (isnumber(gidstr)) 
				*gidp =  (gid_t)strtoul(gidstr, (char **) 0, 10); 
			else {
				if (!fflag)
					fprintf( stderr,  MSGSTR(BADGID, 
					"chown: %s is an unknown groupname.\n"), gidstr);
				exit(4);
			}
		}
		else
			*gidp = grp->gr_gid;
	}
	else
		*gidp= (unsigned long) -1;    /* set to -1 so the call to chown() will not change group id */
}


static void
cnv2posix(char	*arg)
{
	char	*p;

	/*
	 * if the string contains a colon then it's already POSIX
	 * or an error, which will be caught in the mainline code
	 */
	if (strchr(arg, ':'))
		return;

	/*
	 * if there are no periods in it either, then there's no
	 * need to do anything
	 */
	if ((p=strchr(arg, '.')) == NULL)
		return;

	/*
	 * if the whole string, including period(s), forms a known
	 * username then use it "as is"
	 */
	if (getpwnam(arg))
		return;

	/*
	 * else, we must assume this is an old AIX format and change
	 * the period(s) into colon(s)
	 *
	 * if there are multiple separators, the error will be caught 
	 * in the mainline code
	 */
	*p++ = ':';
	while (p=strchr(p, '.'))
		*p++ = ':';
	return;
}
	
