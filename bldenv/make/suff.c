/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: SuffAddLevel
 *		SuffAddSrc
 *		SuffApplyTransform1276
 *		SuffDirDestroy
 *		SuffExpandChildren1116
 *		SuffFindArchiveDeps1383
 *		SuffFindCmds
 *		SuffFindNormalDeps1505
 *		SuffFindThem
 *		SuffFree
 *		SuffFreeSrc
 *		SuffGNHasNameP
 *		SuffInsert
 *		SuffParseTransform
 *		SuffPrintName
 *		SuffPrintSuff
 *		SuffPrintTrans
 *		SuffRebuildGraph
 *		SuffSuffHasNameP
 *		SuffSuffIsPrefix
 *		SuffSuffIsSuffix
 *		Suff_AddInclude
 *		Suff_AddLib
 *		Suff_AddSuffix
 *		Suff_AddTransform
 *		Suff_ClearSuffixes
 *		Suff_DoPaths
 *		Suff_EndTransform
 *		Suff_FindDeps
 *		Suff_GetPath
 *		Suff_Init
 *		Suff_IsTransform
 *		Suff_PrintAll
 *		Suff_SetNull
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
 * $Log: suff.c,v $
 * Revision 1.2.4.1  1993/09/22  17:55:18  damon
 * 	CR 73. Initialize mem ptr to null
 * 	[1993/09/22  17:55:07  damon]
 *
 * Revision 1.2.2.3  1992/12/03  19:07:24  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:38  damon]
 * 
 * Revision 1.2.2.2  1992/09/24  19:27:32  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:55:47  gm]
 * 
 * Revision 1.2  1991/12/05  20:45:19  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  17:19:41  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:10:34  mckeen]
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

#ifndef lint
static char sccsid[] = "@(#)53  1.4  src/bldenv/make/suff.c, bldprocess, bos412, GOLDA411a 1/19/94 16:31:44";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)suff.c	5.6 (Berkeley) 6/1/90";
#endif /* not lint */

/*-
 * suff.c --
 *	Functions to maintain suffix lists and find implicit dependents
 *	using suffix transformation rules
 *
 * Interface:
 *	Suff_Init 	    	Initialize all things to do with suffixes.
 *
 *	Suff_DoPaths	    	This function is used to make life easier
 *	    	  	    	when searching for a file according to its
 *	    	  	    	suffix. It takes the global search path,
 *	    	  	    	as defined using the .PATH: target, and appends
 *	    	  	    	its directories to the path of each of the
 *	    	  	    	defined suffixes, as specified using
 *	    	  	    	.PATH<suffix>: targets. In addition, all
 *	    	  	    	directories given for suffixes labeled as
 *	    	  	    	include files or libraries, using the .INCLUDES
 *	    	  	    	or .LIBS targets, are played with using
 *	    	  	    	Dir_MakeFlags to create the .INCLUDES and
 *	    	  	    	.LIBS global variables.
 *
 *	Suff_ClearSuffixes  	Clear out all the suffixes and defined
 *	    	  	    	transformations.
 *
 *	Suff_IsTransform    	Return TRUE if the passed string is the lhs
 *	    	  	    	of a transformation rule.
 *
 *	Suff_AddSuffix	    	Add the passed string as another known suffix.
 *
 *	Suff_GetPath	    	Return the search path for the given suffix.
 *
 *	Suff_AddInclude	    	Mark the given suffix as denoting an include
 *	    	  	    	file.
 *
 *	Suff_AddLib	    	Mark the given suffix as denoting a library.
 *
 *	Suff_AddTransform   	Add another transformation to the suffix
 *	    	  	    	graph. Returns  GNode suitable for framing, I
 *	    	  	    	mean, tacking commands, attributes, etc. on.
 *
 *	Suff_SetNull	    	Define the suffix to consider the suffix of
 *	    	  	    	any file that doesn't have a known one.
 *
 *	Suff_FindDeps	    	Find implicit sources for and the location of
 *	    	  	    	a target based on its suffix. Returns the
 *	    	  	    	bottom-most node added to the graph or NILGNODE
 *	    	  	    	if the target had no implicit sources.
 */

#include    	  <stdio.h>
#include	  "make.h"

static Lst       sufflist;	/* Lst of suffixes */
static Lst       transforms;	/* Lst of transformation rules */

static int        sNum = 0;	/* Counter for assigning suffix numbers */

/*
 * Structure describing an individual suffix.
 */
typedef struct _Suff {
    string_t     name;	    	/* The suffix itself */
    short	 flags;      	/* Type of suffix */
#define SUFF_INCLUDE	  0x01	    /* One which is #include'd */
#define SUFF_LIBRARY	  0x02	    /* One which contains a library */
#define SUFF_NULL 	  0x04	    /* The empty suffix */
    Lst    	 searchPath;	/* The path along which files of this suffix
				 * may be found */
    int          sNum;	      	/* The suffix number */
    Lst          parents;	/* Suffixes we have a transformation to */
    Lst          children;	/* Suffixes we have a transformation from */
} Suff;

/*
 * Structure used in the search for implied sources.
 */
typedef struct _Src {
    string_t        file;	/* The file to look for */
    string_t	    pref;  	/* Prefix from which file was formed */
    Suff            *suff;	/* The suffix on the file */
    struct _Src     *parent;	/* The Src for which this is a source */
    GNode           *node;	/* The node describing the file */
    int	    	    children;	/* Count of existing children (so we don't free
				 * this thing too early or never nuke it) */
} Src;

static Suff 	    *suffNull;	/* The NULL suffix for this run */
static Suff 	    *emptySuff;	/* The empty suffix required for POSIX
				 * single-suffix transformation rules */

	/*************** Lst Predicates ****************/
/*-
 *-----------------------------------------------------------------------
 * SuffSuffIsSuffix  --
 *	See if suff is a suffix of str.
 *
 * Results:
 *	0 if the suffix is the one desired, non-zero if not.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static int
SuffSuffIsSuffix (
    ClientData	s,		/* possible suffix of str */
    ClientData	str)		/* string to examine */
{
    return(string_suff((string_t)str) != string_suff(((Suff *)s)->name));
}

/*-
 *-----------------------------------------------------------------------
 * SuffSuffIsPrefix  --
 *	See if the suffix described by s is a prefix of the string. Care
 *	must be taken when using this to search for transformations and
 *	what-not, since there could well be two suffixes, one of which
 *	is a prefix of the other...
 *
 * Results:
 *	0 if s is a prefix of str. non-zero otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
SuffSuffIsPrefix (
    ClientData	sCD,		/* suffix to compare */
    ClientData	strCD)		/* string to examine */
{
    Suff	*s = (Suff *)sCD;
    string_t	str = (string_t)strCD;
    string_t	s_suff, str_suff;

    str_suff = string_suff(str);
    if (str_suff == (string_t) NULL)
	return(1);
    s_suff = string_suff(s->name);
    return (s_suff != string_pref(str) &&
	    (str_suff != str || s_suff != str_suff));
}

/*-
 *-----------------------------------------------------------------------
 * SuffSuffHasNameP --
 *	Callback procedure for finding a suffix based on its name. Used by
 *	Suff_GetPath.
 *
 * Results:
 *	0 if the suffix is of the given name. non-zero otherwise.
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
SuffSuffHasNameP (
    ClientData	s,		/* Suffix to check */
    ClientData	sname)		/* Desired name */
{
    return ((string_t)sname != ((Suff *)s)->name);
}

