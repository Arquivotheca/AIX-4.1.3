#ifndef lint
static char sccsid[] = "@(#)10	1.8 src/bos/usr/ccs/bin/make/dir.c, cmdmake, bos411, 9433B411a 8/18/94 13:15:17";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: DirAddDir
 *		DirFindName
 *		Dir_FindFile
 *		Dir_Init
 *		Dir_MTime
 *		Dir_PrintDirectories
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: dir.c,v $ $Revision: 1.2.2.2 $ (OSF) $Date: 1991/11/14 10:32:00 $";
#endif
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * dir.c --
 *	Directory searching using wildcards and/or normal names...
 *	Used both for source wildcarding in the Makefile and for finding
 *	implicit sources.
 *
 * The interface for this module is:
 *	Dir_Init  	    Initialize the module.
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
 * For debugging:
 *	Dir_PrintDirectories	Print stats about the directory cache.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/dir.h>
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

Lst   dirSearchPath;	/* main search path */

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
    char         *name;	    	/* Name of directory */
    int	    	  refCount; 	/* Number of paths with this directory */
    int		  hits;	    	/* the number of times a file in this
				 * directory has been found */
    Hash_Table    files;    	/* Hash table of files in directory */
} Path;

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
    dirSearchPath = Lst_Init (FALSE);
    openDirectories = Lst_Init (FALSE);
    Hash_InitTable(&mtimes, 0);
    
    /*
     * Since the Path structure is placed on both openDirectories and
     * the path we give DirAddDir (which in this case is openDirectories),
     * we need to remove "." from openDirectories and what better time to
     * do it than when we have to fetch the thing anyway?
     */
    DirAddDir (openDirectories, ".");
    dot = (Path *) Lst_DeQueue (openDirectories);

    /*
     * We always need to have dot around, so we increment its reference count
     * to make sure it's not destroyed.
     */
    dot->refCount += 1;
}

/*-
 *-----------------------------------------------------------------------
 * DirFindName --
 *	See if the Path structure describes the same directory as the
 *	given one by comparing their names. Called from DirAddDir via
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
    Path          *p,	      /* Current name */
    char	  *dname      /* Desired name */
    )
{
    return (strcmp (p->name, dname));
}

