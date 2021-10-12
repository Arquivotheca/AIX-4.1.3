/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ParseAddCmd
 *		ParseAddDir
 *		ParseAddDirs
 *		ParseBlankLines
 *		ParseClearPath
 *		ParseDoDependency
 *		ParseDoInclude
 *		ParseDoOp
 *		ParseDoSources
 *		ParseDoSrc
 *		ParseDoVar
 *		ParseEOF
 *		ParseErrorCond
 *		ParseFindKeyword
 *		ParseFindMain
 *		ParseFinishLine
 *		ParseHasCommands1546
 *		ParseLinkSrc
 *		ParseReadLine
 *		ParseReadc
 *		ParseVarOrDep
 *		Parse_AddIncludeDir1569
 *		Parse_DoVar
 *		Parse_Error
 *		Parse_File
 *		Parse_Init
 *		Parse_MainName
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
 * $Log: parse.c,v $
 * Revision 1.2.5.5  1992/12/03  19:07:09  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:25  damon]
 *
 * Revision 1.2.5.4  1992/11/09  21:50:30  damon
 * 	CR 296. Cleaned up to remove warnings
 * 	[1992/11/09  21:49:20  damon]
 * 
 * Revision 1.2.5.3  1992/09/24  19:26:57  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:55:25  gm]
 * 
 * Revision 1.2.5.2  1992/06/24  16:31:45  damon
 * 	CR 181. Changed vfork to fork
 * 	[1992/06/24  16:19:35  damon]
 * 
 * Revision 1.2  1991/12/05  20:44:42  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:38:34  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:08:14  mckeen]
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
static char sccsid[] = "@(#)49  1.4  src/bldenv/make/parse.c, bldprocess, bos412, GOLDA411a 1/19/94 16:31:15";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)parse.c	5.18 (Berkeley) 2/19/91";
#endif /* not lint */

/*-
 * parse.c --
 *	Functions to parse a makefile.
 *
 *	One function, Parse_Init, must be called before any functions
 *	in this module are used. After that, the function Parse_File is the
 *	main entry point and controls most of the other functions in this
 *	module.
 *
 *	Most important structures are kept in Lsts. Directories for
 *	the #include "..." function are kept in the 'dirSearchPath' Lst, while
 *	those for the #include <...> are kept in the 'parseIncPath' and
 *	'sysIncPath' Lst's.  The targets currently being defined are kept
 *	in the 'targets' Lst.
 *
 *	The variables 'fname' and 'lineno' are used to track the name
 *	of the current file and the line number in that file so that error
 *	messages can be more meaningful.
 *
 * Interface:
 *	Parse_Init	    	    Initialization function which must be
 *	    	  	    	    called before anything else in this module
 *	    	  	    	    is used.
 *
 *	Parse_File	    	    Function used to parse a makefile. It must
 *	    	  	    	    be given the name of the file, which should
 *	    	  	    	    already have been opened, and a function
 *	    	  	    	    to call to read a character from the file.
 *
 *	Parse_DoVar	    	    Returns zero if the given line is a
 *	    	  	    	    variable assignment. Used by MainParseArgs
 *	    	  	    	    to determine if an argument is a target
 *	    	  	    	    or a variable assignment. Used internally
 *	    	  	    	    for pretty much the same thing...
 *
 *	Parse_Error	    	    Function called when an error occurs in
 *	    	  	    	    parsing. Used by the variable and
 *	    	  	    	    conditional modules.
 *	Parse_MainName	    	    Returns a Lst of the main target to create.
 */

#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>
#include "make.h"
#include "buf.h"
#include "hash.h"
#include "pathnames.h"

/*
 * These values are returned by ParseEOF to tell Parse_File whether to
 * CONTINUE parsing, i.e. it had only reached the end of an include file,
 * or if it's DONE.
 */
#define	CONTINUE	1
#define	DONE		0
static int 	    ParseEOF(int);

static Lst     	    targets;	/* targets we're working on */
static Boolean	    inLine;	/* true if currently in a dependency
				 * line or its commands */

string_t	    fname;	/* name of current file (for errors) */
static int          lineno;	/* line number in current file */
static FILE   	    *curFILE; 	/* current makefile */

static Buffer	    lookaheadBuf;
static Boolean	    haveLookahead = FALSE;

static int	    makeincludecompat;

static int	    fatals = 0;

static GNode	    *mainNode;	/* The main target to create. This is the
				 * first target on the first dependency
				 * line in the first makefile */
/*
 * Definitions for handling #include specifications
 */
typedef struct IFile {
    string_t        fname;	    /* name of previous file */
    int             lineno;	    /* saved line number */
    FILE *       F;		    /* the open stream */
}              	  IFile;

static Lst      includes;  	/* stack of IFiles generated by
				 * #includes */
Lst         	parseIncPath;	/* list of directories for <...> includes */
Lst         	sysIncPath;	/* list of directories for <...> includes */

Lst		specPaths;	/* special paths */

/*-
 * specType contains the SPECial TYPE of the current target. It is
 * Not if the target is unspecial. If it *is* special, however, the children
 * are linked as children of the parent but not vice versa. This variable is
 * set in ParseDoDependency
 */
typedef enum {
    Default,	    /* .DEFAULT */
    Ignore,	    /* .IGNORE */
    Includes,	    /* .INCLUDES */
    Libs,	    /* .LIBS */
    MFlags,	    /* .MFLAGS or .MAKEFLAGS */
    Main,	    /* .MAIN and we don't have anything user-specified to
		     * make */
    Not,	    /* Not special */
    NotMain,  	    /* .BEGIN, .END, .INTERRUPT, .ERROR or .EXIT */
    NotParallel,    /* .NOTPARALLEL */
    Null,   	    /* .NULL */
    Order,  	    /* .ORDER */
    Path,	    /* .PATH */
    Precious,	    /* .PRECIOUS */
    Silent,	    /* .SILENT */
    Suffixes,	    /* .SUFFIXES */
    Attribute	    /* Generic attribute */
} ParseSpecial;

ParseSpecial specType;

/*
 * Predecessor node for handling .ORDER. Initialized to NILGNODE when .ORDER
 * seen, then set to each successive source on the line.
 */
static GNode	*predecessor;

/*
 * The parseKeywords table is searched using binary search when deciding
 * if a target or source is special. The 'spec' field is the ParseSpecial
 * type of the keyword ("Not" if the keyword isn't special as a target) while
 * the 'op' field is the operator to apply to the list of targets if the
 * keyword is used as a source ("0" if the keyword isn't special as a source)
 */
static struct {
    const char	  *name;    	/* Name of keyword */
    ParseSpecial  spec;	    	/* Type when used as a target */
    int	    	  op;	    	/* Operator when used as a source */
} parseKeywords[] = {
{ ".BEGIN", 	  NotMain,    	0 },
{ ".DEFAULT",	  Default,  	0 },
{ ".OPTIONAL",	  Attribute,   	OP_OPTIONAL },
{ ".END",   	  NotMain,	0 },
{ ".ERROR",   	  NotMain,	0 },
{ ".EXIT",   	  NotMain,	0 },
{ ".EXEC",	  Attribute,   	OP_EXEC },
{ ".IGNORE",	  Ignore,   	OP_IGNORE },
{ ".INCLUDES",	  Includes, 	0 },
{ ".INTERRUPT",	  NotMain,	0 },
{ ".INVISIBLE",	  Attribute,   	OP_INVISIBLE },
{ ".JOIN",  	  Attribute,   	OP_JOIN },
{ ".LIBS",  	  Libs,	    	0 },
{ ".LINKS",  	  Attribute,   	OP_LINK },
{ ".MAIN",	  Main,		0 },
{ ".MAKE",  	  Attribute,   	OP_MAKE },
{ ".MAKEFLAGS",	  MFlags,   	0 },
{ ".MFLAGS",	  MFlags,   	0 },
{ ".NOTMAIN",	  Attribute,   	OP_NOTMAIN },
{ ".NOTPARALLEL", NotParallel,	0 },
{ ".NULL",  	  Null,	    	0 },
{ ".ORDER", 	  Order,    	0 },
{ ".PATH",	  Path,		0 },
{ ".PRECIOUS",	  Precious, 	OP_PRECIOUS },
{ ".RECURSIVE",	  Attribute,	OP_MAKE },
{ ".SILENT",	  Silent,   	OP_SILENT },
{ ".SUFFIXES",	  Suffixes, 	0 },
{ ".USE",   	  Attribute,   	OP_USE },
};

