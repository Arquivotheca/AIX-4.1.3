static char sccsid[] = "@(#)58	1.5  src/bos/usr/ccs/lib/libc/ftw.c, libcenv, bos411, 9428A410j 3/4/94 15:31:15";
/*
 *   COMPONENT_NAME: libcenv
 *
 *   FUNCTIONS: ftw, ftw_setup, ftw_work, list_append, list_empty,
 *		list_first, list_free, nftw
 *
 *   ORIGINS: 3,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * OSF/1 1.2
 */

/*
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <string.h>


/*
 * NAME:	ftw
 *                                                                    
 * FUNCTION:	ftw - file tree walk
 *                                                                    
 * NOTES:
 *	Given a path name, ftw starts from the file given by that path
 *	name and visits each file and directory in the tree beneath
 *	that file.  Any symbolic links in the *inital* path will
 *	be traversed, including the final component (per the OSF/AES).
 *	Links encountered while walking the directory tree will not
 *	be traversed. If a single file has multiple (hard)links within the
 *	structure, it will be visited once for each such link.
 *	For each object visited, fn is called with three arguments.
 *	The first contains the path name of the object, the second
 *	contains a pointer to a stat buffer which will usually hold
 *	appropriate information for the object and the third will
 *	contain an integer value giving additional information about
 *
 *		FTW_F	The object is a file for which stat was
 *			successful.  It does not guarantee that the
 *			file can actually be read.
 *
 *		FTW_D	The object is a directory for which stat and
 *			open for read were both successful.
 *
 *		FTW_DP	The object is a directory and the subdirectories
 *			have already been visited.
 *
 *		FTW_DNR	The object is a directory for which stat
 *			succeeded, but which cannot be read.  Because
 *			the directory cannot be read, fn will not be
 *			called for any descendants of this directory.
 *
 *		FTW_SL	The object is a symbolic link for which the
 *			lstat was successful.
 *
 *		FTW_SLN	The object is a symbolic link for which the
 *			lstat was successful but points to a non-
 *			existant file.  (Only if FTW_PHYS is off).
 *
 *		FTW_NS	Stat failed on the object because of lack of
 *			appropriate permission.  This indication will
 *			be given, for example, for each file in a
 *			directory with read but no execute permission.
 *			Because stat failed, it is not possible to
 *			determine whether this object is a file or a
 *			directory.  The stat buffer passed to fn will
 *			contain garbage.  Stat failure for any reason
 *			other than lack of permission will be
 *			considered an error and will cause ftw to stop 
 *			and return -1 to its caller.
 *
 *	If fn returns nonzero, ftw stops and returns the same value
 *	to its caller.  If ftw gets into other trouble along the way,
 *	it returns -1 and leaves an indication of the cause in errno.
 *
 *	The third argument is no longer used, because the tree walk
 *	algorithm was changed so that at each level the directory is opened,
 *	it's contents read into a linked list, and then it's closed.
 *	This is all done *before* the routine recurses; so there is
 *	never more than one descriptor open at a time.
 *
 *	Old story:
 *	The third argument to ftw does not limit the depth to which
 *	ftw will go.  Rather, it limits the depth to which ftw will
 *	go before it starts recycling file descriptors.  In general,
 *	it is necessary to use a file descriptor for each level of the
 *	tree, but they can be recycled for deep trees by saving the
 *	position, closing, re-opening, and seeking.  It is possible
 *	to start recycling file descriptors by sensing when we have
 *	run out, but in general this will not be terribly useful if
 *	fn expects to be able to open files.  We could also figure out
 *	how many file descriptors are available and guarantee a certain
 *	number to fn, but we would not know how many to guarantee,
 *	and we do not want to impose the extra overhead on a caller who
 *	knows how many are available without having to figure it out.
 *
 *	It is possible for ftw to die with a memory fault in the event
 *	of a file system so deeply nested that the stack overflows.
 *
 * RETURN VALUE DESCRIPTION:	-1 if an error of any kind occurs (system
 *	call fails, malloc fails, etc), any non-zero value returned by
 *	a call to 'fn', else 0
 */

