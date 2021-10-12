#ifndef lint
static char sccsid[] = "@(#)20  1.20 src/bos/usr/ccs/bin/make/main.c, cmdmake, bos41J, 9519B_all 5/5/95 13:21:47";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Error
 *		Fatal
 *		MainParseArgLine
 *		MainParseArgs
 *		Punt
 *		ReadMakefile
 *		SccsGet
 *		SccsReadMakefile
 *		emalloc
 *		enomem
 *		main
 *		usage
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
static char rcsid[] = "@(#)$RCSfile: main.c,v $ $Revision: 2.12.6.4 $ (OSF) $Date: 1992/09/24 14:42:16 $";
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
 * main.c --
 *	The main file for this entire program. Exit routines etc
 *	reside here.
 *
 * Utility functions defined in this file:
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
 */

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/localedef.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include "make.h"
#include "pathnames.h"

extern Lst   		dirSearchPath;  /* main search path */

static Lst		create;		/* Targets to be made */
time_t			now;		/* Time at start of make */
GNode			*DEFAULT;	/* .DEFAULT node */
Boolean			allPrecious;	/* .PRECIOUS given on line by itself */
int			mb_cur_max;	/* Max length of multi-byte char */
char			**months;	/* Array of abbreviated month names */

Boolean		noBuiltins;	/* -r flag */
static Lst		makefiles;	/* ordered list of makefiles to read */
Boolean			debug;		/* -d flag */
Boolean			noExecute;	/* -n flag */
Boolean			keepgoing;	/* -k flag */
int	keepgoing_error_count;
Boolean			queryFlag;	/* -q flag */
int			queryExit;	/* -q exit value */
Boolean			touchFlag;	/* -t flag */
Boolean			ignoreErrors;	/* -i flag */
Boolean			beSilent;	/* -s flag */
Boolean			checkEnvFirst;	/* -e flag */
Boolean			isFpathSet;	/* is FPATH in shell environment? */
Boolean			addLeadBlank;	/* add leading blank to variable
					 * assignments?  For Var_Append. */
static Boolean		ReadMakefileGlobal(char *);
static Boolean		ReadMakefile(char *,const GNode *ctxt);
static Boolean		SccsReadMakefile(char *,const GNode *ctxt);
static void		SccsGet(char *);

static void		usage(void);

/*-
 * MainParseArgs --
 *	Parse a given argument vector.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Various global and local flags will be set depending on the flags
 *	given
 */
