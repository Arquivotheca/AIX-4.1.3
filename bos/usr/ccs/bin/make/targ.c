#ifndef lint
static char sccsid[] = "@(#)27	1.6 src/bos/usr/ccs/bin/make/targ.c, cmdmake, bos41J, 9516A_all 4/13/95 15:11:56";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
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
static char rcsid[] = "@(#)$RCSfile: targ.c,v $ $Revision: 1.2.2.3 $ (OSF) $Date: 1992/03/23 22:36:53 $";
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

static Lst        allTargets;	/* the list of all targets found so far */
static Hash_Table targets;	/* a hash table of same */

extern char	**months;	/* array of abbreviated month names */

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
    allTargets = Lst_Init (FALSE);
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
Targ_NewGN (
    const	char           *name	/* the name to stick in the new node */
    )
{
    GNode *gn;

    emalloc(gn,sizeof (GNode));
    gn->name = strdup (name);
    gn->path = (char *) 0;
    gn->type =		0;
    gn->unmade =    	0;
    gn->make = 	    	FALSE;
    gn->made = 	    	UNMADE;
    gn->childMade = 	FALSE;
    gn->mtime = gn->cmtime = 0;
    gn->iParents =  	Lst_Init (FALSE);
    gn->cohorts =   	Lst_Init (FALSE);
    gn->parents =   	Lst_Init (FALSE);
    gn->children =  	Lst_Init (FALSE);
    gn->successors = 	Lst_Init(FALSE);
    gn->preds =     	Lst_Init(FALSE);
    gn->context =   	Lst_Init (FALSE);
    gn->commands =  	Lst_Init (FALSE);

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
    const	char           *name,	/* the name to find */
    const	int             flags	/* flags governing events when target not
				 * found */
    )
{
    GNode         *gn;	      /* node in that element */
    Hash_Entry	  *he;	      /* New or used hash entry for node */
    Boolean	  isNew;      /* Set TRUE if Hash_CreateEntry had to create */
			      /* an entry for the node */


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
    int            flags	/* flags used if no node is found for a given
				 * name */
    )
{
    Lst            nodes;	/* result list */
    register LstNode  ln;		/* name list element */
    register GNode *gn;		/* node in tLn */
    char    	  *name;

    nodes = Lst_Init (FALSE);

    if (Lst_Open (names) == FAILURE) {
	return (nodes);
    }
    while ((ln = Lst_Next (names)) != NILLNODE) {
	name = (char *)Lst_Datum(ln);
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
	    Error (MSGSTR(UNKNOWN, "make: Target \"%s\" not known."), 
		name);
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
Targ_Ignore (
    GNode          *gn		/* node to check for */
    )
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
Targ_Silent (
    GNode          *gn		/* node to check for */
    )
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
Targ_Precious (
    GNode          *gn		/* the node to check */
    )
{
    GNode    *archive;
    char     *archive_name;
    char     *tmp;

    if (allPrecious || (gn->type & (OP_PRECIOUS|OP_DOUBLEDEP))) {
	return (TRUE);
    } else {
        if (gn->type & OP_ARCHV) {
          /*  strip off the (foo.o) from libfoobar.a(foo.o)  
                and check if libfoobar.a is precious  */ 
          emalloc(archive_name, strlen(gn->name) + 1);
          strcpy(archive_name, gn->name);
          tmp = index(archive_name, '(');

          /*  if there is a '(' in the string  */
          if (tmp)
          {
            *tmp = '\0';
            archive = Targ_FindNode(archive_name, TARG_NOCREATE);
            free(archive_name);
            if (archive && (archive->type & (OP_PRECIOUS|OP_DOUBLEDEP)))
              return (TRUE);
          }
        }

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
Targ_SetMain (
    GNode   *gn  	/* The main target we'll create */
    )
{
    mainTarg = gn;
}

static int
TargPrintName (
    GNode          *gn,
    int		    ppath
    )
{
    fprintf (stderr,"%s ", gn->name);
#ifdef notdef
    if (ppath) {
	if (gn->path) {
	    fprintf (stderr,"[%s]  ", gn->path);
	}
	if (gn == mainTarg) {
	   fprintf(stderr,MSGSTR(MAINNAME, "(MAIN NAME)  "));
	}
    }
#endif /* notdef */
    return (0);
}


int
Targ_PrintCmd (
    char           *cmd
    )

{
    fprintf (stderr,"\t%s\n", cmd);
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
Targ_FmtTime (
    const	time_t    time
    )
{
    struct tm	  	*parts;
    char	  	buf[40];

    parts = localtime(&time);

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
Targ_PrintType (
    register int    type
    )
{
    register int    tbit;
    
#ifdef __STDC__
#define PRINTBIT(attr)	case CONCAT(OP_,attr): fprintf(stderr,"." #attr " "); break
#define PRINTDBIT(attr) case CONCAT(OP_,attr): if (DEBUG(TARG)) fprintf(stderr,"." #attr " "); break
#else
#define PRINTBIT(attr) 	case CONCAT(OP_,attr): fprintf(stderr,".attr "); break
#define PRINTDBIT(attr)	case CONCAT(OP_,attr): if (DEBUG(TARG)) fprintf(stderr,".attr "); break
#endif /* __STDC__ */

    type &= ~OP_OPMASK;

    while (type) {
	tbit = 1 << (ffs(type) - 1);
	type &= ~tbit;

	switch(tbit) {
	    PRINTBIT(IGNORE);
	    PRINTBIT(PRECIOUS);
	    PRINTBIT(SILENT);
	    PRINTBIT(MAKE);
	    PRINTBIT(INVISIBLE);
	    PRINTBIT(NOTMAIN);
	    /*XXX: MEMBER is defined, so CONCAT(OP_,MEMBER) gives OP_"%" */
	    case OP_MEMBER: if (DEBUG(TARG)) fputs(".MEMBER ",stderr); break;
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
TargPrintNode (
    GNode         *gn,
    int	    	  pass
    )
{
    if (!OP_NOP(gn->type)) {
	fputs("#\n",stderr);
	if (gn == mainTarg) {
	   fprintf(stderr,MSGSTR(MAINTARG, "# *** MAIN TARGET ***\n"));
	}
	if (pass == 2) {
	    if (gn->unmade) {
	fprintf(stderr,MSGSTR(UNMADEYES, "# %d unmade children\n"), gn->unmade);
	    } else {
	fprintf(stderr,MSGSTR(UNMADENO, "# No unmade children\n"));
	    }
	    if (gn->mtime != 0) {
	fprintf(stderr,MSGSTR(LASTMOD, "# last modified %s: %s\n"),
			  Targ_FmtTime(gn->mtime),
			  (gn->made == UNMADE ? (MSGSTR(UNMADE1, "unmade")) :
			   (gn->made == MADE ? (MSGSTR(MADE1, "made")) :
			    (gn->made == UPTODATE ? (MSGSTR(UPTODATE1, 
				"up-to-date")) :
			     (MSGSTR(ERRMADE, "error when made"))))));
	    } else if (gn->made != UNMADE) {
	fprintf(stderr,MSGSTR(NOMAYBE, "# non-existent (maybe): %s\n"),
			  (gn->made == MADE ? (MSGSTR(MADE1, "made")) :
			   (gn->made == UPTODATE ? (MSGSTR(UPTODATE1, 
				"up-to-date")) :
			    (gn->made == ERROR ? (MSGSTR(ERRMADE, 
				"error when made")) :
			     (MSGSTR(ABORTED1, "aborted"))))));
	    } else {
	fprintf(stderr,MSGSTR(UNMADE2, "# unmade\n"));
	    }
	    if (!Lst_IsEmpty (gn->iParents)) {
	fprintf(stderr,MSGSTR(IPARENTS, "# implicit parents: "));
		Lst_ForEach (gn->iParents, TargPrintName, (ClientData)0);
		putc ('\n', stderr);
	    }
	}
	if (!Lst_IsEmpty (gn->parents)) {
	   fprintf(stderr,MSGSTR(PARENTS, "# parents: "));
	    Lst_ForEach (gn->parents, TargPrintName, (ClientData)0);
	    putc ('\n', stderr);
	}
	
	fprintf(stderr,"%-16s", gn->name);
	switch (gn->type & OP_OPMASK) {
	    case OP_DEPENDS:
		fputs(": ",stderr); break;
	    case OP_DOUBLEDEP:
		fputs(":: ",stderr); break;
	}
	Targ_PrintType (gn->type);
	Lst_ForEach (gn->children, TargPrintName, (ClientData)0);
	putc ('\n', stderr);
	Lst_ForEach (gn->commands, Targ_PrintCmd, (ClientData)0);
	fputs("\n\n",stderr);
	if (gn->type & OP_DOUBLEDEP) {
	    Lst_ForEach (gn->cohorts, TargPrintNode, (ClientData)pass);
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
TargPrintOnlySrc(
    GNode   	  *gn
    )
{
    if (OP_NOP(gn->type)) {
	fprintf(stderr,"#\t%s [%s]\n", gn->name,
		  gn->path ? gn->path : gn->name);
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
Targ_PrintGraph (
    int	    pass 	/* Which pass this is. 1 => no processing
			 * 2 => processing done */
    )
{
   fprintf(stderr,MSGSTR(GRAPHMSG1, "#*** Input graph:\n"));
    Lst_ForEach (allTargets, TargPrintNode, (ClientData)pass);
    fputs("\n\n",stderr);
   fprintf(stderr,MSGSTR(GRAPHMSG2, "#\n#   Files that are only sources:\n"));
    Lst_ForEach (allTargets, TargPrintOnlySrc, NULL);
   fprintf(stderr,MSGSTR(GRAPHMSG5, "#*** Internal (default) Variables:\n"));
    Var_Dump (VAR_INTERNAL);
   fprintf(stderr,MSGSTR(GRAPHMSG3, "#*** Global Variables:\n"));
    Var_Dump (VAR_GLOBAL);
   fprintf(stderr,MSGSTR(GRAPHMSG4, "#*** Command-line Variables:\n"));
    Var_Dump (VAR_CMD);
    fputs("\n",stderr);
    Dir_PrintDirectories();
    fputs("\n",stderr);
    Suff_PrintAll();
}
