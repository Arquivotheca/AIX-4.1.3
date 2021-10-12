/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: DieHorribly
 *		Error
 *		Fatal
 *		Finish
 *		MainParseArgs
 *		Main_ParseArgLine
 *		MakeSetWorkingDir
 *		Punt
 *		ReadMakefile
 *		ReadMakefileP
 *		confmove
 *		didchdir
 *		emalloc
 *		enomem
 *		fixvar
 *		ltop
 *		main
 *		makechdir
 *		makepath
 *		movedmake
 *		plainmake
 *		ptol
 *		rdircat
 *		reldir
 *		setpathvars
 *		strdup2
 *		ui_print_revision1595
 *		usage
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
 * $Log: main.c,v $
 * Revision 1.2.13.1  1993/09/21  19:17:52  damon
 * 	CR 635. Bumped version to 2.3.2
 * 	[1993/09/21  19:17:42  damon]
 *
 * Revision 1.2.11.1  1993/08/11  16:05:01  damon
 * 	CR 620. Updated version to 2.3.1
 * 	[1993/08/11  16:04:51  damon]
 * 
 * Revision 1.2.9.3  1993/05/04  19:13:01  damon
 * 	CR 393. Update version string to 2.3
 * 	[1993/05/04  19:12:53  damon]
 * 
 * Revision 1.2.9.2  1993/04/26  20:27:19  damon
 * 	CR 424. make now handles error returns better
 * 	[1993/04/26  20:26:56  damon]
 * 
 * Revision 1.2.4.8  1992/12/03  19:06:54  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:14  damon]
 * 
 * Revision 1.2.4.7  1992/11/23  14:53:31  damon
 * 	Changed version string
 * 	[1992/11/23  14:53:12  damon]
 * 
 * Revision 1.2.4.6  1992/11/13  15:19:50  root
 * 	Added bogus strdup2 function for systems which have
 * 	a bad strdup declaration. See make.h for details.
 * 	[1992/11/13  14:56:04  root]
 * 
 * Revision 1.2.4.5  1992/11/09  21:50:27  damon
 * 	CR 296. Cleaned up to remove warnings
 * 	[1992/11/09  21:49:14  damon]
 * 
 * Revision 1.2.4.4  1992/09/24  19:26:22  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:54:49  gm]
 * 
 * Revision 1.2.4.3  1992/06/16  22:42:31  damon
 * 	more 2.1.1 touch-up
 * 	[1992/06/16  22:40:20  damon]
 * 
 * Revision 1.2.2.3  1992/04/02  17:04:59  damon
 * 	Fixed BUILD_DATE for bootstrap
 * 	[1992/04/02  17:04:44  damon]
 * 
 * Revision 1.2.2.2  1992/04/02  15:02:22  damon
 * 	Added ui_print_revision and BUILD_DATE
 * 	[1992/04/02  15:00:44  damon]
 * 
 * Revision 1.2  1991/12/05  20:44:24  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:38:13  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:06:16  mckeen]
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
static char sccsid[] = "@(#)44  1.9  src/bldenv/make/main.c, bldprocess, bos412, 9443A412a 10/27/94 16:30:54";
#endif /* not lint */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)main.c	5.24 (Berkeley) 2/2/91";
#endif /* not lint */

/*-
 * main.c --
 *	The main file for this entire program. Exit routines etc
 *	reside here.
 *
 * Utility functions defined in this file:
 *	Main_ParseArgLine	Takes a line of arguments, breaks them and
 *				treats them as if they were given when first
 *				invoked. Used by the parse module to implement
 *				the .MFLAGS target.
 *
 *	Error			Print a tagged error message. The global
 *				MAKE variable must have been defined. This
 *				takes a format string and two optional
 *				arguments for it.
 *
 *	Fatal			Print an error message and exit. Also takes
 *				a format string and two arguments.
 *
 *	Punt			Aborts all jobs and exits with a message. Also
 *				takes a format string and two arguments.
 *
 *	Finish			Finish things up by printing the number of
 *				errors which occured, as passed to it, and
 *				exiting.
 */

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include "make.h"
#include "buf.h"
#include "pathnames.h"

#ifndef PATH_MAX
#define PATH_MAX MAXPATHLEN
#endif

string_t sALL_DEPEND;
string_t sPTF_UPDATE;
string_t sPTF_UPDATE_ALL;
string_t sTDPATH;
string_t sTDPATH_ALL;
string_t sNULL;
string_t sDOT;
string_t sTARGET;
string_t sOODATE;
string_t sALLSRC;
string_t sIMPSRC;
string_t sPREFIX;
string_t sARCHIVE;
string_t sMEMBER;
string_t s_TARGET;
string_t s_OODATE;
string_t s_ALLSRC;
string_t s_IMPSRC;
string_t s_PREFIX;
string_t s_ARCHIVE;
string_t s_MEMBER;
string_t s_TARGETS;
string_t s_INCLUDES;
string_t s_LIBS;
string_t sMAKE;
string_t sMAKEFLAGS;
string_t sMFLAGS;
string_t sMACHINE;
string_t sNPROC;
string_t sVPATH;
string_t sMAKEFILE;
string_t sLIBSUFF;
string_t sMAKEOBJDIR;
string_t sMAKESRCDIRPATH;
string_t sMAKEDIR;
string_t sMAKETOP;
string_t sMAKESUB;
string_t s_DEFAULT;
string_t s_INTERRUPT;
string_t s_BEGIN;
string_t s_END;
string_t s_ERROR;
string_t s_EXIT;

const char		*progname = "make";

Lst			create;		/* Targets to be made */
time_t			now;		/* Time at start of make */
GNode			*DEFAULT;	/* .DEFAULT node */
Boolean			allPrecious;	/* .PRECIOUS given on line by itself */

static Boolean		noBuiltins;	/* -r flag */
static Lst		makefiles;	/* ordered list of makefiles to read */
int			maxJobs;	/* -J argument */
static int		maxLocal;	/* -L argument */
Boolean			debug;		/* -d flag */
Boolean			noExecute;	/* -n flag */
Boolean			keepgoing;	/* -k flag */
Boolean			queryFlag;	/* -q flag */
Boolean			touchFlag;	/* -t flag */
Boolean			ignoreErrors;	/* -i flag */
Boolean			beSilent;	/* -s flag */
Boolean			oldVars;	/* variable substitution style */
Boolean			checkEnvFirst;	/* -e flag */
Boolean			noisy;		/* -N flag */
static Boolean		jobsRunning;	/* TRUE if the jobs might be running */