/*-
 *-----------------------------------------------------------------------
 * SuffGNHasNameP  --
 *	See if the graph node has the desired name
 *
 * Results:
 *	0 if it does. non-zero if it doesn't
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
SuffGNHasNameP (
    ClientData	gn,		/* current node we're looking at */
    ClientData	name)		/* name we're looking for */
{
    return ((string_t)name != ((GNode *)gn)->name);
}

 	    /*********** Maintenance Functions ************/

static void
SuffDirDestroy(ClientData p)
{
    Dir_Destroy((struct Path *)p);
}

/*-
 *-----------------------------------------------------------------------
 * SuffFree  --
 *	Free up all memory associated with the given suffix structure.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the suffix entry is detroyed
 *-----------------------------------------------------------------------
 */
static void
SuffFree (ClientData sCD)
{
    Suff *s = (Suff *)sCD;

    Lst_Destroy (s->children, NOFREE);
    Lst_Destroy (s->parents, NOFREE);
    Lst_Destroy (s->searchPath, SuffDirDestroy);
    string_deref(s->name);
    free ((Address)s);
}

/*-
 *-----------------------------------------------------------------------
 * SuffInsert  --
 *	Insert the suffix into the list keeping the list ordered by suffix
 *	numbers.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Not really
 *-----------------------------------------------------------------------
 */
static void
SuffInsert (
    Lst           l,		/* the list where in s should be inserted */
    Suff          *s)		/* the suffix to insert */
{
    LstNode 	  ln;		/* current element in l we're examining */
    Suff          *s2;		/* the suffix descriptor in this element */

    Lst_Open (l);
    s2 = (Suff *) NULL;		/* LINT */
    while ((ln = Lst_Next (l)) != NILLNODE) {
	s2 = (Suff *) Lst_Datum (ln);
	if (s2->sNum >= s->sNum) {
	    break;
	}
    }

    Lst_Close (l);
    if (DEBUG(SUFF)) {
	printf("inserting %s(%d)...", s->name->data, s->sNum);
    }
    if (ln == NILLNODE) {
	if (DEBUG(SUFF)) {
	    printf("at end of list\n");
	}
	(void)Lst_AtEnd (l, (ClientData)s);
    } else if (s2->sNum != s->sNum) {
	if (DEBUG(SUFF)) {
	    printf("before %s(%d)\n", s2->name->data, s2->sNum);
	}
	(void)Lst_Insert (l, ln, (ClientData)s);
    } else if (DEBUG(SUFF)) {
	printf("already there\n");
    }
}

/*-
 *-----------------------------------------------------------------------
 * Suff_ClearSuffixes --
 *	This is gross. Nuke the list of suffixes but keep all transformation
 *	rules around. The transformation graph is destroyed in this process,
 *	but we leave the list of rules so when a new graph is formed the rules
 *	will remain.
 *	This function is called from the parse module when a
 *	.SUFFIXES:\n line is encountered.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the sufflist and its graph nodes are destroyed
 *-----------------------------------------------------------------------
 */
void
Suff_ClearSuffixes (void)
{
    Lst_Destroy (sufflist, SuffFree);

    sufflist = Lst_Init();
    sNum = 0;
    suffNull = emptySuff;
}

