static char sccsid[] = "@(#)58    1.49.5.1  src/bos/usr/ccs/lib/libdbx/main.c, libdbx, bos41J, 9511A_all 2/7/95 16:33:06";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_main, dpi_scanargs, dpi_process_init, scanargs, 
 * 	      setoption
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */
#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "defs.h"
#include "envdefs.h"
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include "main.h"
#include "tree.h"
#include "eval.h"
#include "debug.h"
#include "symbols.h"
#include "scanner.h"
#include "keywords.h"
#include "process.h"
#include "runtime.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "coredump.h"
#include "cma_thread.h"
#include "dbx_msg.h"
#include "dpi_mem.h"
#include <sys/ptrace.h>

nl_catd  scmc_catd;                     /* Cat descriptor for msg conversion */

public boolean coredump;		/* true if using a core dump */
public boolean runfirst;		/* run program immediately */
public boolean interactive;		/* standard input IS a terminal */
public boolean lexdebug;		/* trace scanner return values */
public boolean tracebpts;		/* trace create/delete breakpoints */
public boolean traceexec;		/* trace execution */
public boolean tracesyms;		/* print symbols are they are read */
public boolean traceblocks;		/* trace blocks while reading */
public boolean vaddrs;			/* map addresses through page tables */
public boolean quiet;			/* don't print heading */
public Address *addrstk;
extern boolean lazy;			/* lazy read this file */
extern int last_module;			/* last module processed for lazy */
extern boolean unique_fns;		/* prepend '@' to filenames */
public boolean strip_fortran = true;    /* strip postfix _ from fortran syms */
public boolean attach;                  /* set attach to program flag  */
public boolean reattach;		/* set reattach to program flag(xde) */
public boolean norun;                   /* run and re-run not allowed  */
public boolean isXDE;			/* true if debugger which uses DPI */
public String graphical_debugger;	/* name of the graphical debugger */
			                /*    dbx is running under        */
boolean keep_linkage = false;

#ifndef _NO_PROTO
public int (*rpt_output)( FILE *, const char *, ... );
public int (*rpt_error)( FILE *, const char *, ...);
public int (*rpt_message)( FILE *, const char *, ...);
public int (*rpt_save)( FILE *, const char *, ...);
public int dpi_message ( FILE *, const char *, ...);
public int (*rpt_multiprocess)( int );
public int (*rpt_ctx_level)( int );
#else
public int (*rpt_output)( );
public int (*rpt_error)( );
public int (*rpt_message)( );
public int (*rpt_save)( );
extern int dpi_message();
public int (*rpt_multiprocess)( );
public int (*rpt_ctx_level)( );
#endif
public int (*rpt_executing)( );
public int (*rpt_shell)( );
public int (*rpt_trace)( );
public int (*rpt_open)( );
public void (*resolved_choices)( ) = NULL;
public char *msgptr;

public File corefile;			/* File id of core dump */
public String corename;			/* name of core file */
public jmp_buf profile_env;             /* used for profiling DBX */

#define FIRST_TIME 0			/* initial value setjmp returns */

public  int	*envptr;			/* setjmp/longjmp data */
private boolean scanargs();

private char outbuf[BUFSIZ];		/* standard output buffer */
private char namebuf[512];		/* possible name of object file */
private int firstarg;			/* first program argument (for -r) */

extern Ttyinfo ttyinfo;
/*
 *  Variables for subarrays
 */
struct subdim {
	long ub;
	long lb;
	struct subdim *next, *back;
};
extern struct subdim *subdim_head;
extern struct subdim *subdim_tail;
extern boolean subarray;
extern Address array_addr;
extern Symbol  array_sym;
extern Boolean call_command;
extern long bp_skips;

extern String prevsource;
extern AddrOfFunc *functab;
extern int nfuncs, functablesize;
extern boolean noexec;
extern boolean nonobject;
extern boolean initdone;
extern boolean hascobolsrc;
extern Symbol fake_main;
extern unsigned int nesting_limit;	/* Limit to scoping depths allowed */
extern char *fname;
private setoption();

#define ATTACH 30		/* Attach to a process. */
#define NAME 34			/* Return name of program. */

private boolean scanned_args = false;	/* If arguments have been scanned */
extern boolean badfile;

#ifdef KDBX
extern int kdbx_debug;	/* Debug option of kdbx (-D) : gives information */
			/* about the communication between the two debuggers */
#endif /* KDBX */