/*
 * The global state holds those items that are constant across
 * different levels of recursion within ftw_work.  Allocated once
 * by the caller of ftw_work.
 */
struct global_state {
	char *path;
	char *cwd;
	char *user_cwd;
	int (*ftw_fn)(const char *, const struct stat *, int);
	int (*nftw_fn)(const char *, const struct stat *, int, struct FTW *);
	int flags;
};

/*
 * The local state holds those items that change during different
 * levels of recursion within ftw_work.  Allocated by the caller
 * of ftw_work and by ftw_work itself for use by the callee.
 */
struct local_state {
	char *endpath;
	struct stat sb;
	struct FTW ftw;
};

/*
 * Linked-list ADT - Used to hold the contents of the directories
 * currently being scanned.  The lists are freed "as we go" so that
 * we are only ever saving the work "left to do" along the current
 * path.
 */
struct elem {
	struct elem *next;
	char *data;
};

struct list {
	struct elem *first;
	struct elem *last;
};

static void
list_empty(struct list *listp)
{
	listp->first = listp->last = NULL;
}

/*
 * Only need routines to implement a FIFO
 */
static int
list_append(struct list *listp, char *data)
{
	struct elem *ep;

	ep = (struct elem *) malloc(sizeof(struct elem));
	if (ep == NULL) {
		errno = ENOMEM;
		return(-1);
	}
	ep->next = NULL;
	ep->data = strdup(data);
	if (ep->data == NULL) {
		errno = ENOMEM;
		free((char *)ep);
		return(-1);
	}
	if (listp->first == NULL)
		listp->first = ep;
	else
		listp->last->next = ep;
	listp->last = ep;
	return(0);
}

static char *
list_first(struct list *listp)
{
	struct elem *ep;
	char *data;

	if ((ep = listp->first) == NULL)
		return(NULL);
	listp->first = ep->next;
	data = ep->data;
	free((char *)ep);
	return(data);
}

void
list_free(struct list *listp)
{
	struct elem *ep;

	while ((ep = listp->first) != NULL) {
		listp->first = ep->next;
		free(ep->data);
		free((char *)ep);
	}
}

/*
 * This is the actual worker routine, and the only one that is recursive.
 * The caller does the setup for the callee, and will only recurse to
 * process subdirectories, thereby minimizing recursion somewhat.
 */
static int
ftw_work(struct global_state *gsp,
	 struct local_state *lsp)
{
	int rc, n, space_left;
	DIR *dirp;
	struct dirent *entry;
	struct list list;
	char *elem;
	int (*statfunc)(const char *, struct stat *);
	struct local_state ls;
#ifdef _THREAD_SAFE
	struct dirent dir_entry;
	entry = &dir_entry;
#endif /* _THREAD_SAFE */

	/*
	 * Open the directory.  If FTW_CHDIR, we chdir to the directory
	 * before opening it.  Note, only nftw may set FTW_CHDIR.
	 */
	if (gsp->flags & FTW_CHDIR) {
		if (chdir(gsp->cwd) < 0) {
			if (errno != EACCES)
				return(-1);
			rc = (*gsp->nftw_fn)(gsp->path, &lsp->sb, FTW_DNR, &lsp->ftw);
			return(rc);
			}
		dirp = opendir(".");
		if (dirp == NULL)
			chdir(gsp->user_cwd);
	} else
		dirp = opendir(gsp->path);

	/*
	 * If the directory can't be opened, call the user function,
	 * telling it the directory is not readable.
	 */
	if (dirp == NULL) {
		if (gsp->ftw_fn)
			return((*gsp->ftw_fn)(gsp->path, &lsp->sb, FTW_DNR));
		return((*gsp->nftw_fn)(gsp->path, &lsp->sb, FTW_DNR, &lsp->ftw));
	}