static char _MAKECONF[]	= "_MAKECONF";
static char _MAKECWD[]	= "_MAKECWD";
static char _MAKEPSD[]	= "_MAKEPSD";
static char _MAKEIOD[]	= "_MAKEIOD";

static Boolean		ReadMakefile(const char *, Boolean);
static int		ReadMakefileP(ClientData, ClientData);
static void usage(void);

static void MakeSetWorkingDir(const char *);
static void plainmake(char *, const char *);
static void movedmake(char *, char *, char *);
static void setpathvars(const char *);
static void confmove(char *, char *, char *);
static void fixvar(Lst, char *, char *);
static void reldir(char *, string_t, Lst);
static void makechdir(string_t, char *, Lst);
static int didchdir(void);
static void makepath(const char *, const char *);
static void rdircat(char *, char *, Buffer);
static void ptol(const char *, Lst *);
static void ltop(Lst, Buffer);
static void ui_print_revision(void);

#ifdef BAD_STRDUP_DECL
char *strdup2 ( char * str)
{
}
#endif

/*-
 * MainParseArgs --
 *	Parse a given argument vector. Called from main() and from
 *	Main_ParseArgLine() when the MAKEFLAGS target is used.
 *
 *	XXX: Deal with command line overriding MAKEFLAGS in makefile
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Various global and local flags will be set depending on the flags
 *	given
 */
static void
MainParseArgs(int argc, char **argv)
{
	int c;
	string_t argstr;

	optind = 1;	/* since we're called more than once */
rearg:	while((c = getopt(argc, argv, "D:I:L:NPSd:ef:ij:knqrstv")) != EOF) {
		switch(c) {
		case 'D':
			argstr = string_create(optarg);
			Var_Set(argstr, string_create("1"), VAR_GLOBAL);
			Var_Append(sMAKEFLAGS, string_create("-D"),
				   VAR_GLOBAL);
			Var_Append(sMAKEFLAGS, argstr, VAR_GLOBAL);
			string_deref(argstr);
			break;
		case 'I':
			argstr = string_create(optarg);
			Parse_AddIncludeDir(argstr);
			Var_Append(sMAKEFLAGS, string_create("-I"),
				   VAR_GLOBAL);
			Var_Append(sMAKEFLAGS, argstr, VAR_GLOBAL);
			string_deref(argstr);
			break;
		case 'L':
			maxLocal = atoi(optarg);
			Var_Append(sMAKEFLAGS, string_create("-L"),
				   VAR_GLOBAL);
			argstr = string_create(optarg);
			Var_Append(sMAKEFLAGS, argstr, VAR_GLOBAL);
			string_deref(argstr);
			break;
		case 'N':
			noisy = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-N"),
				   VAR_GLOBAL);
			break;
		case 'S':
			keepgoing = FALSE;
			Var_Append(sMAKEFLAGS, string_create("-S"),
				   VAR_GLOBAL);
			break;
		case 'd': {
			char *modules = optarg;

			for (; *modules; ++modules)
				switch (*modules) {
				case 'A':
					debug = ~0;
					break;
				case 'a':
					debug |= DEBUG_ARCH;
					break;
				case 'c':
					debug |= DEBUG_COND;
					break;
				case 'd':
					debug |= DEBUG_DIR;
					break;
				case 'g':
					if (modules[1] == '1') {
						debug |= DEBUG_GRAPH1;
						++modules;
					}
					else if (modules[1] == '2') {
						debug |= DEBUG_GRAPH2;
						++modules;
					}
					break;
				case 'j':
					debug |= DEBUG_JOB;
					break;
				case 'm':
					debug |= DEBUG_MAKE;
					break;
				case 's':
					debug |= DEBUG_SUFF;
					break;
				case 't':
					debug |= DEBUG_TARG;
					break;
				case 'v':
					debug |= DEBUG_VAR;
					break;
				}
			Var_Append(sMAKEFLAGS, string_create("-d"),
				   VAR_GLOBAL);
			argstr = string_create(optarg);
			Var_Append(sMAKEFLAGS, argstr, VAR_GLOBAL);
			string_deref(argstr);
			break;
		}
		case 'e':
			checkEnvFirst = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-e"),
				   VAR_GLOBAL);
			break;
		case 'f':
			(void)Lst_AtEnd(makefiles, (ClientData)optarg);
			break;
		case 'i':
			ignoreErrors = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-i"),
				   VAR_GLOBAL);
			break;
		case 'j':
			maxJobs = atoi(optarg);
			Var_Append(sMAKEFLAGS, string_create("-j"),
				   VAR_GLOBAL);
			argstr = string_create(optarg);
			Var_Append(sMAKEFLAGS, argstr, VAR_GLOBAL);
			string_deref(argstr);
			break;
		case 'k':
			keepgoing = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-k"),
				   VAR_GLOBAL);
			break;
		case 'n':
			noExecute = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-n"),
				   VAR_GLOBAL);
			break;
		case 'q':
			queryFlag = TRUE;
			/* Kind of nonsensical, wot? */
			Var_Append(sMAKEFLAGS, string_create("-q"),
				   VAR_GLOBAL);
			break;
		case 'r':
			noBuiltins = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-r"),
				   VAR_GLOBAL);
			break;
		case 's':
			beSilent = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-s"),
				   VAR_GLOBAL);
			break;
		case 't':
			touchFlag = TRUE;
			Var_Append(sMAKEFLAGS, string_create("-t"),
				   VAR_GLOBAL);
			break;
		case 'v':
			ui_print_revision();
			exit(2);
		default:
		case '?':
			usage();
		}
	}

	oldVars = TRUE;

	/*
	 * See if the rest of the arguments are variable assignments and
	 * perform them if so. Else take them to be targets and stuff them
	 * on the end of the "create" list.
	 */
	for (argv += optind, argc -= optind; *argv; ++argv, --argc)
		if (Parse_DoVar(*argv, VAR_CMD) != 0) {
			if (!**argv)
				Punt("illegal (null) argument.");
			if (**argv == '-') {
				optind = 0;
				goto rearg;
			}
			(void)Lst_AtEnd(create,
					(ClientData)string_create(*argv));
		}
}

