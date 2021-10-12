#ifndef lint
static char sccsid[] = "@(#)21   1.11  src/bos/usr/ccs/bin/make/parse.c, cmdmake, bos41J, 9514A_all  3/29/95  09:53:32";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: ParseAddCmd
 *		ParseDoDependency
 *		ParseDoInclude
 *		ParseDoOp
 *		ParseDoSrc
 *		ParseEOF
 *		ParseFindKeyword
 *		ParseFindMain
 *		ParseFinishLine
 *		ParseHasCommands
 *		ParseLinkSrc
 *		ParseReadLine
 *		ParseReadc
 *		Parse_DoVar
 *		Parse_Error
 *		Parse_File
 *		Parse_Init
 *		Parse_IsVar
 *		Parse_MainName
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1995
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
static char rcsid[] = "@(#)$RCSfile: parse.c,v $ $Revision: 1.2.6.3 $ (OSF) $Date: 1992/11/09 19:23:30 $";
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
 *	the #include "..." function are kept in the 'parseIncPath' Lst, while
 *	those for the #include <...> are kept in the 'sysIncPath' Lst. The
 *	targets currently being defined are kept in the 'targets' Lst.
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
 *	Parse_IsVar	    	    Returns TRUE if the given line is a
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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include "make.h"
#include "buf.h"
#include "pathnames.h"

static void ParseDoInclude(const char *file);

extern int mb_cur_max;

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

static char    	    *fname;	/* name of current file (for errors) */
static int          lineno;	/* line number in current file */
static FILE   	    *curFILE; 	/* current makefile */

static int	    fatals = 0;

static GNode	    *mainNode;	/* The main target to create. This is the
				 * first target on the first dependency
				 * line in the first makefile */
/*
 * Definitions for handling #include specifications
 */
typedef struct IFile {
    char           *fname;	    /* name of previous file */
    int             lineno;	    /* saved line number */
    FILE *       F;		    /* the open stream */
}              	  IFile;

static Lst      includes;  	/* stack of IFiles generated by
				 * #includes */

/*-
 * specType contains the SPECial TYPE of the current target. It is
 * Not if the target is unspecial. If it *is* special, however, the children
 * are linked as children of the parent but not vice versa. This variable is
 * set in ParseDoDependency
 */
typedef enum {
    Default,	    /* .DEFAULT */
    Ignore,	    /* .IGNORE */
    Not,	    /* Not special */
    Posix,	    /* .POSIX */
    Precious,	    /* .PRECIOUS */
    Silent,	    /* .SILENT */
    Suffixes	    /* .SUFFIXES */
} ParseSpecial;

static ParseSpecial specType;

/*
 * The parseKeywords table is searched using binary search when deciding
 * if a target or source is special. The 'spec' field is the ParseSpecial
 * type of the keyword ("Not" if the keyword isn't special as a target) while
 * the 'op' field is the operator to apply to the list of targets if the
 * keyword is used as a source ("0" if the keyword isn't special as a source)
 */
static struct {
    char    	  *name;    	/* Name of keyword */
    ParseSpecial  spec;	    	/* Type when used as a target */
    int	    	  op;	    	/* Operator when used as a source */
} parseKeywords[] = {
{ ".DEFAULT",	  Default,  	0 },
{ ".IGNORE",	  Ignore,   	OP_IGNORE },
{ ".POSIX",	  Posix, 	0 },
{ ".PRECIOUS",	  Precious, 	OP_PRECIOUS },
{ ".SILENT",	  Silent,   	OP_SILENT },
{ ".SUFFIXES",	  Suffixes, 	0 },
};