	/*
	 * We could open the directory.  Call user function on the
	 * directory itself if not depth first.
	 */
	if ((gsp->flags & FTW_DEPTH) == 0) {
		if (gsp->ftw_fn)
			rc = (*gsp->ftw_fn)(gsp->path, &lsp->sb, FTW_D);
		else
			rc = (*gsp->nftw_fn)(gsp->path, &lsp->sb, FTW_D, &lsp->ftw);
		if (rc != 0) {
			closedir(dirp);
			return(rc);
		}
	}

	/*
	 * Read the directory into our FIFO.  We ignore "." and ".."
	 * entries.
	 */
	list_empty(&list);
#ifdef _THREAD_SAFE
	while(readdir_r(dirp, &dir_entry, &entry) == 0 && (entry != NULL)) 
#else
	while((entry = readdir(dirp)) != NULL) 
#endif /* _THREAD_SAFE */
	{
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0)
			continue;

		if (list_append(&list, entry->d_name) == -1) {
			closedir(dirp);
			list_free(&list);
			return(-1);
		}
	}
	closedir(dirp);

	/* NOTE:  In the FTW_CHDIR case, if the original path passed in
	 * is a relative path, then the following problem occurs.
	 * Assume path is "dir1" and exists in the current directory "/tmp".
	 * Then gsp->path = "dir1" which is really a pointer into gsp->cwd!
	 * And the variable ls.endpath points to the end of gsp->path.
	 * By this point, gsp->cwd will be "/tmp/dir1" with gsp->path pointing
	 * to the 6th byte (i.e. 'd').  We change directory to gsp->cwd and
	 * read the directory...each entry in dir1 is stored in the linked
	 * list.  The directory is then closed, but we're still in /tmp/dir1.
	 * Then we go through each entry in the linked list and append the
	 * filename at ls.endpath (i.e. pathname begins at gsp->path so we
	 * basically strcat to gsp->path each filename) and call stat() with
	 * gsp->path.  So the problem is that we're in "/tmp/dir1" and are
	 * trying to stat "dir1/*".  To fix this, nftw() saves the original
	 * cwd in gsp->user_cwd and restores it everytime after we're done
	 * reading the entries.  Now, cwd=/tmp, gsp->cwd=/tmp/dir1, & gsp->
	 * path=dir1, so we look for "dir1/*" in /tmp which is the correct
	 * way.
	 * Note that this problem doesn't occur if the path is absolute
	 * (because gsp->path="/tmp/dir1") or if not in FTW_CHDIR mode 
	 * (because we don't use chdir().)
	 */

	if (gsp->flags & FTW_CHDIR)
		if (chdir(gsp->user_cwd) < 0) {
			list_free(&list);
			return(-1);
			}
	/*
	 * Setup local state for recursion, we are one level deeper.
	 * Setup path for concatenation, making sure we have room
	 * for the slash.
	 */
	ls = *lsp;
	ls.ftw.level++;
	space_left = (PATH_MAX+1) - (ls.endpath - gsp->cwd);
	if (*(ls.endpath - 1) != '/') {
		if (space_left < 1) {
			errno = ENAMETOOLONG;
			list_free(&list);
			return(-1);
		}
		*ls.endpath++ = '/';
		space_left--;
	}
	ls.ftw.base = ls.endpath - gsp->path;

	/*
	 * Process the entries on the list.  We concatenate each directory
	 * entry to the end of our path, making sure we have room, allowing
	 * us to immediately free up the entry.  Setup "n" to indicate that
	 * we are starting through the loop now (see endpath comment below).
	 */
	n = 0;
	while ((elem = list_first(&list)) != NULL) {

		/*
		 * Reset end pointer back from previous time through the
		 * loop, if any.
		 */
		ls.endpath -= n;

		/*
		 * Append component name to the working path, checking for
		 * room.
		 */
		n = strlen(elem);
		if (n >= space_left) {
			errno = ENAMETOOLONG;
			free(elem);
			list_free(&list);
			return(-1);
		}
		memcpy(ls.endpath, elem, n + 1);
		ls.endpath += n;
		free(elem);

		/*
		 * Try to get file status.  If FTW_PHYS is not set, we
		 * follow links.
		 */

		if (gsp->flags & FTW_PHYS)
			statfunc = lstat;
		else
			statfunc = stat;
		if ((*statfunc)(gsp->path, &ls.sb) < 0) {
			/* If FTW_PHYS, lstat failed, AND it wasn't EACCES */
			if ((errno != EACCES) && (gsp->flags & FTW_PHYS)) {
				list_free(&list);
				return(-1);
			}
			rc = -1;	/* temp flag meaning EACCES */
			if (errno!=EACCES && (rc=lstat(gsp->path, &ls.sb))<0) {
				list_free(&list);
				return(-1);
				}
			if (gsp->ftw_fn)
				rc = (*gsp->ftw_fn)(gsp->path, &ls.sb, FTW_NS);
			else
				rc = (*gsp->nftw_fn)(gsp->path, &ls.sb, 
						rc<0 ? FTW_NS:FTW_SLN, &ls.ftw);
			if (rc != 0) {
				list_free(&list);
				return(rc);
			}
			continue;
		}

		/*
		 * If FTW_MOUNT, we only do files on the same device as our
		 * parent.
		 */
		if ((gsp->flags & FTW_MOUNT) && ls.sb.st_dev != lsp->sb.st_dev)
			continue;

		/*
		 * If a directory, recurse.  Otherwise, call the user
		 * function ourselves.
		 */
		if (S_ISDIR(ls.sb.st_mode)) {
			rc = ftw_work(gsp, &ls);
		} else if (S_ISLNK(ls.sb.st_mode)) {
			if (gsp->ftw_fn)
				rc = (*gsp->ftw_fn)(gsp->path, &ls.sb, FTW_SL);
			else
				rc = (*gsp->nftw_fn)(gsp->path, &ls.sb, FTW_SL, &ls.ftw);
		} else {
			if (gsp->ftw_fn)
				rc = (*gsp->ftw_fn)(gsp->path, &ls.sb, FTW_F);
			else
				rc = (*gsp->nftw_fn)(gsp->path, &ls.sb, FTW_F, &ls.ftw);
		}
		if (rc != 0) {
			list_free(&list);
			return(rc);
		}
	}

	/*
	 * In FTW_DEPTH, call user function on this directory now
	 * indicating that the node's children have already been visited.
	 * This can only happen if called by nftw, so we skip the usual
	 * ftw_fn check.
	 */
	if (gsp->flags & FTW_DEPTH) {
		*lsp->endpath = '\0';
		if (gsp->flags & FTW_CHDIR) {
			if (chdir(gsp->user_cwd) < 0)
				return(-1);
		}
		rc = (*gsp->nftw_fn)(gsp->path, &lsp->sb, FTW_DP, &lsp->ftw);
		return(rc);
	}

	return(0);
}

