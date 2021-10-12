/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: TargPrintName
 *		TargPrintNode
 *		TargPrintOnlySrc
 *		Targ_FindList
 *		Targ_FindNode
 *		Targ_FmtTime
 *		Targ_Ignore
 *		Targ_Init
 *		Targ_NewGN
 *		Targ_Precious
 *		Targ_PrintCmd
 *		Targ_PrintGraph
 *		Targ_PrintType
 *		Targ_SetMain
 *		Targ_Silent
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
 * $Log: targ.c,v $
 * Revision 1.2.2.3  1992/12/03  19:07:28  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:42  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:27:43  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:55:56  gm]
 * 
 * Revision 1.2  1991/12/05  20:45:23  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  17:19:49  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:10:42  mckeen]
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
static char sccsid[] = "@(#)54  1.10  src/bldenv/make/targ.c, bldprocess, bos412, 9443A412a 10/28/94 14:10:20";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)targ.c	5.9 (Berkeley) 3/1/91";
#endif /* not lint */

/*-
 * targ.c --
 *	Functions for maintaining the Lst allTargets. Target nodes are
 * kept in two structures: a Lst, maintained by the list library, and a
 * hash table, maintained by the hash library.
 *
 * Interface:
 *	Targ_Init 	    	Initialization procedure.
 *
 *	Targ_NewGN	    	Create a new GNode for the passed target
 *	    	  	    	(string). The node is *not* placed in the
 *	    	  	    	hash table, though all its fields are
 *	    	  	    	initialized.
 *
 *	Targ_FindNode	    	Find the node for a given target, creating
 *	    	  	    	and storing it if it doesn't exist and the
 *	    	  	    	flags are right (TARG_CREATE)
 *
 *	Targ_FindList	    	Given a list of names, find nodes for all
 *	    	  	    	of them. If a name doesn't exist and the
 *	    	  	    	TARG_NOCREATE flag was given, an error message
 *	    	  	    	is printed. Else, if a name doesn't exist,
 *	    	  	    	its node is created.
 *
 *	Targ_Ignore	    	Return TRUE if errors should be ignored when
 *	    	  	    	creating the given target.
 *
 *	Targ_Silent	    	Return TRUE if we should be silent when
 *	    	  	    	creating the given target.
 *
 *	Targ_Precious	    	Return TRUE if the target is precious and
 *	    	  	    	should not be removed if we are interrupted.
 *
 * Debugging:
 *	Targ_PrintGraph	    	Print out the entire graphm all variables
 *	    	  	    	and statistics for the directory cache. Should
 *	    	  	    	print something for suffixes, too, but...
 */

#include	  <stdio.h>
#include	  <time.h>
#include	  "make.h"
#include	  "hash.h"
#include	  "sprite.h"
#include	  "lstInt.h"

static Lst        allTargets;	/* the list of all targets found so far */
static Hash_Table targets;	/* a hash table of same */

#define HTSIZE	191		/* initial size of hash table */

/*-
 *-----------------------------------------------------------------------
 * Targ_Init --
 *	Initialize this module
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The allTargets list and the targets hash table are initialized
 *-----------------------------------------------------------------------
 */
