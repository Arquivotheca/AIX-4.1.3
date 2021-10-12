/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: DirAddDir
 *		DirFindMtime
 *		DirFindName
 *		DirFindSubName
 *		DirFindTail
 *		DirPrintDir
 *		Dir_AddDir
 *		Dir_ClearPath
 *		Dir_Concat
 *		Dir_CopyDir
 *		Dir_Destroy
 *		Dir_FindFile
 *		Dir_FindFileOrLink
 *		Dir_Init
 *		Dir_MTime
 *		Dir_MakeFlags
 *		Dir_PrintDirectories1023
 *		Dir_PrintPath
 *		Dir_ReInit
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: dir.c,v $
 * Revision 1.2.2.3  1992/12/03  19:05:08  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:05  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:23:55  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:53:57  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:21  devrcs
 * 	Added note about a change to dir.c
 * 	[91/08/23  13:55:47  mhickey]
 * 
 * 	Added code to make sure that non-existent directories do not have
 * 	the refCount field of their path entries modified; this lets the
 * 	Dir_FindFile routine correctly determine whether a directory is
 * 	real or not.
 * 	[91/08/22  09:22:34  mhickey]
 * 
 * 	Added check to Dir_FindFile to skipp the lstat if the directory
 * 	is non-existent and the name to append is a subdirectory.
 * 	[91/08/21  14:57:24  mhickey]
 * 
 * 	Added code to make sure that non-existent directories that we want
 * 	to add to search path are either absolute, or the relative path to
 * 	the source directory.  See the comments in the file for the reason
 * 	this is necessary.
 * 	[91/08/20  15:56:15  mhickey]
 * 
 * 	       07/26/91; meh: Changed Dir_AddDir and Dir_FindFile to
 * 	                      allow non-existent directories in the
 * 	                      search path.
 * 	[91/07/26  15:42:55  mhickey]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  15:42:46  mckeen]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-------------------------------------------------------------------------*/
/*--------------*  OSF_CHANGES *-------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*---* 08/23/91; meh: Moved check for relsrcpath into contitional in DirAddDir
 *---*                so that non-exitent dirs can be added before we change
 *---*                directories (if indeed we do).
 *---*
 *---* 08/22/91; meh: Added code to make sure that non-existent directories
 *---*                do not have the refCount field of their path entries
 *---*                modified; this lets the Dir_FindFile routine correctly
 *---*                determine whether a directory is real or not.
 *---*
 *---* 08/21/91; meh: Replaced missing line in DirAddDir, and added check 
 *---*                for a non-existent directory before checking subdirs 
 *---*                Dir_FindFile().
 *---*
 *---* 08-20-91; meh: Added rest of fix for non-existent directories. This 
 *---*                means adding code to check that non-existent dirs are
 *---*                the relative path to the theoretical source dir, or are
 *---*                absolute.  We need to allow the relative path back to 
 *---*                the source dir even if it doesn't exist so that relative
 *---*                VPATH'ing works from a non-existent source directory.
 #---*
 *---* 07/26/91; meh: Changed Dir_AddDir to allow non-existent directories
 *---*                in the search path.  This is necessary to allow relative
 *---*                pathing to work correctly for directories that exist in
 *---*                one source tree, but not in all of them.  If src/x/y/z.mk
 *---*                exists in a tree on the search path, and is included by
 *---*                src/x/y/t/Makefile via ".include <../t/z.mk>", and
 *---*                src/x/y/t does not exist in the tree containing z.mk, 
 *---*                then z.mk will never be found because the correct base 
 *---*                directory (src/x/y/t) for the tree contining z.mk is not
 *---*                on the search path (bbecause it is non-existent).  The 
 *---*                change to Dir_AddDir is to create a path structure for 
 *---*                the non-existent directory, but set it's refcount to 
 *---*                DIR_NOTREAL.  This is positive so the directory stays
 *---*                around,  and is ridiculously large so that is an obvious
 *---*                flag value.  The other half of the change is to make 
 *---*                Dir_FindFile check for the flag value so that it doesn't
 *---*                waste time trying to search the placeholder path list.
 *-------------------------------------------------------------------------*/