/*
 * NAME: dpi_main
 *
 * FUNCTION: Initializes debugger state
 *
 * PARAMETERS:
 *	argc		- Number of arguments
 *	argv		- Array of arguments
 *	outputFun	- Function used to display stdout
 *	errorFun	- Function used to display stderr
 *	execFun		- Informs debugger that the program being debugged is
 *			  currently executing.
 *	shellFun	- Informs debugger that the user has spawned a shell
 *			  from within dbx
 *	traceFun	- Informs debugger a tracepoint has been hit
 *	multpFun	- Informs debugger that dbx has forked
 *	ctxLevelFun	- Informs debugger that an up/down has been executed.
 *	openFun		- 
 *	pid		- Set to the process id of the program being debugged
 *	dpi_state	- Set to the latest action of the debugger
 *	xde_mode	- Indicates if the debugger that called us uses the DPI
 *			  style interface
 *	graph_dbg	- Indicates the name of the debugger, either ADC or XDE
 *
 * RECOVERY OPERATION: Uses setjmp()
 *
 * DATA STRUCTURES: The following external variables are initialized here
 *	graphical_debugger	- Allocated
 *	isXDE
 *	rpt_output
 *	rpt_error
 *	rpt_executing
 *	rpt_shell
 *	rpt_trace
 *	rpt_multiprocess
 *	rpt_ctx_level
 *	rpt_open
 *	rpt_message
 *	action_mask
 *	scmc_catd
 *
 * RETURNS: 0 for success; -1 for failure
 */
int dpi_main (argc, argv, outputFun, errorFun, execFun, shellFun, traceFun,
	  multpFun, ctxLevelFun, openFun, pid, dpi_state, xde_mode, graph_dbg)
int		argc;
char		*argv[];
#ifndef _NO_PROTO
int 		(*outputFun)(FILE *, const char *, ...);
int 		(*errorFun)(FILE *, const char *, ...);
int 		(*multpFun)(int);
int 		(*ctxLevelFun)(int);
#else
int 		(*outputFun)();
int 		(*errorFun)();
int 		(*multpFun)();
int 		(*ctxLevelFun)();
#endif
int 		(*execFun)();
int 		(*shellFun)();
int 		(*traceFun)();
int 		(*openFun)();
int		*pid;
unsigned int	*dpi_state;
boolean 	xde_mode;
char            *graph_dbg;

{
    integer i;
    boolean b;
    String	line;
    jmp_buf	env;
    jmp_buf	rf_env;
    int *svenv;

#ifdef KDBXRT
    int fprintf();
#endif

    /* Initialize external rpt_error before seeking a license because
     * netls_print_error() in netls_funcs.c must use rpt_error to give any error
     * messages to the user.
     *
     * Also need to initialize isXDE, so in case the the license fails we won't
     * exit dex
     */
    rpt_error = errorFun;
    isXDE = xde_mode;

    /*
     * Initialize externals
     */
    graphical_debugger = malloc( strlen( graph_dbg ) + 1 );
    strcpy( graphical_debugger, graph_dbg );
    rpt_output = outputFun;
    rpt_executing = execFun;
    rpt_shell = shellFun;
    rpt_trace = traceFun;
    rpt_multiprocess = multpFun;
    rpt_ctx_level = ctxLevelFun;
    rpt_open = openFun;
    rpt_message = dpi_message;

    action_mask = 0;

    /*  The following line prevents the .env (.kshrc) file
          from being executed when the shell is called
          from shell_popen to expand arguments.  If an
          echo statement is in the .env file, the run 
          subcommand does not work correctly without this
          line.  */
    putenv("ENV=");
    svenv = envptr;
    envptr = env;
    switch (setjmp( env )) {
	case ENVCONT:
		*dpi_state = action_mask;
		restoretty(stdout, &ttyinfo);
		return 0;
	case ENVEXIT:
		*dpi_state = action_mask |= EXECUTION_COMPLETED;
		return 0;
	case ENVQUIT:
		*dpi_state = action_mask |= DBX_TERMINATED;
		return 0;
	case ENVFATAL:
		*dpi_state = action_mask |= DBX_TERMINATED;
		return -1;
	default:
		break;
    }

    if ( scmc_catd == nil ) {
        scmc_catd=catopen("dbx.cat", NL_CAT_LOCALE);
    }

    /*
     * Create dbx prompt; The 3 refers to the '(', ')', and NULL characters
     */
    cmdname = argv[0];
    prompt = (char *) malloc (strlen(cmdname)+3);
    strcpy(prompt,"("); strcat(prompt,cmdname); strcat(prompt,")");

    /*
     * Initialization
     */
    catcherrs();
    onsyserr(EINTR, nil);
    onsyserr(ENXIO, nil);

    setbuf(stdout, outbuf);
    if ( !isXDE ) {
        b = scanargs(argc, argv);
    } else {
	/*
	 * DPI style debuggers do their own scanargs() call.  scanned_args will
	 * be false the first time
	 */
	b = scanned_args;
    }
    if (not runfirst and not quiet) {
	/*
	 * Print version of dbx
	 */
	printheading();
    }
    objsize = 0;
    if ( !attach ) {
	/*
	 * Opens object file and core file if necessary
	 */
	openfiles(b);
        if ( isXDE && badfile ) {
		/*
		 * Couldn't read object file
		 */
                envptr = svenv;
                return -1;
        }
    }
    stab_init();
    language_init();
    symbols_init();
    process_init();
    if (runfirst) {
	/*
	 * Used -r flag
	 */
        savetty(stdout, &ttyinfo);
	envptr = rf_env;
	if (setjmp(rf_env) == FIRST_TIME) {
	    /* Initializes arguments to send to program to debug */
    	    arginit();
	    /* Add arguments to arguments to send to program to debug */
	    for (i = firstarg; i < argc; i++) {
            	newarg(argv[i],false);
   	    }
	    /* Initialize default variable */
	    defvar(identname("$ignoreload", true), nil);
	    /* Start program to debug executing */
	    run();
	    /* NOTREACHED */
	} else {
	    runfirst = false;
	}
	*dpi_state = action_mask;
        if (( action_mask & DBX_TERMINATED ) && (isXDE)) {
             envptr = svenv;
             return -1;
        }
    } else {
	init();
	if ( not isstdin( )) {
	    line = nil;
	    /*
	     * Send a blank command to dbx; Allows us to set dpi_state
	     * to appropriate state of debugger
	     */
    	    if (dpi_command(line, dpi_state) != nil) {
		envptr = svenv;
                return -1;
            }
	}
	*pid = process->pid;
    }
    envptr = svenv;
    return 0;
}