int specOp;

char parseMeta[UCHAR_MAX+1];

/*-
 *----------------------------------------------------------------------
 * ParseFindKeyword --
 *	Look in the table of keywords for one matching the given string.
 *
 * Results:
 *	The index of the keyword, or -1 if it isn't there.
 *
 * Side Effects:
 *	None
 *----------------------------------------------------------------------
 */
static int
ParseFindKeyword (const char *str)		/* String to find */
{
    register int    start,
		    end,
		    cur;
    register int    diff;
    
    start = 0;
    end = (sizeof(parseKeywords)/sizeof(parseKeywords[0])) - 1;

    do {
	cur = start + ((end - start) / 2);
	diff = strcmp (str, parseKeywords[cur].name);

	if (diff == 0) {
	    return (cur);
	} else if (diff < 0) {
	    end = cur - 1;
	} else {
	    start = cur + 1;
	}
    } while (start <= end);
    return (-1);
}

/*-
 * Parse_Error  --
 *	Error message abort function for parsing. Prints out the context
 *	of the error (line number and file) as well as the message with
 *	two optional arguments.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	"fatals" is incremented if the level is PARSE_FATAL.
 */
/* VARARGS */
void
Parse_Error(int type, const char *fmt, ...)
				/* Error type (PARSE_WARNING, PARSE_FATAL) */
{
	va_list ap;

	(void)fflush(stdout);
	(void)fprintf(stderr, "\"%s\", line %d: ", fname->data, lineno);
	if (type == PARSE_WARNING)
		(void)fprintf(stderr, "warning: ");
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);
	if (type == PARSE_FATAL)
		fatals += 1;
}

/*-
 *---------------------------------------------------------------------
 * ParseLinkSrc  --
 *	Link the parent node to its new child. Used in a Lst_ForEach by
 *	ParseDoDependency. If the specType isn't 'Not', the parent
 *	isn't linked as a parent of the child.
 *
 * Results:
 *	Always = 0
 *
 * Side Effects:
 *	New elements are added to the parents list of cgn and the
 *	children list of cgn. the unmade field of pgn is updated
 *	to reflect the additional child.
 *---------------------------------------------------------------------
 */
static int
ParseLinkSrc (
    ClientData	pgnCD,	/* The parent node */
    ClientData	cgnCD)	/* The child node */
{
    GNode          *pgn = (GNode *)pgnCD;
    GNode          *cgn = (GNode *)cgnCD;
    Boolean	   new;

    if (Lst_Length(pgn->children)) {
	(void) Hash_CreateEntry(&pgn->childHT, cgn->name, &new);
	if (!new)
	    return(0);
    }
    (void)Lst_AtEnd (pgn->children, (ClientData)cgn);
    if (specType == Not)
	(void)Lst_AtEnd (cgn->parents, (ClientData)pgn);
    pgn->unmade += 1;
    return (0);
}

/*-
 *---------------------------------------------------------------------
 * ParseDoOp  --
 *	Apply the parsed operator to the given target node. Used in a
 *	Lst_ForEach call by ParseDoDependency once all targets have
 *	been found and their operator parsed. If the previous and new
 *	operators are incompatible, a major error is taken.
 *
 * Results:
 *	Always 0
 *
 * Side Effects:
 *	The type field of the node is altered to reflect any new bits in
 *	the op.
 *---------------------------------------------------------------------
 */
static int
ParseDoOp (
    ClientData	gnCD,		/* The node to which the operator is to be
				 * applied */
    ClientData	opCD)		/* The operator to apply */
{
    GNode          *gn = (GNode *)gnCD;
    int             op = (int) opCD;

    /*
     * If the dependency mask of the operator and the node don't match and
     * the node has actually had an operator applied to it before, and
     * the operator actually has some dependency information in it, complain. 
     */
    if (((op & OP_OPMASK) != (gn->type & OP_OPMASK)) &&
	!OP_NOP(gn->type) && !OP_NOP(op))
    {
	Parse_Error (PARSE_FATAL, "Inconsistent operator for %s", gn->name);
	return (1);
    }

    if ((op == OP_DOUBLEDEP) && ((gn->type & OP_OPMASK) == OP_DOUBLEDEP)) {
	/*
	 * If the node was the object of a :: operator, we need to create a
	 * new instance of it for the children and commands on this dependency
	 * line. The new instance is placed on the 'cohorts' list of the
	 * initial one (note the initial one is not on its own cohorts list)
	 * and the new instance is linked to all parents of the initial
	 * instance.
	 */
	register GNode	*cohort;
	LstNode	    	ln;
			
	cohort = Targ_NewGN(gn->name);
	/*
	 * Duplicate links to parents so graph traversal is simple. Perhaps
	 * some type bits should be duplicated?
	 *
	 * Make the cohort invisible as well to avoid duplicating it into
	 * other variables. True, parents of this target won't tend to do
	 * anything with their local variables, but better safe than
	 * sorry.
	 */
	Lst_ForEach(gn->parents, ParseLinkSrc, (ClientData)cohort);
	cohort->type = OP_DOUBLEDEP|OP_INVISIBLE;
	(void)Lst_AtEnd(gn->cohorts, (ClientData)cohort);

	/*
	 * Replace the node in the targets list with the new copy
	 */
	ln = Lst_Member(targets, (ClientData)gn);
	Lst_Replace(ln, (ClientData)cohort);
	gn = cohort;
    }
    /*
     * We don't want to nuke any previous flags (whatever they were) so we
     * just OR the new operator into the old 
     */
    gn->type |= op;

    return (0);
}

/*-
 *---------------------------------------------------------------------
 * ParseDoSrc  --
 *	Given the name of a source, figure out if it is an attribute
 *	and apply it to the targets if it is. Else decide if there is
 *	some attribute which should be applied *to* the source because
 *	of some special target and apply it if so. Otherwise, make the
 *	source be a child of the targets in the list 'targets'
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Operator bits may be added to the list of targets or to the source.
 *	The targets may have a new source added to their lists of children.
 *---------------------------------------------------------------------
 */
