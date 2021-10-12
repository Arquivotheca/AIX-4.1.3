/* @(#)47       1.3  src/bldenv/make/make.h, bldprocess, bos412, GOLDA411a 1/19/94 16:30:57
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: CONCAT
 *		DEBUG
 *		OP_NOP
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
 * $Log: make.h,v $
 * Revision 1.2.2.5  1992/12/03  19:07:05  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:21  damon]
 *
 * Revision 1.2.2.4  1992/11/13  15:19:52  root
 * 	define strdup to be strdup2 for systems with bad
 * 	strdup declarations. This hides the bad declaration
 * 	and then the correct one can be given.
 * 	[1992/11/13  14:57:04  root]
 * 
 * Revision 1.2.2.3  1992/11/11  21:14:01  damon
 * 	CR 329. Added include of portable.h
 * 	[1992/11/11  21:13:28  damon]
 * 
 * Revision 1.2.2.2  1992/09/24  19:26:44  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:55:06  gm]
 * 
 * Revision 1.2  1991/12/05  20:44:35  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:38:28  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:08:00  mckeen]
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
 *
 *	@(#)make.h	5.13 (Berkeley) 3/1/91
 */

/*-
 * make.h --
 *	The global definitions for pmake
 */

#ifndef _MAKE_H_
#define _MAKE_H_

#include <ode/portable.h>
#include <sys/types.h>
#ifdef BAD_STRDUP_DECL
#define strdup strdup2
#include <string.h>
#undef strdup2
extern char * strdup ( const char * );
#else
#include <string.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "sprite.h"
#include "lst.h"
#include "hash.h"
#include "config.h"
#include "str.h"

/*-
 * The structure for an individual graph node. Each node has several
 * pieces of data associated with it.
 *	1) the name of the target it describes
 *	2) the location of the target file in the file system.
 *	3) the type of operator used to define its sources (qv. parse.c)
 *	4) whether it is involved in this invocation of make
 *	5) whether the target has been remade
 *	6) whether any of its children has been remade
 *	7) the number of its children that are, as yet, unmade
 *	8) its modification time
 *	9) the modification time of its youngest child (qv. make.c)
 *	10) a list of nodes for which this is a source
 *	11) a list of nodes on which this depends
 *	12) a list of nodes that depend on this, as gleaned from the
 *	    transformation rules.
 *	13) a list of nodes of the same name created by the :: operator
 *	14) a list of nodes that must be made (if they're made) before
 *	    this node can be, but that do no enter into the datedness of
 *	    this node.
 *	15) a list of nodes that must be made (if they're made) after
 *	    this node is, but that do not depend on this node, in the
 *	    normal sense.
 *	16) a Lst of ``local'' variables that are specific to this target
 *	   and this target only (qv. var.c [$@ $< $?, etc.])
 *	17) a Lst of strings that are commands to be given to a shell
 *	   to create this target. 
 */
typedef struct GNode {
    string_t        name;     	/* The target's name */
    string_t   	    path;     	/* The full pathname of the file */
    int             type;      	/* Its type (see the OP flags, below) */

    Boolean         make;      	/* TRUE if this target needs to be remade */
    enum {
	UNMADE, BEINGMADE, MADE, UPTODATE, ERROR, ABORTED,
	CYCLE, ENDCYCLE
    }	    	    made;    	/* Set to reflect the state of processing
				 * on this node:
				 *  UNMADE - Not examined yet
				 *  BEINGMADE - Target is already being made.
				 *  	Indicates a cycle in the graph. (compat
				 *  	mode only)
				 *  MADE - Was out-of-date and has been made
				 *  UPTODATE - Was already up-to-date
				 *  ERROR - An error occured while it was being
				 *  	made (used only in compat mode)
				 *  ABORTED - The target was aborted due to
				 *  	an error making an inferior (compat).
				 *  CYCLE - Marked as potentially being part of
				 *  	a graph cycle. If we come back to a
				 *  	node marked this way, it is printed
				 *  	and 'made' is changed to ENDCYCLE.
				 *  ENDCYCLE - the cycle has been completely
				 *  	printed. Go back and unmark all its
				 *  	members.
				 */
    Boolean 	    childMade; 	/* TRUE if one of this target's children was
				 * made */
    int             unmade;    	/* The number of unmade children */

    int             mtime;     	/* Its modification time */
    int        	    cmtime;    	/* The modification time of its youngest
				 * child */

    Lst     	    iParents;  	/* Links to parents for which this is an
				 * implied source, if any */
    Lst	    	    cohorts;  	/* Other nodes for the :: operator */
    Lst             parents;   	/* Nodes that depend on this one */
    Lst             children;  	/* Nodes on which this one depends */
    Hash_Table	    childHT;	/* hash table of child names */
    Lst	    	    successors;	/* Nodes that must be made after this one */
    Lst	    	    preds;  	/* Nodes that must be made before this one */

    Lst             context;   	/* The local variables */
    int		    contextExtras; /* context contains extra variables */
    Lst             commands;  	/* Creation commands */

    struct _Suff    *suffix;	/* Suffix for the node (determined by
				 * Suff_FindDeps and opaque to everyone
				 * but the Suff module) */
    string_t        makefilename; /* makefile that defined this node, 
				 * for graph debugging */
} GNode;