void
Targ_Init (void)
{
    allTargets = Lst_Init();
    Hash_InitTable (&targets, HTSIZE);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_NewGN  --
 *	Create and initialize a new graph node
 *
 * Results:
 *	An initialized graph node with the name field filled with a copy
 *	of the passed name
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */
GNode *
Targ_NewGN (string_t name)	/* the name to stick in the new node */
{
    register GNode *gn;
    extern string_t fname;	/* from parse.c */

    gn = (GNode *) emalloc (sizeof (GNode));
    gn->name = string_ref(name);
    gn->path = (string_t) 0;
    if (fname != NULL)
	gn->makefilename = string_ref(fname);
    else
	gn->makefilename = NULL;
    if (name->data[0] == '-' && name->data[1] == 'l') {
	gn->type = OP_LIB;
    } else {
	gn->type = 0;
    }
    gn->unmade =    	0;
    gn->make = 	    	FALSE;
    gn->made = 	    	UNMADE;
    gn->childMade = 	FALSE;
    gn->mtime = gn->cmtime = 0;
    gn->iParents =  	Lst_Init();
    gn->cohorts =   	Lst_Init();
    gn->parents =   	Lst_Init();
    gn->children =  	Lst_Init();
    Hash_InitTable(&gn->childHT, 0);
    gn->successors = 	Lst_Init();
    gn->preds =     	Lst_Init();
    gn->context =   	Lst_Init();
    gn->contextExtras = 0;
    gn->commands =  	Lst_Init();

    return (gn);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_FindNode  --
 *	Find a node in the list using the given name for matching
 *
 * Results:
 *	The node in the list if it was. If it wasn't, return NILGNODE of
 *	flags was TARG_NOCREATE or the newly created and initialized node
 *	if it was TARG_CREATE
 *
 * Side Effects:
 *	Sometimes a node is created and added to the list
 *-----------------------------------------------------------------------
 */
GNode *
Targ_FindNode (
    string_t        name,	/* the name to find */
    int             flags)	/* flags governing events when target not
				 * found */
{
    GNode         *gn;	      /* node in that element */
    Hash_Entry	  *he;	      /* New or used hash entry for node */
    Boolean	  isNew;      /* Set TRUE if Hash_CreateEntry had to create */
			      /* an entry for the node */

    if (DEBUG(TARG)) {
	printf("Targ_FindNode(name %s)\n", name->data);
    }
    if (flags & TARG_CREATE) {
	he = Hash_CreateEntry (&targets, name, &isNew);
	if (isNew) {
	    gn = Targ_NewGN (name);
	    Hash_SetValue (he, gn);
	    (void) Lst_AtEnd (allTargets, (ClientData)gn);
	}
    } else {
	he = Hash_FindEntry (&targets, name);
    }

    if (he == (Hash_Entry *) NULL) {
	return (NILGNODE);
    } else {
	return ((GNode *) Hash_GetValue (he));
    }
}

/*-
 *-----------------------------------------------------------------------
 * Targ_FindList --
 *	Make a complete list of GNodes from the given list of names 
 *
 * Results:
 *	A complete list of graph nodes corresponding to all instances of all
 *	the names in names. 
 *
 * Side Effects:
 *	If flags is TARG_CREATE, nodes will be created for all names in
 *	names which do not yet have graph nodes. If flags is TARG_NOCREATE,
 *	an error message will be printed for each name which can't be found.
 * -----------------------------------------------------------------------
 */
Lst
Targ_FindList (
    Lst        	   names,	/* list of names to find */
    int            flags)	/* flags used if no node is found for a given
				 * name */
{
    Lst            nodes;	/* result list */
    register LstNode  ln;		/* name list element */
    register GNode *gn;		/* node in tLn */
    string_t   	   name;

    nodes = Lst_Init();

    Lst_Open (names);
    while ((ln = Lst_Next (names)) != NILLNODE) {
	name = (string_t)Lst_Datum(ln);
	gn = Targ_FindNode (name, flags);
	if (gn != NILGNODE) {
	    /*
	     * Note: Lst_AtEnd must come before the Lst_Concat so the nodes
	     * are added to the list in the order in which they were
	     * encountered in the makefile.
	     */
	    (void) Lst_AtEnd (nodes, (ClientData)gn);
	    if (gn->type & OP_DOUBLEDEP) {
		(void)Lst_Concat (nodes, gn->cohorts, LST_CONCNEW);
	    }
	} else if (flags == TARG_NOCREATE) {
	    Error ("\"%s\" -- target unknown.", name->data);
	}
    }
    Lst_Close (names);
    return (nodes);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_Ignore  --
 *	Return true if should ignore errors when creating gn
 *
 * Results:
 *	TRUE if should ignore errors
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Targ_Ignore (GNode *gn)		/* node to check for */
{
    if (ignoreErrors || gn->type & OP_IGNORE) {
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Targ_Silent  --
 *	Return true if be silent when creating gn
 *
 * Results:
 *	TRUE if should be silent
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Targ_Silent (GNode *gn)		/* node to check for */
{
    if (beSilent || gn->type & OP_SILENT) {
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Targ_Precious --
 *	See if the given target is precious
 *
 * Results:
 *	TRUE if it is precious. FALSE otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Targ_Precious (GNode *gn)		/* the node to check */
{
    if (allPrecious || (gn->type & (OP_PRECIOUS|OP_DOUBLEDEP))) {
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/******************* DEBUG INFO PRINTING ****************/

static GNode	  *mainTarg;	/* the main target, as set by Targ_SetMain */
/*- 
 *-----------------------------------------------------------------------
 * Targ_SetMain --
 *	Set our idea of the main target we'll be creating. Used for
 *	debugging output.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	"mainTarg" is set to the main target's node.
 *-----------------------------------------------------------------------
 */
void
Targ_SetMain (GNode *gn)  	/* The main target we'll create */
{
    mainTarg = gn;
}

static int
TargPrintName (ClientData gnCD, ClientData ppath)
{
    GNode *gn = (GNode *)gnCD;

    printf ("%s ", gn->name->data);
#ifdef notdef
    if ((int)ppath) {
	if (gn->path) {
	    printf ("[%s]  ", gn->path);
	}
	if (gn == mainTarg) {
	    printf ("(MAIN NAME)  ");
	}
    }
#endif /* notdef */
    return (0);
}


int
Targ_PrintCmd (ClientData cmd, ClientData unused)
{
    printf ("\t%s\n", ((string_t)cmd)->data);
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_FmtTime --
 *	Format a modification time in some reasonable way and return it.
 *
 * Results:
 *	The time reformatted.
 *
 * Side Effects:
 *	The time is placed in a static area, so it is overwritten
 *	with each call.
 *
 *-----------------------------------------------------------------------
 */
char *
Targ_FmtTime (time_t t)
{
    struct tm	  	*parts;
    static char	  	buf[40];
    static const char  	*months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    parts = localtime(&t);

    sprintf (buf, "%d:%02d:%02d %s %d, 19%d",
	     parts->tm_hour, parts->tm_min, parts->tm_sec,
	     months[parts->tm_mon], parts->tm_mday, parts->tm_year);
    return(buf);
}
    
/*-
 *-----------------------------------------------------------------------
 * Targ_PrintType --
 *	Print out a type field giving only those attributes the user can
 *	set.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
Targ_PrintType (register int type)
{
    register int    tbit;
    
#ifdef __STDC__
#define PRINTBIT(attr)	case CONCAT(OP_,attr): printf("." #attr " "); break
#define PRINTDBIT(attr) case CONCAT(OP_,attr): if (DEBUG(TARG)) printf("." #attr " "); break
#else
#define PRINTBIT(attr) 	case CONCAT(OP_,attr): printf(".attr "); break
#define PRINTDBIT(attr)	case CONCAT(OP_,attr): if (DEBUG(TARG)) printf(".attr "); break
#endif /* __STDC__ */

    type &= ~OP_OPMASK;

    while (type) {
	tbit = 1 << (ffs(type) - 1);
	type &= ~tbit;

	switch(tbit) {
	    PRINTBIT(OPTIONAL);
	    PRINTBIT(USE);
	    PRINTBIT(EXEC);
	    PRINTBIT(IGNORE);
	    PRINTBIT(PRECIOUS);
	    PRINTBIT(SILENT);
	    PRINTBIT(MAKE);
	    PRINTBIT(JOIN);
	    PRINTBIT(INVISIBLE);
	    PRINTBIT(NOTMAIN);
	    PRINTDBIT(LIB);
	    /*XXX: MEMBER is defined, so CONCAT(OP_,MEMBER) gives OP_"%" */
	    case OP_MEMBER: if (DEBUG(TARG)) printf(".MEMBER "); break;
	    PRINTDBIT(ARCHV);
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * TargPrintNode --
 *	print the contents of a node
 *-----------------------------------------------------------------------
 */
static int
TargPrintNode (ClientData gnCD, ClientData pass)
{
    GNode *gn = (GNode *)gnCD;

    if (!OP_NOP(gn->type)) {
	printf("#\n# Target %s assigned in %s\n",
	       gn->name->data,
	       gn->makefilename ?
	       gn->makefilename->data
	       : "(unknown makefile)");
	if (gn == mainTarg) {
	    printf("# *** MAIN TARGET ***\n");
	}
	if ((int)pass == 2) {
	    if (gn->unmade) {
		printf("# %d unmade children\n", gn->unmade);
	    } else {
		printf("# No unmade children\n");
	    }
	    if (! (gn->type & (OP_JOIN|OP_USE|OP_EXEC))) {
		if (gn->mtime != 0) {
		    printf("# last modified %s: %s\n",
			      Targ_FmtTime(gn->mtime),
			      (gn->made == UNMADE ? "unmade" :
			       (gn->made == MADE ? "made" :
				(gn->made == UPTODATE ? "up-to-date" :
				 "error when made"))));
		} else if (gn->made != UNMADE) {
		    printf("# non-existent (maybe): %s\n",
			      (gn->made == MADE ? "made" :
			       (gn->made == UPTODATE ? "up-to-date" :
				(gn->made == ERROR ? "error when made" :
				 "aborted"))));
		} else {
		    printf("# unmade\n");
		}
	    }
	    if (!Lst_IsEmpty (gn->iParents)) {
		printf("# implicit parents: ");
		Lst_ForEach (gn->iParents, TargPrintName, (ClientData)0);
		putc ('\n', stdout);
	    }
	}
	if (!Lst_IsEmpty (gn->parents)) {
	    printf("# parents: ");
	    Lst_ForEach (gn->parents, TargPrintName, (ClientData)0);
	    putc ('\n', stdout);
	}
	
	printf("%-16s", gn->name->data);
	switch (gn->type & OP_OPMASK) {
	    case OP_DEPENDS:
		printf(": "); break;
	    case OP_FORCE:
		printf("! "); break;
	    case OP_DOUBLEDEP:
		printf(":: "); break;
	}
	Targ_PrintType (gn->type);
	Lst_ForEach (gn->children, TargPrintName, (ClientData)0);
	putc ('\n', stdout);
	Lst_ForEach (gn->commands, Targ_PrintCmd, (ClientData)0);
	printf("\n\n");
	if (gn->type & OP_DOUBLEDEP) {
	    Lst_ForEach (gn->cohorts, TargPrintNode, pass);
	}
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * TargPrintOnlySrc --
 *	Print only those targets that are just a source.
 *
 * Results:
 *	0.
 *
 * Side Effects:
 *	The name of each file is printed preceeded by #\t
 *
 *-----------------------------------------------------------------------
 */
static int
TargPrintOnlySrc(ClientData gnCD, ClientData unused)
{
    GNode *gn = (GNode *)gnCD;

    if (OP_NOP(gn->type)) {
	printf("#\t%s [%s]\n", gn->name->data,
		  gn->path ? gn->path->data : gn->name->data);
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_PrintGraph --
 *	print the entire graph. heh heh
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	lots o' output
 *-----------------------------------------------------------------------
 */
void
Targ_PrintGraph (int pass) 	/* Which pass this is. 1 => no processing
				 * 2 => processing done */
{
    printf("#*** Input graph:\n");
    Lst_ForEach (allTargets, TargPrintNode, (ClientData)pass);
    printf("\n\n");
    printf("#\n#   Files that are only sources:\n");
    Lst_ForEach (allTargets, TargPrintOnlySrc, (ClientData)NULL);
    printf("#*** Global Variables:\n");
    Var_Dump (VAR_GLOBAL);
    printf("#*** Command-line Variables:\n");
    Var_Dump (VAR_CMD);
    printf("\n");
    Dir_PrintDirectories();
    printf("\n");
    Suff_PrintAll();
}

/*-
 * The following functions are used to write "target dependency" lists for
 * use in update builds.
 *
 * The following writes to the file defined by TDPATH_ALL:
 *
 *	GetChildrenList		Get a list of children for a specific target.
 *				Called from TargPrintAllDeps.
 *
 *	TargPrintAllDeps	Writes a target and all of it's children as
 *				target dependency pairs.  One target
 *				dependency pair per line.
 *
 *	GetNextParents		Called from GetNextChildren.  It will add
 *				a child to the next_Children list.  This
 *				marks it to be processed as a parent since
 *				it is a node that has children.
 *
 *	GetNextChildren		Builds the list of parents that will be
 *				written out to the target/dependency file
 *				as the target.
 *
 *	TargGetChildren		Builds a list of nodes steming from a singular
 *				target that are parents themselves.  This list
 *				is used as the targets in the target/dependency
 *				file.
 *
 *	Targ_PrintAllTargDep	This is the function that is called from main
 *				when the variable PTF_UPDATE_ALL=yes.  It
 *				will call TargGetChildren for each node in
 *				allTargets.
 *
 * The following writes to the file defined by TDPATH:
 *
 *	TargPrintDeps		Writes the "target dependency" pair for
 *				a target that has dependencies listed in the
 *				sOODATE variable.  So that only the
 *				new dependencies are written.  Also "NEW"
 *				is added at the end of the line to show that
 *				it caused the target to rebuild.
 *
 *	Targ_PrintTargDep	This function is called from main when
 *				PTF_UPDATE=yes.  It will call TargPrintDeps
 *				for each node in allTargets.
 */

/*
 * declarations for the update functions
 */
FILE	*tracefile;       /* Target/Dependency file for update builds     */
FILE	*tracefile_all;   /* All Target/Dependency file for update builds */
static	Lst	next_Children;
static	Lst	next_ChildrenTmp;
static	Lst	AllParent;
static	Lst	Empty_Lst;
extern  string_t sALL_DEPEND;
extern  string_t sTDPATH;
extern  string_t sTDPATH_ALL;
Boolean IsParent = FALSE;

static int
GetChildrenList (ClientData gnCD, ClientData unused)
{
	GNode *gn = (GNode *)gnCD;

	if (gn->path != (string_t) NULL) {
		Var_Append(sALL_DEPEND, gn->path, VAR_GLOBAL);
	} else if (gn->type & OP_ARCHV) {
		Var_Append(sALL_DEPEND, gn->name, VAR_GLOBAL);
	}
        return (0);
}

/*-
 *-----------------------------------------------------------------------
 * TargPrintAllDeps --
 *	Print "target dependency" pair.
 *
 * Results:
 *	0.
 *
 * Side Effects:
 *	The full path of a target and the full path of it's dependency are
 *	written to the file defined in the envirionment variable TDPATH_ALL.
 *-----------------------------------------------------------------------
 */
static int
TargPrintAllDeps (ClientData gnCD, ClientData unused)
{
	GNode *gn = (GNode *)gnCD;

	static  int     first = 1;
	static  char    curdir[PATH_MAX+1];
	char    full_target[PATH_MAX+1];
	char    full_dependency[PATH_MAX+1];
	char    *ptr;
	char    *ptr2;
	const	char	*Children;
	int     len;
	Boolean depend_process	= TRUE;

	if (first) {
		if (getwd(curdir) == 0) {
			fprintf(stderr, "make: getwd() call failed!\n");
			exit(1);
		}
		strcat(curdir, "/");
		first = 0;
	}
	
	if (gn->path == (string_t) NULL) {
		if (gn->name->data[0] == '/') {
			strcpy(full_target, gn->name->data);
		} else {
			strcpy(full_target, curdir);
			strcat(full_target, gn->name->data);
		}
	} else {
		if (gn->path->data[0] == '/') {
			strcpy(full_target, gn->path->data);
		} else {
			strcpy(full_target, curdir);
			strcat(full_target, gn->path->data);
		}
	}

		Var_Set(sALL_DEPEND, sNULL, VAR_GLOBAL);
		Lst_ForEach (gn->children, GetChildrenList, (ClientData)0);
		Children = Var_Value(sALL_DEPEND, VAR_GLOBAL);
	
		if (Children[0] == '\0') {
		  	depend_process = FALSE;
		} else {
		  	depend_process = TRUE;
		}

		while (depend_process) {
			ptr = strchr(Children, ' ');
			if (ptr) {
				len = ptr-Children;
			} else {
				len = strlen(Children);
				depend_process = FALSE;
			}
			if (*Children == '/') {
				strncpy(full_dependency, Children, len);
				full_dependency[len] = '\0';
			}
			else {
				strcpy(full_dependency, curdir);
				strncat(full_dependency, Children, len);
			}
			ptr2 = strchr(full_dependency, '$');
			ptr2++;
			if (*ptr2 != '{') {
				fprintf(tracefile_all, "%s %s\n", full_target, full_dependency);
			}
			Children = ptr+1;
		}
	return (0);
}

static int
GetNextParents (ClientData gnCD, ClientData unused)
{
	GNode *gn = (GNode *)gnCD;

	if (!Lst_IsEmpty (gn->children)) {
	   (void) Lst_AtEnd (next_Children, (ClientData)gn);
	}
	return (0);
}

static int
GetNextChildren (ClientData gnCD, ClientData unused)
{
	GNode *gn = (GNode *)gnCD;

	if (!Lst_IsEmpty(gn->children) && !Lst_IsEmpty(gn->commands)) {
	   if (Lst_Member(AllParent, (ClientData)gn) == NILLNODE) {
	      (void) Lst_AtEnd (AllParent, (ClientData)gn);
	   }
	   IsParent = TRUE;
	   Lst_ForEach (gn->children, GetNextParents, (ClientData)0);
	}
	return (0);
}

static int
TargGetChildren (ClientData gnCD, ClientData pass)
{
	GNode *gn = (GNode *)gnCD;

	if (!Lst_IsEmpty(gn->children)) {
	   Lst_ForEach (gn->children, GetNextChildren, (ClientData)0);
	   while (!Lst_IsEmpty (next_Children)) {
	      Lst_Destroy(next_ChildrenTmp, NOFREE);
	      next_ChildrenTmp = Lst_Duplicate(next_Children, NOCOPY);
	      Lst_Destroy(next_Children, NOFREE);
	      next_Children = Lst_Duplicate(Empty_Lst, NOCOPY);
	      if (!Lst_IsEmpty(next_ChildrenTmp)) {
	         Lst_ForEach(next_ChildrenTmp,GetNextChildren,(ClientData)0);
	      }
	   }
	   if (IsParent) {
	      if (Lst_Member(AllParent, (ClientData)gn) == NILLNODE)
	         (void) Lst_AtEnd (AllParent, (ClientData)gn);
	      IsParent = FALSE;
	   }
        }
	return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_PrintAllTargDep --
 *	print *all* of the target dependency pairs for update builds
 * 
 * Results:
 *	none
 *
 * Side Effects:
 *	all target dependency pairs written to file
 *-----------------------------------------------------------------------
 */
Targ_PrintAllTargDep (pass)
    int	    pass; 	/* Which pass this is. 1 => no processing
			 * 2 => processing done */
{
	if (Var_Value(sTDPATH_ALL, VAR_GLOBAL) == NULL) {
		fprintf(stderr, "make: TDPATH_ALL is not set. Target/Dependency file will not be written.\n");
		return;
	}
	else
		if ((tracefile_all = fopen((Var_Value(sTDPATH_ALL, VAR_GLOBAL)),"a")) == NULL) {
			fprintf(stderr, "make: Cannot open %s! Target/Dependency file will not be written.\n", Var_Value(sTDPATH_ALL, VAR_GLOBAL));
			return;
		}

	next_Children = Lst_Init();
	next_ChildrenTmp = Lst_Init();
	AllParent = Lst_Init();
	Empty_Lst = Lst_Init();

	printf("Writing lmallmakelist data to: %s\n", Var_Value(sTDPATH_ALL, VAR_GLOBAL));

	Lst_ForEach (allTargets, TargGetChildren, (ClientData)pass);

	if (!Lst_IsEmpty (AllParent)) {
	   Lst_ForEach (AllParent, TargPrintAllDeps, (ClientData)pass);
	}

	fclose (tracefile_all);
	return;
}

/*-
 *-----------------------------------------------------------------------
 * TargPrintDeps --
 *	Print "target dependency" pair.
 *
 * Results:
 *	0.
 *
 * Side Effects:
 *	The full path of a target and the full path of it's dependency
 *	are written to the file defined in the envirionment variable TDPATH.
 *	If the dependency caused the target to be rebuilt, "NEW" is added
 *	to the end of the line. Used for selective fix.
 *-----------------------------------------------------------------------
 */
static int
TargPrintDeps (ClientData gnCD, ClientData pass)
{
	GNode *gn = (GNode *)gnCD;

	static	char	curdir[PATH_MAX+1];
	static	int	first = 1;
	char	full_target[PATH_MAX+1];
	char	full_dependency[PATH_MAX+1];
	const	char	*oodate;
	char	*ptr;
	int	len;
	Boolean	oodate_process = TRUE;
	struct	stat	sb;

	oodate = Var_Value(sOODATE, gn);

	if (first) {
		if (getwd(curdir) == 0) {
			fprintf(stderr, "make: getwd() call failed!\n");
			exit(1);
		}
		strcat(curdir, "/");
		first = 0;
	}

	/*
	 * Print the dependencies that were out of date and put NEW at the end.
	 */
	if (*oodate != '\0') {
		if (gn->name->data[0] == '/')
			strcpy(full_target, gn->name->data);
		else {
			strcpy(full_target, curdir);
			strcat(full_target, gn->name->data);
		}

		if (stat(full_target, &sb) == 0) {
			oodate_process = TRUE;
			while (oodate_process) {
				ptr = strchr(oodate, ' ');
				if (ptr)
					len = ptr-oodate;
				else {
					len = strlen(oodate);
					oodate_process = FALSE;
				}
				if (*oodate == '/') {
					strncpy(full_dependency, oodate, len);
					full_dependency[len] = '\0';
				}
				else {
					strcpy(full_dependency, curdir);
					strncat(full_dependency, oodate, len);
				}
				fprintf(tracefile, "%s %s NEW\n", full_target, full_dependency);
				oodate = ptr+1;
			}
		}
	}
	return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_PrintTargDep --
 *	print target dependency pair for update builds
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	target dependency pairs written to file
 *-----------------------------------------------------------------------
 */
Targ_PrintTargDep (pass)
    int	    pass; 	/* Which pass this is. 1 => no processing
			 * 2 => processing done */
{

	if (Var_Value(sTDPATH, VAR_GLOBAL) == NULL) {
		fprintf(stderr, "make: TDPATH is not set. Target/Dependency file will not be written.\n");
		return;
	}
	else
		if ((tracefile = fopen((Var_Value(sTDPATH, VAR_GLOBAL)),"a")) == NULL) {
			fprintf(stderr, "make: Cannot open %s! Target/Dependency file will not be written.\n", Var_Value(sTDPATH, VAR_GLOBAL));
			return;
		}

	printf("Writing lmmakelist data to: %s\n", Var_Value(sTDPATH, VAR_GLOBAL));
	Lst_ForEach (allTargets, TargPrintDeps, (ClientData)pass);
	fclose (tracefile);
}
