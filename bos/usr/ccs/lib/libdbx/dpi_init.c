static char sccsid[] = "@(#)74	1.31.2.22  src/bos/usr/ccs/lib/libdbx/dpi_init.c, libdbx, bos41J, 9520A_a 5/17/95 17:37:29";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: catchintr, erecover, exec_init, init, openfiles, printheading,
 *	      quit, reinit, restoretty, savetty, dpi_set_jump_env,
 *	      dpi_is_runfirst, dpi_get_object_name
 *            update_lib_type
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/limits.h>
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
#include "dbx_msg.h"                    /* include file for message texts */
#include "frame.h"
#include "cma_thread.h"
#if defined (CMA_THREAD) || defined (K_THREADS)
#include "k_thread.h"
#endif /* CMA_THREAD || K_THREADS */



nl_catd  scmc_catd;                    /* Cat descriptor for scmc conversion */

public boolean coredump;		/* true if using a core dump */
public boolean runfirst;		/* run program immediately */
public boolean interactive;		/* standard input IS a terminal */
public boolean lexdebug;		/* trace scanner return values */
public boolean tracebpts;		/* trace create/delete breakpoints */
public boolean traceexec;		/* trace execution */
public boolean tracesyms;		/* print symbols are they are read */
public boolean traceblocks;	        /* trace blocks while reading */
public boolean vaddrs;			/* map addresses through page tables */
public boolean quiet;			/* don't print heading */
/* public boolean autostrip; */		/* strip C++ prefixes */
public fork_type multproc = off;        /* multi-process */
public boolean attach;                  /* set attach to program flag  */
public boolean norun;                   /* run and re-run not allowed  */
public boolean isXDE;			/* true if running XDE */

public File corefile;			/* File id of core dump */
public String corename;			/* name of core file */
public jmp_buf profile_env;             /* used for profiling DBX */

#define FIRST_TIME 0			/* initial value setjmp returns */

public  Boolean initdone = false;	/* true if initialization done */
public  jmp_buf env;			/* setjmp/longjmp data */
private char outbuf[BUFSIZ];		/* standard output buffer */
private char namebuf[512];		/* possible name of object file */
private int firstarg;			/* first program argument (for -r) */

public Ttyinfo ttyinfo;
public Ttyinfo ttyinfo_in;		/* saved terminal info for stdin */

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
void kill_parents();

#ifndef _NO_PROTO
public void catchintr(int);
#else
public void catchintr();
#endif
public boolean just_attached = false;
extern Boolean call_command;
extern Frame callframe;
extern long bp_skips;
extern int *envptr;
extern jmp_buf profile_env;

public void (*dpi_update_proc) = NULL;	/* Proc to call while wait()ing	*/
public int  dpi_update_count = 0;	/* whether above has been called*/
public int  dpi_update_delay = 0;	/* Amount of time between calls	*/
#if defined (CMA_THREAD) || defined (K_THREADS)
public int  lib_type = NO_THREAD;       /* lib type : libcma or libpthreads */
#endif /* CMA_THREAD || K_THREADS */

#define ATTACH 30		/* Attach to a process. */
#define NAME 34			/* Return name of program. */
#define ERR_MSG -1              /* Return value if msg catalog fails to open */

#if defined (CMA_THREAD) || defined (K_THREADS)

/*
 * NAME: update_lib_type
 *
 * FUNCTION:  update lib_type : indicate the type of threads we are
 *            debugging (linkedited with libcma or libpthreads or with nothing).
 *
 * PARAMETERS:NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */

public void  update_lib_type()
{
    Symbol known;               /* symbol for thread object list */
    known = lookup(identname("cma__g_known_threads",true));
    if (known == nil) {
        known = lookup(identname("__dbx_known_pthreads",true));
        if (known == nil) lib_type = NO_THREAD;
        else lib_type = KERNEL_THREAD;
    }
    else lib_type = C_THREAD;
}
#endif /* CMA_THREAD || K_THREADS */

public printheading ()
{
    if (!isXDE)
      (*rpt_output)(stdout, catgets(scmc_catd, MS_main, MSG_100,
			"Type 'help' for help.\n"));
}

/*
 * Initialize the world, including setting initial input file
 * if the file exists.
 */

