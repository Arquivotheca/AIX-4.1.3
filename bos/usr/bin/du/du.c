static char sccsid[] = "@(#)90	1.32  src/bos/usr/bin/du/du.c, cmdfiles, bos412, 9446C 11/14/94 16:45:56";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: du
 *
 * ORIGINS: 3, 27, 71
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 * OSF/1 1.1
 *
 *	du -- summarize disk usage
 *              du [-a | -s] [-rlkx] [name ...]
 */

#include	<stdio.h>
#include	<string.h>
#include	<sys/limits.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<dirent.h>

#ifndef NAME_MAX
#define NAME_MAX MAXNAMLEN
#endif

/* return the number of DEV_BSIZE sized blocks */
#define UBLOCKS(n) (n.st_blocks)

/* DEV_BSIZE is always 512 for AIX 3.2/4.1, however, prepare for	*/
/* different DEV_BSIZE values.						*/

#if DEV_BSIZE == 512
	#define OUTPUT_BLOCKS(n) (kflag ? (n+1) >> 1 : n)
#else
	#define OUTPUT_BLOCKS(n) ((n*DEV_BSIZE + block_size - 1 ) / block_size)

	static int	block_size = 512;   /* default block size is 512 bytes.    */
				    /* if -k flag is specified, use 1024   */
#endif

#include <locale.h>
#include <nl_types.h>
#include "du_msg.h"
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_DU,Num,Str)

static char    path[PATH_MAX];

#define EQ(x,y)	(strcmp(x,y)==0)
#define ISDIR() ((Statb.st_mode&S_IFMT)==S_IFDIR)
#define ML	2000

struct ml {
	struct ml *next;
	dev_t	dev[ML];
	ino_t	ino[ML];
};

static struct ml *curptr;       /* pointer to current structure */
static struct ml firststrct;    /* first structure */

static int	linkc;

static struct	stat	Statb;
static dev_t	current_device; /* current device of file system for -x flag use */

static int 	errcode;
static char    aflag = 0;
static char    sflag = 0;
static char    lflag = 0;
static char    xflag = 0;
static char 	kflag = 0;

static char    nodotdot[] = "du: fatal error - can't cd to .. (%s)\n";

static long	descend();

main(argc, argv)
int argc;
char **argv;
{
	long blocks = 0;
	char userdir[PATH_MAX+2];
	struct ml *p, *tmpp;
	int c;

	(void) setlocale (LC_ALL,"");
	catd = catopen((char *)MF_DU, NL_CAT_LOCALE);
	while((c=getopt(argc, argv, "arslkx")) != EOF) 
	switch(c) {
			case 'a':
				aflag++;
				continue;

			case 'r':
				continue;

			case 's':
				sflag++;
				continue;

			case 'l':
				lflag++;
				continue;

			case 'k':
				kflag++;
				#if DEV_BSIZE != 512
					block_size = 1024;
				#endif
				continue;

			case 'x':
				xflag++;
				continue;

			default:
				fprintf(stderr,MSGSTR(USAGE,"Usage: du [-a | -s] [-rlkx] [name ...]\n"));
				exit(2);
			}

	if (aflag && sflag) {
				fprintf(stderr,MSGSTR(USAGE,"Usage: du [-a | -s] [-rlkx] [name ...]\n"));
				exit(2);
	}

	argv = &argv[optind-1];
	argc -= optind;
	if(argc == 0) {
		argc = 1;
		argv[1] = ".";
	}

	if ((char *)getcwd(userdir,PATH_MAX+2) == NULL) {
		fprintf(stderr,MSGSTR(CANTFIND,"du: can't find current directory\n"));
		exit(2);
	}

	curptr = &firststrct;
	curptr->next = NULL;

	while(argc--) {
		int	len;

		linkc = 0;
		if ((len = strlen(*++argv)) >= sizeof(path)) {
			fprintf(stderr,MSGSTR(NAMETOOLONG,"du: %s: name too long\n"), *argv);
			continue;
		}
		strcpy(path, *argv);

		if(lstat(path,&Statb)<0) {
			error(path);
			continue;
		}

		if (!ISDIR()) {
			blocks = UBLOCKS(Statb);
			if(Statb.st_nlink > 1) {
				if (lflag) {
					blocks += Statb.st_nlink - 1;
					blocks /= Statb.st_nlink;
				}
			}
      			printf("%ld\t%s\n",OUTPUT_BLOCKS(blocks),path);
			continue;
		}

		if (chdir(path) < 0) {
			error(path);
			continue;
		}

		current_device = Statb.st_dev;
 
		blocks = descend(path + len);

		curptr = &firststrct;

		if(sflag)
 			printf("%ld\t%s\n", OUTPUT_BLOCKS(blocks), path);

		if (chdir(userdir)) {
			error(userdir);
			exit(2);
		}
	}

	exit(errcode ? 2 : 0);
}