/*-
 *-----------------------------------------------------------------------
 * Dir_FindFile  --
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
char *
Dir_FindFile (
    char    	  *name     /* the file to find */
    )
{
    char *p1;	    /* pointer into p->name */
    char *p2;	    /* pointer into name */
    LstNode       ln;	    /* a list element */
    char *file;    /* the current filename to check */
    Path *p;	    /* current path member */
    char *cp;	    /* index of first slash, if any */
    Boolean	  hasSlash; /* true if 'name' contains a / */
    struct stat	  stb;	    /* Buffer for stat, if necessary */
    Hash_Entry	  *entry;   /* Entry for mtimes table */
    Lst           path;	    /* the Lst of directories to search */
    
    /*
     * Find the final component of the name and note whether it has a
     * slash in it (the name, I mean)
     */
    cp = rindex (name, '/');
    if (cp) {
	hasSlash = TRUE;
	cp += 1;
    } else {
	hasSlash = FALSE;
	cp = name;
    }
    
    if (DEBUG(DIR)) {
fprintf(stderr,MSGSTR(DIRMSG1, "Searching for %s..."), name);
    }
    /*
     * No matter what, we always look for the file in the current directory
     * before anywhere else and we *do not* add the ./ to it if it exists.
     * This is so there are no conflicts between what the user specifies
     * (fish.c) and what pmake finds (./fish.c).
     */
    if ((!hasSlash || (cp - name == 2 && *name == '.')) &&
	(Hash_FindEntry (&dot->files, (Address)cp) != (Hash_Entry *)NULL)) {
	    if (DEBUG(DIR)) {
	fprintf(stderr,MSGSTR(DIRMSG24, "in '.'\n"));
	    }
	    hits += 1;
	    dot->hits += 1;
	    return (strdup (name));
    }
    
    path = dirSearchPath;
    if (Lst_Open (path) == FAILURE) {
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG2, "could not open path, file not found\n"));
	}
	misses += 1;
	return ((char *) NULL);
    }
    
    /*
     * We look through all the directories on the path seeking one which
     * contains the final component of the given name and whose final
     * component(s) match the name's initial component(s). If such a beast
     * is found, we concatenate the directory name and the final component
     * and return the resulting string. If we don't find any such thing,
     * we go on to phase two...
     */
    while ((ln = Lst_Next (path)) != NILLNODE) {
	p = (Path *) Lst_Datum (ln);
	if (DEBUG(DIR)) {
	   fprintf(stderr,"%s...", p->name);
	}
	if (Hash_FindEntry (&p->files, (Address)cp) != (Hash_Entry *)NULL) {
	    if (DEBUG(DIR)) {
	fprintf(stderr,MSGSTR(DIRMSG3, "here..."));
	    }
	    if (hasSlash) {
		/*
		 * If the name had a slash, its initial components and p's
		 * final components must match. This is false if a mismatch
		 * is encountered before all of the initial components
		 * have been checked (p2 > name at the end of the loop), or
		 * we matched only part of one of the components of p
		 * along with all the rest of them (*p1 != '/').
		 */
		p1 = p->name + strlen (p->name) - 1;
		p2 = cp - 2;
		while (p2 >= name && *p1 == *p2) {
		    p1 -= 1; p2 -= 1;
		}
		if (p2 >= name || (p1 >= p->name && *p1 != '/')) {
		    if (DEBUG(DIR)) {
		fprintf(stderr,MSGSTR(DIRMSG4, 
				"component mismatch -- continuing..."));
		    }
		    continue;
		}
	    }
	    file = Str_Concat (p->name, cp, STR_ADDSLASH);
	    if (DEBUG(DIR)) {
	fprintf(stderr,MSGSTR(DIRMSG5, "returning %s\n"), file);
	    }
	    Lst_Close (path);
	    p->hits += 1;
	    hits += 1;
	    return (file);
	} else if (hasSlash) {
	    /*
	     * If the file has a leading path component and that component
	     * exactly matches the entire name of the current search
	     * directory, we assume the file doesn't exist and return NULL.
	     */
	    for (p1 = p->name, p2 = name; *p1 && *p1 == *p2; p1++, p2++) {
		continue;
	    }
	    if (*p1 == '\0' && p2 == cp - 1) {
		if (DEBUG(DIR)) {
		   fprintf(stderr,MSGSTR(DIRMSG6, 
			"must be here but is not -- returing NULL\n"));
		}
		Lst_Close (path);
		return ((char *) NULL);
	    }
	}
    }
    
    /*
     * We didn't find the file on any existing members of the directory.
     * If the name doesn't contain a slash, that means it doesn't exist.
     * If it *does* contain a slash, however, there is still hope: it
     * could be in a subdirectory of one of the members of the search
     * path. (eg. /usr/include and sys/types.h. The above search would
     * fail to turn up types.h in /usr/include, but it *is* in
     * /usr/include/sys/types.h) If we find such a beast, we assume there
     * will be more (what else can we assume?) and add all but the last
     * component of the resulting name onto the search path (at the
     * end). This phase is only performed if the file is *not* absolute.
     */
    if (!hasSlash) {
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG7, "failed.\n"));
	}
	misses += 1;
	return ((char *) NULL);
    }
    
    if (*name != '/') {
	Boolean	checkedDot = FALSE;
	
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG8, "failed. Trying subdirectories..."));
	}
	(void) Lst_Open (path);
	while ((ln = Lst_Next (path)) != NILLNODE) {
	    p = (Path *) Lst_Datum (ln);
	    if (p != dot) {
		file = Str_Concat (p->name, name, STR_ADDSLASH);
	    } else {
		/*
		 * Checking in dot -- DON'T put a leading ./ on the thing.
		 */
		file = strdup(name);
		checkedDot = TRUE;
	    }
	    if (DEBUG(DIR)) {
	fprintf(stderr,MSGSTR(DIRMSG9, "checking %s..."), file);
	    }
	    
		
	    if (stat (file, &stb) == 0) {
		if (DEBUG(DIR)) {
		   fprintf(stderr,MSGSTR(DIRMSG10, "got it.\n"));
		}
		
		Lst_Close (path);
		
		/*
		 * We've found another directory to search. We know there's
		 * a slash in 'file' because we put one there. We nuke it after
		 * finding it and call DirAddDir to add this new directory
		 * onto the existing search path. Once that's done, we restore
		 * the slash and triumphantly return the file name, knowing
		 * that should a file in this directory every be referenced
		 * again in such a manner, we will find it without having to do
		 * numerous numbers of access calls. Hurrah!
		 */
		cp = rindex (file, '/');
		*cp = '\0';
		DirAddDir (path, file);
		*cp = '/';
		
		/*
		 * Save the modification time so if it's needed, we don't have
		 * to fetch it again.
		 */
		if (DEBUG(DIR)) {
		   fprintf(stderr,MSGSTR(DIRMSG11, "Caching %s for %s\n"), 
			Targ_FmtTime(stb.st_mtime), file);
		}
		entry = Hash_CreateEntry(&mtimes, file,
					 (Boolean *)NULL);
		Hash_SetValue(entry, stb.st_mtime);
		nearmisses += 1;
		return (file);
	    } else {
		free (file);
	    }
	}
	
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG12, "failed. "));
	}
	Lst_Close (path);

	if (checkedDot) {
	    /*
	     * Already checked by the given name, since . was in the path,
	     * so no point in proceeding...
	     */
	    if (DEBUG(DIR)) {
	fprintf(stderr,MSGSTR(DIRMSG13, "Checked . already, returning NULL\n"));
	    }
	    return(NULL);
	}
    }
    
    /*
     * Didn't find it that way, either. Sigh. Phase 3. Add its directory
     * onto the search path in any case, just in case, then look for the
     * thing in the hash table. If we find it, grand. We return a new
     * copy of the name. Otherwise we sadly return a NULL pointer. Sigh.
     * Note that if the directory holding the file doesn't exist, this will
     * do an extra search of the final directory on the path. Unless something
     * weird happens, this search won't succeed and life will be groovy.
     *
     * Sigh. We cannot add the directory onto the search path because
     * of this amusing case:
     * $(INSTALLDIR)/$(FILE): $(FILE)
     *
     * $(FILE) exists in $(INSTALLDIR) but not in the current one.
     * When searching for $(FILE), we will find it in $(INSTALLDIR)
     * b/c we added it here. This is not good...
     */
    if (DEBUG(DIR)) {
fprintf(stderr,MSGSTR(DIRMSG14, "Looking for \"%s\"..."), name);
    }
    
    bigmisses += 1;
    entry = Hash_FindEntry(&mtimes, name);
    if (entry != (Hash_Entry *)NULL) {
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG15, "got it (in mtime cache)\n"));
	}
	return(strdup(name));
    } else if (stat (name, &stb) == 0) {
	entry = Hash_CreateEntry(&mtimes, name, (Boolean *)NULL);
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG11, 
		"Caching %s for %s\n"), Targ_FmtTime(stb.st_mtime), name);
	}
	Hash_SetValue(entry, stb.st_mtime);
	return (strdup (name));
    } else {
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG16, "failed. Returning NULL\n"));
	}
	return ((char *)NULL);
    }
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
Dir_MTime (
    GNode         *gn	      /* the file whose modification time is
			       * desired */
    )
{
    char          *fullName;  /* the full pathname of name */
    struct stat	  stb;	      /* buffer for finding the mod time */
    Hash_Entry	  *entry;
    
    /* If this node is an archive. */
    if (gn->type & OP_ARCHV)
    {
	return Arch_MTime (gn);
    }

    if (gn->path == (char *)NULL) {
	fullName = Dir_FindFile (gn->name);
    } else {
	fullName = gn->path;
    }
    
    if (fullName == (char *)NULL) {
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
	   fprintf(stderr,MSGSTR(DIRMSG17, "Using cached time %s for %s\n"),
		    Targ_FmtTime((time_t)Hash_GetValue(entry)), fullName);
	}
	stb.st_mtime = (time_t)Hash_GetValue(entry);
	Hash_DeleteEntry(&mtimes, entry);
    } else if (stat (fullName, &stb) < 0) {
	if (gn->type & OP_MEMBER) {
	    return Arch_MemMTime (gn);
	}
	
	/* if this node is an archive then it's modification time is
	   that of its children. */
	if (gn->type & OP_ARCHV)
	{
	    stb.st_mtime=gn->cmtime;
	}
	else {
	    stb.st_mtime = 0;
	}
    }
    if (fullName && gn->path == (char *)NULL) {
	gn->path = fullName;
    }
    
    gn->mtime = stb.st_mtime;
    return (gn->mtime);
}

