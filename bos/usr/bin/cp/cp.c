static char sccsid[] = "@(#)82  1.17  src/bos/usr/bin/cp/cp.c, cmdfiles, bos41J, 9508A 2/19/95 23:15:40";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: cp, copy
 *
 * ORIGINS: 3, 18, 26, 27, 71
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
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * David Hitz of Auspex Systems Inc.
 *
 * static char rcsid[] = "RCSfile: cp.c,v  Revision: 1.3.2.3  (OSF) Date: 90/10/22 11:36:02 "
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include <sys/stat.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "cp_msg.h"

static nl_catd	catd;

#define MSGSTR(Num,Str) catgets(catd,MS_CP,Num,Str)
#define	type(st)	((st).st_mode & S_IFMT)

#ifdef _AIX				/* AIX security */
#define COPY_RETURN		{if (aclp) free ((void *) aclp); return;}
extern char *acl_get();
#else
#define COPY_RETURN			{return;}
#endif

typedef struct {
	char	p_path[MAXPATHLEN + 1];	/* pointer to the start of a path. */
	char	*p_end;			/* pointer to NULL at end of path. */
} PATH_T;

static PATH_T from = { "", from.p_path };
static PATH_T to = { "", to.p_path };

static uid_t myuid;
static int exit_val;
static mode_t myumask;
static int fflag, iflag, hflag, pflag, rflag, Rflag;
static int (*statfcn)(char *, struct stat *);
static char *buf, *pname;
static void  copy(void);
static void  copy_file(struct stat *, int);
static void  copy_dir(void);
static void  copy_link(struct stat *, int);
static void  copy_fifo(struct stat *, int);
static void  copy_special(struct stat *, int);
static void  setfile(register struct stat *, int);
static void  error(char *);
static int   path_set(register PATH_T *, char *);
static char *path_append(register PATH_T *, char *, int);
static void  path_restore(PATH_T *, char *);
static char *path_basename(PATH_T *);
static void  usage(void);

/*
 * NAME: cp [-fhip] [--] src target or cp [-fhip] [--] src1 ... srcN directory
 *       cp {-R | -r} [-fhip] [--] src1 ... srcN target_directory 
 * 
 * FUNCTION: copies files
 *
 * NOTES:   -f causes cp to try to remove an existing destination file if it
 *             cannot gain write access first.
 *          -h  forces cp to copy symbolic links instead of following them.
 *          -i  issues a prompt before copying to any existing
 *              destination file
 *          -p  duplicates the file characteristics such as last modification,
 *              last access, user I.D., etc., of each source file for the
 *              corresponding destination file
 *          -R  copies file hierarchies
 *          -r  copies file hierarchies and treats special files the same
 *              as regular files
 *          --  indicates that following arguments are to be treated as file names.
 *              This use to be a single hyphen.  With the implementation of
 *              getopts, the hyphen became unnecessary.  This is documented here
 *              for clarity with previous releases.
 *
 *              -R or -r must be specified if any of the source files are directories.
 *
 * The global PATH_T structures "to" and "from" always contain paths to the
 * current source and target files, respectively.  Since cp does not change
 * directories, these paths can be either absolute or dot-realative.
 * 
 * The basic algorithm is to initialize "to" and "from", and then call the
 * recursive copy() function to do the actual work.  If "from" is a file,
 * copy copies the data.  If "from" is a directory, copy creates the
 * corresponding "to" directory, and calls itself recursively on all of
 * the entries in the "from" directory.
 */