/*-
 * Main_ParseArgLine --
 *  	Used by the parse module when a .MFLAGS or MAKEFLAGS target
 *	is encountered and by main() when reading the MAKEFLAGS envariable.
 *	Takes a line of arguments and breaks it into its
 * 	component words and passes those words and the number of them to the
 *	MainParseArgs function.
 *	The line should have all its leading whitespace removed.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Only those that come from the various arguments.
 */
void
Main_ParseArgLine(char *line)		/* Line to fracture */
{
	char **argv;			/* Manufactured argument vector */
	int argc;			/* Number of arguments in argv */

	if (line == NULL)
		return;
	for (; *line == ' '; ++line);
	if (!*line)
		return;

	argv = brk_string(line, &argc);
	MainParseArgs(argc, argv);
}

/*-
 * main --
 *	The main function, for obvious reasons. Initializes variables
 *	and a few modules, then parses the arguments give it in the
 *	environment and on the command line. Reads the system makefile
 *	followed by either Makefile, makefile or the file given by the
 *	-f argument. Sets the MAKEFLAGS PMake variable based on all the
 *	flags it has received by then uses either the Make or the Compat
 *	module to create the initial list of targets.
 *
 * Results:
 *	If -q was given, exits -1 if anything was out-of-date. Else it exits
 *	0.
 *
 * Side Effects:
 *	The program exits when done. Targets are created. etc. etc. etc.
 */
