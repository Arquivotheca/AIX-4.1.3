static char sccsid[] = "@(#)92  1.18  src/bos/usr/bin/diff/diffdir.c, cmdfiles, bos41J, 9510A_all 3/3/95 10:32:16";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 18, 26, 27
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
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */

/*
 * diff - directory comparison
 */
#define _ILS_MACROS
#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include "diff.h"

extern nl_catd catd;

/*
 * Output format options
 */
extern int	opt;

/*
 * Options on hierarchical diffs.
 */
extern int	lflag;			/* long output format with header */
extern int	rflag;			/* recursively trace directories */
extern int	sflag;			/* announce files which are same */
extern char	*begin;			/* do file only if name >= this */

/*
 * State for exit status.
 */
extern int	status;
extern int oldstatus;
static int	anychange;

/*
 * Variables for diffdir.
 */
extern char	**diffargv;	/* option list to pass to recursive diffs */

/*
 * Input file names.
 * With diffdir, file1 and file2 are allocated BUFSIZ space,
 * and padded with a '/', and then efile0 and efile1 point after
 * the '/'.
 */
extern char	*file1, *file2, *efile1, *efile2;
extern struct	stat stb1, stb2;
extern int	done();

#define	ONLY	1		/* Only in this directory */
#define	SAME	2		/* Both places and same */
#define	DIFFER	4		/* Both places and different */
#define	DIRECT	8		/* Directory */
#define INIT	16
#define MAGIC	"/etc/magic"

extern char diffh[], diff[], pr[];
/*
extern	pid_t	fork(), wait();
*/
extern int name_max;	/* Passes in NAME_MAX */
extern int optind;

static char *filetype();
static struct	dirent **setupdir();
static int	header;
static char	title[2*BUFSIZ], *etitle;

/* 
 * NAME: diffdir
 * FUNCTION: directs diffs between directories
 */
diffdir(argv)
char **argv;
{
	struct dirent **dir1, **dir2;
	int nentries1,nentries2;
	register int i,j;
	int cmp;

	if (opt == DI_IFDEF) {
		fprintf(stderr,MSGSTR(ENOI,
			 "diff: can't specify -D with directories\n"));
		status=2;
		done();
	}
	if (opt == DI_EDIT && (sflag || lflag))
		fprintf(stderr,MSGSTR(ENOSL,
		    "diff: warning: shouldn't give -s or -l with -e\n"));
	title[0] = 0;
	strcpy(title, "diff ");
	for (i = 1; diffargv[i+2]; i++) {
		strcat(title, diffargv[i]);
		strcat(title, " ");
	}
	for (etitle = title; *etitle; etitle++)
		;
	setfile(&file1, &efile1, file1);
	setfile(&file2, &efile2, file2);
	argv[optind  ] = file1;
	argv[optind+1] = file2;
	dir1 = setupdir(file1,&nentries1);
	dir2 = setupdir(file2,&nentries2);
	i = j = 0;
	while (i < nentries1 || j < nentries2) {
		if (i < nentries1 && dir1[i]->d_name[0] && !useless(dir1[i])) {
			i++;
			continue;
		}
		if (j < nentries2 && dir2[j]->d_name[0] && !useless(dir2[j])) {
			j++;
			continue;
		}
		if (i >= nentries1 || dir1[i]->d_name[0] == 0)
			cmp = 1;
		else if (j >= nentries2 || dir2[j]->d_name[0] == 0)
			cmp = -1;
		else
			cmp = strncmp(dir1[i]->d_name,dir2[j]->d_name,name_max+1);
		if (cmp < 0) {
			if (lflag)
				dir1[i]->d_ino |= ONLY;
			else if (opt == 0 || opt == 2) {
				only(dir1[i], 1);
				printf(": %.*s\n", name_max+1, dir1[i]->d_name);
			}
			i++;
		} else if (cmp == 0) {
			compare(dir1[i]);
			i++;
			j++;
		} else {
			if (lflag)
				dir2[j]->d_ino |= ONLY;
			else if (opt == 0 || opt == 2) {
				only(dir2[j], 2);
				printf(": %.*s\n", name_max+1, dir2[j]->d_name);
			}
			j++;
		}
	} /* end of while loop */
	if (lflag) {
		scanpr(dir1,ONLY,MSGSTR(EONLY2,"Only in %.*s"),
                       file1, efile1, 0, 0, nentries1);
		scanpr(dir2,ONLY,MSGSTR(EONLY2,"Only in %.*s"),
                       file2, efile2, 0, 0, nentries2);
		if (sflag)
                        scanpr(dir1,SAME,MSGSTR(EIDENT2,"Common identical files in %.*s and %.*s"),
                               file1, efile1, file2, efile2, nentries1);
		scanpr(dir1,DIFFER,MSGSTR(EBINARY2,"Binary files which differ in %.*s and %.*s"),
                       file1, efile1, file2, efile2, nentries1);
		scanpr(dir1,DIRECT,MSGSTR(ESUBDIR2,"Common subdirectories of %.*s and %.*s"),
                       file1, efile1, file2, efile2, nentries1);
	}
	if (rflag) {
		if (header && lflag)
			fputc('\f',stdout);

		for (i = 0; i < nentries1; i++)  {
			if ((dir1[i]->d_ino & DIRECT) == 0)
				continue;
			strncpy(efile1, dir1[i]->d_name, name_max+1);
			strncpy(efile2, dir1[i]->d_name, name_max+1);
			calldiff(0);
		}
	}
}