/*
 * Manifest constants 
 */
#define NILGNODE	((GNode *) NIL)

/*
 * The OP_ constants are used when parsing a dependency line as a way of
 * communicating to other parts of the program the way in which a target
 * should be made. These constants are bitwise-OR'ed together and
 * placed in the 'type' field of each node. Any node that has
 * a 'type' field which satisfies the OP_NOP function was never never on
 * the lefthand side of an operator, though it may have been on the
 * righthand side... 
 */
#define OP_DEPENDS	0x00000001  /* Execution of commands depends on
				     * kids (:) */
#define OP_FORCE	0x00000002  /* Always execute commands (!) */
#define OP_DOUBLEDEP	0x00000004  /* Execution of commands depends on kids
				     * per line (::) */
#define OP_OPMASK	(OP_DEPENDS|OP_FORCE|OP_DOUBLEDEP)

#define OP_OPTIONAL	0x00000008  /* Don't care if the target doesn't
				     * exist and can't be created */
#define OP_USE		0x00000010  /* Use associated commands for parents */
#define OP_EXEC	  	0x00000020  /* Target is never out of date, but always
				     * execute commands anyway. Its time
				     * doesn't matter, so it has none...sort
				     * of */
#define OP_IGNORE	0x00000040  /* Ignore errors when creating the node */
#define OP_PRECIOUS	0x00000080  /* Don't remove the target when
				     * interrupted */
#define OP_SILENT	0x00000100  /* Don't echo commands when executed */
#define OP_MAKE		0x00000200  /* Target is a recurrsive make so its
				     * commands should always be executed when
				     * it is out of date, regardless of the
				     * state of the -n or -t flags */
#define OP_JOIN 	0x00000400  /* Target is out-of-date only if any of its
				     * children was out-of-date */
#define OP_LINK		0x00000800  /* The node is a symlink, we should use
				     * lstat instead of stat in dir.c */
#define OP_INVISIBLE	0x00004000  /* The node is invisible to its parents.
				     * I.e. it doesn't show up in the parents's
				     * local variables. */
#define OP_NOTMAIN	0x00008000  /* The node is exempt from normal 'main
				     * target' processing in parse.c */
/* Attributes applied by PMake */
#define OP_TRANSFORM	0x80000000  /* The node is a transformation rule */
#define OP_MEMBER 	0x40000000  /* Target is a member of an archive */
#define OP_LIB	  	0x20000000  /* Target is a library */
#define OP_ARCHV  	0x10000000  /* Target is an archive construct */
#define OP_HAS_COMMANDS	0x08000000  /* Target has all the commands it should.
				     * Used when parsing to catch multiple
				     * commands for a target */
#define OP_SAVE_CMDS	0x04000000  /* Saving commands on .END (Compat) */
#define OP_DEPS_FOUND	0x02000000  /* Already processed by Suff_FindDeps */

/*
 * OP_NOP will return TRUE if the node with the given type was not the
 * object of a dependency operator
 */
#define OP_NOP(t)	(((t) & OP_OPMASK) == 0x00000000)

/*
 * The TARG_ constants are used when calling the Targ_FindNode and
 * Targ_FindList functions in targ.c. They simply tell the functions what to
 * do if the desired node(s) is (are) not found. If the TARG_CREATE constant
 * is given, a new, empty node will be created for the target, placed in the
 * table of all targets and its address returned. If TARG_NOCREATE is given,
 * a NIL pointer will be returned. 
 */
#define TARG_CREATE	0x01	  /* create node if not found */
#define TARG_NOCREATE	0x00	  /* don't create it */

/*
 * These constants are all used by the string_concat function to decide how the
 * final string should look. If STR_ADDSPACE is given, a space will be
 * placed between the two strings. If STR_ADDSLASH is given, a '/' will
 * be used instead of a space. If neither is given, no intervening characters
 * will be placed between the two strings in the final output. If the
 * STR_DOFREE bit is set, the two input strings will be freed before
 * string_concat returns. 
 */
#define STR_ADDSPACE	0x01	/* add a space when string_concat'ing */
#define STR_DOFREE	0x02	/* free source strings after concatenation */
#define STR_ADDSLASH	0x04	/* add a slash when string_concat'ing */

/*
 * Error levels for parsing. PARSE_FATAL means the process cannot continue
 * once the makefile has been parsed. PARSE_WARNING means it can. Passed
 * as the first argument to Parse_Error.
 */
#define PARSE_WARNING	2
#define PARSE_FATAL	1

/*
 * Definitions for the "local" variables. Used only for clarity.
 */