int
main(int argc, char **argv)
{
	Lst targs;	/* target nodes to create -- passed to Make_Init */
	Boolean outOfDate; 	/* FALSE if all targets up to date */
	Boolean hadErrors;	/* TRUE if we had errors */
	const char *p;

	create = Lst_Init();
	makefiles = Lst_Init();
	beSilent = FALSE;		/* Print commands as executed */
	ignoreErrors = FALSE;		/* Pay attention to non-zero returns */
	noExecute = FALSE;		/* Execute all commands */
	keepgoing = FALSE;		/* Stop on error */
	allPrecious = FALSE;		/* Remove targets when interrupted */
	queryFlag = FALSE;		/* This is not just a check-run */
	noBuiltins = FALSE;		/* Read the built-in rules */
	touchFlag = FALSE;		/* Actually update targets */
	debug = 0;			/* No debug verbosity, please. */
	jobsRunning = FALSE;
	noisy = FALSE;

	maxJobs = 0;			/* Set default max concurrency */
	maxLocal = -1;			/* Set default local max concurrency */
    
	/*
	 * create internal commonly used strings
	 */
	string_init();

	sNULL = string_create("");
	sDOT = string_create(".");

	sTARGET = string_create("@");
	sOODATE = string_create("?");
	sALLSRC = string_create(">");
	sIMPSRC = string_create("<");
	sPREFIX = string_create("*");
	sARCHIVE = string_create("!");
	sMEMBER = string_create("%");

	s_TARGET = string_create(".TARGET");
	s_OODATE = string_create(".OODATE");
	s_ALLSRC = string_create(".ALLSRC");
	s_IMPSRC = string_create(".IMPSRC");
	s_PREFIX = string_create(".PREFIX");
	s_ARCHIVE = string_create(".ARCHIVE");
	s_MEMBER = string_create(".MEMBER");

	s_TARGETS = string_create(".TARGETS");
	s_INCLUDES = string_create(".INCLUDES");
	s_LIBS = string_create(".LIBS");

	sMAKE = string_create("MAKE");
	sMAKEFLAGS = string_create("MAKEFLAGS");
	sMFLAGS = string_create("MFLAGS");
	sMACHINE = string_create("MACHINE");
	sNPROC = string_create("NPROC");
	sVPATH = string_create("VPATH");
	sMAKEFILE = string_create("MAKEFILE");
	sLIBSUFF = string_create(LIBSUFF);

	sMAKEOBJDIR = string_create("MAKEOBJDIR");
	sMAKESRCDIRPATH = string_create("MAKESRCDIRPATH");
	sMAKEDIR = string_create("MAKEDIR");
	sMAKETOP = string_create("MAKETOP");
	sMAKESUB = string_create("MAKESUB");

	s_DEFAULT = string_create(".DEFAULT");
	s_INTERRUPT = string_create(".INTERRUPT");
	s_BEGIN = string_create(".BEGIN");
	s_END = string_create(".END");
	s_ERROR = string_create(".ERROR");
	s_EXIT = string_create(".EXIT");

	sALL_DEPEND = string_create("ALL_DEPEND");
	sPTF_UPDATE = string_create("PTF_UPDATE");
	sPTF_UPDATE_ALL = string_create("PTF_UPDATE_ALL");
	sTDPATH = string_create("TDPATH");
	sTDPATH_ALL = string_create("TDPATH_ALL");

	/*
	 * Initialize the parsing, directory and variable modules to prepare
	 * for the reading of inclusion paths and variable settings on the
	 * command line
	 */
	Dir_Init();		/* Initialize directory structures so -I flags
				 * can be processed correctly */
	Parse_Init();		/* Need to initialize the paths of #include
				 * directories */
	Var_Init();		/* As well as the lists of variables for
				 * parsing arguments */

	/*
	 * Initialize various variables.
	 *	MAKE also gets this name, for compatibility
	 *	MAKEFLAGS gets set to the empty string just in case.
	 *	MFLAGS also gets initialized empty, for compatibility.
	 */
	Var_Set(sMAKE, string_create(argv[0]), VAR_GLOBAL);
	Var_Set(sMAKEFLAGS, sNULL, VAR_GLOBAL);
	Var_Set(sMFLAGS, sNULL, VAR_GLOBAL);
	Var_Set(sMACHINE, string_create(MACHINE), VAR_GLOBAL);
	Var_Set(sNPROC, string_create((p = getenv("NPROC")) ? p : "1"),
		VAR_GLOBAL);

	/*
	 * First snag any flags out of the MAKE environment variable.
	 * (Note this is *not* MAKEFLAGS since /bin/make uses that and it's
	 * in a different format).
	 */
	Main_ParseArgLine(getenv("MAKEFLAGS"));
    
	MainParseArgs(argc, argv);

	Cond_Setup();

	/*
	 * Now make sure that maxJobs and maxLocal are positive.  If
	 * not, use the value of NPROC (or 1 if NPROC is not positive)
	 */
	{
		int nproc = atoi(Var_Value(sNPROC, VAR_GLOBAL));

		if (nproc < 1)
			nproc = 1;
		if (maxJobs < 1)
			maxJobs = nproc;
	}

	/*
	 * Initialize archive, target and suffix modules in preparation for
	 * parsing the makefile(s)
	 */
	Arch_Init();
	Targ_Init();
	Suff_Init();

	DEFAULT = NILGNODE;
	(void)time(&now);

	/*
	 * Set up the .TARGETS variable to contain the list of targets to be
	 * created. If none specified, make the variable empty -- the parser
	 * will fill the thing in with the default or .MAIN target.
	 */
	if (!Lst_IsEmpty(create)) {
		LstNode ln;

		for (ln = Lst_First(create); ln != NILLNODE;
		    ln = Lst_Succ(ln)) {
			string_t name = (string_t)Lst_Datum(ln);

			Var_Append(s_TARGETS, name, VAR_GLOBAL);
		}
	} else
		Var_Set(s_TARGETS, sNULL, VAR_GLOBAL);

	/*
	 * Locate the correct working directory for make.  Adjust for any
	 * movement taken from the directory of our parent if we are a
	 * sub-make.
	 */
	MakeSetWorkingDir("Makeconf");

	/*
	 * Read in the built-in rules first, followed by the specified
	 * makefile, if it was (makefile != (char *) NULL), or the default
	 * Makefile and makefile, in that order, if it wasn't.
	 */
	 if (!noBuiltins && !ReadMakefile(_PATH_DEFSYSMK, TRUE))
		Fatal("make: no system rules (%s).", _PATH_DEFSYSMK);

	if (!Lst_IsEmpty(makefiles)) {
		LstNode ln;

		ln = Lst_Find(makefiles, (ClientData)FALSE, ReadMakefileP);
		if (ln != NILLNODE)
			Fatal("make: cannot open %s.", (char *)Lst_Datum(ln));
	} else if (!ReadMakefile("makefile", FALSE))
		(void)ReadMakefile("Makefile", FALSE);

	Var_Append(sMFLAGS, Var_StrValue(sMAKEFLAGS, VAR_GLOBAL), VAR_GLOBAL);

	/* Install all the flags into the MAKEFLAGS envariable. */
	if ((p = Var_Value(sMAKEFLAGS, VAR_GLOBAL)) && *p)
		setenv("MAKEFLAGS", p, 1);

	/*
	 * For compatibility, look at the directories in the VPATH variable
	 * and add them to the search path, if the variable is defined. The
	 * variable's value is in the same format as the PATH envariable, i.e.
	 * <directory>:<directory>:<directory>...
	 */
	if (Var_Exists(sVPATH, VAR_CMD)) {
		char *vpath, *path, *cp, savec;
		/*
		 * GCC stores string constants in read-only memory, but
		 * Var_Subst will want to write this thing, so store it
		 * in an array
		 */
		static char VPATH[] = "${VPATH}";

		vpath = Var_Subst(VPATH, VAR_CMD, FALSE);
		path = vpath;
		do {
			/* skip to end of directory */
			for (cp = path; *cp != ':' && *cp != '\0'; cp++);
			/* Save terminator character so know when to stop */
			savec = *cp;
			*cp = '\0';
			/* Add directory to search path */
			Dir_AddDir(dirSearchPath, string_create(path));
			*cp = savec;
			path = cp + 1;
		} while (savec == ':');
		(void)free((Address)vpath);
	}

	/*
	 * Now that all search paths have been read for suffixes et al, it's
	 * time to add the default search path to their lists...
	 */
	Suff_DoPaths();

	/* print the initial graph, if the user requested it */
	if (DEBUG(GRAPH1))
		Targ_PrintGraph(1);

	/*
	 * Have now read the entire graph and need to make a list of targets
	 * to create. If none was given on the command line, we consult the
	 * parsing module to find the main target(s) to create.
	 */
	if (Lst_IsEmpty(create))
		targs = Parse_MainName();
	else
		targs = Targ_FindList(create, TARG_CREATE);

	/*
	 * Initialize job module before traversing the graph, now that
	 * any .BEGIN and .END targets have been read.  This is done
	 * only if the -q flag wasn't given (to prevent the .BEGIN from
	 * being executed should it exist).
	 */
	if (!queryFlag) {
		if (maxLocal < 0 || maxLocal > maxJobs)
			maxLocal = maxJobs;
		Job_Init(maxJobs, maxLocal);
		jobsRunning = TRUE;
	}

	/* Traverse the graph, checking on all the targets */
	outOfDate = Make_Run(targs, &hadErrors);
    
	/* print the graph now it's been processed if the user requested it */
	if (DEBUG(GRAPH2))
		Targ_PrintGraph(2);

	/* print the target dependency file if PTF_UPDATE is set to yes */
	/* prints only the dependencies that were out of date           */
	if (strcmp(Var_Value(sPTF_UPDATE, VAR_GLOBAL), "yes") == 0)
		Targ_PrintTargDep(2);

	/* print all dependencies for every target if PTF_UPDATE_ALL=yes */
	if (strcmp(Var_Value(sPTF_UPDATE_ALL, VAR_GLOBAL), "yes") == 0)
		Targ_PrintAllTargDep(2);

	return (hadErrors || (queryFlag && outOfDate));
}

/*-
 * ReadMakefile  --
 *	Open and parse the given makefile.
 *
 * Results:
 *	TRUE if ok. FALSE if couldn't open file.
 *
 * Side Effects:
 *	lots
 */
static int
ReadMakefileP(
	ClientData fname,	/* makefile to read */
	ClientData isSystem)	/* system makefile */
{
	return(ReadMakefile((char *)fname, (Boolean)isSystem));
}