static void
MainParseArgs(
	int argc,
	const char **argv
	)
{
	int c;

	short *argmask;  /* array of shorts to mask non-options from argv */
	optind = 1;	/* since we're called more than once */

	if (!(argmask = (short *)calloc(argc,sizeof(short))))
		enomem();

	/* Loop around getopts() so we can handle the "-" option. */
	while (1)
	{
		/*
		 * NOTE:
		 *
		 *	Make uses its own version of getopt() to take advantage
		 *	of the optional parameter feature, as indicated by the
		 *	semicolon in the options list. The AIX libc version of
		 *	getopt does not support this feature, so the OSF 1.2
		 *	libc version of getopt was included as part of make
		 *	itself. Once AIX libc supports the optional parameter
		 *	feature, make can go back to using the regular AIX libc
		 *	version of getopt.
		 */
	#ifdef DEBUG_FLAG
		while((c = getopt(argc, argv, "D:Sd;ef:iknpqrst")) != -1) {
	#else
		while((c = getopt(argc, argv, "D:Sef:iknpqrst")) != -1) {
	#endif
			switch(c) {
			case 'D':
				Var_Set(optarg, "1", VAR_GLOBAL);
				Var_Append("MAKEFLAGS", "-D", VAR_GLOBAL);
				Var_Append("MAKEFLAGS", optarg, VAR_GLOBAL);
				break;
			case 'S':
				keepgoing = FALSE;
				Var_Append("MAKEFLAGS", "-S", VAR_GLOBAL);
				break;
	#ifdef DEBUG_FLAG
			case 'd': {
				char *modules = optarg;
				if (modules == NULL) 
					modules = "A";

				for (; *modules; ++modules)
					switch (*modules) {
					case 'A':
						debug = ~0;
						break;
					case 'a':
						debug |= DEBUG_ARCH;
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
				Var_Append("MAKEFLAGS", "-d", VAR_GLOBAL);
				Var_Append("MAKEFLAGS", optarg, VAR_GLOBAL);
				break;
			}
	#endif /* DEBUG_FLAG */
			case 'e':
				checkEnvFirst = TRUE;
				Var_Append("MAKEFLAGS", "-e", VAR_GLOBAL);
				break;
			case 'f':
				(void)Lst_AtEnd(makefiles, (ClientData)optarg);
				break;
			case 'i':
				ignoreErrors = TRUE;
				Var_Append("MAKEFLAGS", "-i", VAR_GLOBAL);
				break;
			case 'k':
				keepgoing = TRUE;
				Var_Append("MAKEFLAGS", "-k", VAR_GLOBAL);
				break;
			case 'n':
				allPrecious = TRUE;
				noExecute = TRUE;
				Var_Append("MAKEFLAGS", "-n", VAR_GLOBAL);
				break;
			case 'p':
				allPrecious = TRUE;
				/* Print out the complete set of macro
				   definitions, target descriptions, etc. */
				debug|=DEBUG_GRAPH1;
				/* POSIX says we don't put the "-p" option in
				   MAKEFLAGS. */
				break;
			case 'q':
				allPrecious = TRUE;
				noExecute = TRUE;
				queryFlag = TRUE;
				/* Kind of nonsensical, wot? */
				Var_Append("MAKEFLAGS", "-q", VAR_GLOBAL);
				break;
			case 'r':
				noBuiltins = TRUE;
				Var_Append("MAKEFLAGS", "-r", VAR_GLOBAL);
				break;
			case 's':
				beSilent = TRUE;
				Var_Append("MAKEFLAGS", "-s", VAR_GLOBAL);
				break;
			case 't':
				touchFlag = TRUE;
				Var_Append("MAKEFLAGS", "-t", VAR_GLOBAL);
				break;
			default:
			case '?':
				usage();
			}
		}

		/* If the argument getopt() returned a -1 on was beyond
		   end of options, we're done parsing cmd line. */
                if (optind >= argc || argv[optind] == NULL)
			break;

		/* if we encounter a "--", then all remaining items are
			not options. */
		if (strcmp(argv[optind], "--") == 0) 
		{
			for ( ; optind < argc; ++optind)
				argmask[optind] = 1;
			break;
		}

		/* If the argument getopt() returned a -1 on wasn't "-"
		   then we've found a "non-option" item (macro or target).
		   Add it to the non-option argument mask.		*/
		if (strcmp(argv[optind],"-"))
			argmask[optind] = 1;

		/* Increment past "-" and call getopts() again. */
		optind++;
	}

	/*
	 * Rescan the arguments for variable assignments and perform them 
	 * if found.  Else take them to be targets and stuff them
	 * on the end of the "create" list.
	 */
	optind = 1;
	for (++argv ; *argv; ++argv, ++optind) {
		if (argmask[optind]) {
			if (Parse_IsVar(*argv))
			{
				/* Put quotes around the variable assignment so
				that if MAKEFLAGS gets evaluated, such as in a
				recursive make, MAKEFLAGS will be parsed
				correctly -- but we don't want blanks within
				the string, like " VAR=value ".  Bad. */
				Var_Append("MAKEFLAGS", "\"", VAR_GLOBAL);
				addLeadBlank = FALSE;
				Var_Append("MAKEFLAGS", *argv, VAR_GLOBAL);
				Var_Append("MAKEFLAGS", "\"", VAR_GLOBAL);
				addLeadBlank = TRUE;  /* restore default */
				Parse_DoVar(*argv, VAR_CMD);
			}
			else {
				if (!**argv)
					Punt(MSGSTR(ARGCORRUPT, 
						"Argument list is corrupted."));
				(void)Lst_AtEnd(create, (ClientData)*argv);
			}
		}
	}
}

/*-
 * MainParseArgLine --
 *  	Used by main() when reading the MAKEFLAGS environment variable.
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
static void
MainParseArgLine(
	char *prog,
	char *line			/* Line to fracture */
	)
{
	char **argv;			/* Manufactured argument vector */
	int argc;			/* Number of arguments in argv */
	char *pre_dash;			/* String with prepended dash */

	if (line == NULL)
		return;
	for (; *line == ' '; ++line);
	if (!*line)
		return;

	argv = Str_Break(prog, line, &argc);
	if (!Parse_IsVar(argv[1]) && *argv[1] != '-') {
		emalloc(pre_dash,(int)strlen("-") +
			 (int)strlen(argv[1]) + 1);
		strcpy(pre_dash, "-");
		strcat(pre_dash, argv[1]);
		argv[1] = pre_dash;
	}
	MainParseArgs(argc, argv);
}

/*-
 * main --
 *	The main function, for obvious reasons. Initializes variables
 *	and a few modules, then parses the arguments give it in the
 *	environment and on the command line. Reads the system makefile
 *	followed by either Makefile, makefile or the file given by the
 *	-f argument. Sets the MAKEFLAGS Make variable based on all the
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
main(
	int argc,
	char **argv
	)
{
	Lst targs;	/* target nodes to create -- passed to Make_Init */
	char *p;
	char *env_value; 	/* value of environment variable */
	int *time_addr;		/* address of LC_TIME structure */
	int month_off;		/* offset of abmon in LC_TIME */

	setlocale( LC_ALL, "");
	catd = catopen(MF_MAKE,NL_CAT_LOCALE);

	mb_cur_max = MB_CUR_MAX;	/* Init max size of multi-byte char */
	create = Lst_Init(FALSE);
	makefiles = Lst_Init(FALSE);
	beSilent = FALSE;		/* Print commands as executed */
	ignoreErrors = FALSE;		/* Pay attention to non-zero returns */
	noExecute = FALSE;		/* Execute all commands */
	keepgoing = FALSE;		/* Stop on error */
	keepgoing_error_count=0;
	allPrecious = FALSE;		/* Remove targets when interrupted */
	queryFlag = FALSE;		/* This is not just a check-run */
	queryExit = 0;			/* Exit value if check-run */
	noBuiltins = FALSE;		/* Read the built-in rules */
	touchFlag = FALSE;		/* Actually update targets */
	isFpathSet = FALSE;		/* Will set if FPATH found in env. */
	addLeadBlank = TRUE;		/* Normally want spaces between vars
					 * when put together. For Var_Append */
	debug = 0;			/* No debug verbosity, please. */
    
	/*
	 * Calculate address of abbreviated month names in the locale
	 * structures. It is kept in the LC_TIME structure. First, get
	 * the address of the LC_TIME structure in the locale. Next,
	 * get the offset of the abbreviated month names. Set the
	 * months variable to point to the array of abbreviated month
	 * names used by the Targ_FmtTime() routine in targ.c
	 */
	time_addr = (int *) &__lc_time;
	month_off = &((_LC_time_t *)0)->abmon;
	months = (char **)(*time_addr + month_off);

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
	Var_Set("MAKEFLAGS", "", VAR_GLOBAL);
	Var_Set("MFLAGS", "", VAR_GLOBAL);
	Var_Set("SHELL", _PATH_SHELL, VAR_GLOBAL);

	/*
	 * First snag any flags out of the MAKEFLAGS environment variable.
	 * If MAKEFLAGS is empty, check MFLAGS for backward compatibility.
	 */
	env_value = getenv("MAKEFLAGS");
	if (env_value[0] == '\0' || env_value == NULL)
		env_value = getenv("MFLAGS");
	MainParseArgLine(argv[0], env_value);
	MainParseArgs(argc, argv);

	/*
	 * Is FPATH set in the environment?  If so, then when it comes
	 * time to build the target, we MUST use the shell for all commands
	 * because for all we know, one of the commands might be a shell
	 * FPATH function.
	 */
	if (NULL != getenv("FPATH"))
		isFpathSet = TRUE;

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
	 * Read in the built-in rules first, followed by the specified makefile,
	 * if it was (makefile != (char *) NULL), or the default Makefile and
	 * makefile, in that order, if it wasn't.
	 */
	env_value = getenv("MAKERULES");
	if (env_value[0] == '\0' || env_value == NULL)
		env_value = _PATH_AIXMK;
 
	if (!noBuiltins && !ReadMakefile(env_value, VAR_INTERNAL))
		Fatal(MSGSTR(INTRULESERR, "make: Cannot open "
			"the internal rules file %s."), env_value);

	if (!Lst_IsEmpty(makefiles)) {
		LstNode ln;

		ln = Lst_Find(makefiles, (ClientData)NULL, ReadMakefileGlobal);
		if (ln != NILLNODE)
			Fatal(MSGSTR(CANTOPEN1, "make: Cannot open %s."), 
				(char *)Lst_Datum(ln));
	} else if (!ReadMakefile("makefile",VAR_GLOBAL))
			if (!ReadMakefile("Makefile",VAR_GLOBAL))
				if (!SccsReadMakefile("makefile",VAR_GLOBAL))
					(void)SccsReadMakefile("Makefile",VAR_GLOBAL);

	/* Install all the flags into the MAKE envariable. */
	if ((p = Var_Value("MAKEFLAGS", VAR_GLOBAL)) && *p)
#ifdef _OSF
		setenv("MAKEFLAGS", p, 1);
#else
	{
		char *envstr; 		/* Environment String	*/

		emalloc(envstr,(int)strlen("MAKEFLAGS=") +
			 (int)strlen(p) + 1);
		strcpy(envstr, "MAKEFLAGS=");
		strcat(envstr, p);
		putenv(envstr);
	}
#endif /* _OSF */

        /*
         * For compatibility, look at the directories in the VPATH variable
         * and add them to the search path, if the variable is defined. The
         * variable's value is in the same format as the PATH envariable, i.e.
         * <directory>:<directory>:<directory>...
         */
        if (Var_Exists("VPATH", VAR_CMD)) {
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
                        DirAddDir(dirSearchPath, path);
                        *cp = savec;
                        path = cp + 1;
                } while (savec == ':');
                (void)free((Address)vpath);
        }
	/* The SCCS subidirectory of the current directory must also be
	 * searched.  We add it last, so that user-defined VPATH entries
	 * (if present) will get picked up first.
	 */
		DirAddDir(dirSearchPath,"SCCS");

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
	 * Compat_Run will take care of creating all the targets as
	 * well as initializing the module.
	 */
	Compat_Run(targs);
    
	/* print the graph now it's been processed if the user requested it */
	if (DEBUG(GRAPH2))
		Targ_PrintGraph(2);

	/* If "-q" given. */
	if (queryFlag)
	{
		/* If "-k" given and there were errors then return with
			keepgoing_error_count. */
		if ((keepgoing) && (keepgoing_error_count))
		{
			return(keepgoing_error_count + 1);
		}

		return(queryExit);
	}

	/* If "-k" given then use the keepgoing_error_count value. */
	if (keepgoing)
	{
		return(keepgoing_error_count);
	}

	return(0);
}