/* On entry, endofname points to the NUL at the end of path and
 * Statb contains a stat of the file whose full name is in path.
 * If path is a directory, it is the working directory.
 */

static long 
descend(endofname)
char *endofname;
{
	register struct	dirent	*dp;
	register char	*c1, *c2;
	long blocks = 0;
	struct  dirent  dentry;
	struct  ml  *p;
	DIR	*dir = NULL;		/* open directory */
	long	offset = 0;
	int	i, count;

	if (xflag && Statb.st_dev != current_device)
		return (0);

	blocks = UBLOCKS(Statb);

	if (!ISDIR()) {
		if(Statb.st_nlink > 1) {
			if (lflag) {
				blocks += Statb.st_nlink - 1;
				blocks /= Statb.st_nlink;
			} else {
				if (linkc == ML) {
					if (curptr->next == NULL) {
						p = curptr;
						curptr = (struct ml *) malloc(sizeof(struct ml));
						if (!curptr) {
					    	perror("du");
					    	exit(2);
						}
						p->next = curptr;
						curptr->next = NULL;
					} else curptr = curptr->next;
					linkc = 0;
				}
				for (p=&firststrct;p!=curptr->next;p=p->next) {
					count = p == curptr ? linkc : ML;
					for(i = 0; i < count; ++i)
						if(p->ino[i]==Statb.st_ino &&
					   	   p->dev[i]==Statb.st_dev) 
							return 0;
				}
				curptr->dev[linkc] = Statb.st_dev;
				curptr->ino[linkc] = Statb.st_ino;
				++linkc;
			}
		}
		if(aflag)
			printf("%ld\t%s\n", OUTPUT_BLOCKS(blocks), path);
		return(blocks);
	}

	if (endofname + (NAME_MAX+1) >= &path[sizeof(path)]) {
		fprintf(stderr,MSGSTR(TOODEEP,"du: %s: TOO DEEP!\n"), path);
		errcode++;
		return(blocks);
	}

	if ((dir=opendir(".")) == NULL) {
		error(path);
		return(0);
	}
	while ((dp=readdir(dir)) != NULL) {
		if(dp->d_ino==0 || EQ(dp->d_name, ".") || EQ(dp->d_name, "..")
			|| dp->d_name[0] == 0)      /* ?? */
				continue;

		if(dir->dd_fd > 10) {
			offset = telldir(dir);
			/* save dp because closedir free's the storage */
			bcopy(dp,&dentry,sizeof(dentry));
			closedir(dir);
			dp = &dentry;
			dir = NULL;
		}
		/* each directory entry */
		c1 = endofname;
		*c1++ = '/';
		c2 = c1;
		c1 = strncpy(c1, dp->d_name, (size_t)NAME_MAX);
		c1 += strlen(dp->d_name); 
		if(lstat(c2, &Statb) < 0) {
			error(path);
			return(0);
		}
		if (ISDIR()) {
			if (chdir(c2)) {
				error(path);
			}
			else {
				blocks += descend(c1);
				*endofname = '\0';
				if (chdir("..")) {
					fprintf(stderr,
					MSGSTR(NODOTDOT,nodotdot), path);
					exit(2);
				}
			}
		}
		else blocks += descend(c1);
		if (dir == NULL) {
			if ((dir=opendir(".")) == NULL) {
				error(path);
				return(0);
			}
			if (offset > 0) 
				seekdir(dir,offset);
		}
	}   /* End of loop over entries in a directory */
	if(dir != NULL)
		closedir(dir);
	*endofname = '\0';
	if(!sflag)
		printf("%ld\t%s\n", OUTPUT_BLOCKS(blocks), path);
	return(blocks);
}

static
error(s)
char *s;
{ 
	fputs("du: ",stderr);
	errcode++;
	perror(s);
}