static Boolean
ReadMakefile(
	const char *fname,	/* makefile to read */
	Boolean isSystem)	/* system makefile */
{
	FILE *stream;
	string_t name;

	if (!strcmp(fname, "-")) {
		Var_Set(sMAKEFILE, sNULL, VAR_GLOBAL);
		Parse_File(string_create("(stdin)"), stdin);
	} else {
		string_t sfname = string_create(fname);
		if (!isSystem) {
		    name = Dir_FindFile(sfname, dirSearchPath);
		} else {
		    name = Dir_FindFile(sfname, parseIncPath);
		    if (name == (string_t) NULL)
			name = Dir_FindFile(sfname, sysIncPath);
		}
		string_deref(sfname);
		if (name == (string_t) NULL ||
		    (stream = fopen(name->data, "r")) == NULL)
			return(FALSE);
		/*
		 * set the MAKEFILE variable desired by System V fans -- the
		 * placement of the setting here means it gets set to the last
		 * makefile specified, as it is set by SysV make.
		 */
		Var_Set(sMAKEFILE, name, VAR_GLOBAL);
		Parse_File(name, stream);
		(void)fclose(stream);
	}
	return(TRUE);
}

/*-
 * Error --
 *	Print an error message given its format.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The message is printed.
 */
void
Error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);
}

/*-
 * Fatal --
 *	Produce a Fatal error message. If jobs are running, waits for them
 *	to finish.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The program exits
 */
void
Fatal(const char *fmt, ...)
{
	va_list ap;

	if (jobsRunning)
		Job_Wait();

	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);

	if (DEBUG(GRAPH2))
		Targ_PrintGraph(2);
	exit(2);		/* Not 1 so -q can distinguish error */
}

/*
 * Punt --
 *	Major exception once jobs are being created. Kills all jobs, prints
 *	a message and exits.
 *
 * Results:
 *	None 
 *
 * Side Effects:
 *	All children are killed indiscriminately and the program Lib_Exits
 */
void
Punt(const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, "make: ");
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);

	DieHorribly();
}

/*-
 * DieHorribly --
 *	Exit without giving a message.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	A big one...
 */
void
DieHorribly(void)
{
	if (jobsRunning)
		Job_AbortAll();
	if (DEBUG(GRAPH2))
		Targ_PrintGraph(2);
	exit(2);		/* Not 1, so -q can distinguish error */
}

/*
 * Finish --
 *	Called when aborting due to errors in child shell to signal
 *	abnormal exit. 
 *
 * Results:
 *	None 
 *
 * Side Effects:
 *	The program exits
 */
void
Finish(int errors)	/* number of errors encountered in Make_Make */
{
	Fatal("%d error%s", errors, errors == 1 ? "" : "s");
}

/*
 * emalloc --
 *	malloc, but die on error.
 */
char *
emalloc(u_int len)
{
	char *p;

	if (!(p = (char *)malloc(len)))
		enomem();
	return(p);
}

/*
 * enomem --
 *	die when out of memory.
 */
void
enomem(void)
{
	(void)fprintf(stderr, "make: %s.\n", strerror(errno));
	exit(2);
}

/*
 * usage --
 *	exit with usage message
 */
void
usage(void)
{
	(void)fprintf(stderr,
"usage: make [-eiknqrst] [-D variable] [-d flags] [-f makefile ]\n\t\
[-I directory] [-j max_jobs] [variable=value]\n");
	exit(2);
}

static void
MakeSetWorkingDir(const char *cf)
{
	char *owd, *psd, cwd[PATH_MAX+1];

	if (getcwd(cwd, sizeof(cwd)) == 0 || *cwd != '/')
		Fatal("getwd: %s", strerror(errno));
	if ((owd = getenv(_MAKECWD)) && (psd = getenv(_MAKEPSD))) {
		if (*owd != '/')
			Fatal("make: bad %s environment variable", _MAKECWD);
		if (*psd == '\0')
			Fatal("make: bad %s environment variable", _MAKEPSD);
		movedmake(cwd, owd, psd);
	} else
		plainmake(cwd, cf);
}

/*
 * find and read the configuration file and move
 * to the object tree
 */
static void
plainmake(char *cwd, const char *cf)
{
	register char *d, *p, *q;
	int n;
	char *pname;
	struct stat sb;

	pname = q = NULL;
	if (stat(cf, &sb) == 0) {
		pname = strdup(cf);
	} else if (cwd[1] != '\0') {
		n = 0;
		for (p = cwd; *p != '\0'; p++)
			if (*p == '/')
				n++;
		p = d = emalloc((n * 3) + strlen(cf) + 1);
		q = cwd;
		while (*q != '\0')
			if (*q++ == '/') {
				*p++ = '.';
				*p++ = '.';
				*p++ = '/';
			}
		(void) strcpy(p, cf);
		q = p;
		for (;;) {
			q -= 3;
			if (stat(q, &sb) == 0) {
				pname = strdup(q);
				*p = '\0';
				break;
			}
			if (q == d) {
				q = NULL;
				break;
			}
		}
	}
	confmove(cwd, pname, q);
}

/*
 * we may have changed directories, so we calulate possible difference
 */