#ifndef lint
static char sccsid[] = "@(#)37  1.7  src/bldenv/make/dir.c, bldprocess, bos412, GOLDA411a 1/19/94 16:26:52";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)dir.c	5.6 (Berkeley) 12/28/90";
#endif /* not lint */

/*-
 * dir.c --
 *	Directory searching using wildcards and/or normal names...
 *	Used both for source wildcarding in the Makefile and for finding
 *	implicit sources.
 *
 * The interface for this module is:
 *	Dir_Init  	    Initialize the module.
 *
 *	Dir_Expand	    Given a pattern and a path, return a Lst of names
 *	    	  	    which match the pattern on the search path.
 *
 *	Dir_FindFile	    Searches for a file on a given search path.
 *	    	  	    If it exists, the entire path is returned.
 *	    	  	    Otherwise NULL is returned.
 *
 *	Dir_MTime 	    Return the modification time of a node. The file
 *	    	  	    is searched for along the default search path.
 *	    	  	    The path and mtime fields of the node are filled
 *	    	  	    in.
 *
 *	Dir_AddDir	    Add a directory to a search path.
 *
 *	Dir_MakeFlags	    Given a search path and a command flag, create
 *	    	  	    a string with each of the directories in the path
 *	    	  	    preceded by the command flag and all of them
 *	    	  	    separated by a space.
 *
 *	Dir_Destroy	    Destroy an element of a search path. Frees up all
 *	    	  	    things that can be freed for the element as long
 *	    	  	    as the element is no longer referenced by any other
 *	    	  	    search path.
 *	Dir_ClearPath	    Resets a search path to the empty list.
 *
 * For debugging:
 *	Dir_PrintDirectories	Print stats about the directory cache.
 */

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "make.h"
#include "hash.h"

/*
 *	A search path consists of a Lst of Path structures. A Path structure
 *	has in it the name of the directory and a hash table of all the files
 *	in the directory. This is used to cut down on the number of system
 *	calls necessary to find implicit dependents and their like. Since
 *	these searches are made before any actions are taken, we need not
 *	worry about the directory changing due to creation commands. If this
 *	hampers the style of some makefiles, they must be changed.
 *
 *	A list of all previously-read directories is kept in the
 *	openDirectories Lst. This list is checked first before a directory
 *	is opened.
 *
 *	The need for the caching of whole directories is brought about by
 *	the multi-level transformation code in suff.c, which tends to search
 *	for far more files than regular make does. In the initial
 *	implementation, the amount of time spent performing "stat" calls was
 *	truly astronomical. The problem with hashing at the start is,
 *	of course, that pmake doesn't then detect changes to these directories
 *	during the course of the make. Three possibilities suggest themselves:
 *
 *	    1) just use stat to test for a file's existence. As mentioned
 *	       above, this is very inefficient due to the number of checks
 *	       engendered by the multi-level transformation code.
 *	    2) use readdir() and company to search the directories, keeping
 *	       them open between checks. I have tried this and while it
 *	       didn't slow down the process too much, it could severely
 *	       affect the amount of parallelism available as each directory
 *	       open would take another file descriptor out of play for
 *	       handling I/O for another job. Given that it is only recently
 *	       that UNIX OS's have taken to allowing more than 20 or 32
 *	       file descriptors for a process, this doesn't seem acceptable
 *	       to me.
 *	    3) record the mtime of the directory in the Path structure and
 *	       verify the directory hasn't changed since the contents were
 *	       hashed. This will catch the creation or deletion of files,
 *	       but not the updating of files. However, since it is the
 *	       creation and deletion that is the problem, this could be
 *	       a good thing to do. Unfortunately, if the directory (say ".")
 *	       were fairly large and changed fairly frequently, the constant
 *	       rehashing could seriously degrade performance. It might be
 *	       good in such cases to keep track of the number of rehashes
 *	       and if the number goes over a (small) limit, resort to using
 *	       stat in its place.
 *
 *	An additional thing to consider is that pmake is used primarily
 *	to create C programs and until recently pcc-based compilers refused
 *	to allow you to specify where the resulting object file should be
 *	placed. This forced all objects to be created in the current
 *	directory. This isn't meant as a full excuse, just an explanation of
 *	some of the reasons for the caching used here.
 *
 *	One more note: the location of a target's file is only performed
 *	on the downward traversal of the graph and then only for terminal
 *	nodes in the graph. This could be construed as wrong in some cases,
 *	but prevents inadvertent modification of files when the "installed"
 *	directory for a file is provided in the search path.
 *
 *	Another data structure maintained by this module is an mtime
 *	cache used when the searching of cached directories fails to find
 *	a file. In the past, Dir_FindFile would simply perform an access()
 *	call in such a case to determine if the file could be found using
 *	just the name given. When this hit, however, all that was gained
 *	was the knowledge that the file existed. Given that an access() is
 *	essentially a stat() without the copyout() call, and that the same
 *	filesystem overhead would have to be incurred in Dir_MTime, it made
 *	sense to replace the access() with a stat() and record the mtime
 *	in a cache for when Dir_MTime was actually called.
 */