static void
ParseDoSrc (
    string_t	src)	/* name of the source to handle */
{
    int		tOp;	/* operator (if any) from special targets */
    int		op;	/* operator (if any) from special source */
    GNode	*gn;

    tOp = specOp;
    op = 0;
    if (*src->data == '.' && isupper (src->data[1])) {
	int keywd = ParseFindKeyword(src->data);
	if (keywd != -1) {
	    op = parseKeywords[keywd].op;
	}
	if (op != 0) {
	    Lst_ForEach (targets, ParseDoOp, (ClientData)op);
	    return;
	}
    }
    if (specType == Main) {
	/*
	 * If we have noted the existence of a .MAIN, it means we need
	 * to add the sources of said target to the list of things
	 * to create. The string 'src' is likely to be free, so we
	 * must make a new copy of it. Note that this will only be
	 * invoked if the user didn't specify a target on the command
	 * line. This is to allow #ifmake's to succeed, or something...
	 */
	(void) Lst_AtEnd (create, (ClientData)string_ref(src));
	/*
	 * Add the name to the .TARGETS variable as well, so the user can
	 * employ that, if desired.
	 */
	Var_Append(s_TARGETS, src, VAR_GLOBAL);
	return;
    }
    gn = Targ_FindNode(src, TARG_CREATE);
    if (specType == Order) {
	/*
	 * Create proper predecessor/successor links between the previous
	 * source and the current one.
	 */
	if (predecessor != NILGNODE) {
	    (void)Lst_AtEnd(predecessor->successors, (ClientData)gn);
	    (void)Lst_AtEnd(gn->preds, (ClientData)predecessor);
	}
	/*
	 * The current source now becomes the predecessor for the next one.
	 */
	predecessor = gn;
	return;
    }
    /*
     * If the source is not an attribute, we need to find/create
     * a node for it. After that we can apply any operator to it
     * from a special target or link it to its parents, as
     * appropriate.
     *
     * In the case of a source that was the object of a :: operator,
     * the attribute is applied to all of its instances (as kept in
     * the 'cohorts' list of the node) or all the cohorts are linked
     * to all the targets.
     */
    if (tOp) {
	gn->type |= tOp;
    } else {
	Lst_ForEach (targets, ParseLinkSrc, (ClientData)gn);
    }
    if ((gn->type & OP_OPMASK) == OP_DOUBLEDEP) {
	register GNode  	*cohort;
	register LstNode	ln;

	for (ln=Lst_First(gn->cohorts); ln != NILLNODE; ln = Lst_Succ(ln)){
	    cohort = (GNode *)Lst_Datum(ln);
	    if (tOp) {
		cohort->type |= tOp;
	    } else {
		Lst_ForEach(targets, ParseLinkSrc, (ClientData)cohort);
	    }
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * ParseFindMain --
 *	Find a real target in the list and set it to be the main one.
 *	Called by ParseDoDependency when a main target hasn't been found
 *	yet.
 *
 * Results:
 *	0 if main not found yet, 1 if it is.
 *
 * Side Effects:
 *	mainNode is changed and Targ_SetMain is called.
 *
 *-----------------------------------------------------------------------
 */
static int
ParseFindMain(ClientData gnCD, ClientData unused)
{
    GNode *gn = (GNode *)gnCD;	    /* Node to examine */

    if ((gn->type & (OP_NOTMAIN|OP_USE|OP_EXEC|OP_TRANSFORM)) == 0) {
	mainNode = gn;
	Targ_SetMain(gn);
	return (1);
    } else {
	return (0);
    }
}

/*-
 *-----------------------------------------------------------------------
 * ParseAddDir --
 *	Front-end for Dir_AddDir to make sure Lst_ForEach keeps going
 *
 * Results:
 *	=== 0
 *
 * Side Effects:
 *	See Dir_AddDir.
 *
 *-----------------------------------------------------------------------
 */
static int
ParseAddDir(ClientData path, ClientData name)
{
    Dir_AddDir((Lst)path, (string_t)name);
    return(0);
}

static void
ParseAddDirs(string_t name)
{
    Lst_ForEach(specPaths, ParseAddDir, (ClientData)name);
}

/*-
 *-----------------------------------------------------------------------
 * ParseClearPath --
 *	Front-end for Dir_ClearPath to make sure Lst_ForEach keeps going
 *
 * Results:
 *	=== 0
 *
 * Side Effects:
 *	See Dir_ClearPath
 *
 *-----------------------------------------------------------------------
 */
static int
ParseClearPath(ClientData path, ClientData unused)
{
    Dir_ClearPath((Lst)path);
    return(0);
}

/*-
 *---------------------------------------------------------------------
 * ParseDoDependency  --
 *	Parse the dependency line in line.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The nodes of the sources are linked as children to the nodes of the
 *	targets. Some nodes may be created.
 *
 *	We parse a dependency line by first extracting words from the line and
 * finding nodes in the list of all targets with that name. This is done
 * until a character is encountered which is an operator character. Currently
 * these are only ! and :. At this point the operator is parsed and the
 * pointer into the line advanced until the first source is encountered.
 * 	The parsed operator is applied to each node in the 'targets' list,
 * which is where the nodes found for the targets are kept, by means of
 * the ParseDoOp function.
 *	The sources are read in much the same way as the targets were except
 * that now they are expanded using the wildcarding scheme of the C-Shell
 * and all instances of the resulting words in the list of all targets
 * are found. Each of the resulting nodes is then linked to each of the
 * targets as one of its children.
 *	Certain targets are handled specially. These are the ones detailed
 * by the specType variable.
 *	The storing of transformation rules is also taken care of here.
 * A target is recognized as a transformation rule by calling
 * Suff_IsTransform. If it is a transformation rule, its node is gotten
 * from the suffix module via Suff_AddTransform rather than the standard
 * Targ_FindNode in the target module.
 *---------------------------------------------------------------------
 */
static void
ParseDoSources(
    Buffer buf,
    char *dependents,
    Boolean doArch,
    void (*func)(string_t))
{
    char	*line;		/* our current target/dependent */
    register char  *cp;		/* our current position */
    register int    c;		/* a place to save a character */
    string_t	sline;

    /*
     * Expand the sources - reuse targets buffer, we are done with it...
     */
    Buf_Discard(buf, Buf_Size(buf));
    cp = dependents;
    for (;;) {
	for (;;) {
	    if (!parseMeta[*(unsigned char *)cp]) {
		Buf_AddByte(buf, (Byte) *cp++);
		continue;
	    }
	    if (*cp == '\0')
		goto out;
	    if (*cp == ' ' || *cp == '\t')
		break;
	    if (*cp == '$') {
		char	*result;
		int	length;
		Boolean	freeIt;

		result = Var_Parse(cp, VAR_CMD, TRUE, &length, &freeIt);
		if (result == var_Error && !oldVars) {
		    Parse_Error(PARSE_FATAL, "Bad imbedded variable");
		    return;
		}
		Buf_AddBytes(buf, strlen(result), (Byte *)result);
		if (freeIt)
		    free(result);
		cp += length;
		continue;
	    }
	    if (*cp == ';') {
		char *cp2 = cp;
		*cp2++ = '\0';
		while (*cp2 == ' ' || *cp2 == '\t')
		    cp2++;
		if (*cp2 == '\0')
		    break;
		Buf_AddBytes(lookaheadBuf, strlen(cp2), (Byte *) cp2);
		haveLookahead = TRUE;
		goto out;
	    }
	    Buf_AddByte(buf, (Byte) *cp++);
	}
	cp++;
	while (*cp == ' ' || *cp == '\t')
	    cp++;
	if (*cp == '\0')
	    break;
	Buf_AddByte(buf, (Byte) ' ');
    }
 out:

    /*
     * Get to the first source 
     */
    cp = (char *) Buf_GetBase(buf);
    while (*cp == ' ' || *cp == '\t')
	cp++;
    if (DEBUG(SUFF)) {
	printf("ParseDoDependency: dependents %s\n", cp);
    }

    /*
     * Several special targets take different actions if present with no
     * sources:
     *	a .SUFFIXES line with no sources clears out all old suffixes
     *	a .PRECIOUS line makes all targets precious
     *	a .IGNORE line ignores errors for all targets
     *	a .SILENT line creates silence when making all targets
     *	a .PATH removes all directories from the search path(s).
     */
    if (*cp == '\0') {
	switch (specType) {
	case Suffixes:
	    Suff_ClearSuffixes ();
	    break;
	case Precious:
	    allPrecious = TRUE;
	    break;
	case Ignore:
	    ignoreErrors = TRUE;
	    break;
	case Silent:
	    beSilent = TRUE;
	    break;
	case NotParallel:
	{
	    extern int  maxJobs;

	    maxJobs = 1;
	    break;
	}
	case Path:
	    if (specPaths == (Lst)NULL)
		break;
	    Lst_ForEach(specPaths, ParseClearPath, (ClientData)NULL);
	    Lst_Destroy(specPaths, NOFREE);
	    break;
	default:
	    break;
	}
	return;
    }

    /*
     * Handle sources that are not dependents
     */
    if (specType == MFlags) {
	/*
	 * Call on functions in main.c to deal with these arguments and
	 * set the initial character to a null-character so the loop to
	 * get sources won't get anything
	 */
	Main_ParseArgLine (cp);
	return;
    }
    if (specType == NotParallel) {
	extern int  maxJobs;

	/*
	 * Need to think of adding support for marking individual nodes
	 * not parallel...
	 */
	maxJobs = 1;
	return;
    }
    
    /*
     * NOW GO FOR THE SOURCES 
     */

    /*
     * If the target was one that doesn't take files as its sources
     * but takes something like suffixes, we take each
     * space-separated word on the line as a something and deal
     * with it accordingly.
     *
     * If the target was .SUFFIXES, we take each source as a
     * suffix and add it to the list of suffixes maintained by the
     * Suff module.
     *
     * If the target was a .PATH, we add the source as a directory
     * to search on the search path.
     *
     * If it was .INCLUDES, the source is taken to be the suffix of
     * files which will be #included and whose search path should
     * be present in the .INCLUDES variable.
     *
     * If it was .LIBS, the source is taken to be the suffix of
     * files which are considered libraries and whose search path
     * should be present in the .LIBS variable.
     *
     * If it was .NULL, the source is the suffix to use when a file
     * has no valid suffix.
     */
    for (;;) {
	line = cp;
	for (;;) {
	    if (!parseMeta[c = *(unsigned char *)cp]) {
		cp++;
		continue;
	    }
	    if (c == '\0' || c == ' ' || c == '\t')
		break;
	    if (doArch && c == LPAREN) {
		Lst sources = Lst_Init ();
		if (Arch_ParseArchive (&line, sources, VAR_CMD) != SUCCESS) {
		    Parse_Error (PARSE_FATAL,
				 "Error in source archive spec \"%s\"", line);
		    return;
		}
		while (!Lst_IsEmpty (sources)) {
		    GNode *gn = (GNode *) Lst_DeQueue (sources);
		    ParseDoSrc (gn->name);
		}
		Lst_Destroy (sources, NOFREE);
		while (*line == ' ' || *line == '\t')
		    line++;
		if (*line == '\0')
		    return;
		cp = line;
		continue;
	    }
	    if (c != '$') {
		cp++;
		continue;
	    }
	    /*
	     * Must be a dynamic source (would have been expanded
	     * otherwise), so call the Var module to parse the puppy
	     * so we can safely advance beyond it...There should be
	     * no errors in this, as they would have been discovered
	     * in the initial Var_Subst and we wouldn't be here.
	     */
	    cp = Var_Skip(cp, VAR_CMD, TRUE);
	}
	*cp = '\0';
	sline = string_create(line);
	(*func)(sline);
	string_deref(sline);
	if (c == '\0')
	    return;
	cp++;
	while (*cp == ' ' || *cp == '\t')
	    cp++;
	if (*cp == '\0')
	    return;
    }
}

static void
ParseDoDependency (
    Buffer	buf,		/* targets buffer - reused for dependents*/
    int		modifier,	/* modifier for targets */
    char	*dependents)	/* dependents */
{
    char	*targs;		/* targets */
    char	*line;		/* our current target/dependent */
    register char  *cp;		/* our current position */
    register GNode *gn;		/* a general purpose temporary node */
    register int    op;		/* the operator on the line */
    register int    c;		/* a place to save a character */
    Lst		paths;   	/* List of search paths to alter when parsing
				 * a list of .PATH targets */
    string_t	sline;

    targs = (char *) Buf_GetBase (buf);

    specType = Not;
    specOp = 0;
    paths = (Lst)NULL;
    line = targs;
    if (DEBUG(SUFF)) {
	printf("ParseDoDependency: targets %s\n", line);
    }

    while (*line) {
	cp = line;
	for (;;) {
	    if (!parseMeta[c = *(unsigned char *)cp]) {
		cp++;
		continue;
	    }
	    if (c == '\0' || c == ' ' || c == '\t')
		break;
	    if (c == LPAREN) {
		/*
		 * Archives must be handled specially to make sure the
		 * OP_ARCHV flag is set in their 'type' field, for one
		 * thing, and because things like "archive(file1.o
		 * file2.o file3.o)" are permissible.  Arch_ParseArchive
		 * will set 'line' to be the first non-blank after the
		 * archive-spec. It creates/finds nodes for the members
		 * and places them on the given list, returning SUCCESS
		 * if all went well and FAILURE if there was an error in
		 * the specification. On error, line should remain untouched.
		 */
		if (Arch_ParseArchive (&line, targets, VAR_CMD) != SUCCESS) {
		    Parse_Error (PARSE_FATAL,
				 "Error in archive specification: \"%s\"",
				 line);
		    return;
		}
		while (*line == ' ' || *line == '\t')
		    line++;
		cp = line;
		if (*line == '\0')
		    goto out;
		continue;
	    }
	    if (c != '$') {
		cp++;
		continue;
	    }
	    /*
	     * Must be a dynamic source (would have been expanded
	     * otherwise), so call the Var module to parse the puppy
	     * so we can safely advance beyond it...There should be
	     * no errors in this, as they would have been discovered
	     * in the initial Var_Subst and we wouldn't be here.
	     */
	    cp = Var_Skip(cp, VAR_CMD, TRUE);
	}

	/*
	 * This assumes that line is in a buffer, not a string...
	 */
	*cp = '\0';
	sline = string_create(line);

	/*
	 * Have a word in line. See if it's a special target and set
	 * specType to match it.
	 */
	if (*line == '.' && isupper (line[1])) {
	    /*
	     * See if the target is a special target that must have it
	     * or its sources handled specially. 
	     */
	    int keywd = ParseFindKeyword(line);
	    if (keywd != -1) {
		if (specType == Path && parseKeywords[keywd].spec != Path) {
		    Parse_Error(PARSE_FATAL, "Mismatched special targets");
		    return;
		}
		
		specType = parseKeywords[keywd].spec;
		specOp = parseKeywords[keywd].op;

		/*
		 * Certain special targets have special semantics:
		 *	.PATH		Have to set the dirSearchPath
		 *			variable too
		 *	.MAIN		Its sources are only used if
		 *			nothing has been specified to
		 *			create.
		 *	.DEFAULT    	Need to create a node to hang
		 *			commands on, but we don't want
		 *			it in the graph, nor do we want
		 *			it to be the Main Target, so we
		 *			create it, set OP_NOTMAIN and
		 *			add it to the list, setting
		 *			DEFAULT to the new node for
		 *			later use. We claim the node is
		 *	    	    	A transformation rule to make
		 *	    	    	life easier later, when we'll
		 *	    	    	use Make_HandleUse to actually
		 *	    	    	apply the .DEFAULT commands.
		 *	.BEGIN
		 *	.END
		 *	.ERROR
		 *	.EXIT
		 *	.INTERRUPT  	Are not to be considered the
		 *			main target.
		 *  	.NOTPARALLEL	Make only one target at a time.
		 *  	.ORDER	    	Must set initial predecessor to NIL
		 */
		switch (specType) {
		    case Path:
			if (paths == NULL) {
			    paths = Lst_Init();
			}
			(void)Lst_AtEnd(paths, (ClientData)dirSearchPath);
			break;
		    case Main:
			if (!Lst_IsEmpty(create)) {
			    specType = Not;
			}
			break;
		    case NotMain:
			gn = Targ_FindNode(sline, TARG_CREATE);
			gn->type |= OP_NOTMAIN;
			(void)Lst_AtEnd(targets, (ClientData)gn);
			break;
		    case Default:
			gn = Targ_NewGN(s_DEFAULT);
			gn->type |= (OP_NOTMAIN|OP_TRANSFORM);
			(void)Lst_AtEnd(targets, (ClientData)gn);
			DEFAULT = gn;
			break;
		    case Order:

			predecessor = NILGNODE;
			break;
		    default:
			break;
		}
	    } else if (strncmp (line, ".PATH", 5) == 0) {
		/*
		 * .PATH<suffix> has to be handled specially.
		 * Call on the suffix module to give us a path to
		 * modify.
		 */
		Lst 	path;
		string_t pathSuff;
		
		specType = Path;
		pathSuff = string_create(&line[5]);
		path = Suff_GetPath (pathSuff);
		string_deref(pathSuff);
		if (path == NILLST) {
		    Parse_Error (PARSE_FATAL,
				 "Suffix '%s' not defined (yet)",
				 pathSuff->data);
		    return;
		}
		if (paths == (Lst)NULL)
		    paths = Lst_Init();
		(void)Lst_AtEnd(paths, (ClientData)path);
	    }
	}

	/*
	 * Have word in line. Get or create its node and stick it at
	 * the end of the targets list 
	 */
	if (specType == Not && *line != '\0') {
	    if (DEBUG(SUFF))
		printf("ParseDoDep: %s\n", line);
	    if (Suff_IsTransform (sline))
		gn = Suff_AddTransform (sline);
	    else
		gn = Targ_FindNode (sline, TARG_CREATE);
	    (void)Lst_AtEnd (targets, (ClientData)gn);
	} else if (specType == Path && *line != '.' && *line != '\0')
	    Parse_Error(PARSE_WARNING, "Extra target (%s) ignored",
			sline->data);
	*cp = c;
	string_deref(sline);

	/*
	 * If it is a special type and not .PATH, it's the only target we
	 * allow on this line...
	 */
	if (specType != Not && specType != Path) {
	    while (*cp && *cp != ' ' && *cp != '\t')
		cp++;
	    if (*cp)
		Parse_Error(PARSE_WARNING, "Extra target(s) ignored");
	    break;
	}
	while (*cp == ' ' || *cp == '\t')
	    cp++;
	line = cp;
    }
 out:

    switch(specType) {
    case Default:
    case NotMain:
	/*
	 * These create nodes on which to hang commands, so
	 * targets shouldn't be empty...
	 */
    case Not:
	/*
	 * Nothing special here -- targets can be empty if it wants.
	 */
	break;
    default:
	if (!Lst_IsEmpty(targets))
	    Parse_Error(PARSE_WARNING,
			"Special and normal targets, normal ones ignored.");
	break;
    }

    /*
     * Have now parsed all the target names. Must parse the operator next. The
     * result is left in  op .
     */
    if (modifier == '!')
	op = OP_FORCE;
    else if (modifier == ':')
	op = OP_DOUBLEDEP;
    else
	op = OP_DEPENDS;

    Lst_ForEach (targets, ParseDoOp, (ClientData)op);

    switch (specType) {
    case Suffixes:
	ParseDoSources(buf, dependents, FALSE, Suff_AddSuffix);
	break;
    case Path:
	specPaths = paths;
	ParseDoSources(buf, dependents, FALSE, ParseAddDirs);
	if (paths)
	    Lst_Destroy(paths, NOFREE);
	break;
    case Includes:
	ParseDoSources(buf, dependents, FALSE, Suff_AddInclude);
	break;
    case Libs:
	ParseDoSources(buf, dependents, FALSE, Suff_AddLib);
	break;
    case Null:
	ParseDoSources(buf, dependents, FALSE, Suff_SetNull);
	break;
    default:
	ParseDoSources(buf, dependents, TRUE, ParseDoSrc);
	if (mainNode == NILGNODE) {
	    /*
	     * If we have yet to decide on a main target to make, in the
	     * absence of any user input, we want the first target on
	     * the first dependency line that is actually a real target
	     * (i.e. isn't a .USE or .EXEC rule) to be made.
	     */
	    Lst_ForEach (targets, ParseFindMain, (ClientData)NULL);
	}
    }
}

/*-
 *---------------------------------------------------------------------
 * ParseDoVar  --
 *	Take the variable assignment in the passed line and do it in the
 *	global context.
 *
 *	Note: There is a lexical ambiguity with assignment modifier characters
 *	in variable names. This routine interprets the character before the =
 *	as a modifier. Therefore, an assignment like
 *	    C++=/usr/bin/CC
 *	is interpreted as "C+ +=" instead of "C++ =".
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the variable structure of the given variable name is altered in the
 *	global context.
 *---------------------------------------------------------------------
 */
static void
ParseDoVar (
    char	*name,		/* the variable name */
    int		modifier,	/* Modifer for variable assignment */
    char	*value,		/* Value for variable */
    GNode	*ctxt)		/* Context in which to do the assignment */
{
    char	result[BUFSIZ];	/* Result of command */
    const char *args[4];   	/* Args for invoking the shell */
    int 	fds[2];	    	/* Pipe streams */
    int 	cpid;	    	/* Child PID */
    int 	pid;	    	/* PID from wait() */
    Boolean	freeCmd;    	/* TRUE if the command needs to be freed, i.e.
				 * if any variable expansion was performed */
    string_t	var, val;

    var = string_create(name);
    while (*value == ' ' || *value == '\t')
	value++;

    if (modifier == '+') {
	val = string_create(value);
	Var_Append (var, val, ctxt);
	string_deref(val);
	return;
    }
    if (modifier == '?') {
	/*
	 * If the variable already has a value, we don't do anything.
	 */
	if (!Var_Exists(var, ctxt)) {
	    val = string_create(value);
	    Var_Set (var, val, ctxt);
	    string_deref(val);
	}
	return;
    }
    if (modifier == ':') {
	/*
	 * Allow variables in the old value to be undefined, but leave their
	 * invocation alone -- this is done by forcing oldVars to be false.
	 * XXX: This can cause recursive variables, but that's not hard to do,
	 * and this allows someone to do something like
	 *
	 *  CFLAGS = $(.INCLUDES)
	 *  CFLAGS := -I.. $(CFLAGS)
	 *
	 * And not get an error.
	 */
	Boolean	  oldOldVars = oldVars;

	oldVars = FALSE;
	value = Var_Subst(value, ctxt, FALSE);
	oldVars = oldOldVars;

	val = string_create(value);
	free(value);
	Var_Set(var, val, ctxt);
	string_deref(val);
	return;
    }
    if (modifier != '!') {
	/*
	 * Normal assignment -- just do it.
	 */
	val = string_create(value);
	Var_Set (var, val, ctxt);
	string_deref(val);
	return;
    }

    /*
     * Set up arguments for shell
     */
    args[0] = "sh";
    args[1] = "-c";
    if (strchr(value, '$') != (char *)NULL) {
	/*
	 * There's a dollar sign in the command, so perform variable
	 * expansion on the whole thing. The resulting string will need
	 * freeing when we're done, so set freeCmd to TRUE.
	 */
	args[2] = Var_Subst(value, VAR_CMD, TRUE);
	freeCmd = TRUE;
    } else {
	args[2] = value;
	freeCmd = FALSE;
    }
    args[3] = (char *)NULL;

    /*
     * Open a pipe for fetching its output
     */
    pipe(fds);

    /*
     * Fork
     */
    cpid = fork();
    if (cpid == 0) {
	/*
	 * Close input side of pipe
	 */
	close(fds[0]);

	/*
	 * Duplicate the output stream to the shell's output, then
	 * shut the extra thing down. Note we don't fetch the error
	 * stream...why not? Why?
	 */
	dup2(fds[1], 1);
	close(fds[1]);

	execv("/bin/sh", (char * const *)args);
	_exit(1);
    } else if (cpid < 0) {
	/*
	 * Couldn't fork -- tell the user and make the variable null
	 */
	Parse_Error(PARSE_FATAL, "Couldn't exec \"%s\"", value);
	Var_Set(var, sNULL, ctxt);
    } else {
	int	status;
	int cc;

	/*
	 * No need for the writing half
	 */
	close(fds[1]);

	/*
	 * Wait for the process to exit.
	 *
	 * XXX: If the child writes more than a pipe's worth, we will
	 * deadlock.
	 */
	if ((pid = waitpid(cpid, &status, 0)) != cpid) {
	    Parse_Error(PARSE_FATAL, "unexpected error from waitpid: %s",
			strerror(errno));
	}

	/*
	 * Read all the characters the child wrote.
	 */
	cc = read(fds[0], result, sizeof(result));

	if (cc < 0) {
	    /*
	     * Couldn't read the child's output -- tell the user and
	     * set the variable to null
	     */
	    Parse_Error(PARSE_FATAL, "Couldn't read shell's output");
	    cc = 0;
	}

	if (status) {
	    /*
	     * Child returned an error -- tell the user but still use
	     * the result.
	     */
	    Parse_Error(PARSE_FATAL, "\"%s\" returned non-zero", value);
	}
	/*
	 * Null-terminate the result, convert newlines to spaces and
	 * install it in the variable.
	 */
	result[cc] = '\0';
	value = &result[cc] - 1;

	if (*value == '\n') {
	    /*
	     * A final newline is just stripped
	     */
	    *value-- = '\0';
	}
	while (value >= result) {
	    if (*value == '\n') {
		*value = ' ';
	    }
	    value--;
	}
	val = string_create(result);
	Var_Set(var, val, ctxt);
	string_deref(val);

	/*
	 * Close the input side of the pipe.
	 */
	close(fds[0]);
    }
    if (freeCmd)
	free((void *)args[2]);
}

/*-
 *---------------------------------------------------------------------
 * Parse_DoVar  --
 *	Take the variable assignment in the passed line and do it in the
 *	global context.
 *
 *	Note: There is a lexical ambiguity with assignment modifier characters
 *	in variable names. This routine interprets the character before the =
 *	as a modifier. Therefore, an assignment like
 *	    C++=/usr/bin/CC
 *	is interpreted as "C+ +=" instead of "C++ =".
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the variable structure of the given variable name is altered in the
 *	global context.
 *---------------------------------------------------------------------
 */
int
Parse_DoVar(char *line, GNode *ctxt)
{
    register char *cp;

    for (cp = line; *cp != '='; cp++)
	if (*cp == '\0' || *cp == ' ' || *cp == '\t')
	    return(1);

    *cp++ = '\0';
    ParseDoVar(line, 0, cp, ctxt);
    return(0);
}

/*-
 *---------------------------------------------------------------------
 * ParseVarOrDep  --
 *	Take the variable assignment in the passed line and do it in the
 *	global context.
 *
 *	Note: There is a lexical ambiguity with assignment modifier characters
 *	in variable names. This routine interprets the character before the =
 *	as a modifier. Therefore, an assignment like
 *	    C++=/usr/bin/CC
 *	is interpreted as "C+ +=" instead of "C++ =".
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the variable structure of the given variable name is altered in the
 *	global context.
 *---------------------------------------------------------------------
 */
int
ParseVarOrDep(Buffer buf, char **linePtr, int *modifierPtr)
{
    register char *cp;		/* pointer into line */
    char	*line = *linePtr;
    Boolean	oweSpace;

    *modifierPtr = '\0';

    /*
     * Skip to operator character, nulling out whitespace as we go
     */
    cp = line;
    oweSpace = FALSE;
    for (;;) {
	while (*cp == ' ' || *cp == '\t')
	    cp++;
	for (;;) {
	    if (*cp == '$') {
		char	*result;
		int	length;
		Boolean	freeIt;

		result = Var_Parse(cp, VAR_CMD, TRUE, &length, &freeIt);
		if (result == var_Error && !oldVars) {
		    Parse_Error(PARSE_FATAL, "Bad embedded variable");
		    return(1);
		}
		if (oweSpace) {
		    Buf_AddByte(buf, (Byte) ' ');
		    oweSpace = FALSE;
		}
		Buf_AddBytes(buf, strlen(result), (Byte *)result);
		if (freeIt)
		    free(result);
		cp += length;
		continue;
	    }
	    if (*cp == '=')
		break;
	    if (*cp == ' ' || *cp == '\t')
		break;
	    if (*cp == '\0') {
		Parse_Error(PARSE_FATAL, "\"%s\" unrecognized input", line);
		return(1);
	    }
	    if (*(cp+1) == '=') {
		if (*cp == '+' || *cp == '?' || *cp == ':' || *cp == '!')
		    *modifierPtr = (int) *cp++;
		else
		    Buf_AddByte(buf, (Byte) *cp++);
		break;
	    }
	    if (*cp == '!') {
		*modifierPtr = (int) *cp;
		*cp = ':';
		break;
	    }
	    if (*cp == ':') {
		if (*(cp+1) == ':')
		    *modifierPtr = (int) *cp++;
		break;
	    }
	    if (oweSpace) {
		Buf_AddByte(buf, (Byte) ' ');
		oweSpace = FALSE;
	    }
	    Buf_AddByte(buf, (Byte) *cp++);
	}
	if (*cp == '=' || *cp == ':')
	    break;
	oweSpace = TRUE;
	cp++;
    }

    /*
     * Handle empty lhs
     */
    if (Buf_Size(buf) == 0 && *cp == '=') {
	Parse_Error(PARSE_FATAL, "\"%s\" null variable assignment", line);
	return(1);
    }

    *linePtr = cp;
    return(0);
}

/*-
 * ParseAddCmd  --
 *	Lst_ForEach function to add a command line to all targets
 *
 * Results:
 *	Always 0
 *
 * Side Effects:
 *	A new element is added to the commands list of the node.
 */
static int
ParseAddCmd(
    ClientData gnCD,	/* the node to which the command is to be added */
    ClientData cmd)	/* the command to add */
{
    GNode *gn = (GNode *)gnCD;

    /* if target already supplied, ignore commands */
    if ((gn->type & OP_HAS_COMMANDS) == 0)
	(void)Lst_AtEnd(gn->commands, (ClientData)string_ref((string_t)cmd));
    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * ParseHasCommands --
 *	Callback procedure for Parse_File when destroying the list of
 *	targets on the last dependency line. Marks a target as already
 *	having commands if it does, to keep from having shell commands
 *	on multiple dependency lines.
 *
 * Results:
 *	Always 0.
 *
 * Side Effects:
 *	OP_HAS_COMMANDS may be set for the target.
 *
 *-----------------------------------------------------------------------
 */
static void
ParseHasCommands(ClientData gnCD)
{
    GNode *gn = (GNode *)gnCD;	    /* Node to examine */

    if (!Lst_IsEmpty(gn->commands))
	gn->type |= OP_HAS_COMMANDS;
}

/*-
 *-----------------------------------------------------------------------
 * Parse_AddIncludeDir --
 *	Add a directory to the path searched for included makefiles
 *	bracketed by double-quotes. Used by functions in main.c
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The directory is appended to the list.
 *
 *-----------------------------------------------------------------------
 */
void
Parse_AddIncludeDir (string_t dir)    /* The name of the directory to add */
{
    Dir_AddDir (parseIncPath, dir);
}

/*-
 *---------------------------------------------------------------------
 * ParseDoInclude  --
 *	Push to another file.
 *	
 *	The input is the line minus the #include. A file spec is a string
 *	enclosed in <> or "". The former is looked for only in sysIncPath.
 *	The latter in . and the directories specified by -I command line
 *	options
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	A structure is added to the includes Lst and readProc, lineno,
 *	fname and curFILE are altered for the new file
 *---------------------------------------------------------------------
 */
static void
ParseDoInclude (char *file,	/* file specification */
		Boolean ignoreFailure)
{
    string_t      fullname;	/* full pathname of file */
    IFile         *oldFile;	/* state associated with current file */
    char          endc;	    	/* the character which ends the file spec */
    char          *cp;		/* current position in file spec */
    Boolean 	  isSystem; 	/* TRUE if makefile is a system makefile */
    string_t	  sfile;

    /*
     * Skip to delimiter character so we know where to look
     */
    while ((*file == ' ') || (*file == '\t')) {
	file++;
    }

    if ((*file != '"') && (*file != '<')) {
	Parse_Error (PARSE_FATAL,
		     ".%sinclude filename must be delimited by '\"' or '<'",
		     ignoreFailure ? "try" : "");
	return;
    }

    /*
     * Set the search path on which to find the include file based on the
     * characters which bracket its name. Angle-brackets imply it's
     * a system Makefile while double-quotes imply it's a user makefile
     */
    if (*file == '<') {
	isSystem = TRUE;
	endc = '>';
    } else {
	isSystem = FALSE;
	endc = '"';
    }

    /*
     * Skip to matching delimiter
     */
    for (cp = ++file; *cp && *cp != endc; cp++) {
	continue;
    }

    if (*cp != endc) {
	Parse_Error (PARSE_FATAL,
		     "Unclosed .%sinclude filename. '%c' expected",
		     ignoreFailure ? "try" : "", endc);
	return;
    }
    *cp = '\0';

    /*
     * Substitute for any variables in the file name before trying to
     * find the thing.
     */
    if (strchr(file, '$') != (char *) NULL)
	file = Var_Subst (file, VAR_CMD, FALSE);

    /*
     * Now we know the file's name and its search path, we attempt to
     * find the durn thing. A return of NULL indicates the file don't
     * exist.
     */
    sfile = string_create(file);
    if (makeincludecompat) {
	fullname = Dir_FindFile(sfile, dirSearchPath);
	if (fullname == (string_t) NULL)
	    fullname = Dir_FindFile (sfile, parseIncPath);
	if (fullname == (string_t) NULL)
	    fullname = Dir_FindFile(sfile, sysIncPath);
    } else {
	if (!isSystem) {
	    fullname = Dir_FindFile(sfile, dirSearchPath);
	} else {
	    fullname = Dir_FindFile (sfile, parseIncPath);
	    if (fullname == (string_t) NULL)
		fullname = Dir_FindFile(sfile, sysIncPath);
	}
    }

    if (fullname == (string_t) NULL) {
	if (!ignoreFailure)
	    Parse_Error (PARSE_FATAL, "Could not find %s", file);
	return;
    }

    /*
     * Once we find the absolute path to the file, we get to save all the
     * state from the current file before we can start reading this
     * include file. The state is stored in an IFile structure which
     * is placed on a list with other IFile structures. The list makes
     * a very nice stack to track how we got here...
     */
    oldFile = (IFile *) emalloc (sizeof (IFile));
    oldFile->fname = fname;

    oldFile->F = curFILE;
    oldFile->lineno = lineno;

    (void) Lst_AtFront (includes, (ClientData)oldFile);

    /*
     * Once the previous state has been saved, we can get down to reading
     * the new file. We set up the name of the file to be the absolute
     * name of the include file so error messages refer to the right
     * place. Naturally enough, we start reading at line number 0.
     */
    fname = fullname;
    lineno = 0;

    curFILE = fopen (fullname->data, "r");
    if (curFILE == (FILE * ) NULL) {
	Parse_Error (PARSE_FATAL, "Cannot open %s", fullname->data);
	/*
	 * Pop to previous file
	 */
	(void) ParseEOF(0);
    }
    if (DEBUG(SUFF)) {
	printf("ParseDoInclude: now reading %s\n", fullname->data);
	fflush(stdout);
    }
}

/*-
 *---------------------------------------------------------------------
 * ParseEOF  --
 *	Called when EOF is reached in the current file. If we were reading
 *	an include file, the includes stack is popped and things set up
 *	to go back to reading the previous file at the previous location.
 *
 * Results:
 *	CONTINUE if there's more to do. DONE if not.
 *
 * Side Effects:
 *	The old curFILE, is closed. The includes list is shortened.
 *	lineno, curFILE, and fname are changed if CONTINUE is returned.
 *---------------------------------------------------------------------
 */
static int
ParseEOF (int opened)
{
    IFile     *ifile;	/* the state on the top of the includes stack */

    if (DEBUG(SUFF)) {
	printf("ParseEOF: %s\n", fname->data);
    }
    if (Lst_IsEmpty (includes)) {
	return (DONE);
    }

    ifile = (IFile *) Lst_DeQueue (includes);
    string_deref (fname);
    fname = ifile->fname;
    lineno = ifile->lineno;
    if (opened)
	(void) fclose (curFILE);
    curFILE = ifile->F;
    free ((Address)ifile);
    return (CONTINUE);
}

/*-
 *---------------------------------------------------------------------
 * ParseReadc  --
 *	Read a character from the current file and update the line number
 *	counter as necessary
 *
 * Results:
 *	The character that was read
 *
 * Side Effects:
 *	The lineno counter is incremented if the character is a newline
 *---------------------------------------------------------------------
 */
#define ParseReadc() (getc(curFILE))

/*-
 *---------------------------------------------------------------------
 * ParseReadLine --
 *	Read an entire line from the input file. Called only by Parse_File.
 *	To facilitate escaped newlines and what have you, a character is
 *	buffered in 'lastc', which is '\0' when no characters have been
 *	read. When we break out of the loop, c holds the terminating
 *	character and lastc holds a character that should be added to
 *	the line (unless we don't read anything but a terminator).
 *
 * Results:
 *	A line w/o its newline
 *
 * Side Effects:
 *	Only those associated with reading a character
 *---------------------------------------------------------------------
 */

static Boolean
ParseBlankLines (int *cptr)
{
    register int c;

    /*
     * Discard completely blank and comment lines.
     */
    for (;;) {
	c = ParseReadc();
	if (c == EOF)
	    return(FALSE);
	if (!parseMeta[c])
	    break;
	if (c == '\n') {
	    lineno++;
	    continue;
	}
	if (c != '#')
	    break;
	for (;;) {
	    int lastc;
	    lastc = c;
	    c = ParseReadc();
	    if (c == EOF)
		return(FALSE);
	    if (c == '\n') {
		lineno++;
		if (lastc != '\\')
		    break;
	    }
	}
    }
    *cptr = c;
    return(TRUE);
}

static Boolean
ParseReadLine (register Buffer buf)
{
    int  c;		      	/* the current character */

    if (!ParseBlankLines(&c))
	return(FALSE);

    /*
     * Handle special-characters at the beginning of the line. Either a
     * leading tab (shell command) or dot (possible conditional)
     * forces us to ignore comments and dependency operators and treat
     * semi-colons as semi-colons (by leaving semiNL FALSE).
     */
    if (c == '\t') {
	Buf_AddByte (buf, (Byte)c);
	while ((c = ParseReadc()) == ' ' || c == '\t')
	    ;
	while (c != EOF) {
	    /*
	     * Nothing special
	     */
	    if (!parseMeta[c]) {
		Buf_AddByte (buf, (Byte)c);
		c = ParseReadc();
		continue;
	    }
	    /*
	     * Unescaped newline - we are done
	     */
	    if (c == '\n') {
		lineno++;
		return(TRUE);
	    }
	    /*
	     * Maybe escaped newline?
	     */
	    if (c != '\\') {
		Buf_AddByte (buf, (Byte)c);
		c = ParseReadc();
		continue;
	    }
	    c = ParseReadc();
	    if (c != '\n') {
		Buf_AddByte (buf, (Byte)'\\');
		Buf_AddByte (buf, (Byte)c);
		c = ParseReadc();
		continue;
	    }
	    /*
	     * Escaped newline: read characters until a non-space or an
	     * unescaped newline and replace them all by a single space.
	     * This is done by storing the space over the backslash and
	     * dropping through with the next nonspace. If it is a
	     * semi-colon and semiNL is TRUE, it will be recognized as a
	     * newline in the code below this...
	     */
	    lineno++;
	    while ((c = ParseReadc()) == ' ' || c == '\t')
		;
	    Buf_AddByte (buf, (Byte)' ');
	}
	return(FALSE);
    }
    Buf_AddByte (buf, (Byte)c);
    c = ParseReadc();
    while (c != EOF) {
	/*
	 * Nothing special
	 */
	if (!parseMeta[c]) {
	    Buf_AddByte (buf, (Byte)c);
	    c = ParseReadc();
	    continue;
	}
	/*
	 * Unescaped newline - we are done
	 */
	if (c == '\n') {
	    lineno++;
	    return(TRUE);
	}
	/*
	 * Maybe escaped newline?
	 */
	if (c == '\\') {
	    c = ParseReadc();
	    if (c == '\n') {
		/*
		 * Escaped newline: read characters until a non-space or an
		 * unescaped newline and replace them all by a single space.
		 * This is done by storing the space over the backslash and
		 * dropping through with the next nonspace. If it is a
		 * semi-colon and semiNL is TRUE, it will be recognized as a
		 * newline in the code below this...
		 */
		lineno++;
		while ((c = ParseReadc()) == ' ' || c == '\t')
		    ;
		Buf_AddByte (buf, (Byte)' ');
		continue;
	    }
	    Buf_AddByte (buf, (Byte)'\\');
	}
	/*
	 * Not a comment...
	 */
	if (c != '#') {
	    Buf_AddByte (buf, (Byte)c);
	    c = ParseReadc();
	    continue;
	}
	/*
	 * If the character is a hash mark, this is a comment.
	 * Skip to the end of the line.
	 */
	for (;;) {
	    c = ParseReadc();
	    if (c == EOF)
		return(FALSE);
	    if (c == '\n') {
		lineno++;
		return(TRUE);
	    }
	    while (c == '\\') {
		c = ParseReadc();
		if (c == '\n') {
		    lineno++;
		    break;
		}
		if (c == EOF)
		    return(FALSE);
	    }
	}
    }
    return(FALSE);
}

/*-
 *-----------------------------------------------------------------------
 * ParseFinishLine --
 *	Handle the end of a dependency group.
 *
 * Results:
 *	Nothing.
 *
 * Side Effects:
 *	inLine set FALSE. 'targets' list destroyed.
 *
 *-----------------------------------------------------------------------
 */
static void
ParseFinishLine(void)
{
    if (inLine) {
	Lst_ForEach(targets, Suff_EndTransform, (ClientData)NULL);
	Lst_Destroy (targets, ParseHasCommands);
	inLine = FALSE;
    }
}
		    

/*-
 *---------------------------------------------------------------------
 * Parse_File --
 *	Parse a file into its component parts, incorporating it into the
 *	current dependency graph. This is the main function and controls
 *	almost every other function in this module
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Loads. Nodes are added to the list of all targets, nodes and links
 *	are added to the dependency graph. etc. etc. etc.
 *---------------------------------------------------------------------
 */
void
Parse_File(
    string_t      name,		/* the name of the file being read */
    FILE *	  stream)   	/* Stream open to makefile to parse */
{
    register char *cp;		/* pointer into the line */
    char	*line;		/* the line we're working on */
    Buffer	buf;	    	/* Buffer for current line */
    Buffer	parseBuf;
    int		lineLength;	/* Length of result */
    int		modifier;

    inLine = FALSE;
    fname = name;
    curFILE = stream;
    lineno = 0;
    fatals = 0;

    buf = Buf_Init(0);
    parseBuf = Buf_Init(0);

    do {
	while (ParseReadLine (buf)) {
	    line = (char *)Buf_GetBase (buf);
	    lineLength = Buf_Size (buf);
	    if (*line == '.' && !isupper(line[1])) {
		/*
		 * Lines that begin with the special character are either
		 * include, undef or conditional directives.
		 */
		for (cp = line + 1; *cp == ' ' || *cp == '\t'; cp++) {
		    continue;
		}
		if (*cp && cp[1] == 'n' && strncmp(cp, "include", 7) == 0) {
		    ParseDoInclude (cp + 7, FALSE);
		    Buf_Discard (buf, lineLength);
		    continue;
		}
		if (*cp == 't' && strncmp(cp, "tryinclude", 10) == 0) {
		    ParseDoInclude (cp + 10, TRUE);
		    Buf_Discard (buf, lineLength);
		    continue;
		}
#ifdef SNI_MIPSMAG
		} else if (strncmp(cp, "MAKEFLAGS", 9) == 0) {
printf("This seems wrong - explain this to OSF !!!\n");
		    cp += 10;
		    Var_Append(".MAKEFLAGS", cp, VAR_GLOBAL);
		    goto nextLine;
#endif /* SNI_MIPSMAG */
		if (*cp == 'u' && strncmp(cp, "undef", 5) == 0) {
		    char *cp2;
		    for (cp += 5; *cp == ' ' || *cp == '\t'; cp++)
			continue;

		    for (cp2 = cp; *cp2 && *cp2 != ' ' && *cp2 != '\t'; cp2++)
			continue;

		    *cp2 = '\0';

		    Var_Delete(string_create(cp), VAR_GLOBAL);
		    Buf_Discard (buf, lineLength);
		    continue;
		}
		/*
		 * The line might be a conditional. Ask the conditional module
		 * about it and act accordingly
		 */
		switch (Cond_Eval (cp)) {
		case 0:
		    break;
		case 1:
		    Buf_Discard (buf, lineLength);
		    continue;
		default:
		    goto next;
		}
		/*
		 * Skip to next conditional that evaluates to 1.
		 */
		do {
		    int c = '\0';
		    Buf_Discard (buf, lineLength);
		    for (;;) {
			int lastc;
			lastc = c;
			c = ParseReadc();
			/*
			 * Skip lines until get to one that begins with a
			 * special char (dot).
			 */
			if ((c == '.') || (c == EOF))
			    break;
			while ((c != '\n' || lastc == '\\') && c != EOF) {
			    /*
			     * Advance to next unescaped newline
			     */
			    if ((lastc = c) == '\n')
				lineno++;
			    c = ParseReadc();
			}
			lineno++;
		    }
		    if (c == EOF || !ParseReadLine (buf)) {
			Parse_Error (PARSE_FATAL, "Unclosed conditional");
			goto eof;
		    }

		    line = (char *)Buf_GetBase(buf); 
		    lineLength = Buf_Size (buf);
		} while (Cond_Eval(line) != 1);
	        Buf_Discard (buf, lineLength);
		continue;
	    }
	    if (*line == '\t')
	    {
		/*
		 * If a line starts with a tab, it
		 * can only hope to be a creation command.
		 */
		cp = line + 1;
		if (*cp) {
		    if (inLine) {
			/*
			 * So long as it's not a blank line and we're actually
			 * in a dependency spec, add the command to the list of
			 * commands of all targets in the dependency spec 
			 */
			Lst_ForEach (targets, ParseAddCmd,
				     (ClientData)string_create(cp));
		    } else {
			Parse_Error (PARSE_FATAL,
				     "Unassociated shell command \"%.20s\"",
				     cp);
		    }
		}
		Buf_Discard (buf, lineLength);
		continue;
	    }
	    for (cp = line; *cp == ' ' || *cp == '\t'; cp++)
		continue;
	    if (*cp == '\0') {
		Buf_Discard (buf, lineLength);
		continue;
	    }
	next:
	    ParseFinishLine();

	    /*
	     * Parse the line as a variable assignment or dependency line.
	     * On success, parseBuf will either contain the variable name
	     * or the list of targets, each separated by a single space.
	     * line will be updated to point to either an equal-sign (=) or
	     * colon (:).  modifier, if non-null, will contain either the
	     * variable or dependency modifier character.
	     */
	    if (ParseVarOrDep (parseBuf, &line, &modifier) != 0) {
		Buf_Discard (buf, lineLength);
		if ((lineLength = Buf_Size (parseBuf)))
		    Buf_Discard (parseBuf, lineLength);
		continue;
	    }

	    /*
	     * Handle the variable assignment
	     */
	    if (*line == '=') {
		cp = (char *) Buf_GetBase (parseBuf);
		ParseDoVar (cp, modifier, ++line, VAR_GLOBAL);
		Buf_Discard (buf, lineLength);
		Buf_Discard (parseBuf, Buf_Size(parseBuf));
		continue;
	    }

	    /*
	     * Need a non-circular list for the target nodes 
	     */
	    targets = Lst_Init();
	    inLine = TRUE;

	    for (cp = line + 1; *cp == ' ' || *cp == '\t'; cp++)
		;
	    ParseDoDependency (parseBuf, modifier, cp);

	    if (haveLookahead) {
		cp = (char *) Buf_GetBase (lookaheadBuf);
		Lst_ForEach (targets, ParseAddCmd,
			     (ClientData)string_create(cp));
		Buf_Discard(lookaheadBuf, Buf_Size(lookaheadBuf));
		haveLookahead = FALSE;
	    }

	    Buf_Discard (buf, lineLength);
	    Buf_Discard (parseBuf, Buf_Size(parseBuf));
	}
    eof:;
	/*
	 * Reached EOF, but it may be just EOF of an include file... 
	 */
    } while (ParseEOF(1) == CONTINUE);

    Buf_Destroy (buf);

    /*
     * Make sure conditionals are clean
     */
    Cond_End();

    if (fatals) {
	fprintf (stderr, "Fatal errors encountered -- cannot continue\n");
	exit (1);
    }
}

/*-
 *---------------------------------------------------------------------
 * Parse_Init --
 *	initialize the parsing module
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the parseIncPath list is initialized...
 *---------------------------------------------------------------------
 */
void
Parse_Init (void)
{
    char *cp, *start;
    char *syspath;
    string_t start_str;
    
    mainNode = NILGNODE;
    parseIncPath = Lst_Init();
    sysIncPath = Lst_Init();
    includes = Lst_Init();

    lookaheadBuf = Buf_Init(0);

    for (cp = (char *)" \t\\\n#$;("; *cp; cp++)
	parseMeta[*(unsigned char *)cp] = 1;
    parseMeta['\0'] = 1;

    makeincludecompat = (getenv("MAKEINCLUDECOMPAT") != NULL);
    if ((syspath = getenv("MAKESYSPATH")) == NULL)
	syspath = strdup(_PATH_DEFSYSPATH);
    else
	syspath = strdup(syspath);
    if (syspath == NULL)
	enomem();

    /*
     * Add the directories from the syspath (more than one may be given
     * as dir1:...:dirn) to the system include path.
     */
    for (start = syspath; *start != '\0'; start = cp) {
	for (cp = start; *cp != '\0' && *cp != ':'; cp++) {
	    ;
	}
	if (*cp != '\0')
	    *cp++ = '\0';
	start_str = string_create(start);
	Dir_AddDir(sysIncPath, start_str);
    }
    free(syspath);
}

/*-
 *-----------------------------------------------------------------------
 * Parse_MainName --
 *	Return a Lst of the main target to create for main()'s sake. If
 *	no such target exists, we Punt with an obnoxious error message.
 *
 * Results:
 *	A Lst of the single node to create.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
Lst
Parse_MainName(void)
{
    Lst           main;	/* result list */

    main = Lst_Init();

    if (mainNode == NILGNODE) {
	Punt ("make: no target to make.\n");
    	/*NOTREACHED*/
    } else if (mainNode->type & OP_DOUBLEDEP) {
	Lst_Concat(main, mainNode->cohorts, LST_CONCNEW);
    }
    (void) Lst_AtEnd (main, (ClientData)mainNode);
    return (main);
}

void
ParseErrorCond(const char *fmt, ...)
{
    va_list ap;

    (void)fflush(stdout);
    (void)fprintf(stderr, "\"%s\", line %d: ", fname->data, lineno);
    va_start(ap, fmt);
    (void)vfprintf(stderr, fmt, ap);
    va_end(ap);
    (void)fprintf(stderr, "\n");
    (void)fflush(stderr);
    fatals += 1;
}