static void
movedmake(char *cwd, char *owd, char *psd)
{
	register char *p, *q, *r, *s;
	char *ep;
	struct stat sb;
	int cnt;
	Lst newsd;
	LstNode ln;
	string_t str;
	Buffer buf = Buf_Init(0);

	/*
	 * Locate the difference between the current working directory
	 * and the previous working directory.
	 */
	for (p = r = owd, q = s = cwd; *r && *r == *s; r++, s++)
		if (*r == '/') {
			while (*(r+1) == '/')
				r++;
			while (*(s+1) == '/')
				s++;
			p = r + 1;
			q = s + 1;
		}
	if (*r == 0 && *s == 0) {
		p = r;
		q = s;
	} else if (*r == '/' && *s == 0) {
		while (*++r && *r == '/')
			;
		p = r;
		q = s;
	} else if (*r == 0 && *s == '/') {
		p = r;
		while (*++s && *s == '/')
			;
		q = s;
	}

	ptol(psd, &newsd);
	Lst_Open(newsd);
	while ((ln = Lst_Next(newsd)) != NILLNODE) {
		s = (char *) Lst_Datum(ln);
		if (*s != '/') {
			rdircat(q, p, buf);
			cnt = Buf_Size(buf);
			if (cnt != 0) {
				r = (char *)Buf_GetBase(buf);
				if (r[cnt-1] != '/')
					Buf_AddByte(buf, (Byte)'/');
			}
		}
		Buf_AddBytes(buf, strlen(s), (Byte *)s);
		if ((*p || *q) && *s && (cnt = Buf_Size(buf)) != 0) {
			r = (char *)Buf_GetBase(buf);
			if (r[cnt-1] != '/')
				Buf_AddByte(buf, (Byte)'/');
		}
		rdircat(p, q, buf);
		str = string_create((char *)Buf_GetBase(buf));
		str = string_flatten(str);
		Buf_Discard(buf, Buf_Size(buf));
		Lst_Replace(ln, (ClientData)strdup(str->data));
	}
	Lst_Close(newsd);
	if ((ep = getenv(sMAKEDIR->data))) {
		while (*ep == '/')
			ep++;
		Buf_AddByte(buf, (Byte)'/');
		Buf_AddBytes(buf, strlen(ep), (Byte *)ep);
		if (*p || *q) {
			cnt = Buf_Size(buf);
			r = (char *)Buf_GetBase(buf);
			if (r[cnt-1] != '/')
				Buf_AddByte(buf, (Byte)'/');
		}
	} else
		Buf_AddByte(buf, (Byte)'/');
	rdircat(p, q, buf);
	str = string_create((char *)Buf_GetBase(buf));
	str = string_flatten(str);
	Buf_Discard(buf, Buf_Size(buf));
	setenv(sMAKEDIR->data, str->data, 1);
	if (DEBUG(VAR)) {
		printf("%s=%s\n", sMAKEDIR->data, str->data);
	}
	setpathvars(str->data);
	if ((ep = getenv(_MAKECONF))) {
		if (stat(ep, &sb) == 0)
			(void) ReadMakefile(ep, FALSE);
	}
	if (!didchdir())
		Var_Delete(sMAKEOBJDIR, VAR_GLOBAL);
	setenv(_MAKECWD, cwd, 1);
	if (DEBUG(VAR)) {
		printf("%s=%s\n", _MAKECWD, cwd);
	}
	ltop(newsd, buf);
	r = (char *)Buf_GetBase(buf);
	setenv(_MAKEPSD, r, 1);
	if (DEBUG(VAR)) {
		printf("%s=%s\n", _MAKEPSD, r);
	}
	Buf_Destroy(buf);
	Dir_ReInit(newsd);
	Lst_Destroy(newsd, free);
}

static void
setpathvars(const char *dir)
{
	register const char *p, *q;
	int cnt;
	Buffer buf = Buf_Init(0);

	Var_Set(sMAKEDIR, string_create(dir), VAR_GLOBAL);
	while (*dir == '/')
		dir++;
	q = dir;
	while (*q) {
		if (*q == '.' && strncmp(q, "../", 3) == 0)
			Fatal("%s: bad MAKEDIR", dir);
		if (*q++ == '/') {
			while (*q == '/')
				q++;
		} else {
			Buf_AddBytes(buf, 3, (Byte *)"../");
			while (*q && *q != '/')
				q++;
		}
	}
	p = (char *)Buf_GetBase(buf);
	Var_Set(sMAKETOP, string_create(p), VAR_GLOBAL);
	Buf_Discard(buf, Buf_Size(buf));
	Buf_AddBytes(buf, strlen(dir), (Byte *)dir);
	if ((cnt = Buf_Size(buf)) != 0) {
		p = (char *)Buf_GetBase(buf);
		if (p[cnt-1] != '/')
			Buf_AddByte(buf, (Byte)'/');
	}
	p = (char *)Buf_GetBase(buf);
	Var_Set(sMAKESUB, string_create(p), VAR_GLOBAL);
	Buf_Destroy(buf);
}