public init()
{
    File f;
    String home;
    char buf[PATH_MAX];
    int flag_initfile;
    extern String getenv();

    savetty(stdout, &ttyinfo);
    savetty(stdin, &ttyinfo_in);
    keywords_free();
    enterkeywords();
    scanner_init();
    if (not coredump and not runfirst) {
#ifdef KDBX
	kdbx_init();
#endif
	startprog(nil, nil, nil);
    }
    (*rpt_output)(stdout, catgets(scmc_catd, MS_main, MSG_99,
					  "reading symbolic information ..."));
    if ( isXDE ) (*rpt_output)(stdout, "\n");
    fflush(stdout);
    if ((!coredump) && (!norun))
        getldrinfo(false);
    if (!norun) {
        readobj(objname);
    } else {
        readobj(nil);
    }
    (*rpt_output)(stdout, "\n");
    if (coredump) {
	(*rpt_output)(stdout, catgets(scmc_catd, MS_main, MSG_136,
				    "[using memory image in %s]\n"), corename);
	if (vaddrs) {
	    coredump_getkerinfo();
	}
	getsrcpos();
	setcurfunc(whatblock(pc));
#if defined (CMA_THREAD) ||  defined (K_THREADS)
	threads(th_get_info, nil);    /* get info about any existing threads */
#endif /* CMA_THREAD || K_THREADS */
	printstatus();
    } else if (attach) {
	pc = reg(SYSREGNO(PROGCTR));
	getsrcpos();
	setcurfunc(whatblock(pc));
#if defined (CMA_THREAD) ||  defined (K_THREADS)
	threads(th_get_info, nil);    /* get info about any existing threads */
#endif /* CMA_THREAD || K_THREADS */
	just_attached = true;
	printstatus();
	just_attached = false;
    } else {
	pc = reg(SYSREGNO(PROGCTR));
/*	setcurfunc(program); */
    }
    bpinit();
    tbinit();
    ehinit();

    flag_initfile = 0;
    f = fopen(initfile, "r");
    if (f != nil) {
        fclose(f);
        setinput(initfile);
        flag_initfile = 1;
    }

    home = getenv("HOME");
    if (home != nil) { 
       struct stat stat1, stat2;
       int ret_stat;

       getcwd(buf,PATH_MAX+2);

       /* Due to aliasing and symbolic link, two different directory names may
        actually be the same directory.  Therefore, use inode and dev id 
        to determine if home directory is the same as the current directory. */
        ret_stat = stat(home,&stat1);
        ret_stat = stat(buf,&stat2);

        /* process $HOME/<initfile> only if current directory is not $HOME. */
        if ((stat1.st_dev != stat2.st_dev) || (stat1.st_ino != stat2.st_ino)) {
           sprintf(buf, "%s/%s", home, initfile);
           f = fopen(buf, "r");
           if (f != nil) {
              fclose(f);
              setinput(strdup(buf));
              flag_initfile = 1;
           }
        }
    }

    if((flag_initfile == 0) && (strcmp(initfile,".dbxinit") != 0)) {
        (*rpt_error)(stderr, catgets(scmc_catd, MS_source, MSG_360,
        "warning: could not read command file \"%s\", action ignored.\n"),
        initfile);
    }

    initdone = true;
#ifdef KDBX
    kdbx_post_init();
#endif
}

/*
 * Re-initialize the world, first de-allocating all storage.
 * This is necessary when the symbol information must be re-read
 * from the object file when it has changed.
 *
 * Before "forgetting" things, we save the current tracing/breakpoint
 * information to a temp file.  Then after re-creating the world,
 * we read the temp file as commands.  This isn't always the right thing;
 * if a procedure that was being traced is deleted, an error message
 * will be generated.
 *
 * If the argument vector is not nil, then this is re-initialize is being
 * done in preparation for running the program.  Since we want to process
 * the commands in the temp file before running the program, we add the
 * run command at the end of the temp file.  In this case, reinit longjmps
 * back to parsing rather than returning.
 */

public reinit(argv, infile, outfile)
String *argv;
String infile;
String outfile;
{
    register Integer i;
    String tmpfile;
    extern String mktemp();

    static char template[] = "/tmp/dbXXXXXX";
    char tmpname[sizeof template];
    strcpy(tmpname, template);
    tmpfile = mktemp(tmpname);

    setout(tmpfile);
    status();
    alias(nil, nil, nil);
    if (argv != nil) {
	(*rpt_output)(stdout, "run");
	for (i = 1; argv[i] != nil; i++) {
	   (*rpt_output)(stdout, " %s", argv[i]);
	}
	if (infile != nil) {
	   (*rpt_output)(stdout, " < %s", infile);
	}
	if (outfile != nil) {
	   (*rpt_output)(stdout, " > %s", outfile);
	}
	(*rpt_output)(stdout, "\n");
    }
    unsetout();
    bpfree();
    objfree();
    symbols_init();
    process_init();
    enterkeywords();
    scanner_init();
    readobj(objname);
    bpinit();
    tbinit();
    ehinit();
    setinput(tmpfile);
    unlink(tmpfile);
    if (argv != nil) {
	longjmp(envptr, 1);
	/* NOTREACHED */
    }
}