/*
 * Scan the argument list.
 *
 * Return whether or not an object file was specified.
 */

private boolean scanargs (argc, argv)
int argc;
String argv[];
{
    int i, j, procid;
    boolean foundfile;

#ifdef KDBX
    kdbx_debug = false;
#endif /* KDBX */
    runfirst = false;
    interactive = false;
    lexdebug = false;
    tracebpts = false;
    traceexec = false;
    tracesyms = false;
    traceblocks = false;
    vaddrs = false;
    quiet = false;
    /* autostrip = true; */
    foundfile = false;
    corefile = nil;
#ifdef KDBX
    coredump = false;
#else
    coredump = true;
#endif
    multproc = off;                     /* no multi-processing    */
    attach = false;                     /* set attach to program flag  */
    norun = false;                      /* run and re-run allowed  */

    /*
     * Set the default lazy reading mode based on whether this is a DPI style
     * debugger or dbx
     *
     * Since the DPI style debuggers are already released, they don't know about
     * the -F flag to turn off lazy reading.  They only know the -f flag to turn
     * on lazy reading.  Therefore, we need the default for these debuggers to
     * be non-lazy reading, and only if the -f flag is specified do we want to
     * turn lazy reading on.
     */
    if( isXDE ) {
	lazy = false;
    } else {
	lazy = true;
    }
    unique_fns = false;
    strip_fortran = true;
    sourcepath = list_alloc();
    initfile = ".dbxinit";
    nesting_limit = 25;	/* Matches initialization in object.c */

    /*  start the use path with "@" - use fullpath info  */
    list_append(list_item("@"), nil, sourcepath);

    /*  add the current directory to the use path  */
    list_append(list_item("."), nil, sourcepath);
    i = 1;
    while (i < argc and (not foundfile or (coredump and corefile == nil))) {
	if ((argv[i][0] == '-') and (not foundfile)) {
	    if (!strncmp(argv[i], "-I", 2)) {
		if (strlen(argv[i]) == 2) {
		    ++i;
		    if (i >= argc) {
			fatal( catgets(scmc_catd, MS_main, MSG_229,
						  "missing directory for -I"));
		    }
		list_append(list_item(argv[i]), nil, sourcepath);
		}
		else {
		    list_append(list_item(&argv[i][2]), nil, sourcepath);
		}
	    } else if (!strncmp(argv[i], "-c", 2)) {
		if (strlen(argv[i]) == 2) {
		    ++i;
		    if (i >= argc) {
		        fatal( catgets(scmc_catd, MS_main, MSG_230,
					  "missing command file name for -c"));
		    }
		    initfile = argv[i];
		}
		else {
		    initfile = &argv[i][2];
		}
            } else if ( (!strncmp(argv[i], "-a", 2)) ||    /* attach to pid */
		        (!strncmp(argv[i], "-A", 2)) ) {
		if (!strncmp(argv[i], "-A", 2))
		  reattach = true;	/* do reattach instead of attach */
                if (strlen(argv[i]) == 2) {
                    ++i;
                    if (i >= argc) {
                        fatal( catgets(scmc_catd, MS_main, MSG_231,
					     "missing proc id number for -a"));
                    }
                    procid = atoi(argv[i]);
                }
                else {
                    procid = atoi(&argv[i][2]);
                }
                getprog(procid, false);            /* attach */
		foundfile = true;
            } else if (!strncmp(argv[i], "-t", 2)) {    /* type limit */
                /*  ignore -t flag - continue to allow for backwards
                      compatibility  */
                if (strlen(argv[i]) == 2)
                    ++i;
            } else if (!strncmp(argv[i], "-d", 2)) {    /* nesting limit */
                if (strlen(argv[i]) == 2) {
                    ++i;
                    if (i >= argc) {
                        fatal( catgets(scmc_catd, MS_main, MSG_248,
		    "Nesting depth limit was not specified after -d option"));
                    }
                    nesting_limit = (unsigned int) atoi(argv[i]);
                }
                else {
                    nesting_limit = (unsigned int) atoi(&argv[i][2]);
                }
		if (nesting_limit == 0) {
                        warning( catgets(scmc_catd, MS_main, MSG_253,
			"Specified nesting limit could not be interpreted.\n\
Nesting limit used will be 25.\n"));
                        nesting_limit = 25;
		} else if (nesting_limit > 32767) {	/* Arbitrarily large */
                        warning( catgets(scmc_catd, MS_main, MSG_257,
		"Specified nesting limit is greater than maximum of 32767.\n\
Nesting limit used will be 32767.\n"));
                        nesting_limit = 32767;
		}
	    }
	    else {
	    	for (j = 1; argv[i][j] != '\0'; j++) {
	        setoption(argv[i][j]);
	    }
	    }
	} else if (not foundfile) {
	    objname = argv[i];
	    foundfile = true;
	} else if (coredump and corefile == nil) {
	    corefile = fopen(argv[i], "r");
	    corename = argv[i];
	    if (corefile == nil) {
		coredump = false;
	    }
	}
	++i;
    }
    if (i < argc and not runfirst) {
      if (attach)
      {
	warning( catgets(scmc_catd, MS_main, MSG_249,
					   "extraneous argument %s"), argv[i]);
      }
      else
      {
	fatal( catgets(scmc_catd, MS_main, MSG_249,
					   "extraneous argument %s"), argv[i]);
      }
    }
    firstarg = i;
    return foundfile;
}