Lst          dirSearchPath;	/* main search path */
static Lst   srcBasePath;	/* search path for main search path */
static Lst   pathsToFix;	/* Dir_AddDir paths before object dir move */

static Lst   openDirectories;	/* the list of all open directories */

/*
 * Variables for gathering statistics on the efficiency of the hashing
 * mechanism.
 */
static int    hits,	      /* Found in directory cache */
	      misses,	      /* Sad, but not evil misses */
	      nearmisses,     /* Found under search path */
	      bigmisses;      /* Sought by itself */

typedef struct Path {
    string_t      name;	    	/* Name of directory */
    int		  exists;	/* TRUE if the directory exists */
    int	    	  refCount; 	/* Number of paths with this directory */
    int		  hits;	    	/* the number of times a file in this
				 * directory has been found */
    Hash_Table    files;    	/* Hash table of files in directory */
    Lst		  subPaths;	/* List of paths relative to this one */
} Path;

typedef struct SubPath {
    string_t      name;	    	/* Name of subdirectory */
    Path	 *path;		/* Path corresponding to this subpath */
} SubPath;

static Path    	  *dot;	    /* contents of current directory */
static Hash_Table mtimes;   /* Results of doing a last-resort stat in
			     * Dir_FindFile -- if we have to go to the
			     * system to find the file, we might as well
			     * have its mtime on record. XXX: If this is done
			     * way early, there's a chance other rules will
			     * have already updated the file, in which case
			     * we'll update it again. Generally, there won't
			     * be two rules to update a single file, so this
			     * should be ok, but... */

static LstNode DirAddDir (Lst, string_t, string_t);

/*-
 *-----------------------------------------------------------------------
 * Dir_Init --
 *	initialize things for this module
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	some directories may be opened.
 *-----------------------------------------------------------------------
 */
void
Dir_Init (void)
{
    dirSearchPath = Lst_Init();
    srcBasePath = Lst_Init();
    pathsToFix = Lst_Init();
    openDirectories = Lst_Init();
    Hash_InitTable(&mtimes, 0);

    /*
     * Since the Path structure is placed on both openDirectories and
     * the path we give Dir_AddDir (which in this case is openDirectories),
     * we need to remove "." from openDirectories and what better time to
     * do it than when we have to fetch the thing anyway?
     */
    Dir_AddDir (openDirectories, sDOT);
    dot = (Path *) Lst_DeQueue (openDirectories);

    /*
     * We always need to have dot around, so we increment its reference count
     * to make sure it's not destroyed.
     */
    dot->refCount += 1;
}

/*-
 *-----------------------------------------------------------------------
 * Dir_ReInit --
 *	Re-initialize things for this module.  Called when we have moved
 *	to an "object" directory.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	some directories may be opened.
 *-----------------------------------------------------------------------
 */