/*-
 *-----------------------------------------------------------------------
 * DirAddDir --
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
DirAddDir (
    const Lst           path,	      /* the path to which the directory should be
			       * added */
    const char          *name       /* the name of the directory to add */
    )
{
    LstNode       ln;	      /* node in case Path structure is found */
    Path *p;	      /* pointer to new Path structure */
    DIR     	  *d;	      /* for reading directory */
    struct dirent *dp; /* entry in directory */
    
    ln = Lst_Find (openDirectories, (ClientData)name, DirFindName);
    if (ln != NILLNODE) {
	p = (Path *)Lst_Datum (ln);
	if (Lst_Member(path, (ClientData)p) == NILLNODE) {
	    p->refCount += 1;
	    (void)Lst_AtEnd (path, (ClientData)p);
	}
    } else {
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG18, "Caching %s..."), name);
	    fflush(stderr);
	}
	
	if ((d = opendir (name)) != (DIR *) NULL) {
	    emalloc(p,sizeof (Path));
	    p->name = strdup (name);
	    p->hits = 0;
	    p->refCount = 1;
	    Hash_InitTable (&p->files, -1);
	    
	    /*
	     * Skip the first two entries -- these will *always* be . and ..
	     */
	    (void)readdir(d);
	    (void)readdir(d);
	    
	    while ((dp = readdir (d)) != (struct dirent *) NULL) {
		(void)Hash_CreateEntry(&p->files, dp->d_name, (Boolean *)NULL);
	    }
	    (void) closedir (d);
	    (void)Lst_AtEnd (openDirectories, (ClientData)p);
	    (void)Lst_AtEnd (path, (ClientData)p);
	}
	if (DEBUG(DIR)) {
	   fprintf(stderr,MSGSTR(DIRMSG19, "done\n"));
	}
    }
}

/********** DEBUG INFO **********/
void
Dir_PrintDirectories(void)
{
    LstNode	ln;
    Path	*p;
    
   fprintf(stderr,MSGSTR(DIRMSG20, "#*** Directory Cache:\n"));
   fprintf(stderr,MSGSTR(DIRMSG21, 
	      "# Stats: %d hits %d misses %d near misses %d losers (%d%%)\n"),
	      hits, misses, nearmisses, bigmisses,
	      (hits+bigmisses+nearmisses ?
	       hits * 100 / (hits + bigmisses + nearmisses) : 0));
   fprintf(stderr,MSGSTR(DIRMSG22, 
	"# %-20s referenced\thits\n"), MSGSTR(DIRMSG23, "directory"));
    if (Lst_Open (openDirectories) == SUCCESS) {
	while ((ln = Lst_Next (openDirectories)) != NILLNODE) {
	    p = (Path *) Lst_Datum (ln);
	    fprintf (stderr,"# %-20s %10d\t%4d\n", p->name, p->refCount, p->hits);
	}
	Lst_Close (openDirectories);
    }
}