static void
confmove(char *cwd, char *cf, char *pre)
{
	register char *src, *r;
	register const char *vp;
	char *suf;
	int cnt;
	struct stat sb;
	Lst newsd;
	string_t str, obj;
	Buffer buf = Buf_Init(0);

	Buf_AddByte(buf, (Byte)'/');
	if (pre && *pre) {
		suf = cwd + strlen(cwd);
		for (r = pre;
		     *r == '.' && *(r+1) == '.' && *(r+2) == '/';
		     r += 3) {
			while (*(suf-1) == '/')
				suf--;
			if (suf == cwd || suf == cwd + 1) {
				suf = 0;
				break;
			}
			while (*(suf-1) != '/')
				suf--;
		}
		if (suf && *suf) {
			suf = strdup(suf);
			Buf_AddBytes(buf, strlen(suf), (Byte *)suf);
		} else
			suf = 0;
	} else {
		pre = 0;
		suf = 0;
	}
	src = (char *)Buf_GetBase(buf);
	setenv(sMAKEDIR->data, src, 1);
	if (DEBUG(VAR)) {
		printf("%s=%s\n", sMAKEDIR->data, src);
	}
	setpathvars(src);
	Buf_Discard(buf, Buf_Size(buf));
	if (cf) {
		if (*cf != '/') {
			Buf_AddBytes(buf, strlen(cwd), (Byte *)cwd);
			Buf_AddByte(buf, (Byte)'/');
		}
		Buf_AddBytes(buf, strlen(cf), (Byte *)cf);
		str = string_create((char *)Buf_GetBase(buf));
		str = string_flatten(str);
		Buf_Discard(buf, Buf_Size(buf));
		setenv(_MAKECONF, str->data, 1);
		if (DEBUG(VAR)) {
			printf("%s=%s\n", _MAKECONF, str->data);
		}
		Var_Set(string_create(_MAKECONF), str, VAR_GLOBAL);
		(void) ReadMakefile(str->data, FALSE);
	}
	obj = (string_t) NULL;
	if ((vp = Var_Value(sMAKEOBJDIR, VAR_GLOBAL))) {
		if (strchr (vp, '$') != (char *)NULL)
			vp = Var_Subst(vp, VAR_GLOBAL, FALSE);
		else
			vp = strdup(vp);
		if (DEBUG(VAR)) {
			printf("< %s=%s\n", sMAKEOBJDIR->data, vp);
		}
		if (strchr(vp, ':'))
			Fatal("%s: only one component allowed",
			      sMAKEOBJDIR->data);
		if (*vp == '\0') {
			Var_Delete(sMAKEOBJDIR, VAR_GLOBAL);
		} else {
			if (*vp != '/' && pre)
				Buf_AddBytes(buf, strlen(pre), (Byte *)pre);
			Buf_AddBytes(buf, strlen(vp), (Byte *)vp);
			r = (char *)Buf_GetBase(buf);
			if (stat(r, &sb) < 0)
				Fatal("No such directory: %s", r);
			if (suf) {
				cnt = Buf_Size(buf);
				if (cnt != 0 && r[cnt-1] != '/')
					Buf_AddByte(buf, (Byte)'/');
				Buf_AddBytes(buf, strlen(suf), (Byte *)suf);
			}
			obj = string_create((char *)Buf_GetBase(buf));
			obj = string_flatten(obj);
			Buf_Discard(buf, Buf_Size(buf));
			if (DEBUG(VAR)) {
				printf("> %s=%s\n", sMAKEOBJDIR->data,
				       obj->data);
			}
		}
	}
	if ((vp = Var_Value(sMAKESRCDIRPATH, VAR_GLOBAL))) {
		if (strchr (vp, '$') != (char *)NULL)
			vp = Var_Subst(vp, VAR_GLOBAL, FALSE);
		else
			vp = strdup(vp);
		if (DEBUG(VAR)) {
			printf("< %s=%s\n", sMAKESRCDIRPATH->data, vp);
		}
		if (*vp == '\0')
			newsd = Lst_Init();
		else {
			ptol(vp, &newsd);
			(void) fixvar(newsd, pre, suf);
			if (DEBUG(VAR)) {
				ltop(newsd, buf);
				r = (char *)Buf_GetBase(buf);
				printf("> %s=%s\n", sMAKESRCDIRPATH->data, r);
				Buf_Discard(buf, Buf_Size(buf));
			}
		}
	} else
		newsd = Lst_Init();
	Buf_Destroy(buf);
	Lst_AtFront(newsd, (ClientData) strdup("."));
	if (obj || Lst_First(newsd) != Lst_Last(newsd))
		reldir(cwd, obj, newsd);
	Dir_ReInit(newsd);
	Lst_Destroy(newsd, free);
}

static void
fixvar(Lst l, char *pre, char *suf)
{
	register char *p;
	int cnt;
	LstNode ln;
	string_t str;
	Buffer buf = Buf_Init(0);

	Lst_Open(l);
	while ((ln = Lst_Next(l)) != NILLNODE) {
		p = (char *) Lst_Datum(ln);
		if (*p != '/' && pre)
			Buf_AddBytes(buf, strlen(pre), (Byte *)pre);
		Buf_AddBytes(buf, strlen(p), (Byte *)p);
		if (suf) {
			if ((cnt = Buf_Size(buf)) != 0) {
				p = (char *)Buf_GetBase(buf);
				if (p[cnt-1] != '/')
					Buf_AddByte(buf, (Byte)'/');
			}
			Buf_AddBytes(buf, strlen(suf), (Byte *)suf);
		}
		str = string_create((char *)Buf_GetBase(buf));
		str = string_flatten(str);
		Buf_Discard(buf, Buf_Size(buf));
		Lst_Replace(ln, (ClientData)strdup(str->data));
	}
	Buf_Destroy(buf);
	Lst_Close(l);
}

/*
 * find names for the source directories after a
 * chdir(obj) and use them to adjust search path
 */
static void
reldir(char *cwd, string_t obj, Lst newsd)
{
	register char *h;
	register const char *t, *tt;
	register char *p, *q, *r;
	char lastc;
	char *twd;
	int cnt;
	LstNode ln;
	string_t str;
	Buffer buf = Buf_Init(0);

	Lst_Open(newsd);
	while ((ln = Lst_Next(newsd)) != NILLNODE) {
		p = (char *) Lst_Datum(ln);
		if (*p == '/' || obj == (string_t) NULL)
			continue;
		Buf_AddBytes(buf, strlen(cwd), (Byte *)cwd);
		if (*obj->data == '/') {
			r = (char *)Buf_GetBase(buf);
			cnt = Buf_Size(buf);
			if (r[cnt-1] != '/')
				Buf_AddByte(buf, (Byte)'/');
			Buf_AddBytes(buf, strlen(p), (Byte *)p);
			str = string_create((char *)Buf_GetBase(buf));
			str = string_flatten(str);
			Buf_Discard(buf, Buf_Size(buf));
			Lst_Replace(ln, (ClientData)strdup(str->data));
			continue;
		}
		r = (char *)Buf_GetBase(buf);
		cnt = Buf_Size(buf);
		twd = emalloc(cnt + obj->len + 2);
		memcpy(twd, r, cnt + 1);
		Buf_Discard(buf, cnt);
		r = twd + cnt - 1;
		lastc = '\0';
		for (tt = t = obj->data; *tt; tt = t) {
			while (*t && *t != '/')
				t++;
			if (t == tt + 1 && *tt == '.') {
				while (*t == '/')
					t++;
			}
			if (lastc != '\0') {
				Buf_AddByte(buf, (Byte)'/');
				lastc = '/';
			}
			if (t == tt + 2 && *tt == '.' && *(tt+1) == '.') {
				if (r == twd) {
					if (lastc != '/')
						Buf_AddByte(buf, (Byte)'/');
					break;
				}
				while (*r != '/') {
					Buf_AddByte(buf, (Byte)*r);
					lastc = *r--;
				}
				while (r != twd && *r == '/')
					--r;
			} else {
				*++r = '/';
				while (tt != t)
					*++r = *tt++;
				Buf_AddByte(buf, (Byte)'.');
				Buf_AddByte(buf, (Byte)'.');
				lastc = '.';
			}
			while (*t == '/')
				t++;
		}
		(void)free(twd);
		q = (char *)Buf_GetBase(buf);
		cnt = Buf_Size(buf);
		t = q + cnt - 1;
		r = emalloc(cnt + strlen(p) + 2);
		h = r;
		while (cnt--) {
			*h++ = *t--;
		}
		Buf_Discard(buf, Buf_Size(buf));
		if (h != r && *(h-1) != '/')
			*h++ = '/';
		(void) strcpy(h, p);
		str = string_create(r);
		str = string_flatten(str);
		free(r);
		Lst_Replace(ln, (ClientData)strdup(str->data));
	}
	Buf_Destroy(buf);
	Lst_Close(newsd);
	makechdir(obj, cwd, newsd);
}