/* After a program has performed an exec in multiprocess mode, then we need
 * to reinitialize a lot of the world, but not necessarily all of it. 
 */

public exec_init()
{
    bpfree();
    objfree();
    symbols_init();
    process_init();
    enterkeywords();
    scanner_init();
    /*
    types_reinit();
    */
    objsize = 0;
    running_thread = NULL;
    startprog(nil, nil, nil);
    if (!call_command)
      (*rpt_output)(stdout,  catgets(scmc_catd, MS_main, MSG_99,
					  "reading symbolic information ..."));
    else 
       call_command = false;
    (*rpt_output)(stdout, "\n");
    if (norun) {
        readobj(nil);
    } else {
        readobj(objname);
    }
    (*rpt_output)(stdout, "\n");
    action_mask |= EXECUTION | EXECCALL;
    action_mask &= ~CONTEXT_CHANGE;
    action_mask &= ~LISTING;
    action_mask &= ~ELISTING;
    bpinit();
    tbinit();
    ehinit();
    setcurfunc(program);
    pc = reg(SYSREGNO(PROGCTR));
    yyparse();
    (*rpt_output)(stdout, "\n");
    quit(0);
}

/*
 * After a non-fatal error we skip the rest of the current input line, and
 * jump back to command parsing.
 */

public erecover()
{
    reset_subarray_vars();
    /* Reset call_command only if we are finished and back from the call */
    /* Normally, call_command is reset in procreturn.                    */
    if (call_command) {
      struct Frame frame;
      Frame frp = &frame;
      pc = reg(SYSREGNO(PROGCTR));
      getcurframe(frp);
      if (frameeq(frp, callframe)) { 
        call_command = false;
        callframe = nil;
      }
    }

    bp_skips = 0;
    if (initdone)
    {
	gobble();
	if (isredirected())
          unsetout();
	longjmp( envptr, 1);
    }
}

/*
 * This routine is called when an interrupt occurs.
 */

/* ARGSUSED */
#ifndef _NO_PROTO
public void catchintr(int signo)
#else
public void catchintr(signo)
int signo;
#endif
{
    signal(SIGINT,  catchintr);
    if (isredirected()) {
	(*rpt_output)(stdout, "\n");
	unsetout();
    }
    (*rpt_output)(stdout, "\n" );
    longjmp(envptr, 1);
}

boolean badfile ;
public openfiles (foundfile)
boolean foundfile;
{
    File f = nil;
    char *tmp;
    struct stat obj_data;
    boolean retry = true;
    struct termios newtty;

    if (norun) { 
	f = openfd(loader_info[0].ldinfo_fd);
	if (f != nil) {
	    objname = fd_info[0].pathname;
	    (*rpt_output)(stdout,  catgets(scmc_catd, MS_main, MSG_250,
				   "Successfully attached to %s.\n"), objname);
	    foundfile = true;
	} else {
	    warning( catgets(scmc_catd, MS_main, MSG_251,
				      "Could not determine object file name"));
	    foundfile = false;
	}
    }
    while ((f == nil) && (retry)) {
       if (!isatty(0)) {
	   retry = foundfile;
       }
       else if (not foundfile && !isXDE) {
	   objname = DEFAULTOBJNM;
       	   if(!(tcgetattr(0,&newtty))) {
                if (iscntrl(newtty.c_cc[VEOF])) {
		    newtty.c_cc[VEOF+1] ^= newtty.c_cc[VEOF]^'@';
		    newtty.c_cc[VEOF] = '^';
		    newtty.c_cc[VEOF+2]='\0';
	        }
	        else 
	    	    newtty.c_cc[VEOF+1]='\0';
	            (*rpt_output)(stdout,  catgets(scmc_catd, MS_main, MSG_275,
	            "enter object file name (default is `%s', %s to exit): "),
	            objname, &newtty.c_cc[VEOF]);
	    } 
	    else
	       (*rpt_output)(stdout,  catgets(scmc_catd, MS_main, MSG_252,
	       "enter object file name (default is `%s', ^D to exit): "), 
	       objname);
   	   fflush(stdout);
   	   retry = (boolean)(gets(namebuf) != nil);
   	   if (namebuf[0] != '\0') {
   	       objname = namebuf;
   	   }
       }
       if (!retry) {
          if (attach)
          {
            detach(process->pid, 0);
            exit(0);
          }
          else
	    quit(0);
       } else {
          f = fopen(objname, "r");
       }
       if (f == nil) {
   	   error( catgets(scmc_catd, MS_main, MSG_255, "cannot read %s"),
								      objname);
 	   badfile = true;
	   foundfile = false;
       } else { 
 	   badfile = false;
	   if (!norun)
   	   	fclose(f);
       }
       if ( badfile && isXDE )
		return badfile;
    }
    if (rindex(objname, '/') != nil) {
	tmp = strdup(objname);
	*(rindex(tmp, '/')) = '\0';
	list_append(list_item(tmp), nil, sourcepath);
    }
    if (norun) {
	if (findsource(objname, NULL) == nil) {
	    warning( catgets(scmc_catd, MS_main, MSG_544,
		"Directory containing %s could not be determined.\n\
Apply 'use' command to initialize source path.\n"), objname );
	}
    }
    if (coredump and corefile == nil) {
	if (vaddrs) {
	    corefile = fopen("/dev/mem", "r");
	    corename = "/dev/mem";
	    if (corefile == nil) {
		panic( catgets(scmc_catd, MS_main, MSG_256,
						       "cannot open /dev/mem"));
                coredump = false;
	    }
	} else {
	    corefile = fopen("core", "r");
	    corename = "core";
	    if (corefile == nil) {
		coredump = false;
	    }
	}
    }
}