/*-
 *-----------------------------------------------------------------------
 * SuffParseTransform --
 *	Parse a transformation string to find its two component suffixes.
 *
 * Results:
 *	TRUE if the string is a valid transformation and FALSE otherwise.
 *
 * Side Effects:
 *	The passed pointers are overwritten.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
SuffParseTransform(
    string_t   	  	str,	    	/* String being parsed */
    Suff    	  	**srcPtr,   	/* Place to store source of trans. */
    Suff    	  	**targPtr)  	/* Place to store target of trans. */
{
    register LstNode	srcLn;	    /* element in suffix list of trans source*/
    register Suff    	*src;	    /* Source of transformation */
    register LstNode    targLn;	    /* element in suffix list of trans target*/
    LstNode 	    	singleLn;   /* element in suffix list of any suffix
				     * that exactly matches str */
    Suff    	    	*single;    /* Source of possible transformation to
				     * null suffix */

    srcLn = NILLNODE;
    singleLn = NILLNODE;
    single = (Suff *) NULL;	/* LINT */
    
    /*
     * Loop looking first for a suffix that matches the start of the
     * string and then for one that exactly matches the rest of it. If
     * we can find two that meet these criteria, we've successfully
     * parsed the string.
     */
    while (1) {
	if (srcLn == NILLNODE) {
	    srcLn = Lst_Find(sufflist, (ClientData)str, SuffSuffIsPrefix);
	} else {
	    srcLn = Lst_FindFrom (sufflist, Lst_Succ(srcLn), (ClientData)str,
				  SuffSuffIsPrefix);
	}
	if (srcLn == NILLNODE) {
	    /*
	     * Ran out of source suffixes -- no such rule
	     */
	    if (singleLn != NILLNODE) {
		/*
		 * Not so fast Mr. Smith! There was a suffix that encompassed
		 * the entire string, so we assume it was a transformation
		 * to the null suffix (thank you POSIX). We still prefer to
		 * find a double rule over a singleton, hence we leave this
		 * check until the end.
		 *
		 * XXX: Use emptySuff over suffNull?
		 */
		*srcPtr = single;
		*targPtr = suffNull;
		return(TRUE);
	    }
	    return (FALSE);
	}
	src = (Suff *) Lst_Datum (srcLn);
	if (str == src->name) {
	    single = src;
	    singleLn = srcLn;
	} else {
	    targLn = Lst_Find(sufflist, (ClientData)string_suff(str),
			      SuffSuffHasNameP);
	    if (targLn != NILLNODE) {
		*srcPtr = src;
		*targPtr = (Suff *)Lst_Datum(targLn);
		return (TRUE);
	    }
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * Suff_IsTransform  --
 *	Return TRUE if the given string is a transformation rule
 *
 *
 * Results:
 *	TRUE if the string is a concatenation of two known suffixes.
 *	FALSE otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Suff_IsTransform (string_t str)	    	/* string to check */
{
    Suff	*src, *targ;
    string_t	pref;

    if (string_suff(str) == (string_t) NULL ||
	string_head(str) != (string_t) NULL)
	return(FALSE);
    if ((pref = string_pref(str)) != sNULL && *pref->data != '.')
	return(FALSE);
    return (SuffParseTransform(str, &src, &targ));
}

/*-
 *-----------------------------------------------------------------------
 * Suff_AddTransform --
 *	Add the transformation rule described by the line to the
 *	list of rules and place the transformation itself in the graph
 *
 * Results:
 *	The node created for the transformation in the transforms list
 *
 * Side Effects:
 *	The node is placed on the end of the transforms Lst and links are
 *	made between the two suffixes mentioned in the target name
 *-----------------------------------------------------------------------
 */
GNode *
Suff_AddTransform (string_t line)	/* name of transformation to add */
{
    GNode         *gn;		/* GNode of transformation rule */
    Suff          *s,		/* source suffix */
                  *t;		/* target suffix */
    LstNode 	  ln;	    	/* Node for existing transformation */

    ln = Lst_Find (transforms, (ClientData)line, SuffGNHasNameP);
    if (ln == NILLNODE) {
	/*
	 * Make a new graph node for the transformation. It will be filled in
	 * by the Parse module. 
	 */
	gn = Targ_NewGN (line);
	(void)Lst_AtEnd (transforms, (ClientData)gn);
    } else {
	/*
	 * New specification for transformation rule. Just nuke the old list
	 * of commands so they can be filled in again... We don't actually
	 * free the commands themselves, because a given command can be
	 * attached to several different transformations.
	 */
	gn = (GNode *) Lst_Datum (ln);
	Lst_Destroy (gn->commands, NOFREE);
	Lst_Destroy (gn->children, NOFREE);
	gn->commands = Lst_Init();
	gn->children = Lst_Init();
    }

    gn->type = OP_TRANSFORM;

    (void)SuffParseTransform(line, &s, &t);

    /*
     * link the two together in the proper relationship and order 
     */
    if (DEBUG(SUFF)) {
	printf("defining transformation from `%s' to `%s'\n",
		s->name->data, t->name->data);
    }
    SuffInsert (t->children, s);
    SuffInsert (s->parents, t);

    return (gn);
}

/*-
 *-----------------------------------------------------------------------
 * Suff_EndTransform --
 *	Handle the finish of a transformation definition, removing the
 *	transformation from the graph if it has neither commands nor
 *	sources. This is a callback procedure for the Parse module via
 *	Lst_ForEach
 *
 * Results:
 *	=== 0
 *
 * Side Effects:
 *	If the node has no commands or children, the children and parents
 *	lists of the affected suffices are altered.
 *
 *-----------------------------------------------------------------------
 */
int
Suff_EndTransform(ClientData gnCD, ClientData unused)
{
    GNode *gn = (GNode *)gnCD;    	/* Node for transformation */

    if ((gn->type & OP_TRANSFORM) && Lst_IsEmpty(gn->commands) &&
	Lst_IsEmpty(gn->children))
    {
	Suff	*s, *t;
	LstNode	ln;

	(void)SuffParseTransform(gn->name, &s, &t);

	if (DEBUG(SUFF)) {
	    printf("deleting transformation from %s to %s\n",
		    s->name->data, t->name->data);
	}

	/*
	 * Remove the source from the target's children list. We check for a
	 * nil return to handle a beanhead saying something like
	 *  .c.o .c.o:
	 *
	 * We'll be called twice when the next target is seen, but .c and .o
	 * are only linked once...
	 */
	ln = Lst_Member(t->children, (ClientData)s);
	if (ln != NILLNODE) {
	    Lst_Remove(t->children, ln);
	}

	/*
	 * Remove the target from the source's parents list
	 */
	ln = Lst_Member(s->parents, (ClientData)t);
	if (ln != NILLNODE) {
	    Lst_Remove(s->parents, ln);
	}
    } else if ((gn->type & OP_TRANSFORM) && DEBUG(SUFF)) {
	printf("transformation %s complete\n", gn->name->data);
    }

    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * SuffRebuildGraph --
 *	Called from Suff_AddSuffix via Lst_ForEach to search through the
 *	list of existing transformation rules and rebuild the transformation
 *	graph when it has been destroyed by Suff_ClearSuffixes. If the
 *	given rule is a transformation involving this suffix and another,
 *	existing suffix, the proper relationship is established between
 *	the two.
 *
 * Results:
 *	Always 0.
 *
 * Side Effects:
 *	The appropriate links will be made between this suffix and
 *	others if transformation rules exist for it.
 *
 *-----------------------------------------------------------------------
 */
static int
SuffRebuildGraph(
    ClientData	transformCD,	/* Transformation to test */
    ClientData	sCD)		/* Suffix to rebuild */
{
    GNode   	  	*transform = (GNode *)transformCD;
    Suff    	  	*s = (Suff *)sCD;
    register LstNode	ln;
    register Suff  	*s2;

    /*
     * First see if it is a transformation from this suffix.
     */
    if (SuffSuffIsPrefix((ClientData)s, (ClientData)transform->name) == 0) {
	ln = Lst_Find(sufflist, (ClientData)string_suff(transform->name),
		      SuffSuffHasNameP);
	if (ln != NILLNODE) {
	    /*
	     * Found target. Link in and return, since it can't be anything
	     * else.
	     */
	    s2 = (Suff *)Lst_Datum(ln);
	    SuffInsert(s2->children, s);
	    SuffInsert(s->parents, s2);
	    return(0);
	}
    }

    /*
     * Not from, maybe to?
     */
    if (SuffSuffIsSuffix(s, transform->name) == 0) {
	ln = Lst_Find(sufflist, (ClientData)string_pref(transform->name),
		      SuffSuffHasNameP);
	if (ln != NILLNODE) {
	    /*
	     * Found it -- establish the proper relationship
	     */
	    s2 = (Suff *)Lst_Datum(ln);
	    SuffInsert(s->children, s2);
	    SuffInsert(s2->parents, s);
	}
    }
    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * Suff_AddSuffix --
 *	Add the suffix in string to the end of the list of known suffixes.
 *	Should we restructure the suffix graph? Make doesn't...
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	A GNode is created for the suffix and a Suff structure is created and
 *	added to the suffixes list unless the suffix was already known.
 *-----------------------------------------------------------------------
 */
void
Suff_AddSuffix (string_t str)	    /* the name of the suffix to add */
{
    Suff          *s;	    /* new suffix descriptor */
    LstNode 	  ln;

    ln = Lst_Find (sufflist, (ClientData)str, SuffSuffHasNameP);
    if (ln == NILLNODE) {
	s = (Suff *) emalloc (sizeof (Suff));

	s->name =   	string_ref (str);
	s->searchPath = Lst_Init();
	s->children = 	Lst_Init();
	s->parents = 	Lst_Init();
	s->sNum =   	sNum++;
	s->flags =  	0;

	(void)Lst_AtEnd (sufflist, (ClientData)s);
	/*
	 * Look for any existing transformations from or to this suffix.
	 * XXX: Only do this after a Suff_ClearSuffixes?
	 */
	Lst_ForEach (transforms, SuffRebuildGraph, (ClientData)s);
    } 
}

/*-
 *-----------------------------------------------------------------------
 * Suff_GetPath --
 *	Return the search path for the given suffix, if it's defined.
 *
 * Results:
 *	The searchPath for the desired suffix or NILLST if the suffix isn't
 *	defined.
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Lst
Suff_GetPath (string_t sname)
{
    LstNode   	  ln;
    Suff    	  *s;

    ln = Lst_Find (sufflist, (ClientData)sname, SuffSuffHasNameP);
    if (ln == NILLNODE) {
	return (NILLST);
    } else {
	s = (Suff *) Lst_Datum (ln);
	return (s->searchPath);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Suff_DoPaths --
 *	Extend the search paths for all suffixes to include the default
 *	search path.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The searchPath field of all the suffixes is extended by the
 *	directories in dirSearchPath. If paths were specified for the
 *	".h" suffix, the directories are stuffed into a global variable
 *	called ".INCLUDES" with each directory preceeded by a -I. The same
 *	is done for the ".a" suffix, except the variable is called
 *	".LIBS" and the flag is -L.
 *-----------------------------------------------------------------------
 */
void
Suff_DoPaths(void)
{
    register Suff   	*s;
    register LstNode  	ln;
    Lst	    	    	inIncludes; /* Cumulative .INCLUDES path */
    Lst	    	    	inLibs;	    /* Cumulative .LIBS path */
    string_t		str;

    s = suffNull;
    if (!Lst_IsEmpty (s->searchPath)) {
	Dir_Concat(s->searchPath, dirSearchPath);
    } else {
	Lst_Destroy (s->searchPath, SuffDirDestroy);
	s->searchPath = Lst_Duplicate(dirSearchPath, Dir_CopyDir);
    }

    Lst_Open (sufflist);

    inIncludes = Lst_Init();
    inLibs = Lst_Init();

    while ((ln = Lst_Next (sufflist)) != NILLNODE) {
	s = (Suff *) Lst_Datum (ln);
	if (!Lst_IsEmpty (s->searchPath)) {
#ifdef INCLUDES
	    if (s->flags & SUFF_INCLUDE) {
		Dir_Concat(inIncludes, s->searchPath);
	    }
#endif /* INCLUDES */
#ifdef LIBRARIES
	    if (s->flags & SUFF_LIBRARY) {
		Dir_Concat(inLibs, s->searchPath);
	    }
#endif /* LIBRARIES */
	    Dir_Concat(s->searchPath, dirSearchPath);
	} else {
	    Lst_Destroy (s->searchPath, SuffDirDestroy);
	    s->searchPath = Lst_Duplicate(dirSearchPath, Dir_CopyDir);
	}
    }

    str = Dir_MakeFlags(string_create("-I"), inIncludes);
    Var_Set(s_INCLUDES, str, VAR_GLOBAL);
    str = Dir_MakeFlags(string_create("-L"), inLibs);
    Var_Set(s_LIBS, str, VAR_GLOBAL);

    Lst_Destroy(inIncludes, SuffDirDestroy);
    Lst_Destroy(inLibs, SuffDirDestroy);

    Lst_Close (sufflist);
}

/*-
 *-----------------------------------------------------------------------
 * Suff_AddInclude --
 *	Add the given suffix as a type of file which gets included.
 *	Called from the parse module when a .INCLUDES line is parsed.
 *	The suffix must have already been defined.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The SUFF_INCLUDE bit is set in the suffix's flags field
 *
 *-----------------------------------------------------------------------
 */
void
Suff_AddInclude (string_t sname)     /* Name of suffix to mark */
{
    LstNode	  ln;
    Suff	  *s;

    ln = Lst_Find (sufflist, (ClientData)sname, SuffSuffHasNameP);
    if (ln != NILLNODE) {
	s = (Suff *) Lst_Datum (ln);
	s->flags |= SUFF_INCLUDE;
    }
}

/*-
 *-----------------------------------------------------------------------
 * Suff_AddLib --
 *	Add the given suffix as a type of file which is a library.
 *	Called from the parse module when parsing a .LIBS line. The
 *	suffix must have been defined via .SUFFIXES before this is
 *	called.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The SUFF_LIBRARY bit is set in the suffix's flags field
 *
 *-----------------------------------------------------------------------
 */
void
Suff_AddLib (string_t sname)     /* Name of suffix to mark */
{
    LstNode	  ln;
    Suff	  *s;

    ln = Lst_Find (sufflist, (ClientData)sname, SuffSuffHasNameP);
    if (ln != NILLNODE) {
	s = (Suff *) Lst_Datum (ln);
	s->flags |= SUFF_LIBRARY;
    }
}

 	  /********** Implicit Source Search Functions *********/
/*
 * A structure for passing more than one argument to the Lst-library-invoked
 * function...
 */
typedef struct {
    Lst            l;
    Src            *s;
} LstSrc;

/*-
 *-----------------------------------------------------------------------
 * SuffAddSrc  --
 *	Add a suffix as a Src structure to the given list with its parent
 *	being the given Src structure. If the suffix is the null suffix,
 *	the prefix is used unaltered as the file name in the Src structure.
 *
 * Results:
 *	always returns 0
 *
 * Side Effects:
 *	A Src structure is created and tacked onto the end of the list
 *-----------------------------------------------------------------------
 */
static int
SuffAddSrc (
    ClientData	sCD,	    /* suffix for which to create a Src structure */
    ClientData	lsCD)	    /* list and parent for the new Src */
{
    Suff	*s = (Suff *)sCD;
    LstSrc      *ls = (LstSrc *)lsCD;
    Src         *s2;	    /* new Src structure */
    Src    	*targ; 	    /* Target structure */

    targ = ls->s;
    
    if ((s->flags & SUFF_NULL) && (s->name != sNULL)) {
	/*
	 * If the suffix has been marked as the NULL suffix, also create a Src
	 * structure for a file with no suffix attached. Two birds, and all
	 * that...
	 */
	s2 = (Src *) emalloc (sizeof (Src));
	s2->file =  	string_ref(targ->pref);
	s2->pref =  	string_ref(targ->pref);
	s2->parent = 	targ;
	s2->node =  	NILGNODE;
	s2->suff =  	s;
	s2->children =	0;
	targ->children += 1;
	(void)Lst_AtEnd (ls->l, (ClientData)s2);
    }
    s2 = (Src *) emalloc (sizeof (Src));
    s2->file =	    string_concat(targ->pref, s->name, 0);
    s2->pref =	    string_ref(targ->pref);
    s2->parent =    targ;
    s2->node = 	    NILGNODE;
    s2->suff = 	    s;
    s2->children =  0;
    targ->children += 1;
    (void)Lst_AtEnd (ls->l, (ClientData)s2);

    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * SuffAddLevel  --
 *	Add all the children of targ as Src structures to the given list
 *
 * Results:
 *	None
 *
 * Side Effects:
 * 	Lots of structures are created and added to the list
 *-----------------------------------------------------------------------
 */
static void
SuffAddLevel (
    Lst            l,		/* list to which to add the new level */
    Src            *targ)	/* Src structure to use as the parent */
{
    LstSrc         ls;

    ls.s = targ;
    ls.l = l;

    Lst_ForEach (targ->suff->children, SuffAddSrc, (ClientData)&ls);
}

/*-
 *----------------------------------------------------------------------
 * SuffFreeSrc --
 *	Free all memory associated with a Src structure
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The memory is free'd.
 *----------------------------------------------------------------------
 */
static void
SuffFreeSrc (ClientData sCD)
{
    Src *s = (Src *)sCD;

    string_deref (s->file);
    if (!s->parent) {
	string_deref(s->pref);
    } else if (--s->parent->children == 0 && s->parent->parent) {
	/*
	 * Parent has no more children, now we're gone, and it's not
	 * at the top of the tree, so blow it away too.
	 */
	SuffFreeSrc((ClientData)s->parent);
    }
    free ((Address)s);
}

/*-
 *-----------------------------------------------------------------------
 * SuffFindThem --
 *	Find the first existing file/target in the list srcs
 *
 * Results:
 *	The lowest structure in the chain of transformations
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static Src *
SuffFindThem (Lst srcs)	/* list of Src structures to search through */
{
    Src            *s;		/* current Src */
    Src		   *rs;		/* returned Src */

    rs = (Src *) NULL;

    while (!Lst_IsEmpty (srcs)) {
	s = (Src *) Lst_DeQueue (srcs);

	if (DEBUG(SUFF)) {
	    printf ("\ttrying %s...", s->file->data);
	}
	/*
	 * A file is considered to exist if either a node exists in the
	 * graph for it or the file actually exists.
	 */
	if ((Targ_FindNode(s->file, TARG_NOCREATE) != NILGNODE) ||
	    (Dir_FindFile (s->file, s->suff->searchPath) != (string_t) NULL))
	{
	    if (DEBUG(SUFF)) {
		printf ("got it\n");
	    }
	    rs = s;
	    break;
	} else {
	    if (DEBUG(SUFF)) {
		printf ("not there\n");
	    }
	    SuffAddLevel (srcs, s);
	}
    }
    return (rs);
}

/*-
 *-----------------------------------------------------------------------
 * SuffFindCmds --
 *	See if any of the children of the target in the Src structure is
 *	one from which the target can be transformed. If there is one,
 *	a Src structure is put together for it and returned.
 *
 * Results:
 *	The Src structure of the "winning" child, or NIL if no such beast.
 *
 * Side Effects:
 *	A Src structure may be allocated.
 *
 *-----------------------------------------------------------------------
 */
static Src *
SuffFindCmds (Src *targ)	/* Src structure to play with */
{
    LstNode 	  	ln; 	/* General-purpose list node */
    register GNode	*t, 	/* Target GNode */
	    	  	*s; 	/* Source GNode */
    int	    	  	prefLen;/* The length of the defined prefix */
    Suff    	  	*suff;	/* Suffix on matching beastie */
    Src	    	  	*ret;	/* Return value */

    t = targ->node;
    Lst_Open (t->children);
    prefLen = targ->pref->len;

    while ((ln = Lst_Next (t->children)) != NILLNODE) {
	s = (GNode *)Lst_Datum (ln);

	if (string_pref(s->name) == targ->pref) {
	    /*
	     * The node matches the prefix ok, see if it has a known
	     * suffix.
	     */
	    ln = Lst_Find (sufflist, (ClientData)targ->pref, SuffSuffHasNameP);
	    if (ln != NILLNODE) {
		/*
		 * It even has a known suffix, see if there's a transformation
		 * defined between the node's suffix and the target's suffix.
		 *
		 * XXX: Handle multi-stage transformations here, too.
		 */
		suff = (Suff *)Lst_Datum (ln);

		if (Lst_Member (suff->parents,
				(ClientData)targ->suff) != NILLNODE)
		{
		    /*
		     * Hot Damn! Create a new Src structure to describe
		     * this transformation (making sure to duplicate the
		     * source node's name so Suff_FindDeps can free it
		     * again (ick)), and return the new structure.
		     */
		    ret = (Src *)emalloc (sizeof(Src));
		    ret->file = string_ref(s->name);
		    ret->pref = string_ref(targ->pref);
		    ret->suff = suff;
		    ret->parent = targ;
		    ret->node = s;
		    ret->children = 0;
		    targ->children += 1;
		    if (DEBUG(SUFF)) {
			printf ("\tusing existing source %s\n", s->name->data);
		    }
		    return (ret);
		}
	    }
	}
    }
    Lst_Close (t->children);
    return ((Src *)NULL);
}

/*-
 *-----------------------------------------------------------------------
 * SuffExpandChildren --
 *	Expand the names of any children of a given node that contain
 *	variable invocations or file wildcards into actual targets.
 *
 * Results:
 *	=== 0 (continue)
 *
 * Side Effects:
 *	The expanded node is removed from the parent's list of children,
 *	and the parent's unmade counter is decremented, but other nodes
 * 	may be added.
 *
 *-----------------------------------------------------------------------
 */
static int
SuffExpandChildren(
    ClientData	cgnCD,	    /* Child to examine */
    ClientData	pgnCD)	    /* Parent node being processed */
{
    GNode   	*cgn = (GNode *)cgnCD;
    GNode   	*pgn = (GNode *)pgnCD;
    GNode	*gn;	    /* New source 8) */
    LstNode   	prevLN;     /* Node after which new source should be put */
    LstNode	ln; 	    /* List element for old source */
    char	*cp;	    /* Expanded value */
    Lst		members;

    /*
     * First do variable expansion -- this takes precedence over
     * wildcard expansion. If the result contains wildcards, they'll be gotten
     * to later since the resulting words are tacked on to the end of
     * the children list.
     */
    if (DEBUG(SUFF)) {
	printf("Expanding \"%s\"...", cgn->name->data);
    }
    if (!string_hasvar(cgn->name))
	return(0);
    cp = Var_Subst(cgn->name->data, pgn, TRUE);

    members = Lst_Init();
    prevLN = NILLNODE;		/* LINT */

    if (cgn->type & OP_ARCHV) {
	/*
	 * Node was an archive(member) target, so we want to call
	 * on the Arch module to find the nodes for us, expanding
	 * variables in the parent's context.
	 */
	char	*sacrifice = cp;

	(void)Arch_ParseArchive(&sacrifice, members, pgn);
    } else {
	/*
	 * Break the result into a vector of strings whose nodes
	 * we can find, then add those nodes to the members list.
	 * Unfortunately, we can't use brk_string b/c it
	 * doesn't understand about variable specifications with
	 * spaces in them...
	 */
	char	    *start;
	char	    *initcp = cp;   /* For freeing... */

	for (start = cp; *start == ' ' || *start == '\t'; start++) {
	    ;
	}
	for (cp = start; *cp != '\0'; cp++) {
	    if (*cp == ' ' || *cp == '\t') {
		/*
		 * White-space -- terminate element, find the node,
		 * add it, skip any further spaces.
		 */
		*cp++ = '\0';
		gn = Targ_FindNode(string_create(start), TARG_CREATE);
		(void)Lst_AtEnd(members, (ClientData)gn);
		while (*cp == ' ' || *cp == '\t') {
		    cp++;
		}
		/*
		 * Adjust cp for increment at start of loop, but
		 * set start to first non-space.
		 */
		start = cp--;
	    } else if (*cp == '$') {
		/*
		 * Start of a variable spec -- contact variable module
		 * to find the end so we can skip over it.
		 */
		cp = Var_Skip(cp, pgn, TRUE);
		cp--;
	    } else if (*cp == '\\') {
		/*
		 * Escaped something -- skip over it
		 */
		cp++;
	    }
	}

	if (cp != start) {
	    /*
	     * Stuff left over -- add it to the list too
	     */
	    gn = Targ_FindNode(string_create(start), TARG_CREATE);
	    (void)Lst_AtEnd(members, (ClientData)gn);
	}
	/*
	 * Point cp back at the beginning again so the variable value
	 * can be freed.
	 */
	cp = initcp;
    }

    /*
     * New nodes effectively take the place of the child, so place them
     * after the child
     */
    if (!Lst_IsEmpty(members))
	prevLN = Lst_Member(pgn->children, (ClientData)cgn);

    /*
     * Add all elements of the members list to the parent node.
     */
    while(!Lst_IsEmpty(members)) {
	gn = (GNode *)Lst_DeQueue(members);

	if (DEBUG(SUFF)) {
	    printf("%s...", gn->name->data);
	}
	if (Lst_Member(pgn->children, (ClientData)gn) == NILLNODE) {
	    (void)Lst_Append(pgn->children, prevLN, (ClientData)gn);
	    prevLN = Lst_Succ(prevLN);
	    (void)Lst_AtEnd(gn->parents, (ClientData)pgn);
	    pgn->unmade++;
	}
    }
    Lst_Destroy(members, NOFREE);

    /*
     * Free the result
     */
    free((char *)cp);

    /*
     * Now the source is expanded, remove it from the list of children to
     * keep it from being processed.
     */
    ln = Lst_Member(pgn->children, (ClientData)cgn);
    pgn->unmade--;
    Lst_Remove(pgn->children, ln);
    if (DEBUG(SUFF)) {
	printf("\n");
    }

    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * SuffApplyTransform --
 *	Apply a transformation rule, given the source and target nodes
 *	and suffixes.
 *
 * Results:
 *	TRUE if successful, FALSE if not.
 *
 * Side Effects:
 *	The source and target are linked and the commands from the
 *	transformation are added to the target node's commands list.
 *	All attributes but OP_DEPMASK and OP_TRANSFORM are applied
 *	to the target. The target also inherits all the sources for
 *	the transformation rule.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
SuffApplyTransform(
    GNode   	*tGn,	    /* Target node */
    GNode   	*sGn,	    /* Source node */
    Suff    	*t, 	    /* Target suffix */
    Suff    	*s) 	    /* Source suffix */
{
    LstNode 	ln; 	    /* General node */
    string_t   	tname;	    /* Name of transformation rule */
    GNode   	*gn;	    /* Node for same */

    if (Lst_Member(tGn->children, (ClientData)sGn) == NILLNODE) {
	/*
	 * Not already linked, so form the proper links between the
	 * target and source.
	 */
	(void)Lst_AtEnd(tGn->children, (ClientData)sGn);
	(void)Lst_AtEnd(sGn->parents, (ClientData)tGn);
	tGn->unmade += 1;
    }

    if ((sGn->type & OP_OPMASK) == OP_DOUBLEDEP) {
	/*
	 * When a :: node is used as the implied source of a node, we have
	 * to link all its cohorts in as sources as well. Only the initial
	 * sGn gets the target in its iParents list, however, as that
	 * will be sufficient to get the .IMPSRC variable set for tGn
	 */
	for (ln=Lst_First(sGn->cohorts); ln != NILLNODE; ln=Lst_Succ(ln)) {
	    gn = (GNode *)Lst_Datum(ln);

	    if (Lst_Member(tGn->children, (ClientData)gn) == NILLNODE) {
		/*
		 * Not already linked, so form the proper links between the
		 * target and source.
		 */
		(void)Lst_AtEnd(tGn->children, (ClientData)gn);
		(void)Lst_AtEnd(gn->parents, (ClientData)tGn);
		tGn->unmade += 1;
	    }
	}
    }
    /*
     * Locate the transformation rule itself
     */
    tname = string_concat(s->name, t->name, 0);
    ln = Lst_Find(transforms, (ClientData)tname,
		  SuffGNHasNameP);

    if (ln == NILLNODE) {
	/*
	 * Not really such a transformation rule (can happen when we're
	 * called to link an OP_MEMBER and OP_ARCHV node), so return
	 * FALSE.
	 */
	return(FALSE);
    }

    gn = (GNode *)Lst_Datum(ln);
    
    if (DEBUG(SUFF)) {
	printf("\tapplying %s -> %s to \"%s\"\n",
	       s->name->data, t->name->data, tGn->name->data);
    }

    /*
     * Record last child for expansion purposes
     */
    ln = Lst_Last(tGn->children);
    
    /*
     * Pass the buck to Make_HandleUse to apply the rule
     */
    (void)Make_HandleUse(gn, tGn);

    /*
     * Deal with wildcards and variables in any acquired sources
     */
    ln = Lst_Succ(ln);
    if (ln != NILLNODE) {
	Lst_ForEachFrom(tGn->children, ln,
			SuffExpandChildren, (ClientData)tGn);
    }

    /*
     * Keep track of another parent to which this beast is transformed so
     * the .IMPSRC variable can be set correctly for the parent.
     */
    (void)Lst_AtEnd(sGn->iParents, (ClientData)tGn);

    return(TRUE);
}


/*-
 *-----------------------------------------------------------------------
 * SuffFindArchiveDeps --
 *	Locate dependencies for an OP_ARCHV node.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Same as Suff_FindDeps
 *
 *-----------------------------------------------------------------------
 */
static void
SuffFindArchiveDeps(GNode *gn)	    /* Node for which to locate dependencies */
{
    GNode   	*mem;	    /* Node for member */
    Suff    	*ms;	    /* Suffix descriptor for member */
    string_t   	archname;   /* archive's name */
    string_t   	membname;   /* member's name */
    
    /*
     * The node is an archive(member) pair. so we must find a
     * suffix for both of them.
     */
    archname = string_arch(gn->name);
    membname = string_memb(gn->name);

    if (DEBUG(SUFF))
	printf("SuffFindArchiveDeps: gn %s arch %s memb %s\n",
	       gn->name->data, archname->data, membname->data);
    
    /*
     * To simplify things, call Suff_FindDeps recursively on the member now,
     * so we can simply compare the member's .PREFIX and .TARGET variables
     * to locate its suffix. This allows us to figure out the suffix to
     * use for the archive without having to do a quadratic search over the
     * suffix list, backtracking for each one...
     */
    mem = Targ_FindNode(membname, TARG_CREATE);
   /*
    * FIXME: the following line is a temporary fix. ODE CR 73.
    */
    mem->suffix=suffNull;
    Suff_FindDeps(mem);

    /*
     * Create the link between the two nodes right off
     */
    if (Lst_Member(gn->children, (ClientData)mem) == NILLNODE) {
	(void)Lst_AtEnd(gn->children, (ClientData)mem);
	(void)Lst_AtEnd(mem->parents, (ClientData)gn);
	gn->unmade += 1;
    }
    
    /*
     * Copy in the variables from the member node to this one.
     */
    Var_Set(sPREFIX, Var_StrValue(sPREFIX, mem), gn);
    Var_Set(sTARGET, Var_StrValue(sTARGET, mem), gn);

    ms = mem->suffix;
    if (ms == NULL) {
	/*
	 * Didn't know what it was -- use .NULL suffix if not in make mode
	 */
	if (DEBUG(SUFF)) {
	    printf("using null suffix\n");
	}
	ms = suffNull;
    }


    /*
     * Set the other two local variables required for this target.
     */
    Var_Set (sMEMBER, membname, gn);
    Var_Set (sARCHIVE, archname, gn);

    if (ms != NULL) {
	/*
	 * Member has a known suffix, so look for a transformation rule from
	 * it to a possible suffix of the archive. Rather than searching
	 * through the entire list, we just look at suffixes to which the
	 * member's suffix may be transformed...
	 */
	LstNode	    ln;

	/*
	 * Use first matching suffix...
	 */
	ln = Lst_Find(ms->parents, (ClientData)archname, SuffSuffIsSuffix);

	if (ln != NILLNODE) {
	    /*
	     * Got one -- apply it
	     */
	    if (!SuffApplyTransform(gn, mem, (Suff *)Lst_Datum(ln), ms) &&
		DEBUG(SUFF))
	    {
		printf("\tNo transformation from %s -> %s\n",
		       ms->name->data, ((Suff *)Lst_Datum(ln))->name->data);
	    }
	}
    }

    /*
     * Pretend gn appeared to the left of a dependency operator so
     * the user needn't provide a transformation from the member to the
     * archive.
     */
    if (OP_NOP(gn->type)) {
	gn->type |= OP_DEPENDS;
    }

    /*
     * Flag the member as such so we remember to look in the archive for
     * its modification time.
     */
    mem->type |= OP_MEMBER;
}

/*-
 *-----------------------------------------------------------------------
 * SuffFindNormalDeps --
 *	Locate implicit dependencies for regular targets.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Same as Suff_FindDeps...
 *
 *-----------------------------------------------------------------------
 */
static void
SuffFindNormalDeps(GNode *gn)	    /* Node for which to find sources */
{
    LstNode 	ln; 	    /* Next suffix node to check */
    Lst	    	srcs;	    /* List of sources at which to look */
    Lst	    	targs;	    /* List of targets to which things can be
			     * transformed. They all have the same file,
			     * but different suff and pref fields */
    Src	    	*bottom;    /* Start of found transformation path */
    Src 	*src;	    /* General Src pointer */
    char    	*pref;	    /* Prefix to use */
    Src	    	*targ;	    /* General Src target pointer */

    /*
     * Begin at the beginning...
     */
    ln = Lst_First(sufflist);
    srcs = Lst_Init();
    targs = Lst_Init();

    /*
     * We're caught in a catch-22 here. On the one hand, we want to use any
     * transformation implied by the target's sources, but we can't examine
     * the sources until we've expanded any variables/wildcards they may hold,
     * and we can't do that until we've set up the target's local variables
     * and we can't do that until we know what the proper suffix for the
     * target is (in case there are two suffixes one of which is a suffix of
     * the other) and we can't know that until we've found its implied
     * source, which we may not want to use if there's an existing source
     * that implies a different transformation.
     *
     * In an attempt to get around this, which may not work all the time,
     * but should work most of the time, we look for implied sources first,
     * checking transformations to all possible suffixes of the target,
     * use what we find to set the target's local variables, expand the
     * children, then look for any overriding transformations they imply.
     * Should we find one, we discard the one we found before.
     */
    while(ln != NILLNODE) {
	/*
	 * Look for next possible suffix...
	 */
	ln = Lst_FindFrom(sufflist, ln, (ClientData)gn->name, SuffSuffIsSuffix);

	if (ln != NILLNODE) {
	    int	    prefLen;	    /* Length of the prefix */
	    
	    /*
	     * Allocate a Src structure to which things can be transformed
	     */
	    targ = (Src *)emalloc(sizeof(Src));
	    targ->file = string_ref(gn->name);
	    targ->suff = (Suff *)Lst_Datum(ln);
	    targ->node = gn;
	    targ->parent = (Src *)NULL;
	    
	    /*
	     * Allocate room for the prefix, whose end is found by subtracting
	     * the length of the suffix from the end of the name.
	     */
	    prefLen = gn->name->len - targ->suff->name->len;
	    pref = strndup(gn->name->data, prefLen);
	    targ->pref = string_create(pref);
	    free(pref);

	    /*
	     * Add nodes from which the target can be made
	     */
	    SuffAddLevel(srcs, targ);

	    /*
	     * Record the target so we can nuke it
	     */
	    (void)Lst_AtEnd(targs, (ClientData)targ);

	    /*
	     * Search from this suffix's successor...
	     */
	    ln = Lst_Succ(ln);
	}
    }

    /*
     * Handle target of unknown suffix...
     */
    if (Lst_IsEmpty(targs) && suffNull != NULL) {
	if (DEBUG(SUFF)) {
	    printf("\tNo known suffix on %s. Using .NULL suffix\n",
		   gn->name->data);
	}
	
	targ = (Src *)emalloc(sizeof(Src));
	targ->file = string_ref(gn->name);
	targ->suff = suffNull;
	targ->node = gn;
	targ->parent = (Src *)NULL;
	targ->pref = string_ref(gn->name);

	SuffAddLevel(srcs, targ);
	(void)Lst_AtEnd(targs, (ClientData)targ);
    }
    
    /*
     * Using the list of possible sources built up from the target suffix(es),
     * try and find an existing file/target that matches.
     */
    bottom = SuffFindThem(srcs);

    if (bottom == (Src *)NULL) {
	/*
	 * No known transformations -- use the first suffix found for setting
	 * the local variables.
	 */
	if (!Lst_IsEmpty(targs)) {
	    targ = (Src *)Lst_Datum(Lst_First(targs));
	} else {
	    targ = (Src *)NULL;
	}
    } else {
	/*
	 * Work up the transformation path to find the suffix of the
	 * target to which the transformation was made.
	 */
	for (targ = bottom; targ->parent != NULL; targ = targ->parent) {
	    ;
	}
    }

    /*
     * The .TARGET variable we always set to be the name at this point,
     * since it's only set to the path if the thing is only a source and
     * if it's only a source, it doesn't matter what we put here as far
     * as expanding sources is concerned, since it has none...
     *
     * Ummm... since this information propagates up to our parents and
     * their .IMPSRC and .ALLSRC variables, this info should be correct.
     */
    Var_Set(sTARGET, (gn->path != NULL) ? gn->path : gn->name, gn);
    Var_Set(sPREFIX, (targ != NULL) ? targ->pref : gn->name, gn);

    /*
     * Now we've got the important local variables set, expand any sources
     * that still contain variables or wildcards in their names.
     */
    Lst_ForEach(gn->children, SuffExpandChildren, (ClientData)gn);
    
    if (targ == NULL) {
	if (DEBUG(SUFF)) {
	    printf("\tNo valid suffix on %s\n", gn->name->data);
	}

sfnd_abort:
	/*
	 * Deal with finding the thing on the default search path if the
	 * node is only a source (not on the lhs of a dependency operator
	 * or [XXX] it has neither children or commands).
	 */
	if (OP_NOP(gn->type) ||
	    (Lst_IsEmpty(gn->children) && Lst_IsEmpty(gn->commands)))
	{
	    gn->path = Dir_FindFileOrLink(gn->name,
					  (targ == NULL ? dirSearchPath :
					   targ->suff->searchPath),
					  gn->type & OP_LINK);
	    if (gn->path != NULL) {
		Var_Set(sTARGET, gn->path, gn);

		if (targ != NULL) {
		    /*
		     * Suffix known for the thing -- trim the suffix off
		     * the path to form the proper .PREFIX variable.
		     */
		    char	*temp;

		    gn->suffix = targ->suff;

		    temp = strndup(gn->path->data,
				   gn->path->len - targ->suff->name->len);
		    Var_Set(sPREFIX, string_create(temp), gn);
		    free(temp);
		} else {
		    /*
		     * The .PREFIX gets the full path if the target has
		     * no known suffix.
		     */
		    gn->suffix = NULL;

		    Var_Set(sPREFIX, gn->path, gn);
		}
	    }
	} else {
	    /*
	     * Not appropriate to search for the thing -- set the
	     * path to be the name so Dir_MTime won't go grovelling for
	     * it.
	     */
	    gn->suffix = (targ == NULL) ? NULL : targ->suff;
	    gn->path = gn->name;
	}
	
	goto sfnd_return;
    }

    /*
     * If the suffix indicates that the target is a library, mark that in
     * the node's type field.
     */
    if (targ->suff->flags & SUFF_LIBRARY) {
	gn->type |= OP_LIB;
    }

    /*
     * Check for overriding transformation rule implied by sources
     */
    if (!Lst_IsEmpty(gn->children)) {
	src = SuffFindCmds(targ);

	if (src != (Src *)NULL) {
	    /*
	     * Free up all the Src structures in the transformation path
	     * up to, but not including, the parent node.
	     */
	    while (bottom && bottom->parent != NULL) {
		Src *p = bottom->parent;

		SuffFreeSrc((ClientData)bottom);
		bottom = p;
	    }
	    bottom = src;
	}
    }

    if (bottom == NULL) {
	/*
	 * No idea from where it can come -- return now.
	 */
	goto sfnd_abort;
    }

    /*
     * We now have a list of Src structures headed by 'bottom' and linked via
     * their 'parent' pointers. What we do next is create links between
     * source and target nodes (which may or may not have been created)
     * and set the necessary local variables in each target. The
     * commands for each target are set from the commands of the
     * transformation rule used to get from the src suffix to the targ
     * suffix. Note that this causes the commands list of the original
     * node, gn, to be replaced by the commands of the final
     * transformation rule. Also, the unmade field of gn is incremented.
     * Etc. 
     */
    if (bottom->node == NILGNODE) {
	bottom->node = Targ_FindNode(bottom->file, TARG_CREATE);
    }
    
    for (src = bottom; src->parent != (Src *)NULL; src = src->parent) {
	targ = src->parent;

	src->node->suffix = src->suff;

	if (targ->node == NILGNODE) {
	    targ->node = Targ_FindNode(targ->file, TARG_CREATE);
	}

	SuffApplyTransform(targ->node, src->node,
			   targ->suff, src->suff);

	if (targ->node != gn) {
	    /*
	     * Finish off the dependency-search process for any nodes
	     * between bottom and gn (no point in questing around the
	     * filesystem for their implicit source when it's already
	     * known). Note that the node can't have any sources that
	     * need expanding, since SuffFindThem will stop on an existing
	     * node, so all we need to do is set the standard and System V
	     * variables.
	     */
	    targ->node->type |= OP_DEPS_FOUND;

	    Var_Set(sPREFIX, targ->pref, targ->node);
	    Var_Set(sTARGET, (targ->node->path != NULL) ?
		    targ->node->path : targ->node->name,
		    targ->node);
	}
    }

    gn->suffix = src->suff;

    /*
     * So Dir_MTime doesn't go questing for it...
     */
    gn->path = gn->name;

    /*
     * Nuke the transformation path and the Src structures left over in the
     * two lists.
     */
    SuffFreeSrc((ClientData)bottom);

sfnd_return:
    Lst_Destroy(srcs, SuffFreeSrc);
    Lst_Destroy(targs, SuffFreeSrc);

}
	
    


/*-
 *-----------------------------------------------------------------------
 * Suff_FindDeps  --
 *	Find implicit sources for the target described by the graph node
 *	gn
 *
 * Results:
 *	Nothing.
 *
 * Side Effects:
 *	Nodes are added to the graph below the passed-in node. The nodes
 *	are marked to have their IMPSRC variable filled in. The
 *	PREFIX variable is set for the given node and all its
 *	implied children.
 *
 * Notes:
 *	The path found by this target is the shortest path in the
 *	transformation graph, which may pass through non-existent targets,
 *	to an existing target. The search continues on all paths from the
 *	root suffix until a file is found. I.e. if there's a path
 *	.o -> .c -> .l -> .l,v from the root and the .l,v file exists but
 *	the .c and .l files don't, the search will branch out in
 *	all directions from .o and again from all the nodes on the
 *	next level until the .l,v node is encountered.
 *
 *-----------------------------------------------------------------------
 */
void
Suff_FindDeps (GNode *gn)	      	/* node we're dealing with */
{
    if (gn->type & OP_DEPS_FOUND) {
	/*
	 * If dependencies already found, no need to do it again...
	 */
	return;
    } else {
	gn->type |= OP_DEPS_FOUND;
    }
    
    if (DEBUG(SUFF)) {
	printf ("Suff_FindDeps (%s)\n", gn->name->data);
    }
    
    if (gn->type & OP_ARCHV) {
	SuffFindArchiveDeps(gn);
    } else if (gn->type & OP_LIB) {
	/*
	 * If the node is a library, it is the arch module's job to find it
	 * and set the TARGET variable accordingly. We merely provide the
	 * search path, assuming all libraries end in ".a" (if the suffix
	 * hasn't been defined, there's nothing we can do for it, so we just
	 * set the TARGET variable to the node's name in order to give it a
	 * value).
	 */
	LstNode	ln;
	Suff	*s;
	
	ln = Lst_Find (sufflist, (ClientData)sLIBSUFF, SuffSuffHasNameP);
	if (ln != NILLNODE) {
	    gn->suffix = s = (Suff *) Lst_Datum (ln);
	    Arch_FindLib (gn, s->searchPath);
	} else {
	    gn->suffix = NULL;
	    Var_Set (sTARGET, gn->name, gn);
	}
	/*
	 * Because a library (-lfoo) target doesn't follow the standard
	 * filesystem conventions, we don't set the regular variables for
	 * the thing. .PREFIX is simply made empty...
	 */
	Var_Set(sPREFIX, sNULL, gn);
    } else {
	SuffFindNormalDeps(gn);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Suff_SetNull --
 *	Define which suffix is the null suffix.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	'suffNull' is altered.
 *
 * Notes:
 *	Need to handle the changing of the null suffix gracefully so the
 *	old transformation rules don't just go away.
 *
 *-----------------------------------------------------------------------
 */
void
Suff_SetNull(string_t name)	    /* Name of null suffix */
{
    Suff    *s;
    LstNode ln;

    ln = Lst_Find(sufflist, (ClientData)name, SuffSuffHasNameP);
    if (ln != NILLNODE) {
	s = (Suff *)Lst_Datum(ln);
	if (suffNull != (Suff *)NULL) {
	    suffNull->flags &= ~SUFF_NULL;
	}
	s->flags |= SUFF_NULL;
	/*
	 * XXX: Here's where the transformation mangling would take place
	 */
	suffNull = s;
    } else {
	Parse_Error (PARSE_WARNING, "Desired null suffix %s not defined.",
		     name->data);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Suff_Init --
 *	Initialize suffixes module
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Many
 *-----------------------------------------------------------------------
 */
void
Suff_Init (void)
{
    sufflist = Lst_Init();
    transforms = Lst_Init();

    sNum = 0;
    /*
     * Create null suffix for single-suffix rules (POSIX). The thing doesn't
     * actually go on the suffix list or everyone will think that's its
     * suffix.
     */
    emptySuff = suffNull = (Suff *) emalloc (sizeof (Suff));

    suffNull->name =   	    string_ref(sNULL);
    suffNull->searchPath =  Lst_Init();
    suffNull->children =    Lst_Init();
    suffNull->parents =	    Lst_Init();
    suffNull->sNum =   	    sNum++;
    suffNull->flags =  	    SUFF_NULL;

}

/********************* DEBUGGING FUNCTIONS **********************/

static int
SuffPrintName(ClientData s, ClientData unused) {
    printf ("%s ", ((Suff *)s)->name->data);
    return (0);
}

static int
SuffPrintSuff (ClientData sCD, ClientData unused)
{
    Suff    *s = (Suff *)sCD;
    int	    flags;
    int	    flag;

    printf ("# `%s'", s->name->data);
    
    flags = s->flags;
    if (flags) {
	fputs (" (", stdout);
	while (flags) {
	    flag = 1 << (ffs(flags) - 1);
	    flags &= ~flag;
	    switch (flag) {
		case SUFF_NULL:
		    printf ("NULL");
		    break;
		case SUFF_INCLUDE:
		    printf ("INCLUDE");
		    break;
		case SUFF_LIBRARY:
		    printf ("LIBRARY");
		    break;
	    }
	    putc(flags ? '|' : ')', stdout);
	}
    }
    putc ('\n', stdout);
    printf ("#\tTo: ");
    Lst_ForEach (s->parents, SuffPrintName, (ClientData)0);
    putc ('\n', stdout);
    printf ("#\tFrom: ");
    Lst_ForEach (s->children, SuffPrintName, (ClientData)0);
    putc ('\n', stdout);
    printf ("#\tSearch Path: ");
    Dir_PrintPath (s->searchPath);
    putc ('\n', stdout);
    return (0);
}

static int
SuffPrintTrans (ClientData tCD, ClientData unused)
{
    GNode *t = (GNode *)tCD;

    printf ("%-16s: ", t->name->data);
    Targ_PrintType (t->type);
    putc ('\n', stdout);
    Lst_ForEach (t->commands, Targ_PrintCmd, (ClientData)0);
    putc ('\n', stdout);
    return(0);
}

void
Suff_PrintAll(void)
{
    printf ("#*** Suffixes:\n");
    Lst_ForEach (sufflist, SuffPrintSuff, (ClientData)0);

    printf ("#*** Transformations:\n");
    Lst_ForEach (transforms, SuffPrintTrans, (ClientData)0);
}