void
Dir_ReInit (Lst srcbp)
{
    Lst l, newToFix;
    LstNode ln;
    Path *p;
    string_t name;
    struct fixLst {
	Lst path;
	Lst pathnames;
    } *fl;

    Lst_Open(srcbp);
    while ((ln = Lst_Next(srcbp)) != NILLNODE) {
	name = string_create((char *)Lst_Datum(ln));
	Lst_AtEnd(srcBasePath, (ClientData)name);
    }
    Lst_Close(srcbp);

    l = (Lst)Lst_DeQueue(pathsToFix);
    if (l != openDirectories)
	Fatal("Dir_ReInit logic error");
    newToFix = Lst_Init();
    while (!Lst_IsEmpty(pathsToFix)) {
	l = (Lst)Lst_DeQueue(pathsToFix);
	fl = (struct fixLst *) emalloc(sizeof(struct fixLst));
	fl->path = l;
	fl->pathnames = Lst_Init();
	Lst_AtEnd(newToFix, (ClientData)fl);
	while (!Lst_IsEmpty(l)) {
	    p = (Path *)Lst_DeQueue(l);
	    Lst_AtEnd(fl->pathnames, string_ref(p->name));
	    Dir_Destroy(p);
	}
    }
    Lst_Destroy(pathsToFix, NOFREE);
    pathsToFix = NULL;

    if (--dot->refCount != 1)
	Fatal("dot refCount %d", dot->refCount);
    Dir_Destroy(dot);

    Hash_DeleteTable(&mtimes);
    Hash_InitTable(&mtimes, 0);

    Dir_AddDir(dirSearchPath, sDOT);
    dot = (Path *) Lst_Datum(Lst_First(dirSearchPath));

    /*
     * We always need to have dot around, so we increment its reference count
     * to make sure it's not destroyed.
     */
    dot->refCount += 1;

    while (!Lst_IsEmpty(newToFix)) {
	fl = (struct fixLst *) Lst_DeQueue(newToFix);
	l = fl->path;
	while (!Lst_IsEmpty(fl->pathnames)) {
	    name = (string_t)Lst_DeQueue(fl->pathnames);
	    Dir_AddDir(l, name);
	    string_deref(name);
	}
	Lst_Destroy(fl->pathnames, NOFREE);
	free((Address)fl);
    }
    Lst_Destroy(newToFix, NOFREE);
}

/*-
 *-----------------------------------------------------------------------
 * DirFindName --
 *	See if the Path structure describes the same directory as the
 *	given one by comparing their names. Called from Dir_AddDir via
 *	Lst_Find when searching the list of open directories.
 *
 * Results:
 *	0 if it is the same. Non-zero otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
DirFindName (
    ClientData	p,		/* Current name */
    ClientData	dname)		/* Desired name */
{
    return (((Path *)p)->name != (string_t)dname);
}

/*-
 *-----------------------------------------------------------------------
 * DirFindSubName --
 *	See if the SubPath structure describes the same directory as the
 *	given one by comparing their names. Called from Dir_AddDir via
 *	Lst_Find when searching the list of open directories.
 *
 * Results:
 *	0 if it is the same. Non-zero otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
DirFindSubName (
    ClientData	sp,		/* Current name */
    ClientData	dname)		/* Desired name */
{
    return (((SubPath *)sp)->name != (string_t)dname);
}

int
DirFindTail (
    ClientData	pCD,		/* Current name */
    ClientData	name)		/* Desired name */
{
    Path *p = (Path *)pCD;
    int found;

    if (DEBUG(DIR)) {
	printf(" in '%s'...", p->name->data);
    }

    if (!p->exists) {
	if (DEBUG(DIR)) {
	    printf(" directory not found...");
	}
	return(1);
    }

    found = (Hash_FindEntry (&p->files, (string_t)name) != (Hash_Entry *)NULL);

    if (DEBUG(DIR)) {
	printf(" %sfound...", found ? "" : "not ");
    }

    return(!found);
}