/*
 * This is the (mostly) common setup for ftw() and nftw()
 */
static int
ftw_setup(struct global_state *gsp,
	  struct local_state *lsp,
	  int *rcp)
{
	char *path = gsp->path;
	char *sp;
	char *dp;
	int len;

	/*
	 * Starting at the top.
	 */
	lsp->ftw.level = 0;

	/*
	 * Get file status.  If unsuccessful, call user function and
	 * indicate setup failed.
	 */
	if (stat(path, &lsp->sb) < 0) {
		if (errno != EACCES) {
			*rcp = -1;
			if (!(gsp->flags & FTW_PHYS) && lstat(path, &lsp->sb) >= 0)
				*rcp = (*gsp->nftw_fn)(path, &lsp->sb, FTW_SLN, &lsp->ftw);
			return(-1);
		}
		if (gsp->ftw_fn) {
			*rcp = (*gsp->ftw_fn)(path, &lsp->sb, FTW_NS);
			return(-1);
		}
		/*
		 * Need to setup the FTW structure for nftw user function.
		 */
		sp = strchr(path, '/');
		if (sp == NULL)
			sp = path;
		else if (*path != '\0' && *path == '/')
			sp++;
		lsp->ftw.base = sp - path;
		*rcp = (*gsp->nftw_fn)(path, &lsp->sb, FTW_NS, &lsp->ftw);
		return(-1);
	}
	/*
	 * If FTW_CHDIR, we need get our current directory if the
	 * path is relative to avoid getting lost while recursing.
	 * We let getcwd allocate the buffer for us.
	 */
	if ((gsp->flags & FTW_CHDIR) && *path != '/') {
		gsp->cwd = getcwd(NULL, PATH_MAX+1);
		if (gsp->cwd == NULL) {
			*rcp = -1;
			return(-1);
		}
		dp = gsp->cwd + strlen(gsp->cwd);
		if (dp == gsp->cwd || *(dp-1) != '/')
			*dp++ = '/';
		gsp->path = dp;
		lsp->endpath = gsp->cwd + PATH_MAX;
	} else {
		gsp->cwd = (char *)malloc(PATH_MAX+1);
		if (gsp->cwd == NULL) {
			errno = ENOMEM;
			*rcp = -1;
			return(-1);
		}
		gsp->path = gsp->cwd;
		lsp->endpath = gsp->path + PATH_MAX;
	}
	/*
	 * Copy the path into our buffer, checking the length and
	 * keeping track of where the last slash is for FTW.  We really
	 * only need to do this for nftw, but ...
	 */
	sp = path;
	dp = gsp->path;
	len = 0;
	while (*sp != '\0') {
		if (dp >= lsp->endpath) {
			free(gsp->cwd);
			errno = ENAMETOOLONG;
			*rcp = -1;
			return(-1);
		}
		if (*sp == '/')
			len = sp - path;
		*dp++ = *sp++;
	}
	*dp = '\0';
	lsp->endpath = dp;
	if (len > 0 || (*path != '\0' && *path == '/'))
		len++;
	lsp->ftw.base = len;
	return(0);
}