static Boolean maybePosix = TRUE;
static Boolean isPosix = FALSE;

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
ParseFindKeyword (
    char	    *str		/* String to find */
    )
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
static void
Parse_Error(int type, const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, MSGSTR(PARSERR1, 
		"\"%s\", line %d: "), fname, lineno);
	if (type == PARSE_WARNING)
		(void)fprintf(stderr, MSGSTR(PARSEWARN, "warning: "));
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n',stderr);
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
    GNode          *pgn,	/* The parent node */
    GNode          *cgn		/* The child node */
    )
{
    if (Lst_Member (pgn->children, (ClientData)cgn) == NILLNODE) {
	(void)Lst_AtEnd (pgn->children, (ClientData)cgn);
	if (specType == Not) {
	    (void)Lst_AtEnd (cgn->parents, (ClientData)pgn);
	}
	pgn->unmade += 1;
    }
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
    GNode          *gn,		/* The node to which the operator is to be
				 * applied */
    int             op		/* The operator to apply */
    )
{
    /*
     * If the dependency mask of the operator and the node don't match and
     * the node has actually had an operator applied to it before, and
     * the operator actually has some dependency information in it, complain. 
     */
    if (((op & OP_OPMASK) != (gn->type & OP_OPMASK)) &&
	!OP_NOP(gn->type) && !OP_NOP(op))
    {
	Parse_Error (PARSE_FATAL, MSGSTR(BADRULE1, "make: "
		"The dependency lines for target %s cannot contain\n"
		"both single and double colons."), gn->name);
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
    int		tOp,	/* operator (if any) from special targets */
    char	*src	/* name of the source to handle */
    )
{
    GNode	*gn;

    /*
     * We need to find/create a node for the source.  After
     * that we can apply any operator to it
     * from a special target or link it to its parents, as
     * appropriate.
     *
     * In the case of a source that was the object of a :: operator,
     * the attribute is applied to all of its instances (as kept in
     * the 'cohorts' list of the node) or all the cohorts are linked
     * to all the targets.
     */
    gn = Targ_FindNode (src, TARG_CREATE);
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
ParseFindMain(
    GNode   	  *gn	    /* Node to examine */
    )
{
    if ((gn->type & (OP_NOTMAIN|OP_TRANSFORM)) == 0) {
	mainNode = gn;
	Targ_SetMain(gn);
	return (1);
    } else {
	return (0);
    }
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
ParseDoDependency (
    char           *line	/* the line to parse */
    )
{
    extern Boolean     noBuiltins;
    register char  *cp;		/* our current position */
    register GNode *gn;		/* a general purpose temporary node */
    register int    op;		/* the operator on the line */
    char            savec;	/* a place to save a character */
    int	    	    tOp;    	/* operator from special target */
    Lst	    	    sources;	/* list of source names after expansion */
    Lst 	    curTargs;	/* list of target names to be found and added
				 * to the targets list */
    char	   *savedline = line; /* save a copy of dependency line */

    tOp = 0;

    specType = Not;

    curTargs = Lst_Init(FALSE);
    
    do {
	for (cp = line;
	     *cp && !Mbyte_isspace(cp) && (*cp != ':') && (*cp != '(');
	     cp += MBLENF(cp))
	{
	    continue;
	}
	if (*cp == '(') {
	    /*
	     * Archives must be handled specially to make sure the OP_ARCHV
	     * flag is set in their 'type' field, for one thing, and because
	     * things like "archive(file1.o file2.o file3.o)" are permissible.
	     * Arch_ParseArchive will set 'line' to be the first non-blank
	     * after the archive-spec. It creates/finds nodes for the members
	     * and places them on the given list, returning SUCCESS if all
	     * went well and FAILURE if there was an error in the
	     * specification. On error, line should remain untouched.
	     */
	    if (Arch_ParseArchive (&line, targets, VAR_CMD) != SUCCESS) {
		Parse_Error (PARSE_FATAL,
			     MSGSTR(ARCHERR1, "make: "
				"Archive not specified correctly: %s"), line);
		return;
	    } else {
		cp = line;
		continue;
	    }
	}
	savec = *cp;
	
	if (!*cp) {
	    /*
	     * Ending a dependency line without an operator is a Bozo
	     * no-no 
	     */
	    Parse_Error (PARSE_FATAL, MSGSTR(PARSERR2, 
		"make: Dependency line needs colon or double "
		"colon operator."));
	    return;
	}
	*cp = '\0';
	/*
	 * Have a word in line. See if it's a special target and set
	 * specType to match it.
	 */
	if (*line == '.' && Mbyte_isupper (&line[1])) {
	    /*
	     * See if the target is a special target that must have it
	     * or its sources handled specially. 
	     */
	    int keywd = ParseFindKeyword(line);
	    if (keywd != -1) {
		specType = parseKeywords[keywd].spec;
		tOp = parseKeywords[keywd].op;

		/*
		 * Certain special targets have special semantics:
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
		 *	    	    	use Make_HandleTransform to actually
		 *	    	    	apply the .DEFAULT commands.
		 */
		switch (specType) {
		    case Default:
			gn = Targ_NewGN(".DEFAULT");
			gn->type |= (OP_NOTMAIN|OP_TRANSFORM);
			(void)Lst_AtEnd(targets, (ClientData)gn);
			DEFAULT = gn;
			break;
		    case Posix:
			if (maybePosix) {
			    isPosix = TRUE;

			    /* If built-in rules are being used then
			    bring in "posix.mk" so our default rules
			    are conforming to XPG4. */
			    if (noBuiltins == FALSE)
			    {
				ParseDoInclude(_PATH_DEFSYSMK);
			    }
			}
			else
			    Parse_Error(PARSE_WARNING, 
				MSGSTR(POSIXERR, ".POSIX directive is "
					"not first non-comment line."));
		    default:
			break;
		}
	    }
	}
	
	/*
	 * Have word in line. Get or create its node and stick it at
	 * the end of the targets list 
	 */
	if ((specType == Not) && (*line != '\0')) {
	    (void)Lst_AtEnd(curTargs, (ClientData)line);
	    
	    while(!Lst_IsEmpty(curTargs)) {
		char	*targName = (char *)Lst_DeQueue(curTargs);
		
		if (!Suff_IsTransform (targName)) {
		    gn = Targ_FindNode (targName, TARG_CREATE);
		} else {
		    gn = Suff_AddTransform (targName);
		}
		
		(void)Lst_AtEnd (targets, (ClientData)gn);
	    }
	}
	
	*cp = savec;
	/*
	 * If it is a special type and not .PATH, it's the only target we
	 * allow on this line...
	 */
	if (specType != Not) {
	    Boolean warn = FALSE;
	    
	    while ((*cp != '!') && (*cp != ':') && *cp) {
		if (*cp != ' ' && *cp != '\t') {
		    warn = TRUE;
		}
		cp++;
	    }
	    if (warn) {
		Parse_Error(PARSE_WARNING, MSGSTR(EXTARGET, 
			"Extra target ignored"));
	    }
	} else {
	    while (*cp && Mbyte_isspace (cp)) {
		cp += MBLENF(cp);
	    }
	}
	line = cp;
    } while ((*line != ':') && *line);

    /*
     * Don't need the list of target names anymore...
     */
    Lst_Destroy(curTargs, NOFREE);

    if (!Lst_IsEmpty(targets)) {
	switch(specType) {
	    default:
		Parse_Error(PARSE_WARNING, MSGSTR(MIXTARGET, 
		     "Special targets cannot be mixed with regular targets.\n"
		     "Regular targets ignored."));
		break;
	    case Default:
	    case Not:
		/*
		 * Nothing special here -- targets can be empty if it wants.
		 */
		break;
	}
    }

    /*
     * Have now parsed all the target names. Must parse the operator next. The
     * result is left in  op .
     */
    if (*cp == ':') {
	if (cp[1] == ':') {
	    op = OP_DOUBLEDEP;
	    cp++;
	} else {
	    op = OP_DEPENDS;
	}
    } else {
	Parse_Error (PARSE_FATAL, MSGSTR(PARSERR2, "make: "
		"Dependency line needs colon or double colon operator."));
	return;
    }

    cp++;			/* Advance beyond operator */

    Lst_ForEach (targets, ParseDoOp, (ClientData)op);

    /*
     * Get to the first source 
     */
    while (*cp && Mbyte_isspace (cp)) {
	cp += MBLENF(cp);
    }
    line = cp;

    /*
     * Several special targets take different actions if present with no
     * sources:
     *	a .SUFFIXES line with no sources clears out all old suffixes
     *	a .PRECIOUS line makes all targets precious
     *	a .IGNORE line ignores errors for all targets
     *	a .SILENT line creates silence when making all targets
     */
    if (!*line) {
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
	    default:
		break;
	}
    }
    
    /*
     * NOW GO FOR THE SOURCES 
     */
    if (specType == Suffixes)
    {
	while (*line) {
	    /*
	     * If the target was one that doesn't take files as its sources
	     * but takes something like suffixes, we take each
	     * space-separated word on the line as a something and deal
	     * with it accordingly.
	     *
	     * If the target was .SUFFIXES, we take each source as a
	     * suffix and add it to the list of suffixes maintained by the
	     * Suff module.
	     */
	    char  savec;
	    while (*cp && !Mbyte_isspace (cp)) {
		cp += MBLENF(cp);
	    }
	    savec = *cp;
	    *cp = '\0';
	    Suff_AddSuffix (line);
	    *cp = savec;
	    if (savec != '\0') {
		cp++;
	    }
	    while (*cp && Mbyte_isspace (cp)) {
		cp += MBLENF(cp);
	    }
	    line = cp;
	}
    } else {
	while (*line) {
	    /* If we've got a opening backquote. */
	    if (*cp == '`')
	    {
		FILE	*command_pipe;
		char	command_line[LINE_MAX];	
		char	command_token[LINE_MAX];
		char	*cmd_tp;  /* command temp pointer */	
		int	index = 0;

		/* if we have a $@ construct within the backquotes,
		 * then there was a $$@ macro which we will have
		 * to interpret now. (The first $ was stripped by Var_Subst).
		 */
		if (NULL != (cmd_tp = strchr(cp,'$')))  
		{
		    /* There's a $ here; see if it is $@, ${@, or $(@. */
		    if (  (*(cmd_tp+1) == '@')   || 
		    ((*(cmd_tp+1) == '{') || (*(cmd_tp+1) == '(')
			&& (*(cmd_tp+2) == '@'))   )
		    {
		     /* We know the following: we are a dependency, we are
		     * in a backquote expression and must therefore be
		     * executed, and we have an unexpanded macro, $@,
		     * which must be expanded now. If we didn't expand it now,
		     * the value of $@ would have no meaning when the command
		     * within backquotes was executed.
		     * This is a little ugly, but this design of make did not
		     * leave a very clean way of dealing with this situation.
		     *   We will do this by: 
		     *   1. reading the target from input line.
		     *   2. exporting the target as the $@ macro.
		     *   3. substituting for macros on the input line.
		     */
			char *atSignTarget; /* pt to target name */
			char *atIndexPtr;   /* to traverse/copy target name */
			char *n_cp; /* tmp; "new" cp */

    			emalloc(atSignTarget,strlen(savedline));
			atIndexPtr = atSignTarget;

			/* find the and save the target name */
			for (cmd_tp = savedline; 
			    *cmd_tp && !Mbyte_isspace(cmd_tp) && 
			    (*cmd_tp != ':') && (*cmd_tp != '(');
			    cmd_tp += MBLENF(cmd_tp), 
			    atIndexPtr += MBLENF(atIndexPtr))
			{
			    *atIndexPtr = *cmd_tp;
			}
			    *atIndexPtr = '\0';

			/* put this in the environment */
			Var_Set ("@", atSignTarget, VAR_CMD);
			free(atSignTarget);

			/* do the substitution */
			n_cp = Var_Subst (cp, VAR_CMD, FALSE);
			free (cp);
			cp = n_cp;
			
			/* remove from the environment */
			/* (probably not necessary; just cleaning up...) */
			Var_Set ("@", NULL, VAR_CMD);
		    }
		}

		/* Loop forever (not really!). */
		while(1)
		{
		    cp++;

		    /* If we've found the closing backquote. */
		    if (*cp == '`')
		    {
			command_line[index]='\0';

			/* Open up the pipe to the command line. */
			if ((command_pipe=popen(command_line,"r")) ==
				NULL)
			{
			    Parse_Error(PARSE_FATAL,"make: popen(): %s",
				    strerror(errno));
			}

			/* While we can scan in tokens from the command
			   line. */
			while(fscanf(command_pipe,"%s",command_token) !=
				EOF)
			{
			    ParseDoSrc(tOp,command_token);
			}

			/* Close the pipe. */
			if (pclose(command_pipe) == -1)
			{
			    Parse_Error(PARSE_FATAL,"make: pclose(): %s",
				    strerror(errno));
			}

			cp++;
			break;
		    }

		    /* If we found the end of the prerequisites without
		       finding the closing backquote.  */
		    if (*cp == '\0')
		    {
			Parse_Error(PARSE_FATAL,"make: No closing '`'.: %s",line);
			break;
		    }

		    command_line[index]=*cp;
		    index++;
		}

		/* Skip trailing space. */
		while((*cp) && Mbyte_isspace(cp)) 
		{
		    cp+=MBLENF(cp);
		}

		line=cp;
		continue;
	    }

	    /*
	     * The targets take real sources, so we must beware of archive
	     * specifications (i.e. things with left parentheses in them)
	     * and handle them accordingly.
	     */
	    while (*cp && !Mbyte_isspace (cp)) {
		if ((*cp == '(') && (cp > line) && (cp[-1] != '$')) {
		    /*
		     * Only stop for a left parenthesis if it isn't at the
		     * start of a word (that'll be for variable changes
		     * later) and isn't preceded by a dollar sign (a dynamic
		     * source).
		     */
		    break;
		} else {
		    cp += MBLENF(cp);
		}
	    }

	    if (*cp == '(') {
		GNode	  *gn;

		sources = Lst_Init (FALSE);
		if (Arch_ParseArchive (&line, sources, VAR_CMD) != SUCCESS) {
		    Parse_Error (PARSE_FATAL,
			     MSGSTR(ARCHERR1, "make: "
				"Archive not specified correctly: %s"), line);
		    return;
		}

		while (!Lst_IsEmpty (sources)) {
		    gn = (GNode *) Lst_DeQueue (sources);
		    ParseDoSrc (tOp, gn->name);
		}
		Lst_Destroy (sources, NOFREE);
		cp = line;
	    } else {
		if (*cp) {
		    *cp = '\0';
		    cp += 1;
		}

		ParseDoSrc (tOp, line);
	    }
	    while (*cp && Mbyte_isspace (cp)) {
		cp += MBLENF(cp);
	    }
	    line = cp;
	}
    }
    
    if (mainNode == NILGNODE) {
	/*
	 * If we have yet to decide on a main target to make, in the
	 * absence of any user input, we want the first target on
	 * the first dependency line that is actually a real target.
	 */
	Lst_ForEach (targets, ParseFindMain, (ClientData)0);
    }

}

/*-
 *---------------------------------------------------------------------
 * Parse_IsVar  --
 *	Return TRUE if the passed line is a variable assignment. A variable
 *	assignment consists of a single word followed by optional whitespace
 *	followed by either a += or an = operator.
 *	This function is used both by the Parse_File function and main when
 *	parsing the command-line arguments.
 *
 * Results:
 *	TRUE if it is. FALSE if it ain't
 *
 * Side Effects:
 *	none
 *---------------------------------------------------------------------
 */

#pragma isolated_call(Parse_IsVar)

Boolean
Parse_IsVar (
    const char  *line	/* the line to check */
    )
{
    Boolean wasSpace = FALSE;	/* set TRUE if found a space */
    Boolean haveName = FALSE;	/* Set TRUE if have a variable name */

    /*
     * Skip to variable name
     */
    while ((*line == ' ') || (*line == '\t')) {
	line++;
    }

    while (*line != '=') {
	if (*line == '\0') {
	    /*
	     * end-of-line -- can't be a variable assignment.
	     */
	    return (FALSE);
	} else if ((*line == ' ') || (*line == '\t')) {
	    /*
	     * there can be as much white space as desired so long as there is
	     * only one word before the operator 
	     */
	    wasSpace = TRUE;
	} else if (wasSpace && haveName) {
	    /*
	     * This is the start of another word, so not assignment.
	     */
	    return (FALSE);
	} else if ((*line == '{') || (*line == '(')) {
	   /*
	    * We're inside a macro, so an '=' won't mean that we're a new
	    * variable -- at least while we're inside the parenthesis,
	    * we know it isn't.  So, let's keep track of parenthesis
	    * (in case they are embedded), and count forward until we're
	    * out of the { } or ( ).
	    */
	    ushort numParens = 1;
	    line++;

	    while (numParens > 0) {
		if (*line == '\0')
		    /* End of line, so not a variable.  'Course, they've
		     * got an unmatched paren, so Var_Subst will print
		     * them an error message when the macro is interpreted...
		     */
		    return(FALSE);
		else if ((*line == '{') || (*line == '('))
		    numParens++;
		else if ((*line == '}') || (*line == ')'))
		    numParens--;
		/* else it's something else, and we can just move along. */
		line++;
	    }
	    /* we have to back up one, because we'll still hit a "line++"
	     * at the end of the outermost while loop, and we don't want
	     * to skip a character without looking at it.
	     */
	    line--;
	} else {
	    haveName = TRUE; 
	    wasSpace = FALSE;
	}
	line++;
    }

    return (haveName);
}

/*-
 *---------------------------------------------------------------------
 * Parse_DoVar  --
 *	Take the variable assignment in the passed line and do it in the
 *	global context.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the variable structure of the given variable name is altered in the
 *	global context.
 *---------------------------------------------------------------------
 */
void
Parse_DoVar (
    char            *line,	/* a line guaranteed to be a variable
				 * assignment. This reduces error checks */
    const GNode   	    *ctxt    	/* Context in which to do the assignment */
    )
{
    char   *cp;	/* pointer into line */

    /*
     * Skip to variable name
     */
    while ((*line == ' ') || (*line == '\t')) {
	line++;
    }

    /*
     * Skip to equal sign, nulling out whitespace as we go
     */
    for (cp = line + 1; *cp != '='; cp ++) {
	if (Mbyte_isspace (cp)) {
	    *cp = '\0';
	}
    }
    *cp++ = '\0';	/* nuke the = */

    while (Mbyte_isspace (cp)) {
	cp += MBLENF(cp);
    }

    Var_Set (line, cp, ctxt);
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
	GNode *gn,	/* the node to which the command is to be added */
	char *cmd	/* the command to add */
	)
{
	/* if target already supplied, ignore commands */
	if (!(gn->type & OP_HAS_COMMANDS))
		(void)Lst_AtEnd(gn->commands, (ClientData)cmd);
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
static int
ParseHasCommands(
    GNode   	  *gn	    /* Node to examine */
    )
{
    if (!Lst_IsEmpty(gn->commands)) {
	gn->type |= OP_HAS_COMMANDS;
    }
    return(0);
}

/*-
 *---------------------------------------------------------------------
 * ParseDoInclude  --
 *	Push to another file.
 *	
 *	The input is the line minus the include.
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
ParseDoInclude (
    char          *file		/* file specification */
    )
{
    IFile         *oldFile;	/* state associated with current file */
    char	  *cp;

    /*
     * Skip whitespace
     */
    while ((*file == ' ') || (*file == '\t')) {
	file++;
    }

    /*
     * Substitute for any variables in the file name before trying to
     * find the thing.
     */
    if (strchr(file, '$') != (char *) NULL) {
	char *nfile = Var_Subst (file, VAR_CMD, FALSE);
	free (file);
	file = nfile;
    } else
	file = strdup(file);

    cp = file;
    while (*cp && *cp != ' ' && *cp != '\t')
	cp++;
    *cp = '\0';

    emalloc(oldFile,sizeof (IFile));
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
    fname = file;
    lineno = 0;

    curFILE = fopen (file, "r");
    if (curFILE == (FILE * ) NULL) {
	Parse_Error (PARSE_FATAL, MSGSTR(CANTOPEN1, 
		"make: Cannot open %s"), file);
	/*
	 * Pop to previous file
	 */
	(void) ParseEOF(0);
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
ParseEOF (const int opened)
{
    IFile     *ifile;	/* the state on the top of the includes stack */

    if (Lst_IsEmpty (includes)) {
	return (DONE);
    }

    ifile = (IFile *) Lst_DeQueue (includes);
    free (fname);
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
static char *
ParseReadLine (void)
{
    Buffer  	  buf;	    	/* Buffer for current line */
    register int  c;	      	/* the current character */
    register int  lastc;    	/* The most-recent character */
    Boolean	  semiNL;     	/* treat semi-colons as newlines */
    Boolean	  ignDepOp;   	/* TRUE if should ignore dependency operators
				 * for the purposes of setting semiNL */
    Boolean 	  ignComment;	/* TRUE if should ignore comments (in a
				 * shell command */
    char    	  *line;    	/* Result */
    int	    	  lineLength;	/* Length of result */
    int	in_back_quote = 0;	/* Flag for the parsing of '`'s. */

    semiNL = FALSE;
    ignDepOp = FALSE;
    ignComment = FALSE;

    /*
     * Handle special-characters at the beginning of the line. Either a
     * leading tab (shell command) or pound-sign (possible conditional)
     * forces us to ignore comments and dependency operators and treat
     * semi-colons as semi-colons (by leaving semiNL FALSE). This also
     * discards completely blank lines.
     */
    while(1) {
	c = ParseReadc();

	if (c == '\t') {
	    ignComment = ignDepOp = TRUE;
	    break;
	} else if (c == '\n') {
	    lineno++;
	} else if (c == '#') {
		ungetc(c, curFILE); 
		break;
	} else {
	    /*
	     * Anything else breaks out without doing anything
	     */
	    break;
	}
    }
	
    if (c != EOF) {
	lastc = c;
	buf = Buf_Init(DEF_BSIZE);
	
	while (((c = ParseReadc ()) != '\n' || (lastc == '\\')) &&
	       (c != EOF))
	{
test_char:
	    /* If we've come across a '`'. */
	    if (c == '`')
	    {
		in_back_quote=!in_back_quote;
		goto end_of_switch;
	    }

	    /* If we're inside '`'s. */
	    if (in_back_quote)
	    {
		goto end_of_switch;
	    }

	    switch(c) {
	    case '\n':
		/*
		 * Escaped newline: read characters until a non-space or an
		 * unescaped newline and replace them all by a single space.
		 * This is done by storing the space over the backslash and
		 * dropping through with the next nonspace. If it is a
		 * semi-colon and semiNL is TRUE, it will be recognized as a
		 * newline in the code below this...
		 */
		lineno++;
		lastc = ' ';
		while ((c = ParseReadc ()) == ' ' || c == '\t') {
		    continue;
		}
		if (c == EOF || c == '\n') {
		    goto line_read;
		} else {
		    /*
		     * Check for comments, semiNL's, etc. -- easier than
		     * ungetc(c, curFILE); continue;
		     */
		    goto test_char;
		}
		break;
	    case ';':
		/*
		 * Semi-colon: Need to see if it should be interpreted as a
		 * newline
		 */
		if (semiNL) {
		    /*
		     * To make sure the command that may be following this
		     * semi-colon begins with a tab, we push one back into the
		     * input stream. This will overwrite the semi-colon in the
		     * buffer. If there is no command following, this does no
		     * harm, since the newline remains in the buffer and the
		     * whole line is ignored.
		     */
		    ungetc('\t', curFILE);
		    goto line_read;
		} 
		break;
	    case '=':
		if (!semiNL) {
		    /*
		     * Haven't seen a dependency operator before this, so this
		     * must be a variable assignment -- don't pay attention to
		     * dependency operators after this.
		     */
		    ignDepOp = TRUE;
		}
		break;
	    case '#':
		if (!ignComment) {
			char	previous_character;

			/*
			 * If the character is a hash mark and it isn't escaped
			 * (or we're being compatible), the thing is a comment.
			 * Skip to the end of the line.
			 */
			do {
			    /* Save off the current character into
			       the previous character. */
			    previous_character=c;

			    c = ParseReadc();

			    /* If the current character is an unescaped
			       newline or EOF has been reached then we're
			       through reading in the comment. */
			    if (((c == '\n') && (previous_character != '\\'))
				|| (c == EOF))
			    {
				break;
			    }
			} while (1);

			goto line_read;
		}
		break;
	    case ':':
		if (!ignDepOp) {
		    /*
		     * A semi-colon is recognized as a newline only on
		     * dependency lines. Dependency lines are lines with a
		     * colon or an exclamation point. Ergo...
		     */
		    semiNL = TRUE;
		}
		break;
	    }
end_of_switch:
	    /*
	     * Copy in the previous character and save this one in lastc.
	     */
	    Buf_AddByte (buf, (Byte)lastc);
	    lastc = c;
	    
	}
    line_read:
	lineno++;
	
	if (lastc != '\0') {
	    Buf_AddByte (buf, (Byte)lastc);
	}
	Buf_AddByte (buf, (Byte)'\0');
	line = (char *)Buf_GetAll (buf, &lineLength);
	Buf_Destroy (buf, FALSE);
	
	return (line);
    } else {
	/*
	 * Hit end-of-file, so return a NULL line to indicate this.
	 */
	return((char *)NULL);
    }
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
    extern int Suff_EndTransform(GNode *);

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
    char          *name,	/* the name of the file being read */
    FILE *	  stream,   	/* Stream open to makefile to parse */
    const GNode          *ctxt      /* Context in which to do the
    					parsing. */
    )
{
    char *cp,		/* pointer into the line */
                  *line;	/* the line we're working on */

    inLine = FALSE;
    fname = name;
    curFILE = stream;
    lineno = 0;
    fatals = 0;
    maybePosix = TRUE;
    isPosix = FALSE;

    do {
	while (line = ParseReadLine ()) {
	    if (*line == '#') {
		/* If we're this far, the line must be a comment. */
		free (line);
		continue;
	    }
	    
	    if (*line == '\t')
	    {
		maybePosix = FALSE;
		/*
		 * If a line starts with a tab, it
		 * can only hope to be a creation command.
		 */
		for (cp = line + 1; Mbyte_isspace (cp); cp += MBLENF(cp)) {
		    continue;
		}
		if (*cp) {
		    if (inLine) {
			/*
			 * So long as it's not a blank line and we're actually
			 * in a dependency spec, add the command to the list of
			 * commands of all targets in the dependency spec 
			 */
			Lst_ForEach (targets, ParseAddCmd, (ClientData)cp);
			continue;
		    } else {
			Parse_Error (PARSE_FATAL,
				     MSGSTR(SHELLERR, "make: "
					"Shell command not associated with "
					"a dependency:%.20s."), cp);
		    }
		}
		free (line);
		continue;
	    }

	    if (!isPosix && *line == 'i' && strncmp(line, "include", 7) == 0 &&
		(line[7] == ' ' || line[7] == '\t')) {
		ParseDoInclude (line + 7);
		free (line);
		continue;
	    }

	    if (Parse_IsVar (line)) {
		maybePosix = FALSE;
		ParseFinishLine();

		/* If parsing the POSIX internal rules file. */
		if (isPosix)
		{
		   Parse_DoVar(line,VAR_INTERNAL);
		}
		else
		{
		   Parse_DoVar (line, ctxt);
		} 
	    } else {
		/*
		 * We now know it's a dependency line so it needs to have all
		 * variables expanded before being parsed. Tell the variable
		 * module to complain if some variable is undefined...
		 * To make life easier on novices, if the line is indented we
		 * first make sure the line has a dependency operator in it.
		 * If it doesn't have an operator and we're in a dependency
		 * line's script, we assume it's actually a shell command
		 * and add it to the current list of targets.
		 */
		Boolean	nonSpace = FALSE;
		
		cp = line;
		if (line[0] == ' ') {
		    while ((*cp != ':') && (*cp != '\0')) {
			if (!Mbyte_isspace(cp)) {
			    nonSpace = TRUE;
			}
			cp += MBLENF(cp);
		    }
		}
		    
		if (*cp == '\0') {
		    /* If we're parsing commands for a dependency and
		    we've found a line matching /^ [[:space:]]*[[:graph:]]+/ 
		    then it's a parse error. */
                    if (inLine && nonSpace)
                    {
			Parse_Error (PARSE_FATAL, MSGSTR(SHELLTAB, 
			    "make: Shell command needs a leading tab."));
                    }
		    else if (nonSpace) {
	    		Parse_Error (PARSE_FATAL, MSGSTR(PARSERR2, 
				"make: Dependency line needs "
				"colon or double colon operator."));
		    }
		} else {
		    ParseFinishLine();

		    cp = Var_Subst (line, VAR_CMD, TRUE);
		    free (line);
		    line = cp;
		    
		    /*
		     * Need a non-circular list for the target nodes 
		     */
		    targets = Lst_Init (FALSE);
		    inLine = TRUE;
		    
		    ParseDoDependency (line);
		    maybePosix = FALSE;
		}
	    }
	}
	/*
	 * Reached EOF, but it may be just EOF of an include file... 
	 */

	 /* If just finished processing the POSIX internal rules file. */
	 if (isPosix)
	 {
		isPosix=0;
	 }
    } while (ParseEOF(1) == CONTINUE);

    if (fatals) {
	fprintf (stderr, MSGSTR(FATALMSG, "make: Fatal errors "
		"encountered -- cannot continue.\n"));
	exit (2);
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
 *	none
 *---------------------------------------------------------------------
 */
void
Parse_Init (void)
{
    mainNode = NILGNODE;
    includes = Lst_Init (FALSE);
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

    main = Lst_Init (FALSE);

    if (mainNode == NILGNODE) {
	Punt (MSGSTR(NOMAKEFILE, "There must be an existing "
		"description file or specify a target."));
    	/*NOTREACHED*/
    } else if (mainNode->type & OP_DOUBLEDEP) {
	Lst_Concat(main, mainNode->cohorts, LST_CONCNEW);
    }
    (void) Lst_AtEnd (main, (ClientData)mainNode);
    return (main);
}