main(int argc, char **argv)
{
	struct stat to_stat;
	register int c, r;
	char *old_to, *p;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_CP, NL_CAT_LOCALE);

	/*
	 * determine the "called as" program name.  Could be cp, copy,
	 * or mv since mv calls cp under certain conditions.
	 */
	pname = (p = strrchr(*argv,'/')) ? ++p : *argv;

	exit_val=0;
	while ((c = getopt(argc, argv, "Rhiprf")) != EOF) {
	switch ((char)c) {
		case 'f':
			fflag = 1;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'p':
			pflag = 1;
			break;
		case 'R':
			Rflag = 1;
			break;
		case 'r':
			rflag = 1;
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	if (Rflag && rflag) {
		fprintf(stderr,
		    MSGSTR(RANDR, 
			"%s: the -R and -r options are mutually exclusive.\n"), pname);
		exit(1);
	}

	buf = (char *)malloc(MAXBSIZE);
	if (!buf) {
		fprintf(stderr, 
			MSGSTR(SPACE, "%s: out of space.\n"), pname);
		exit(1);
	}

	myuid = getuid();

	/* copy the umask for explicit mode setting */
	myumask = umask(0);
	(void)umask(myumask);

	/* consume last argument first. */
	if (!path_set(&to, argv[--argc]))
		exit(exit_val);

	statfcn = (int (*)(char *, struct stat *)) (hflag ? lstat : stat);

	/*
	 * Cp has two distinct cases:
	 *
	 * Case (1)	  $ cp [-fhip] [-r|-R] [--] source target
	 *
	 * Case (2)	  $ cp [-fhip] [-r|-R] [--] source1 ... directory
	 *
	 * In both cases, source can be either a file or a directory.
	 *
	 * In (1), the target becomes a copy of the source. That is, if the
	 * source is a file, the target will be a file, and likewise for
	 * directories.
	 *
	 * In (2), the real target is not directory, but "directory/source".
	 */

	r = stat(to.p_path, &to_stat);
	/* Only acceptable error return from stat is "non-existent file" */
	if (r == -1 && errno != ENOENT) {
		error(to.p_path);
		exit(1);
	}
	if (r == -1 || type(to_stat) != S_IFDIR) {
		/*
		 * Case (1) where Target doesn't exist or
		 * Target isn't a directory.
		 */
		if (argc > 1) {			/* print an error if more than 2 files specified */
			usage();
			exit(1);
		}
		/*
		 * Set up "from" path and do copy
 		 */
		if (!path_set(&from, *argv))
			exit(exit_val);
		copy();
	}
	else {
		/*
		 * Case (2).  Target is an existing directory.
		 */
		for (;; ++argv) {
			/*
			 * Set up "from" path and do copy
			 */	
			if (!path_set(&from, *argv))
				continue;
			old_to = path_append(&to, path_basename(&from), -1);
			if (!old_to)
				continue;
			copy();
			if (!--argc)
				break;
			path_restore(&to, old_to);
		}
	}
	exit(exit_val);
}

/*
 * NAME: copy
 *
 * FUNCTION: copy file or directory at "from" to "to"
 */

static void
copy(void)
{
	struct stat from_stat, to_stat;
	int dne, statval;
#ifdef _AIX
	char *aclp = NULL;		/* pointer to access control list */
#endif

	/* check "from" path.  return if error */
	statval = statfcn(from.p_path, &from_stat);
	if (statval == -1) {
		error(from.p_path);
		COPY_RETURN
	}

	/*
	 * checked earlier in main -- not an error, but    
	 * need to remember it happened
	 */
	if (stat(to.p_path, &to_stat) == -1)
		dne = 1;	/* set does not exist flag */
	else {
		if (to_stat.st_dev == from_stat.st_dev &&
		    to_stat.st_ino == from_stat.st_ino) {
			fprintf(stderr,
			    MSGSTR(SAME, 
				"%s: %s and %s are identical (not copied).\n"),
			    pname, to.p_path, from.p_path);
			exit_val = 1;
			COPY_RETURN
		}
		dne = 0;
	}

	switch(type(from_stat)) {
	case S_IFLNK:
		copy_link(&from_stat, !dne);
		COPY_RETURN
	case S_IFDIR:
		if (!Rflag && !rflag) {
			fprintf(stderr,
			 MSGSTR(ISDIR, "%s: %s is a directory (not copied).\n"),
			    pname, from.p_path);
			exit_val = 1;
			COPY_RETURN
		}

#ifdef _AIX
		/* AIX security  -- get the access control list */
		/* if preserving modes */
		if (pflag)
			if ((aclp = acl_get(from.p_path)) == NULL) {
				error (from.p_path);
				return;
			}
#endif
	
		if (dne) {
			/*
			 * If the "to" directory doesn't exist, create the new
			 * one with the from file mode plus owner RWX bits,
			 * modified by the umask.  Trade-off between being
			 * able to write the directory (if from directory is
			 * 555) and not causing a permissions race.  If the
			 * umask blocks owner writes cp fails.
			 */
			if (mkdir(to.p_path, from_stat.st_mode|S_IRWXU) < 0) {
				error(to.p_path);
				COPY_RETURN
			}
		}
		/* "to" exists.  Print error if not a directory */
		else if (type(to_stat) != S_IFDIR) {
			fprintf(stderr, 
			    MSGSTR(NOTDIR, "%s: %s: not a directory.\n"),
			    pname, to.p_path);
			exit_val = 1;
			COPY_RETURN
		}
		copy_dir();

		if (pflag) {
			setfile(&from_stat, 0);
#ifdef _AIX
		     /* AIX security -- copy the source's   */
		     /* access control list for preservation */
		     /* Be sure to do acl_put() after setfile() to */
		     /* preserve "extended permissions enabled" bit */
			if (acl_put(to.p_path,aclp,0) == -1) {
				error (to.p_path);
				COPY_RETURN
			}
#endif
		}
		else
			if (chmod(to.p_path,from_stat.st_mode & ~myumask)!=0)
				error (to.p_path);
		COPY_RETURN
	case S_IFCHR:
	case S_IFBLK:
		if (Rflag) {
			copy_special(&from_stat, !dne);
			COPY_RETURN
		}
		break;
	case S_IFIFO:
		if (Rflag) {
			copy_fifo(&from_stat, !dne);
			COPY_RETURN
		}
		break;
	}
	copy_file(&from_stat, dne);
}

/*
 * NAME: copy_file
 *
 * FUNCTION: copies files
 */

static void
copy_file(struct stat *fs, int dne)
{
	register int from_fd, to_fd, rcount, wcount;
	int clserr;
#ifdef _AIX
	char *aclp = NULL;		/* pointer to access control list */
#endif

	if ((from_fd = open(from.p_path, O_RDONLY, 0)) == -1) {
		error(from.p_path);
		return;
	}

#ifdef _AIX
	/* AIX security -- obtain access control list */
	/* if preserving modes  */
	if (pflag)
		if ((aclp = acl_get(from.p_path)) == NULL) {
			error(from.p_path);
			return;
		}
#endif

	/*
	 * If the file exists and we're interactive, verify with the user.
	 * If the file DNE, set the mode to be the from file, minus setuid
	 * bits, modified by the umask; arguably wrong, but it makes copying
	 * executables work right and it's been that way forever.  (The
	 * other choice is 666 or'ed with the execute bits on the from file
	 * modified by the umask.)
	 */
	if (!dne) {
		if (iflag) {
			char response[LINE_MAX];

			fprintf(stderr, MSGSTR(OVER, "overwrite %s? "),
				to.p_path);

			if (gets(response) != NULL) {
				if (rpmatch(response) != 1) {		/* return if response negative */
					(void)close(from_fd);
					COPY_RETURN
				}
			}
			else {				/* EOF, or error, consider negative response */
				(void)close(from_fd);
				COPY_RETURN
			}
		}
		to_fd = open(to.p_path, O_WRONLY|O_TRUNC, 0);
		/* if -f set and cant open for write, try unlinking first */
		if (fflag && (to_fd == -1)) {
		    (void)unlink(to.p_path);
		    to_fd = open(to.p_path, O_WRONLY|O_CREAT|O_TRUNC,
			fs->st_mode & ~(S_ISUID|S_ISGID));
		}
	} else
		to_fd = open(to.p_path, O_WRONLY|O_CREAT|O_TRUNC,
		    fs->st_mode & ~(S_ISUID|S_ISGID));

	if (to_fd == -1) {
		error(to.p_path);
		(void)close(from_fd);
		COPY_RETURN
	}

	while ((rcount = read(from_fd, buf, MAXBSIZE)) > 0) {
		wcount = write(to_fd, buf, rcount);
		if (rcount != wcount || wcount == -1) {
			error(to.p_path);
			break;
		}
	}
	if (rcount < 0)
		error(from.p_path);

	if (pflag) {
		setfile(fs, to_fd);
#ifdef _AIX
	     /* AIX security -- copy the source's acl    */
	     /* for the target's acl if preserving modes */
	     /* Be sure to do acl_put() after setfile() to */
	     /* preserve "extended permissions enabled" bit */
		if (acl_put(to.p_path,aclp,0) == -1) {
			error (to.p_path);
			COPY_RETURN
		}
#endif
	}

	(void)close(from_fd);
	clserr = close(to_fd);
		/* cp used to unlink "to.p_path" here if the close failed.   */
		/* This is against POSIX 1003.2 D12 and XPG/4 standards,     */
		/* which state to simply close the file on errors.           */
	if (clserr == -1)
		error(to.p_path);
	COPY_RETURN
}

/*
 * NAME: copy_dir
 * 
 * FUNCTION: copies directories
 */

static void
copy_dir(void)
{
	struct stat from_stat;
	struct dirent *dp, **dir_list;
	register int dir_cnt, i;
	char *old_from, *old_to;

	dir_cnt = scandir(from.p_path, &dir_list, NULL, NULL);
	if (dir_cnt == -1) {
		fprintf(stderr, MSGSTR(CANTREAD, 
			"%s: cannot read directory %s.\n"),
		    pname, from.p_path);
		exit_val = 1;
	}

	/*
	 * Instead of handling directory entries in the order they appear
	 * on disk, do non-directory files before directory files.
	 * There are two reasons to do directories last.  The first is
	 * efficiency.  Files tend to be in the same cylinder group as
	 * their parent, whereas directories tend not to be.  Copying files
	 * all at once reduces seeking.  Second, deeply nested tree's
	 * could use up all the file descriptors if we didn't close one
	 * directory before recursively starting on the next.
	 */
	/* copy files */
	for (i = 0; i < dir_cnt; ++i) {
		dp = dir_list[i];
		if (dp->d_namlen <= 2 && dp->d_name[0] == '.'
		    && (dp->d_name[1] == '\0' || dp->d_name[1] == '.'))
			goto done;
		old_from = path_append(&from, dp->d_name, (int)dp->d_namlen);
		if (!old_from)
			goto done;

		if (statfcn(from.p_path, &from_stat) < 0) {
			error(dp->d_name);
			path_restore(&from, old_from);
			goto done;
		}
		if (type(from_stat) == S_IFDIR) {
			path_restore(&from, old_from);
			continue;
		}
		old_to = path_append(&to, dp->d_name, (int)dp->d_namlen);
		if (old_to) {
			copy();
			path_restore(&to, old_to);
		}
		path_restore(&from, old_from);
done:		dir_list[i] = NULL;
		(void)free((void *)dp);
	}

	/* copy directories */
	for (i = 0; i < dir_cnt; ++i) {
		dp = dir_list[i];
		if (!dp)
			continue;
		old_from = path_append(&from, dp->d_name, (int) dp->d_namlen);
		if (!old_from) {
			(void)free((void *)dp);
			continue;
		}
		old_to = path_append(&to, dp->d_name, (int) dp->d_namlen);
		if (!old_to) {
			(void)free((void *)dp);
			path_restore(&from, old_from);
			continue;
		}
		copy();
		free((void *)dp);
		path_restore(&from, old_from);
		path_restore(&to, old_to);
	}
	free((void *)dir_list);
}

/*
 * NAME: copy_link
 *   
 * FUNCTION: copies links
 */

static void
copy_link(struct stat *fs, int exists)
{
	int len;
	char linkname[MAXPATHLEN];

	/* get the path of the symbolic link */
	if ((len = readlink(from.p_path, linkname, sizeof(linkname))) == -1) {
		error(from.p_path);
		return;
	}
	linkname[len] = '\0';
	/*
	 * if "to" exists, unlink it first, before 
	 * creating the new symbolic link with "to"
	 */
	if (exists && unlink(to.p_path)) {
		error(to.p_path);
		return;
	}
	if (symlink(linkname, to.p_path)) {
		error(linkname);
		return;
	}
#ifndef _BLD
	if (pflag && hflag) {
		if (lchown(to.p_path, fs->st_uid, fs->st_gid) < 0) {
			if (errno != EPERM)
				error(to.p_path);
		}
	}
#endif
}

/*
 * NAME: copy_fifo
 *
 * FUNCTION: copies fifos
 */

static void
copy_fifo(struct stat *from_stat, int exists)
{
#ifdef _AIX
	char *aclp = NULL;		/* pointer to access control list */

	/* AIX security -- obtain access control list */
	/* if preserving modes */
	if (pflag)
		if ((aclp = acl_get(from.p_path)) == NULL) {
			error(from.p_path);
			return;
		}
#endif

	/* If "to" exists, need to first unlink */
	if (exists && unlink(to.p_path)) {
		error(to.p_path);
		COPY_RETURN
	}
	if (mkfifo(to.p_path, from_stat->st_mode)) {
		error(to.p_path);
		COPY_RETURN
	}

	if (pflag) {
		setfile(from_stat, 0);
#ifdef _AIX
	     /* AIX security -- copy the source's acl     */
	     /* for the target's acl if preserving modes  */
	     /* Be sure to do acl_put() after setfile() to */
	     /* preserve "extended permissions enabled" bit */
		if (acl_put(to.p_path,aclp,0) == -1) {
			error (to.p_path);
			COPY_RETURN
		}
#endif
	}
	COPY_RETURN
}

/*
 * NAME: copy_special
 *
 * FUNCTION: copies special files
 */

static void
copy_special(struct stat *from_stat, int exists)
{
#ifdef _AIX
	char *aclp = NULL;		/* pointer to access control list */

	/* AIX security -- obtain access control list */
	/* if preserving modes or new file */
	if (pflag)
		if ((aclp = acl_get(from.p_path)) == NULL) {
			error(from.p_path);
			return;
		}
#endif

	/* If "to" exists, need to first unlink */
	if (exists && unlink(to.p_path)) {
		error(to.p_path);
		COPY_RETURN
	}
	if (mknod(to.p_path, from_stat->st_mode,  from_stat->st_rdev)) {
		error(to.p_path);
		COPY_RETURN
	}

	if (pflag) {
		setfile(from_stat, 0);
#ifdef _AIX
	     /* AIX security -- copy the source's acl     */
	     /* for the target's acl if preserving modes  */
	     /* Be sure to do acl_put() after setfile() to */
	     /* preserve "extended permissions enabled" bit */
		if (acl_put(to.p_path,aclp,0) == -1) {
			error (to.p_path);
			COPY_RETURN
		}
#endif
	}
	COPY_RETURN
}

/*
 * NAME: setfile
 *     
 * FUNCTION: sets times and modes for the new file 
 */
 
static void
setfile(register struct stat *fs, int fd)
{
	static struct timeval tv[2];

	fs->st_mode &= S_ISUID|S_ISGID|S_IRWXU|S_IRWXG|S_IRWXO;

	tv[0].tv_sec = (int) fs->st_atime;
	tv[1].tv_sec = (int) fs->st_mtime;
	if (utimes(to.p_path, tv))
		error(to.p_path);
	/*
	 * Changing the ownership probably won't succeed, unless we're root
	 * or POSIX_CHOWN_RESTRICTED is not set.  Set uid/gid before setting
	 * the mode; current BSD behavior is to remove all setuid bits on
	 * chown.  If chown fails, lose setuid/setgid bits.
	 */
	if (fd ? fchown(fd, fs->st_uid, fs->st_gid) :
	    chown(to.p_path, fs->st_uid, fs->st_gid)) {
		if (errno != EPERM)
			error(to.p_path);
		fs->st_mode &= ~(S_ISUID|S_ISGID);
	}
	if (fd ? fchmod(fd, fs->st_mode) : chmod(to.p_path, fs->st_mode))
		error(to.p_path);
}

/*
 * NAME: error
 *
 * FUNCTION: prints error message
 */

static void
error(char *s)
{
	exit_val = 1;
	fprintf(stderr, "%s: %s: %s\n", pname, s, strerror(errno));
}

/********************************************************************
 * Path Manipulation Routines.
 ********************************************************************/

/*
 * These functions manipulate paths in PATH_T structures.
 * 
 * They eliminate multiple slashes in paths when they notice them, and keep
 * the path non-slash terminated.
 *
 * Both path_set() and path_append() return 0 if the requested name
 * would be too long.
 */

#define	STRIP_TRAILING_SLASH(p) { \
	while ((p)->p_end > &(p)->p_path[1] && (p)->p_end[-1] == '/') \
		*--(p)->p_end = 0; \
	}