/*
 * ftw
 *
 * Just set the global state for ftw, call ftw_setup and we are ready
 * to go.  We handle non-directories here so that ftw_work only has
 * to deal with directories.
 */
int
ftw(const char *path,
    int (*fn)(const char *, const struct stat *, int),
    int depth)
{
	struct global_state gs;
	struct local_state ls;
	int rc;

	gs.ftw_fn = fn;
	gs.nftw_fn = (int (*)())0;
	gs.flags = FTW_PHYS;
	gs.path = (char *)path;
	if (ftw_setup(&gs, &ls, &rc) != 0)
		return(rc);
	if (S_ISDIR(ls.sb.st_mode))
		rc = ftw_work(&gs, &ls);
	else if (S_ISLNK(ls.sb.st_mode))
		rc = (*fn)(gs.path, &ls.sb, FTW_SL);
	else
		rc = (*fn)(gs.path, &ls.sb, FTW_F);
	free(gs.cwd);
	return(rc);
}

/*
 * nftw
 *
 * Just set the global state for nftw, call ftw_setup and we are ready
 * to go.  We handle non-directories here so that ftw_work only has
 * to deal with directories.
 */
int
nftw(const char *path,
     int (*fn)(const char *, const struct stat *, int, struct FTW *),
     int depth, int flags)
{
	struct global_state gs;
	struct local_state ls;
	int rc;

	gs.ftw_fn = (int (*)())0;
	gs.nftw_fn = fn;
	gs.flags = flags;
	gs.path = (char *)path;
	if ((flags & FTW_CHDIR) && (*path != '/')) {
		gs.user_cwd = getcwd(NULL, PATH_MAX+1);
		if (gs.user_cwd == NULL) {
			free(gs.cwd);
			return(-1);
			}
		}
	else
		gs.user_cwd = NULL;
	if (ftw_setup(&gs, &ls, &rc) != 0)
		return(rc);
	if (S_ISDIR(ls.sb.st_mode))
		rc = ftw_work(&gs, &ls);
	else if (S_ISLNK(ls.sb.st_mode)) {
		rc = (*fn)(gs.path, &ls.sb, FTW_SL, &ls.ftw);
		}
	else
		rc = (*fn)(gs.path, &ls.sb, FTW_F, &ls.ftw);
	if (gs.user_cwd)
		free(gs.user_cwd);
	free(gs.cwd);
	return(rc);
}