/*
 * Take appropriate action for recognized command argument.
 */

private setoption(c)
char c;
{
    switch (c) {
	case 'r':   /* run program before accepting commands */
	    runfirst = true;
	    coredump = false;
	    break;

	case 'i':
	    interactive = true;
	    break;

	case 'b':
	    tracebpts = true;
	    break;

	case 'e':
	    traceexec = true;
	    break;

	case 'F': /* Full symbols */
            lazy = false;
	    break;

	case 'f': /* obsolete - accept for backwards compatibility */
	    if( isXDE ) {
		lazy = true;
	    }
	    break;

	case 's':
	    tracesyms = true;
	    break;

	case 'n':
	    traceblocks = true;
	    break;

	case 'k':
	    vaddrs = true;
	    break;

	case 'q':
	    quiet = true;
	    break;
/****
	case 'o':
	    autostrip = false;
	    break;
****/
	case 'u':
	    unique_fns = true;
	    break;

	case 'x':
	    strip_fortran = false;
	    break;

	case 'L':
	    keep_linkage = true;
	    break;

	case 'l':
#   	    ifdef LEXDEBUG
		lexdebug = true;
#	    else
	        fatal( catgets(scmc_catd, MS_main, MSG_258,
						    "unknown option '%c'"), c);
#	    endif
	    break;

#ifdef KDBX
	case 'D': /* DEBUG */
		kdbx_debug = 1;
		break;
#endif /* KDBX */

	default:
                fatal(catgets(scmc_catd, MS_help, MSG_804,
                     "DBX Startup Options:\n\
\n\
dbx [-a ProcessID] [-c CommandFile] [-d NestingDepth] [-I Directory]\n\
[-k] [-u] [-x] [-F] [-L] [-r] [ObjectFile [CoreFile]]\n\
\n\
\t-a ProcessID        Attach to specified process\n\
\t-c CommandFile      Run dbx subcommands in specified file first\n\
\t-d NestingDepth     Set limit for nesting of program blocks\n\
\t-I Directory        Include Directory in list of directories\n\
\t                    searched for source files\n\
\t-k                  Map memory addresses\n\
\t-u                  Prepend file name symbols with an '@'\n\
\t-x                  Strip postfix '_' from FORTRAN symbols\n\
\t-F                  Read all symbols at start-up time\n\
\t-L                  Keep linkage symbols\n\
\t-r                  Run object file immediately\n"));
    }
}