static Boolean
ReadMakefileGlobal(
char *fname	/* makefile to read */
)
{
	ReadMakefile(fname,VAR_GLOBAL);
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
static Boolean
ReadMakefile(
	char *fname,		/* makefile to read */
	const GNode          *ctxt      /* Context in which to do the
					read. */
	)
{
	FILE *stream;

	if (!strcmp(fname, "-")) {
		Parse_File("(stdin)", stdin, ctxt);
	} else {
		if ((stream = fopen(fname, "r")) == NULL)
			return(FALSE);
		Parse_File(fname, stream, ctxt);
		(void)fclose(stream);
	}
	return(TRUE);
}

/*-
 * SccsReadMakefile  --
 *	SCCS get, open, and parse the given makefile.
 *
 * Results:
 *	TRUE if ok. FALSE if couldn't open file.
 *
 * Side Effects:
 *	lots
 */
static Boolean
SccsReadMakefile(
	char *fname,		/* makefile to read */
	const GNode          *ctxt      /* Context in which to do the
					read. */
	)
{
	FILE *stream;

	SccsGet(fname);
	if ((stream = fopen(fname, "r")) == NULL)
		return(FALSE);

	/* 
	 * Unlink now, so if Parse_File dies unexpectedly, the
	 * temporarily fetched SCCS file will go away. 
	 */
	unlink (fname);	

	Parse_File(fname, stream,ctxt);
	(void)fclose(stream);

	return(TRUE);
}

/*-
 * SccsGet -
 *	SCCS get file.
 *
 * Results:
 *	File created in current directory upon success.
 *
 * Side Effects:
 *	
 */
static void
SccsGet(
	char *fname
	)
{
	char sccs_cmd[] = "get SCCS/s.makefile";
		/* Initialize to the longest possible sccs command. */
	int  cpid, status, reason;
	char **av;	/* Argument vector for thing to exec */
	struct stat     stb;      /* Buffer for stat, if necessary */

	/* Form the proper "sccs" command. */
	sprintf(sccs_cmd,"get s.%s",fname);

	/* if we can't get the status on "s.fname" then try "SCCS/s.fname". */
	if (stat(&sccs_cmd[4], &stb) != 0) 
	{
		/* Form the proper "sccs" command. */
		sprintf(sccs_cmd,"get SCCS/s.%s",fname);

		/* if we can't get the status on "SCCS/s.fname" then return. */
		if (stat(&sccs_cmd[4], &stb) != 0)
		{
			return;
		}
	}

	printf("+ %s\n", sccs_cmd);

	av = Str_Break("sccs", sccs_cmd, (int *)NULL);

	cpid = fork();
	if (cpid < 0) {
		Fatal(MSGSTR(CANTFORK, "make: Cannot fork a "
			"new process for %s."), &sccs_cmd[4]);
	}
	
	if (cpid == 0) { /* child: fetch makefile */
#ifdef _OSF
		int fd;

		/* force sccs command's outputs to /dev/null */
		fd = open(_PATH_DEV_NULL, O_WRONLY);
		dup2(fd, 1); dup2(fd, 2);
#endif /* _OSF */

		/* try using user's current path */
		execvp(av[0], av);

		/* user's path failed, try using hard coded path */
		av[0] = _PATH_SCCS;
		execv(av[0], av);
		exit(1);
	}

	/* parent: wait for child to complete */
	status = waitpid(cpid, &reason, 0); 
	
	if (status == -1) {
	    Fatal(MSGSTR(BADWAIT, "make: The wait system call "
		"failed with status %d."), status);
	}
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
/* VARARGS */
void
Error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n',stderr);
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
/* VARARGS */
void
Fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n',stderr);
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
/* VARARGS */
void
Punt(const char *fmt, ...)
{
	va_list ap;

	fputs("make: ",stderr);
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n',stderr);
	(void)fflush(stderr);

	if (DEBUG(GRAPH2))
		Targ_PrintGraph(2);
	exit(2);		/* Not 1, so -q can distinguish error */
}

/*
 * enomem --
 *	die when out of memory.
 */
void	*
enomem(void)
{
	(void)fprintf(stderr, "make: %s.\n", strerror(errno));
	exit(2);
}

/*
 * usage --
 *	exit with usage message
 */
static void
usage(void)
{
	(void)fprintf(stderr, MSGSTR(USAGE, "usage: make [-eiknqrst] [-k|-S] "
		"[-d[A|adg[1|2]msv]] [-D variable] [-f makefile ] "
		"[variable=value ...] [target ...]\n"));
	exit(2);
}