static string_t
DirFindMtime(string_t name, int islink)
{
    struct stat	  stb;	    /* Buffer for stat, if necessary */
    Hash_Entry	  *entry;   /* Entry for mtimes table */

    /*
     * Look for the name in the hash table. If we find it, grand. We return
     * a new copy of the name. Otherwise we sadly return a NULL pointer.
     */
    if (DEBUG(DIR)) {
	printf("Looking for \"%s\"...", name->data);
    }
    
    bigmisses += 1;
    entry = Hash_FindEntry(&mtimes, name);
    if (entry != (Hash_Entry *)NULL) {
	if (DEBUG(DIR)) {
	    printf("got it (in mtime cache)\n");
	}
	return(string_ref(name));
    } else if ((islink ? lstat : stat) (name->data, &stb) == 0) {
	entry = Hash_CreateEntry(&mtimes, name, (Boolean *)NULL);
	if (DEBUG(DIR)) {
	    printf("Caching %s for %s\n", Targ_FmtTime(stb.st_mtime),
		    name->data);
	}
	Hash_SetValue(entry, stb.st_mtime);
	return (string_ref(name));
    } else {
	if (DEBUG(DIR)) {
	    printf("failed. Returning NULL\n");
	}
	return ((string_t)NULL);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Dir_FindFile  --
 * Dir_FindFileOrLink  --
 *	Find the file with the given name along the given search path.
 *
 * Results:
 *	The path to the file or NULL. This path is guaranteed to be in a
 *	different part of memory than name and so may be safely free'd.
 *
 * Side Effects:
 *	If the file is found in a directory which is not on the path
 *	already (either 'name' is absolute or it is a relative path
 *	[ dir1/.../dirn/file ] which exists below one of the directories
 *	already on the search path), its directory is added to the end
 *	of the path on the assumption that there will be more files in
 *	that directory later on. Sometimes this is true. Sometimes not.
 *-----------------------------------------------------------------------
 */
string_t
Dir_FindFile (
    string_t      name,    /* the file to find */
    Lst           path)	    /* the Lst of directories to search */
{
    return Dir_FindFileOrLink (name, path, 0);
}

string_t
Dir_FindFileOrLink (
    string_t	  name,     /* the file to find */
    Lst           path,	    /* the Lst of directories to search */
    int		  islink)   /* the file should be a link */
{
    LstNode       ln;	    /* a list element */
    register string_t file; /* the current filename to check */
    register Path *p;	    /* current path member */
    string_t	  head;
    string_t	  tail;

    /*
     * Find the final component of the name and note whether it has a
     * slash in it (the name, I mean)
     */
    head = string_head(name);
    tail = string_tail(name);

    if (DEBUG(DIR)) {
	printf("Searching for %s...", name->data);
    }

    /*
     * Search the path looking for name.  The simplest case is where
     * we have no initial path prefix (head == NULL) in name.
     */
    if (head == NULL) {
	ln = Lst_Find(path, (ClientData)tail, DirFindTail);
	if (ln == NILLNODE) {
	    if (DEBUG(DIR)) {
		printf("failed.\n");
	    }
	    misses += 1;
	    return ((string_t) NULL);
	}
	p = (Path *) Lst_Datum (ln);
	if (p->name == sDOT)
	    file = string_ref(tail);
	else
	    file = string_concat(p->name, tail, STR_ADDSLASH);
	if (DEBUG(DIR)) {
	    printf("returning %s\n", file->data);
	}
	p->hits += 1;
	hits += 1;
	return(file);
    }

    /*
     * If the file is absolute then there is no need for a path search
     */
    if (*head->data == '/') {
	return(DirFindMtime(name, islink));
    }

    /*
     * File is in a relative subdirectory - use subPath search lists
     */
    Lst_Open (path);

    while ((ln = Lst_Next (path)) != NILLNODE) {
	SubPath *subp;
	p = (Path *) Lst_Datum (ln);

	if (DEBUG(DIR)) {
	    printf(" in '%s'...", p->name->data);
	}

	if (!p->exists && *head->data != '.') {
	    if (DEBUG(DIR)) {
		printf(" - directory not found...");
	    }
	    continue;
	}
	ln = Lst_Find(p->subPaths, (ClientData)head, DirFindSubName);
	if (ln == NILLNODE) {
	    if (p->name == sDOT)
		file = head;
	    else {
		file = string_concat(p->name, head, STR_ADDSLASH);
		file = string_flatten(file);
	    }
	    ln = DirAddDir(p->subPaths, file, head);
	}
	subp = (SubPath *) Lst_Datum (ln);

	if (!subp->path->exists) {
	    if (DEBUG(DIR)) {
		printf(" - subdirectory '%s' not found...", subp->name->data);
	    }
	    continue;
	}
	if (Hash_FindEntry (&subp->path->files, tail) != (Hash_Entry *)NULL) {
	    if (p->name == sDOT)
		file = string_ref(name);
	    else {
		file = string_concat(p->name, name, STR_ADDSLASH);
		file = string_flatten(file);
	    }
	    if (DEBUG(DIR)) {
		printf("returning %s\n", file->data);
	    }
	    p->hits += 1;
	    hits += 1;
	    Lst_Close (path);
	    return(file);
	}
    }

    Lst_Close (path);

    if (DEBUG(DIR)) {
	printf("failed. ");
    }
    return((string_t)NULL);
}

/*-
 *-----------------------------------------------------------------------
 * Dir_MTime  --
 *	Find the modification time of the file described by gn along the
 *	search path dirSearchPath.
 * 
 * Results:
 *	The modification time or 0 if it doesn't exist
 *
 * Side Effects:
 *	The modification time is placed in the node's mtime slot.
 *	If the node didn't have a path entry before, and Dir_FindFile
 *	found one for it, the full name is placed in the path slot.
 *-----------------------------------------------------------------------
 */
int
Dir_MTime (GNode *gn)	      /* the file whose modification time is
			       * desired */
{
    string_t      fullName;  /* the full pathname of name */
    struct stat	  stb;	      /* buffer for finding the mod time */
    Hash_Entry	  *entry;
    
    if (gn->type & OP_ARCHV) {
	return Arch_MTime (gn);
    }
    if (gn->path == (string_t)NULL) {
	fullName = Dir_FindFileOrLink (gn->name, dirSearchPath,
				       gn->type & OP_LINK);
    } else {
	fullName = gn->path;
    }
    
    if (fullName == (string_t)NULL) {
	fullName = gn->name;
    }

    entry = Hash_FindEntry(&mtimes, fullName);
    if (entry != (Hash_Entry *)NULL) {
	/*
	 * Only do this once -- the second time folks are checking to
	 * see if the file was actually updated, so we need to actually go
	 * to the file system.
	 */
	if (DEBUG(DIR)) {
	    printf("Using cached time %s for %s\n",
		    Targ_FmtTime((time_t)Hash_GetValue(entry)),
		   fullName->data);
	}
	stb.st_mtime = (time_t)Hash_GetValue(entry);
	Hash_DeleteEntry(&mtimes, entry);
    } else if (((gn->type & OP_LINK) ? lstat : stat) (fullName->data, &stb) < 0) {
	if (gn->type & OP_MEMBER) {
	    return Arch_MemMTime (gn);
	} else {
	    stb.st_mtime = 0;
	}
    }
    if (fullName && gn->path == (string_t)NULL) {
	gn->path = fullName;
    }
    
    gn->mtime = stb.st_mtime;
    return (gn->mtime);
}

/*-
 *-----------------------------------------------------------------------
 * Dir_AddDir --
 *	Add the given name to the end of the given path. The order of
 *	the arguments is backwards so ParseDoDependency can do a
 *	Lst_ForEach of its list of paths...
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	A structure is added to the list and the directory is 
 *	read and hashed.
 *-----------------------------------------------------------------------
 */
void
Dir_AddDir (
    Lst           path,	      /* the path to which the directory should be
			       * added */
    string_t	  name)       /* the name of the directory to add */
{
    LstNode       ln;	      /* node in case Path structure is found */
    string_t	  p;

    if (pathsToFix && Lst_Member (pathsToFix, (ClientData)path) == NILLNODE)
	    (void) Lst_AtEnd(pathsToFix, (ClientData)path);
    DirAddDir(path, name, (string_t) NULL);
    if (*name->data == '/' || Lst_IsEmpty(srcBasePath))
	return;
    Lst_Open(srcBasePath);
    while ((ln = Lst_Next(srcBasePath)) != NILLNODE) {
	p = (string_t)Lst_Datum(ln);
	if (p == sDOT)
	    continue;
	if (name == sDOT)
	    DirAddDir(path, p, (string_t) NULL);
	else {
	    p = string_concat(p, name, STR_ADDSLASH);
	    p = string_flatten(p);
	    DirAddDir(path, p, (string_t) NULL);
	}
    }
    Lst_Close(srcBasePath);
}

static LstNode
DirAddDir (
    Lst           path,	      /* the path to which the directory should be
			       * added */
    string_t	  name,       /* the name of the directory to add */
    string_t	  subPath)    /* subpath of this path */
{
    LstNode       ln;	      /* node in case Path structure is found */
    register Path *p;	      /* pointer to new Path structure */
    DIR     	  *d;	      /* for reading directory */
    register struct dirent *dp; /* entry in directory */
    char	  *fName;
    string_t	  sfName;
    SubPath	  *sp;

    ln = Lst_Find (openDirectories, (ClientData)name, DirFindName);
    if (ln != NILLNODE) {
	p = (Path *)Lst_Datum (ln);

	if (DEBUG(DIR)) {
	    printf("Already cached %sdirectory %s ...",
		   p->exists ? "" : "non-existant ", p->name->data);
	    if (subPath != (string_t) NULL)
		printf("New subpath %s...", subPath->data);
	    printf("done\n");
	}
	if (subPath == (string_t) NULL) {
	    ln = Lst_Member(path, (ClientData)p);
	    if (ln == NILLNODE) {
		p->refCount += 1;
		(void)Lst_AtEnd(path, (ClientData)p);
		ln = Lst_Last(path);
	    }
	    return(ln);
	}
	sp = (SubPath *) emalloc (sizeof (SubPath));
	sp->name = string_ref(subPath);
	sp->path = p;
	p->refCount += 1;
	(void)Lst_AtEnd(path, (ClientData)sp);
	ln = Lst_Last(path);
	return(ln);
    }

    p = (Path *) emalloc (sizeof (Path));
    p->name = string_ref(name);
    p->hits = 0;
    p->refCount = 1;

    d = opendir (name->data);

    if (DEBUG(DIR)) {
	printf("Caching %sdirectory '%s'...",
	       (d == (DIR *) NULL) ? "non-existant " : "", name->data);
    }

    p->subPaths = Lst_Init();
    Hash_InitTable (&p->files, -1);
    p->exists = (d != (DIR *) NULL);

    if (p->exists) {
	while ((dp = readdir (d)) != (struct dirent *) NULL) {
	    /*
	     * This may be a little slower, but is more portable...
	     */
	    if (dp->d_ino == 0)
		continue;
	    fName = dp->d_name;
	    if (fName[0] == '.' &&
		(fName[1] == '\0' ||
		 (fName[1] == '.' && fName[2] == '\0')))
		continue;
	    sfName = string_create(fName);
	    (void)Hash_CreateEntry(&p->files, sfName, (Boolean *)NULL);
	}
	(void) closedir (d);
	p->exists = 1;
    }

    (void)Lst_AtEnd (openDirectories, (ClientData)p);
    if (subPath == (string_t) NULL) {
	(void)Lst_AtEnd (path, (ClientData)p);
    } else {
	sp = (SubPath *) emalloc (sizeof (SubPath));
	sp->name = string_ref(subPath);
	sp->path = p;
	p->refCount += 1;
	(void)Lst_AtEnd (path, (ClientData)sp);
    }

    if (DEBUG(DIR)) {
	if (subPath != (string_t) NULL)
	    printf("New subpath %s...", subPath->data);
	printf("done\n");
    }
    return(Lst_Last(path));
}

/*-
 *-----------------------------------------------------------------------
 * Dir_CopyDir --
 *	Callback function for duplicating a search path via Lst_Duplicate.
 *	Ups the reference count for the directory.
 *
 * Results:
 *	Returns the Path it was given.
 *
 * Side Effects:
 *	The refCount of the path is incremented.
 *
 *-----------------------------------------------------------------------
 */
ClientData
Dir_CopyDir(ClientData p)		/* Directory descriptor to copy */
{
    ((Path *)p)->refCount += 1;
    return (p);
}

/*-
 *-----------------------------------------------------------------------
 * Dir_MakeFlags --
 *	Make a string by taking all the directories in the given search
 *	path and preceding them by the given flag. Used by the suffix
 *	module to create variables for compilers based on suffix search
 *	paths.
 *
 * Results:
 *	The string mentioned above. Note that there is no space between
 *	the given flag and each directory. The empty string is returned if
 *	Things don't go well.
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
string_t
Dir_MakeFlags (
    string_t	  flag,  /* flag which should precede each directory */
    Lst	    	  path)	  /* list of directories */
{
    string_t	  str;	  /* the string which will be returned */
    string_t	  tstr;   /* the current directory preceded by 'flag' */
    LstNode	  ln;	  /* the node of the current directory */
    Path	  *p;	  /* the structure describing the current directory */
    
    str = NULL;
    
    Lst_Open (path);
    while ((ln = Lst_Next (path)) != NILLNODE) {
	p = (Path *) Lst_Datum (ln);
	tstr = string_concat (flag, p->name, 0);
	str = string_concat (str, tstr, STR_ADDSPACE | STR_DOFREE);
    }
    Lst_Close (path);
    
    if (str == NULL)
	str = string_ref(sNULL);
    return (str);
}

/*-
 *-----------------------------------------------------------------------
 * Dir_Destroy --
 *	Nuke a directory descriptor, if possible. Callback procedure
 *	for the suffixes module when destroying a search path.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If no other path references this directory (refCount == 0),
 *	the Path and all its data are freed.
 *
 *-----------------------------------------------------------------------
 */
void
Dir_Destroy (Path *p)	    /* The directory descriptor to nuke */
{
    p->refCount -= 1;

    if (p->refCount == 0) {
	LstNode	ln;

	ln = Lst_Member (openDirectories, (ClientData)p);
	Lst_Remove (openDirectories, ln);

	Hash_DeleteTable (&p->files);
	string_deref(p->name);
	free((Address)p);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Dir_ClearPath --
 *	Clear out all elements of the given search path. This is different
 *	from destroying the list, notice.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The path is set to the empty list.
 *
 *-----------------------------------------------------------------------
 */
void
Dir_ClearPath(Lst path) 	/* Path to clear */
{
    Path    *p;
    while (!Lst_IsEmpty(path)) {
	p = (Path *)Lst_DeQueue(path);
	Dir_Destroy(p);
    }
}
	    

/*-
 *-----------------------------------------------------------------------
 * Dir_Concat --
 *	Concatenate two paths, adding the second to the end of the first.
 *	Makes sure to avoid duplicates.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Reference counts for added dirs are upped.
 *
 *-----------------------------------------------------------------------
 */
void
Dir_Concat(
    Lst	    path1,  	/* Dest */
    Lst	    path2)  	/* Source */
{
    LstNode ln;
    Path    *p;

    for (ln = Lst_First(path2); ln != NILLNODE; ln = Lst_Succ(ln)) {
	p = (Path *)Lst_Datum(ln);
	if (Lst_Member(path1, (ClientData)p) == NILLNODE) {
	    p->refCount += 1;
	    (void)Lst_AtEnd(path1, (ClientData)p);
	}
    }
}

/********** DEBUG INFO **********/
void
Dir_PrintDirectories(void)
{
    LstNode	ln;
    Path	*p;
    
    printf ("#*** Directory Cache:\n");
    printf ("# Stats: %d hits %d misses %d near misses %d losers (%d%%)\n",
	      hits, misses, nearmisses, bigmisses,
	      (hits+bigmisses+nearmisses ?
	       hits * 100 / (hits + bigmisses + nearmisses) : 0));
    printf ("# %-20s referenced\thits\n", "directory");
    Lst_Open (openDirectories);
    while ((ln = Lst_Next (openDirectories)) != NILLNODE) {
	p = (Path *) Lst_Datum (ln);
	printf ("# %-20s %10d\t%4d%s\n", p->name->data,
		p->refCount, p->hits,
		p->exists ? "" : " (missing)");
    }
    Lst_Close (openDirectories);
}

static int
DirPrintDir (ClientData p, ClientData nul)
{
    printf ("%s ", ((Path *)p)->name->data);
    return (0);
}

void
Dir_PrintPath (Lst path)
{
    Lst_ForEach (path, DirPrintDir, (ClientData)0);
}