/*
 * NAME: dpi_process_init
 *
 * FUNCTION: Re-Initializes the process data structure and other global
 *	     variables so a new process can be started.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS: NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
void dpi_process_init( outputFun )
#ifndef _NO_PROTO
int 		(*outputFun)(FILE *, const char *, ...);
#else
int 		(*outputFun)();
#endif
{
	int	i;
	extern int typenummax;
	extern Symbol *last_typetable;
	extern Symbol **typetab_ary;
	extern int *typenummax_ary;


	if (process) {
		pterm(process);
		process->pid = 0;
		process->status = 0;
		process->signo = 0;
		process->exitval = 0;
		process->sigset[0] = 0;
		process->sigset[1] = 0;
		process->sigstatus = 0;
                process->is_bp = false;
                cacheflush( process );
	}

	nfiles_total = 0;
	action_mask = 0;
	runfirst  = false;
	coredump = false;
	last_module = -2;	/* Matches initialization in object.c */
	stab_compact_level = 0;	/* Matches initialization in object.c */
	symcase = mixed;	/* Matches initialization in object.c */
	
	norun = false;
	attach = false;
	noexec = false;
	quiet = false;
	dispose(fname);

        hascobolsrc = false;
        fake_main = nil;
#if defined (CMA_THREAD) ||  defined (K_THREADS)
	running_thread = nil;
#endif /* CMA_THREAD || K_THREADS */

	/*
	 * dispose() will free the variable, and then set it to NULL
	 */
        for( i = 0; i < loadcnt; i++ ) {
            if( filetab[i] != NULL ) {
                free( filetab[i] );
            }
            if( linetab[i] != NULL ) {
                free( linetab[i] );
            }
            if( typetab_ary[i] != NULL ) {
                free( typetab_ary[i] );
            }
        }
	dispose (filetab);
	dispose (linetab);
	dispose (lineaux);
	dispose (loader_info);
	dispose (fd_info);
	dispose (functab);
	dispose (addrstk);
	dispose (graphical_debugger);
	dispose (nlhdr);
	dispose (typetab_ary);
	dispose (typenummax_ary);
	last_typetable = nil;

	cursource = 0;
	prevsource = 0;
	symbol_free();
	names_free();
	MemFuncTabInit();
	string_debug_tab_free();
	string_pool_free();
	nfuncs = 0;
	functablesize = 0;
	badfile = false;
	nonobject = false;
	initdone  = false;
	resetUnnamedBlockbnum();
	free_sourcepath();
	typenummax = -1;

        rpt_output = outputFun;
        if ( scmc_catd == nil ) {
            scmc_catd=catopen("dbx.cat", NL_CAT_LOCALE);
        }
}

/*
 * NAME: dpi_scanargs
 *
 * FUNCTION: Calls scanargs() to read command line arguments
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *	argc		- Number of arguments
 *	argv		- Array of arguments
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 if failure;
 *	    false if arguments were not scanned;
 *	    true if arguments scanned
 */
boolean dpi_scanargs(argc, argv)
int argc;
String argv[];
{
	jmp_buf  env;
        int *svenv;
	
        svenv = envptr;
	envptr = env;
	switch ( setjmp( env ) ) {
	case ENVFATAL :
		return -1;
	default : 
		break;
	}
	scanned_args = scanargs(argc, argv);
        envptr = svenv;
	return scanned_args;
}

/*
 * NAME: dpi_set_resolve_callback
 *
 * FUNCTION: Set the resolve callback
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *	func		- Resolve callback function
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Nothing
 */
void dpi_set_resolve_callback(void (*func)())
{
	resolved_choices = func;
}