/*
 * NAME: path_set
 * 
 * FUNCTION: Move specified string into path.  
 *           Convert "" to "." to handle BSD semantics 
 *           for a null path.  Strip trailing slashes.
 *
 * RETURNS:  If successful, returns 1; otherwise, returns 0.
 */

static int
path_set(register PATH_T *p, char *string)
{
	if (strlen(string) > MAXPATHLEN) {
		fprintf(stderr, MSGSTR(TOLNG, "%s: %s: name too long.\n"),
		    pname, string);
		exit_val = 1;
		return(0);
	}

	(void)strcpy(p->p_path, string);
	p->p_end = p->p_path + strlen(p->p_path);

	STRIP_TRAILING_SLASH(p);

	if (p->p_path == p->p_end) {
		*p->p_end++ = '.';
		*p->p_end = 0;
	}
	return(1);
}

/*
 * NAME: path_append
 *
 * FUNCTION: Append specified string to path, inserting 
 *           '/' if necessary.  Return a pointer to the 
 *           old end of path for restoration.
 */

static char *
path_append(register PATH_T *p, char *name, int len)
{
	char *old;

	old = p->p_end;
	if (len == -1)
		len = (int) strlen(name);

	/*
	 * The final "+ 1" accounts for the '/' between old path and name.
	 */
	if ((len + p->p_end - p->p_path + 1) > MAXPATHLEN) {
		fprintf(stderr, MSGSTR(TOLNG2, "%s: %s/%s: name too long.\n"), 
			pname, p->p_path, name);
		exit_val = 1;
		return(0);
	}

	/*
	 * This code should always be executed, since paths shouldn't
	 * end in '/'.
	 */
	if (p->p_end[-1] != '/') {
		*p->p_end++ = '/';
		*p->p_end = 0;
	}

	(void)strncat(p->p_end, name, len);
	p->p_end += len;
	*p->p_end = 0;

	STRIP_TRAILING_SLASH(p);
	return(old);
}

/*
 * NAME: path_restore
 *
 * FUNCTION: Restore path to previous value.  
 *           (As returned by path_append.)
 */

static void
path_restore(PATH_T *p, char *old)
{
	p->p_end = old;
	*p->p_end = 0;
}

/*
 * NAME: path_basename
 *
 * FUNCTION: Return basename of path.  (Like basename(1).)
 */

static char *
path_basename(PATH_T *p)
{
	char *basename;

	basename = strrchr(p->p_path, '/');
	return(basename ? ++basename : p->p_path);
}

/*
 * NAME: usage
 *
 * FUNCTION: prints usage message
 */

static void
usage(void)
{
	fprintf(stderr,
		MSGSTR(USAGE, 
"Usage: %s [-fhip] [-r|-R] [--] src target\n   or: %s [-fhip] [-r|-R] [--] src1 ... srcN directory\n"), pname, pname, pname);
	exit(1);
}