static void
makechdir(string_t obj, char *cwd, Lst newsd)
{
	char wd[PATH_MAX+1];
	char *p;
	Buffer buf = Buf_Init(0);

	if (obj != (string_t) NULL) {
		makepath(obj->data, (char *)Lst_Datum(Lst_First(newsd)));
		if (!beSilent)
			printf("cd %s\n", obj->data);
		if (chdir(obj->data) == -1)
			Fatal("%s: %s", obj->data, strerror(errno));
		Var_Set(string_create(".CURDIR"), string_create(cwd),
			VAR_GLOBAL);
		cwd = wd;
		if (getcwd(wd, sizeof(wd)) == 0 || *cwd != '/')
			Fatal("getwd: %s", strerror(errno));
		setenv(_MAKEIOD, _MAKEIOD, 1);
	} else
		Var_Set(string_create(".CURDIR"), sDOT, VAR_GLOBAL);
	setenv(_MAKECWD, cwd, 1);
	if (DEBUG(VAR)) {
		printf("%s=%s\n", _MAKECWD, cwd);
	}
	ltop(newsd, buf);
	p = (char *)Buf_GetBase(buf);
	setenv(_MAKEPSD, p, 1);
	if (DEBUG(VAR)) {
		printf("%s=%s\n", _MAKEPSD, p);
	}
	Buf_Destroy(buf);
}

static int
didchdir(void)
{
	register char *p;

	return (p = getenv(_MAKEIOD)) != 0 && strcmp(p, _MAKEIOD) == 0;
}

static void
makepath(const char *dir, const char *src)
{
	register char *p, *q, *s, *r;
	char *nsrc, *ndir;
	register char c, c1;
	int len;
	struct stat sb;
	string_t str;

	len = strlen(src) + 1;
	if (*src != '/')
		len += strlen(dir) + 1;
	p = nsrc = emalloc(len);
	ndir = strdup(dir);
	if (*src != '/') {
		for (q = ndir; *q; *p++ = *q++)
			;
		*p++ = '/';
	}
	strcpy(p, src);
	str = string_create(nsrc);
	str = string_flatten(str);
	nsrc = strdup(str->data);
	p = nsrc + strlen(nsrc) - 1;
	q = ndir + strlen(ndir) - 1;
	s = p;
	r = q;
	while (q > ndir && p > nsrc && *--q == *--p) {
		if (*q != '/')
			continue;
		r = q + 1;
		s = p + 1;
		while (q > ndir && *(q - 1) == '/')
			--q;
		while (p > nsrc && *(p - 1) == '/')
			--p;
	}
	for (p = ndir; *p == '/'; p++)
		;
	while (*p) {
		while (*++p && *p != '/')
			;
		c = *p;
		*p = 0;
		if (p > r)
			while (*s && *s != '/')
				s++;
		if (stat(ndir, &sb) == -1) {
			sb.st_mode = 0777;
			if (p > r) {
				c1 = *s;
				*s = 0;
				if (stat(nsrc, &sb) != -1 && DEBUG(VAR))
					printf("using mode of %s\n", nsrc);
				*s = c1;
			}
			if (!beSilent)
				printf("mkdir %s\n", ndir);
			if (mkdir(ndir, (int) sb.st_mode & 0777) == -1)
				Fatal("Couldn't make directory: %s", ndir);
		} else if ((sb.st_mode & S_IFMT) != S_IFDIR)
			Fatal("Not a directory: %s", ndir);
		if (p > r)
			while (*s == '/')
				s++;
		*p = c;
		while (*p == '/')
			p++;
	}
}

/*
 * "rev(a) | '/' | b" into buf
 */
static void
rdircat(char *a, char *b, Buffer buf)
{
	register char *p;

	for (p = a; *p; ) {
		if (*p == '/' ||
		    (*p == '.' &&
		     (*(p+1) == 0 || *(p+1) == '/' ||
		      (*(p+1) == '.' && (*(p+2) == 0 || *(p+2) == '/')))))
			Fatal("bad directory: %s", a);
		while (*p && *p++ != '/')
			;
		Buf_AddByte(buf, (Byte)'.');
		Buf_AddByte(buf, (Byte)'.');
		if (*p || *b != 0)
			Buf_AddByte(buf, (Byte)'/');
	}
	if (*b != 0)
		Buf_AddBytes(buf, strlen(b), (Byte *)b);
}

static void
ptol(register const char *p, register Lst *lp)
{
	const char *q;
	Lst l = Lst_Init();

	*lp = l;
	for (;;) {
		q = p;
		while (*p && *p != ':')
			p++;
		if (*p == 0)
			break;
		(void) Lst_AtEnd(l, (ClientData) strndup(q, p - q));
		p++;
	}
	(void) Lst_AtEnd(l, (ClientData) strndup(q, p - q));
}

static void
ltop(register Lst l, register Buffer buf)
{
	register char *q;
	LstNode ln;

	Lst_Open(l);
	if ((ln = Lst_Next(l)) != NILLNODE)
		for (;;) {
			q = (char *) Lst_Datum(ln);
			Buf_AddBytes(buf, strlen(q), (Byte *)q);
			if ((ln = Lst_Next(l)) == NILLNODE)
				break;
			Buf_AddByte(buf, (Byte)':');
		}
	Lst_Close(l);
}

/*
 * FIXME
 * A better way to get this procedure needs to be devised.
 * It should come from libode.
 */
#  define BUILD_VERSION "ODE 2.3.2"
void
ui_print_revision(void)
{
    printf("program :  %s\nrelease :  %s\nbuilt   :  %s\n",
#ifdef BUILD_DATE
           progname, BUILD_VERSION, BUILD_DATE);
#else
           progname, BUILD_VERSION, "bootstrap version");
#endif
} /* ui_print_revision */