/*
 * Save/restore the state of a tty.
 */

public savetty(f, t)
File f;
Ttyinfo *t;
{
	ioctl(fileno(f), TCGETA, &(t->ttyinfo));
	t->fcflags = fcntl(fileno(f), F_GETFL, 0);
# ifdef BSD
	ioctl(fileno(f), TIOCGETP, &(t->sg));
	ioctl(fileno(f), TIOCGETC, &(t->tc));
	ioctl(fileno(f), TIOCGLTC, &(t->ltc));
	ioctl(fileno(f), TIOCGETD, &(t->ldisc));
	ioctl(fileno(f), TIOCLGET, &(t->local));
	t->fcflags = fcntl(fileno(f), F_GETFL, 0);
	if ((t->fcflags&FASYNC) != 0) {
	    /* (*rpt_error)(stderr, "[async i/o found set -- reset]\n"); */
	    t->fcflags &= ~FASYNC;
	}
# endif
}

public restoretty(f, t)
File f;
Ttyinfo *t;
{
	/* We don't want to restore tty if we are */
	/* not running in the foreground.         */
#ifndef KDBXRT
	if (tcgetpgrp(f) == getpgrp()) {
#endif
	  ioctl(fileno(f), TCSETA, &(t->ttyinfo));
#ifndef KDBXRT
	}
#endif
	(void) fcntl(fileno(f), F_SETFL, t->fcflags);
# ifdef BSD
	ioctl(fileno(f), TIOCSETN, &(t->sg));
	ioctl(fileno(f), TIOCSETC, &(t->tc));
	ioctl(fileno(f), TIOCSLTC, &(t->ltc));
	ioctl(fileno(f), TIOCSETD, &(t->ldisc));
	ioctl(fileno(f), TIOCLSET, &(t->local));
	if ((t->fcflags&FASYNC) != 0) {
	    /* (*rpt_error)(stderr, "[async i/o not set]\n"); */
	    t->fcflags &= ~FASYNC;
	}
	(void) fcntl(fileno(f), F_SETFL, t->fcflags);
# endif
}

/*
 * Exit gracefully.
 */

public quit(r)
Integer r;
{
    kill_parents();
    action_mask |= DBX_TERMINATED;

#ifdef PROFILE
    longjmp(profile_env,1);
#endif
    if ( r == 0 )
	longjmp( envptr, ENVQUIT );
    else {
	longjmp( envptr, ENVFATAL );
    }
}


public int dpi_set_jump_env( env )
jmp_buf env;
{
   envptr = env;
}


public boolean dpi_is_runfirst()
{
   return runfirst;
}


public String dpi_get_object_name()
{
   return objname;
}
 

public Boolean dpi_use_coredump()
{
   return (coredump && (corefile != nil ));
}

/*
 * Register a procedure to be called approx. every delay seconds while
 * we are waiting for the child process to stop or hit a breakpoint.
 */
public void dpi_set_update_proc( proc, delay )
void	(*proc)();	/* procedure to call		*/
int	delay;		/* seconds between calls	*/
{
   dpi_update_proc = proc;
   dpi_update_delay = delay;
}