/*
 * NAME: setfile
 * FUNCTION: finds the end of the path name and adds a / and epp points at
 *   the character following the added /.
 */
static
setfile(fpp, epp, file)
	char **fpp, **epp;
	char *file;
{
	register char *cp;

	*fpp = malloc((size_t)BUFSIZ);
	if (*fpp == 0) {
		fprintf(stderr,MSGSTR(EMEM, "diff: ran out of memory\n"));
		status=2;
		exit(1);
	}
	strcpy(*fpp, file);
	for (cp = *fpp; *cp; cp++)
		continue;
	*cp++ = '/';
	*epp = cp;
}

/*
 * NAME: scanpr
 * FUNCTION: scan the list of files in a directory and prints out its
 *   name if it passes the test.
 */
static
scanpr(dp, test, title, file1, efile1, file2, efile2, nentries)
register struct dirent **dp;
int test;
char *title, *file1, *efile1, *file2, *efile2;
int nentries;
{
	int titled = 0,i;

	for (i=0;i<nentries;i++) {
		if (dp[i]->d_ino & test) {
			if (titled == 0) {
				if (header == 0) {
					if (anychange)
						fputc('\f',stdout);

					header = 1;
				} else
					fputc('\n',stdout);

				printf(title,
                                       efile1 - file1 - 1, file1,
                                       efile2 - file2 - 1, file2);
				fputs(":\n",stdout);
				titled = 1;
			}
			ptname(dp[i]);
		}
	}
}

/*
 * NAME: only
 * FUNCTION: prints outs the message associated with a file that is in only
 *   one of the directories.
 */ 
static
only(dp, which)
	struct dirent *dp;
	int which;
{
	char *file = which == 1 ? file1 : file2;
	char *efile = which == 1 ? efile1 : efile2;

	printf(MSGSTR(EONLY,"Only in %.*s"),
		 efile - file - 1, file, name_max+1, dp->d_name);
	oldstatus = status = 1;
}

/*
 * NAME: ptname
 * FUNCTION: prints out the name of a file
 */
static
ptname(dp)
	struct dirent *dp;
{

	printf("\t%.*s\n", name_max+1, dp->d_name);
}


/*
 * NAME: setupdir
 * FUNCTION: gets all the valid entires for the directory cp
 */
struct dirent **
setupdir(cp,nentries)
char *cp;
int *nentries;
{
	struct dirent **queue;
	int useless(),alphasort(),i;

	if ((*nentries = scandir(cp,&queue,useless,alphasort)) < 0) {
		perror(cp);
		status=2;
		exit(1);
	}
	if (lflag || rflag)
		for (i = 0; i < *nentries; i++)
			queue[i]->d_ino = INIT;
	return(queue);
}


/*
 * NAME: compare
 * FUNCTION: compares two files to determine if they are different or the 
 *    same.
 */