#define TARGET	  	  "@" 	/* Target of dependency */
#define OODATE	  	  "?" 	/* All out-of-date sources */
#define ALLSRC	  	  ">" 	/* All sources */
#define IMPSRC	  	  "<" 	/* Source implied by transformation */
#define PREFIX	  	  "*" 	/* Common prefix */
#define ARCHIVE	  	  "!" 	/* Archive in "archive(member)" syntax */
#define MEMBER	  	  "%" 	/* Member in "archive(member)" syntax */

#define LPAREN '('
#define RPAREN ')'

/*
 * Global Variables 
 */
extern string_t sNULL;
extern string_t sDOT;
extern string_t sTARGET;
extern string_t sOODATE;
extern string_t sALLSRC;
extern string_t sIMPSRC;
extern string_t sPREFIX;
extern string_t sARCHIVE;
extern string_t sMEMBER;
extern string_t s_TARGET;
extern string_t s_OODATE;
extern string_t s_ALLSRC;
extern string_t s_IMPSRC;
extern string_t s_PREFIX;
extern string_t s_ARCHIVE;
extern string_t s_MEMBER;
extern string_t s_TARGETS;
extern string_t s_INCLUDES;
extern string_t s_LIBS;
extern string_t sMAKE;
extern string_t sMAKEFLAGS;
extern string_t sMFLAGS;
extern string_t sMACHINE;
extern string_t sNPROC;
extern string_t sVPATH;
extern string_t sMAKEFILE;
extern string_t sLIBSUFF;
extern string_t sMAKEOBJDIR;
extern string_t sMAKESRCDIRPATH;
extern string_t sMAKEDIR;
extern string_t sMAKETOP;
extern string_t sMAKESUB;
extern string_t s_DEFAULT;
extern string_t s_INTERRUPT;
extern string_t s_BEGIN;
extern string_t s_END;
extern string_t s_ERROR;
extern string_t s_EXIT;

extern Lst  	create;	    	/* The list of target names specified on the
				 * command line. used to resolve #if
				 * make(...) statements */
extern Lst     	dirSearchPath; 	/* The list of directories to search when
				 * looking for targets */
extern Lst	parseIncPath, sysIncPath;

extern Boolean	ignoreErrors;  	/* True if should ignore all errors */
extern Boolean  beSilent;    	/* True if should print no commands */
extern Boolean  noExecute;    	/* True if should execute nothing */
extern Boolean  allPrecious;   	/* True if every target is precious */
extern Boolean  keepgoing;    	/* True if should continue on unaffected
				 * portions of the graph when have an error
				 * in one portion */
extern Boolean 	touchFlag;    	/* TRUE if targets should just be 'touched'
				 * if out of date. Set by the -t flag */
extern Boolean  usePipes;    	/* TRUE if should capture the output of
				 * subshells by means of pipes. Otherwise it
				 * is routed to temporary files from which it
				 * is retrieved when the shell exits */
extern Boolean 	queryFlag;    	/* TRUE if we aren't supposed to really make
				 * anything, just see if the targets are out-
				 * of-date */

extern Boolean	checkEnvFirst;	/* TRUE if environment should be searched for
				 * variables before the global context */
extern Boolean	noisy;		/* Print extra junk when building */

extern GNode    *DEFAULT;    	/* .DEFAULT rule */

extern GNode    *VAR_GLOBAL;   	/* Variables defined in a global context, e.g
				 * in the Makefile itself */
extern GNode    *VAR_CMD;    	/* Variables defined on the command line */
extern char    	var_Error[];   	/* Value returned by Var_Parse when an error
				 * is encountered. It actually points to
				 * an empty string, so naive callers needn't
				 * worry about it. */

extern time_t 	now;	    	/* The time at the start of this whole
				 * process */

extern Boolean	oldVars;    	/* Do old-style variable substitution */


/*
 * debug control:
 *	There is one bit per module.  It is up to the module what debug
 *	information to print.
 */
extern int debug;
#define	DEBUG_ARCH	0x0001
#define	DEBUG_COND	0x0002
#define	DEBUG_DIR	0x0004
#define	DEBUG_GRAPH1	0x0008
#define	DEBUG_GRAPH2	0x0010
#define	DEBUG_JOB	0x0020
#define	DEBUG_MAKE	0x0040
#define	DEBUG_SUFF	0x0080
#define	DEBUG_TARG	0x0100
#define	DEBUG_VAR	0x0200

#ifdef __STDC__
#define CONCAT(a,b)	a##b
#else
#define CONCAT(a,b)	a/**/b
#endif /* __STDC__ */

#define	DEBUG(module)	(debug & CONCAT(DEBUG_,module))

/*
 * Since there are so many, all functions that return non-integer values are
 * extracted by means of a sed script or two and stuck in the file "nonints.h"
 */
#include "nonints.h"

#endif /* _MAKE_H_ */