static
compare(dp)
	register struct dirent *dp;
{
	register int i, j;
	int f1, f2, fmt1, fmt2;
	struct stat stb1, stb2;
	int flag = 0;
	char buf1[BUFSIZ], buf2[BUFSIZ];

	strncpy(efile1, dp->d_name, name_max+1);
	strncpy(efile2, dp->d_name, name_max+1);
	if (lflag)
		dp->d_ino = SAME;
	if (stat(file1, &stb1)) {
		perror(file1);
		status=2;
		return;
	}
 	if (stat(file2, &stb2)) {
		perror(file2);
		status=2;
		return;
	}
	fmt1 = stb1.st_mode & S_IFMT;
	fmt2 = stb2.st_mode & S_IFMT;
	if (fmt1 != S_IFREG || fmt2 != S_IFREG) {
		if ((fmt1 == IFBLK) || (fmt1 == IFCHR) || (fmt1 == IFIFO) || (fmt2 == IFBLK) || (fmt2 == IFCHR) || (fmt2 == IFIFO)) {
			printf(MSGSTR(SPFILE, "File %s is a %s while file %s is a %s\n"), file1, filetype(fmt1), file2, filetype(fmt2));
			return;
		}
		if (fmt1 == fmt2) {
			if (fmt1 != S_IFDIR && stb1.st_rdev == stb2.st_rdev)
				goto same;
			if (fmt1 == S_IFDIR) {
				if (slinkcmp(file1, file2, dp)) return;
				if (lflag || opt == DI_EDIT || rflag) {
					dp->d_ino = DIRECT;
					return;
				}
				printf(MSGSTR(SUBDIR,
					"Common subdirectories: %s and %s\n"),
				    file1, file2);
				return;
			}
		}
		goto notsame;
	}
	if (stb1.st_size != stb2.st_size)
		goto notsame2;      /* do not close files since not opened */
	f1 = open(file1, 0);
	f2 = open(file2, 0);
	for (;;) {
		i = read(f1, buf1, BUFSIZ);
		j = read(f2, buf2, BUFSIZ);
		if (i < 0 || j < 0 || i != j)
			goto notsame;
		if (i == 0 && j == 0)
			goto same;
		for (j = 0; j < i; j++)
			if (buf1[j] != buf2[j])
				goto notsame;
	}
same:
	if (sflag == 0)
		goto closem;
	if (lflag)
		dp->d_ino = SAME;
	else
		printf(MSGSTR(FILES,
			"Files %s and %s are identical\n"), file1, file2);
	goto closem;
notsame:
/*	if (!ascii(f1) || !ascii(f2)) {
		if (lflag)
			dp->d_ino = DIFFER;
		else if (opt == DI_NORMAL || opt == DI_CONTEXT)
			printf(MSGSTR(DBINARY,"Binary files %s and %s differ\n")
			,file1, file2);
		goto closem;
	} Deleted this for 141843 */
	close(f1); 
	close(f2);
notsame2:
	oldstatus = status=1;
	anychange = 1;
	if (lflag) {
		dp->d_ino = INIT;
		calldiff(title);
	} else {
		if (opt == DI_EDIT) {
			printf("ed - %.*s << '-*-END-*-'\n",
			    name_max+1, dp->d_name);
			calldiff(0);
		} else {
			printf("%s%s %s\n", title, file1, file2);
			calldiff(0);
		}
		if (opt == DI_EDIT)
			printf("w\nq\n-*-END-*-\n");
	}
	return;
closem:
	close(f1); 
	close(f2);
}

static char	*prargs[] = { "pr", "-h", 0, "-f", 0, 0 };

/*
 * NAME: calldiff
 * FUNCTION: fork a second process and call diff again 
 */
static
calldiff(wantpr)
char *wantpr;
{
	int status1, status2, pv[2];
	pid_t pid;

        pv[0]=-1; pv[1]=-1;
	prargs[2] = wantpr;
	fflush(stdout);
	if (wantpr) {
		sprintf(etitle, "%s %s", file1, file2);
		pipe(pv);
		pid = fork();
		if (pid == -1) {
			perror("diff");
			status=2;
			done();
		}
		if (pid == 0) {
			close(0);
			dup(pv[0]);
			close(pv[0]);
			close(pv[1]);
			execv(pr+4, prargs);
			execv(pr, prargs);
			perror(pr);
			status=2;
			done();
		}
	}
		/* Reset optind for forked process getopt() */
	optind = 1;
	pid = fork();
	if (pid == -1) {
		perror("diff");
		status=2;
		done();
	}
	if (pid == 0) {
		if (wantpr) {
			close(1);
			dup(pv[1]);
			close(pv[0]);
			close(pv[1]);
		}
		execv(diff+4, diffargv);
		execv(diff+5, diffargv);
		execv(diff, diffargv);
		perror(diff);
		status=2;
		done();
	}
	close(pv[0]);
	close(pv[1]);
	while (wait(&status1) != pid)
		continue;
	while (wait(&status2) != -1)
		continue;
	status = status1 >> 8;
    if (oldstatus < status)
		oldstatus = status;
	else
		status = oldstatus;
/*
	if ((status >> 8) >= 2)
		done();
*/
}

#ifdef _OSF
#include <a.out.h>
#ifdef	multimax
#define	BADMAG(X) (X.f_magic != NS32GMAGIC && X.f_magic != NS32SMAGIC)
#endif	/* multimax */
#ifdef	mips
#define	BADMAG(X) (!(ISCOFF(X.f_magic)))
#endif	/* mips */
#if	defined(i386) || defined (PS2)
#define BADMAG(X) ((X.f_magic != I386MAGIC) && (X.f_magic != COFF386MAGIC))
#endif  /* i386 || PS2 */

ascii_bsd(f)
	int f;
{
	char buf[BUFSIZ];
	register int cnt;
	register char *cp;

	lseek(f, (long)0, 0);
	cnt = read(f, buf, BUFSIZ);
#if	COFF || MACHO
	if (cnt >= sizeof (struct filehdr)) {
		struct filehdr hdr;
		hdr = *(struct filehdr *)buf;
		if (!BADMAG(hdr))
#else	/* COFF || MACHO */
	if (cnt >= sizeof (struct exec)) {
		struct exec hdr;
		hdr = *(struct exec *)buf;
		if (!N_BADMAG(hdr))
#endif	/* COFF || MACHO */
			return (0);
	}
	cp = buf;
	while (--cnt >= 0)
		if (*cp++ & 0200)
			return (0);
	return (1);
}
#endif /* _OSF */

/*
 * NAME: useless
 * FUNCTION: checks to see if a file should be included in current list
 *  of files for the current directory.
 */
static
useless(cp)
struct dirent *cp;
{

	if (cp->d_name[0] == '.') {
		if (cp->d_name[1] == '\0')
			return (0);	/* directory "." */
		if (cp->d_name[1] == '.' && cp->d_name[2] == '\0')
			return (0);	/* directory ".." */
	}
	if (begin && strcmp(begin, cp->d_name) > 0)
		return (0);
	return (1);
}

/* 
 * NAME: slinkcmp
 * FUNCTION: trace symbolic links, determine if link to other directory.
 */
static
slinkcmp(file1, file2, dp)
char *file1, *file2;
struct dirent *dp;
{
	extern char *slinkcmpsbuf1, *slinkcmpsbuf2; /* Set outside recursion in diff.c */
	char *sbuf1, *sbuf2;
	int sfmt1, sfmt2;
	struct stat symb1, symb2;

	lstat(file1, &symb1); lstat(file2, &symb2);
	sfmt1 = symb1.st_mode & S_IFMT;
	sfmt2 = symb2.st_mode & S_IFMT;
	if (sfmt1 == sfmt2 && sfmt1 == S_IFLNK) {
		sbuf1 = slinkcmpsbuf1;
		sbuf2 = slinkcmpsbuf2;
		if (sfmt1 == S_IFLNK) readlink(file1, sbuf1, name_max+1);
		if (sfmt2 == S_IFLNK) readlink(file2, sbuf2, name_max+1);
		if( (strcmp(sbuf1,".") == 0 || strcmp(sbuf1,"..") == 0) &&
		    (strcmp(sbuf2,".") == 0 || strcmp(sbuf2,"..") == 0)) {
			printf(MSGSTR(SUBARE,"Common subdirectories are "));
			printf(MSGSTR(SYMLINK,"symbolic link files:\n\t"));
			if (strcmp(sbuf1,sbuf2) == 0) {
				if (lflag)
					dp->d_ino = SAME;
				else {
					printf(MSGSTR(IDENT,
					   "%s and %s are identical\n"),
					   file1, file2);
				}
			} else {
				if (lflag)
					dp->d_ino = DIFFER;
				else {
				   printf("%s --> %s ",
				     file1, sbuf1);
				   printf(MSGSTR(DDIFFER2,
				     " and %s --> %s differ\n"),file2, sbuf2);
				}
			}
			return(1);
		 }
	}
	return(0);
}

char *filetype(int fmt)
{
	switch (fmt) {
	case IFDIR:
		return (MSGSTR(DIRT, "directory"));
	case IFCHR:
		return (MSGSTR(CHAR, "character special file"));
	case IFBLK:
		return (MSGSTR(BLCK, "block special file"));
	case IFIFO:
		return (MSGSTR(FIFO, "fifo"));
	case IFSOCK:
		return (MSGSTR(SOCK, "socket"));
	case IFLNK:
		return (MSGSTR(SMLK, "symbolic link file"));
	default:
		return (MSGSTR(REGU, "regular file"));
	}
}
