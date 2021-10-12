static char sccsid[] = "@(#)46    1.42.3.38  src/bos/usr/ccs/lib/libdbx/execute.c, libdbx, bos411, 9434B411a 8/24/94 19:25:53";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: arginit, beginproc, cont, contto, curpid, dbxexec, dread, dwrite,
 *	      errarg, errcode, errnum, exitcode, ffork, fixintr, inarg, intr,
 *	      iread, isbperr, isfinished, iwrite, meta_expand, newarg, next,
 *	      notstarted, objfile_close, objfile_readbytes, objfile_readdata,
 *	      objfile_readin, objfile_readtext, outarg, printloc, printsigs,
 *	      printsigscaught, printsigsignored, printstatus, read_err, remade,
 *	      resume, rtnfunc, run, siglookup, startprog, stepc, stepover,
 *	      stepto, write_err, xto, process_load, update_load_maps,
 *	      bp_lastaddr, shell_popen
 *
 * ORIGINS: 26, 27, 83, 3
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
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Process management.
 *
 * This module contains the routines to manage the execution and
 * tracing of the debuggee process.
 */

#include "defs.h"
#include "envdefs.h"
#include "execute.h"
#include "process.h"
#include "eval.h"
#include "object.h"
#include "mappings.h"
#include "main.h"
#include "cma_thread.h"
#ifdef K_THREADS
#include "k_thread.h"
#endif /* K_THREADS */
#include "runtime.h"
#include "frame.h"
#include <signal.h>
#include <errno.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/reg.h>
#include <scnhdr.h>
/* ldfcn.h and fcntl.h share FREAD and FWRITE */
#undef FREAD
#undef FWRITE
#include <ldfcn.h>
#include <setjmp.h>

int newdbg = 0;            /* pid of new debugger (to be ignored in wait) */
int fptype;
int contsig = DEFSIG;		/* Signal variable for cont command */

Boolean just_started;
Boolean noexec = false;
Boolean textaddrs = false;	/* Address text of non-executable */
Boolean dataaddrs = false;	/* Address data of non-executable */
Boolean fileaddrs = false;	/* Address file of non-executable */
unsigned textorg = 0;		/* Origin for referring to text section */
unsigned dataorg = 0;		/* Origin for referring to the data section */
Boolean never_ran = true;
Boolean dynamic_loading = false;/* Set if process dynamically loads */
#ifdef K_THREADS
Boolean next_subcommand = false;
extern int nb_k_threads_sig;
#endif /* K_THREADS */
String infile, outfile, errfile;
extern Address dbargs_addr;
extern boolean fullcore;
extern pid_t debuggee_pgrp;
extern pid_t debugger_pgrp;
extern File srcfp;           	/* currently open File from skimsource() */

extern Boolean branch_link();
private Boolean remade();
/*private contto();*/
private xto();
private read_err();
private write_err();
private printsigs();
#ifndef _NO_PROTO
private void intr(int);
#else
private void intr();
#endif

/*
 *  Variables for $catchbp flag.
 */
typedef List Bplist;
extern Bplist bplist;		       /* List of active breakpoints */
extern boolean catchbp;

private int argc;
private String argv[MAXNCMDARGS];
private LDFILE *objfile = NULL;

extern SCNHDR text_scn_hdr;
extern SCNHDR data_scn_hdr;
extern unsigned short text_scn_num;
extern unsigned short data_scn_num;
extern unsigned short bss_scn_num;
extern int newdebuggee;

extern int rerunning;
/*extern FILHDR filhdr;*/

/*
 * External variables needed to deal with catching
 * breakpoints during a call command.
 */
extern Boolean call_command;
extern Boolean initdone;
extern long bp_skips;
extern int *envptr;

typedef struct parent_pid *parent_pid;

struct parent_pid
{
  int pid;
  parent_pid next;
};

parent_pid parent_pid_list = NULL;

/*
 * NAME: bp_lastaddr
 *
 * FUNCTION: Set dbx brakpoint at last address of program (_exit) to
 * 	     prevent process from existing. Such that dbx can continue
 * 	     to view process content after execution is completed.
 *
 * NOTES: Also handle multiple _exit() in different libraries.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void bp_lastaddr()
{
  Address exitaddr;
  Node cond;
  Symbol s;
  Name exit_name = identname("_exit", true);

  s = lookup(exit_name);
  if (s == nil) {
     warning(catgets(scmc_catd, MS_runtime, MSG_307, "cannot find _exit"));
  }
  while (s != nil) {
     if (s->name == exit_name && isroutine(s) 
			      && ((exitaddr = codeloc(s)) != 0)) {
        cond = build(O_EQ, build(O_SYM, pcsym), build(O_LCON, exitaddr));
        event_once(cond, buildcmdlist(build(O_ENDX)));
     }
     s = s->next_sym;
  }
}


/*
 * Begin execution.
 *
 * We set a breakpoint at the end of the code so that the
 * process data doesn't disappear after the program terminates.
 */

public startprog (argv, infile, outfile)
String argv[];
String infile, outfile;
{
    String pargv[4];
    Boolean init_prog;

    init_prog = false;
    if (coredump) {
	coredump = false;
	fclose(corefile);
	coredump_close();
	init_prog = true;
    }
    if (argv == nil) {
	argv = pargv;
	pargv[0] = objname;
	pargv[1] = nil;
    } else {
	argv[argc] = nil;
    }
    pstart(process, argv, infile, outfile);
    if (!norun && (remade(objname))) {
	reinit(argv, infile, outfile);
    }
    if (init_prog)
	program->symvalue.funcv.beginaddr = reg(SYSREGNO(PROGCTR));
    if (process->status == STOPPED) {
	if (!attach) {
	   pc = CODESTART;
	   setcurfunc(program);
	}
	else {
	   pc = reg(SYSREGNO(PROGCTR));
	}
#ifndef KDBX
	if (objsize != 0) {
	    bp_lastaddr();
	}
#endif /* KDBX */
    }
}

/*
 * Check to see if the object file has changed since the symbolic
 * information last was read.
 */

private time_t modtime;
public char *fname = nil;

private Boolean remade (filename)
String filename;
{
    struct stat s;
    Boolean b;
    int i;

    if (fname == nil) {
	modtime = 0;
	fname = (char *) malloc((unsigned) (strlen(filename) + 1));
    } else if (!streq(fname, filename)) {
	     modtime = 0;
	     fname = (char *) realloc(fname, (unsigned) (strlen(filename) + 1));
    	   }
    strcpy(fname, filename);

    stat(filename, &s);
    b = (Boolean) (modtime != 0 and modtime < s.st_mtime);
    modtime = s.st_mtime;
    return b;
}

/*
 * Initialize the argument list.
 */

public arginit ()
{
    int i;

    infile = nil;
    outfile = nil;
    errfile = nil;
    argv[0] = objname;
    if (!rerunning)
    	argc = 1;
    else {
	(*rpt_output)(stdout, "[ ");
	for (i = 0; i < argc; i++)
	    (*rpt_output)(stdout, "%s ",argv[i]);
	(*rpt_output)(stdout, "]\n");
    }
}


/*
 * NAME: clear_filenames
 *                                                                    
 * FUNCTION: Clear the names of the files for redirection of stdin, stdout, and
 *	stderr of debuggee
 *                                                                    
 * POST CONDITIONS: stdin, stdout, and stderr will not be redirected for
 *	debuggee unless specified on dbx command line
 *
 * PARAMETERS: NONE
 *
 * DATA STRUCTURES:
 *	infile	- Set to NULL
 *	outfile	- Set to NULL
 *	errfile	- Set to NULL
 *
 * RETURNS: NONE
 */
void clear_filenames( void )
{
    infile = NULL;
    outfile = NULL;
    errfile = NULL;
}


/*
 * NAME: shell_popen
 *
 * FUNCTION: Initiates a pipe to/from a process and
 *	     use defined shell to execute command.
 *
 * PARAMETERS:
 *      cmd      - command to be executed in shell
 * 	mode	 - I/O mode of file stream
 *
 * NOTE: shell_popen is same as popen except popen always
 *	 uses 'bsh' for the command but shell_popen will 
 *	 use shell defined by the "SHELL" envir. variable.
 *	 shell_popen and shell_pclose (same as fclose)
 *	 should be used as pair. pclose will return
 *	 error if used with shell_popen.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: File pointer to opened stream, or 
 *          NULL on error.
 */
#define tst(a,b) (*mode == 'r'? (b) : (a))
#define shell_pclose(a) fclose(a)

private FILE *shell_popen(cmd, mode)
char *cmd, *mode;
{
   int	p[2];
   register int myside, yourside;
   int pid;
   String shell;
   int status;

   if (pipe(p) < 0)
      return(NULL);
   myside = tst(p[1], p[0]);
   yourside = tst(p[0], p[1]);
   if ((pid = fork()) == 0) {
      int stdio;
      stdio = tst(0, 1);
      (void) close(myside);
      if (stdio != yourside) {
      	 (void) close(stdio);
      	 (void) fcntl(yourside, F_DUPFD, stdio);
      	 (void) close(yourside);
      }
      if ((shell = getenv("SHELL")) == nil) {
	 /* use "sh" if SHELL is not defined */
         (void) execl("/usr/bin/sh", "sh", "-c", cmd, (char *)0);
      } else {
	 /* else use defined "SHELL" */
         (void) execlp(shell, shell, "-c", cmd, (char *)0);
      }
      exit(1);		/* exec error if we reach here */
   }

   if (pid == -1)
      return(NULL);
   (void) close(yourside);

   /* wait for the exit status of shell command */
   wait(&status);
   return (fdopen(myside, mode));
}


/*
 *  Given a string, use popen and echo to resolve 
 *  any meta characters that are present.
 */

public meta_expand(unresolved_buf, resolved_buf)
String unresolved_buf;
char resolved_buf[];
{
   FILE *stream;
   Boolean special_echo = true;  /* echo -n does not emit newline */

   strcpy(resolved_buf, "echo -n");
   if ((stream = shell_popen(resolved_buf,"r")) != NULL) {
       special_echo =
            (Boolean)(fgets(resolved_buf,MAXCMDLNLEN,stream) == (char *) 0);
       shell_pclose(stream);
   }
   if (special_echo) {
       strcpy(resolved_buf,"echo -n ");
       strcat(resolved_buf,unresolved_buf);
       strcat(resolved_buf,";echo");
   } else {
       strcpy(resolved_buf,"echo ");
       strcat(resolved_buf,unresolved_buf);
   }
   if ((stream = shell_popen(resolved_buf,"r")) != NULL)
   {
      *resolved_buf = '\0';	/* reset buffer */	
      fgets(resolved_buf,MAXCMDLNLEN,stream);
      shell_pclose(stream);
   }
   else
   {
     beginerrmsg();
     (*rpt_error)(stderr, catgets(scmc_catd, MS_execute, MSG_103,
		  "Attempt to resolve \"wildcards\" in run or rerun failed."));
     enderrmsg();
   }
}

/*
 * Add an argument to the list for the debuggee.
 */

public newarg (arg, quoted)
String arg;
Boolean quoted;
{
    String  newarg, ptr;
    char resolved_buf[MAXCMDLNLEN];
    Boolean done = false;

    if (argc >= MAXNCMDARGS) {
	error(catgets(scmc_catd, MS_execute, MSG_105, "too many arguments"));
    }
    if (rerunning == 1) {
	argc = 1;
	rerunning++;
    }
    if (quoted)               /* if string was in quotes */
       argv[argc++] = arg;
    else                      /* else expand it          */
    {
       meta_expand(arg,resolved_buf);
       ptr = malloc(strlen(resolved_buf)+1);
       strcpy(ptr,resolved_buf);
       while (!done)
       {
          newarg = ptr;
          while ((*ptr != '\n') && (*ptr != ' ') && (*ptr != '\0'))
             ptr++;
          if (*ptr == '\n' || (*ptr == '\0')) done = true;
	  *ptr++ = '\0';
	  argv[argc++] = newarg;
       }
    }
}

/*
 * Set the standard input for the debuggee.
 */

public inarg (filename)
String filename;
{
    if (infile != nil) {
	error( catgets(scmc_catd, MS_execute, MSG_109,
						  "multiple input redirects"));
    }
    infile = filename;
}

/*
 * Set the standard output for the debuggee.
 * Probably should check to avoid overwriting an existing file.
 */

public outarg (filename)
String filename;
{
    outfile = filename;
}

/*
 * Set the standard error for the debuggee.
 * Probably should check to avoid overwriting an existing file.
 */

public errarg (filename)
String filename;
{
    errfile = filename;
}

/*
 * Start debuggee executing.
 */

public run ()
{
    Node resetmult;

    prtaddr = 0;
    process->status = STOPPED;
    call_command = false;
    fixbps();
    if (!attach)
	curline = 0;
    startprog(argv, infile, outfile);
    just_started = true;
    isstopped = norun;
    never_ran = false;
    if (dynamic_loading) {
	update_load_maps();
	dynamic_loading = false;
    }
    if (multproc != off) {
       resetmult = build(O_MULTPROC, build(O_LCON,(int) multproc));
       eval(resetmult);
    }
    cont(0);
}

/*
 * Continue execution wherever we left off.
 *
 * Note that this routine never returns.  Eventually bpact() will fail
 * and we'll call printstatus or step will call it.
 */

private Voidfunc dbintr;

public cont (signo)
int signo;
{
    int s;
#ifndef _NO_PROTO
    void intr(int);
#else
    void intr();
#endif

    dbintr = signal(SIGINT, intr);
#ifndef KDBX
    if (never_ran) {
        /* Set breakpoint at end of code if we never run() program so that */
        /* the process data doesn't disappear after the program terminates */
        never_ran = false;
        if (objsize != 0) {
	    bp_lastaddr();
        }
    }
#endif /* KDBX */

#ifdef KDBX
    just_started = false;
#else /* KDBX */
    if (just_started) {
	just_started = false;
    } else {
	if (not isstopped) {
	    error(catgets(scmc_catd, MS_execute, MSG_110,
		  "cannot continue execution"));
	}
	isstopped = false;
	/* If we are blocking signals from the debuggee program */
	/* and user wants to continue execution with a signal,  */
	/* the signal is passed down here thru global variable  */
	/* contsig. (routine resume is not reached in this case)*/
        /* We need this to handle cont with a signo when tracei */
        /* (single_stepping) is on as well.                     */
	if ((varIsSet("$sigblock") or single_stepping) and (signo > 0)) {
	  contsig = signo;
	  signo = DEFSIG;
	}
	stepover();
    }
#endif /* KDBX */
    s = signo;
    for (;;) {
	if (single_stepping) {
	    printnews(false);
	} else {
	    setallbps();
	    resume(s, 0);
	    unsetallbps();
	    s = DEFSIG;
	    if (not isbperr() or not bpact()) {
		printstatus();
	    }
            if (single_stepping)
              /*  print the first line if tracing in a function  */
              printnews(true);
	}
	if (!isstopped)
	    stepover();
	else 
		/* Return if cont part of ; statement */
	    return;
    }
    /* NOTREACHED */
}

/*
 * This routine is called if we get an interrupt while "running"
 * but actually in the debugger.  Could happen, for example, while
 * processing breakpoints.
 *
 * We basically just want to keep going; the assumption is
 * that when the process resumes it will get the interrupt,
 * which will then be handled.
 */

/* ARGSUSED */
#ifndef _NO_PROTO
private void intr (int signo)
#else
private void intr (signo)
int signo;
#endif
{
    signal(SIGINT, intr);
}

public void fixintr ()
{
    signal(SIGINT, dbintr);
}

/*
 * Resume execution.
 */

public resume (signo, last_bp)
int signo;
Address last_bp;
{
    register Process p;
#ifdef K_THREADS
    /* for thread_k_cont in case of all threads held */
    extern Address save_last_bp;
    save_last_bp = last_bp;
#endif /* K_THREADS */

    p = process;
    pcont(p, signo);
    pc = reg(SYSREGNO(PROGCTR));
    if (p->status != STOPPED)
    {
	/*
	 * Must unset all the breakpoints here since we won't be going 
	 * back to continue from here. First we set status to STOPPED 
	 * so that we can unset all the breakpoints, once we return we
	 * set it back so that we represent our true condition.
	 */
        p->status = STOPPED;
	if (last_bp != 0)
	    unsetbp(last_bp);
	else
	    unsetallbps();
        p->status = FINISHED;

	action_mask |= EXECUTION_COMPLETED;
	if (p->signo != 0)
	    error( catgets(scmc_catd, MS_execute, MSG_113,
				 "program terminated by signal %d"), p->signo);
	else if (not runfirst) 
	{
	    if (p->exitval == 0)
		error( catgets(scmc_catd, MS_execute, MSG_114,
							    "program exited"));
	    else
		error( catgets(scmc_catd, MS_execute, MSG_115,
				   "program exited with code %d"), p->exitval);
	}
    }
}

/*
 * Continue execution up to the next source line.
 *
 * There are two ways to define the next source line depending on what
 * is desired when a procedure or function call is encountered.  Step
 * stops at the beginning of the procedure or call; next skips over it.
 *
 * Stepc is what is called when the step command is given.
 * It has to play with the "isstopped" information.
 */

public stepc ()
{

#ifdef K_THREADS
    next_subcommand = true;
#endif /* K_THREADS */
    if (just_started) {
	pc = reg(SYSREGNO(PROGCTR));
	just_started = false;
    }
    else if (not isstopped) {
	error(catgets(scmc_catd, MS_execute, MSG_110,
              "cannot continue execution"));
    }
    isstopped = false;
    dostep(false);
    isstopped = true;
#ifdef K_THREADS
    next_subcommand = false;
#endif /* K_THREADS */
}

public next ()
{
    Address oldfrp, newfrp, orgin;
    Word instr;
    register Breakpoint bp;
    extern Address before_alloca_frp();

#ifdef K_THREADS
    next_subcommand = true;
#endif /* K_THREADS */
    orgin = reg(SYSREGNO(PROGCTR));
    if (not isstopped && not just_started) {
	error(catgets(scmc_catd, MS_execute, MSG_110,
	      "cannot continue execution"));
    }
    just_started = false;
    isstopped = false;
    oldfrp = reg(FRP);
    iread(&instr,reg(SYSREGNO(PROGCTR)),4);

    /*  if $catchbp is set or doing nexti or inside a prolog  */
    if (catchbp || inst_tracing || isprolog(pc))   
    {
      dostep(true);
      pc = reg(SYSREGNO(PROGCTR));
    }
    else
    {
      do {
        dostep(true);
        pc = reg(SYSREGNO(PROGCTR));
        /* test and reset if the frame pointer has be changed by alloca() */
        newfrp = (newfrp = before_alloca_frp()) ? newfrp : reg(FRP);
      } while (newfrp < oldfrp && newfrp != 0); 
    }
    isstopped = true;
#ifdef K_THREADS
    next_subcommand = false;
#endif /* K_THREADS */
}

/*
 * Continue execution until the current function returns, or,
 * if the given argument is non-nil, until execution returns to
 * somewhere within the given function.
 */

public rtnfunc (f)
Symbol f;
{
    Address addr;
    Symbol t;

    if (not isstopped) {
	error(catgets(scmc_catd, MS_execute, MSG_110,
	      "cannot continue execution"));
    } else if (f != nil and not isactive(f)) {
	error(catgets(scmc_catd, MS_execute, MSG_153, "%s is not active"),
	      symname(f));
    } else {
	addr = return_addr();
	if (addr == nil) {
	    error(catgets(scmc_catd, MS_execute, MSG_155,
		  "no place to return to"));
	} else {
	    isstopped = false;
	    contto(addr);
	    if (f != nil) {
		for (;;) {
		    t = whatblock(pc);
		    if (t == f)
			break;
		    addr = return_addr();
		    if (addr == nil)
			break;
		    contto(addr);
		    bpact();	/* do breakpoint commands, but don't stop */
		}
	    }
	    if ((not bpact()) || (pc == addr)) {
		isstopped = true;
		printstatus();
	    }
	}
    }
}

/*
 * Single-step over the current machine instruction.
 *
 * If we're single-stepping by source line we want to step to the
 * next source line.  Otherwise we're going to continue so there's
 * no reason to do all the work necessary to single-step to the next
 * source line.
 */

public stepover ()
{
    Boolean b;

    if (traceexec) {
	(*rpt_output)(stdout, "!! stepping over 0x%x\n",
					      reg(SYSREGNO(PROGCTR)));
    }
    if (single_stepping) {
	dostep(false);
    } else {
	b = inst_tracing;
	inst_tracing = true;
	dostep(false);
	inst_tracing = b;
    }
    if (traceexec) {
	(*rpt_output)(stdout, "!! stepped over to 0x%x\n",
					      reg(SYSREGNO(PROGCTR)));
    }
}

/*
 * Resume execution up to the given address.  We can either ignore
 * breakpoints (stepto) or catch them (contto).
 */

public stepto (addr)
Address addr;
{
#ifdef K_THREADS
    Symbol save_running_thread;
    save_running_thread = running_thread;
#endif /* K_THREADS */

    xto(addr, false);

#ifdef K_THREADS
/* if next step or nexti subcommands we have to stay in same thread */
/* local break-point stop only if it's the good thread */
    if (lib_type == KERNEL_THREAD && save_running_thread )
      while (save_running_thread != running_thread)
        {
        if (!next_subcommand || catchbp) break;
        if (traceexec) {
            (*rpt_output)(stdout, "!! local break-point on next addr = %X \n",
                addr);
        }
        /*we have to stepi the current instruction */
	if (nb_k_threads_sig == 1)
            stepover(); 
        xto(addr,false);
     }

#endif /* K_THREADS */

}

public contto (addr)
Address addr;
{
    xto(addr, true);
}

private xto (addr, catchbps)
Address addr;
boolean catchbps;
{
    Address curpc;
    register Breakpoint bp;
    extern Boolean isdbxbp();

    if (catchbps) {
	stepover();
    }
    curpc = reg(SYSREGNO(PROGCTR));
#ifndef K_THREADS
    if (addr != curpc) {
#else
    if (addr != curpc || nb_k_threads_sig > 1) {
#endif
	if (traceexec) {
	    (*rpt_output)(stdout, "!! stepping from 0x%x to 0x%x\n",
								  curpc, addr);
	}
	if (catchbps) {
	    setallbps();
	}
	setbp(addr);
        if ((varIsSet("$sigblock") or single_stepping)
            and (contsig != DEFSIG)) {
	  int signo = contsig;
          contsig = DEFSIG;
	  resume(signo, addr);
        } else
	  resume(DEFSIG, addr);
	unsetbp(addr);
	if (catchbps) {
	    unsetallbps();
	}
	if ((!isdbxbp(addr) && single_stepping) || (!isbperr())) {
	    printstatus();
	}
    }
}

/*
 * Enter a procedure by creating and executing a call instruction.
 */

#define CALLSIZE 4	/* size of call instruction */

public beginproc (p, argc, ptrtofuncaddr)
Symbol p;
int argc;
Address ptrtofuncaddr;
{
    extern Boolean just_started;
#ifdef K_THREADS
    extern Address addr_dbsubn;
#endif
    Address addr, orig_pc;
    extern Process process;
    Symbol callpt, assemvar;
    Name n;
    int local_module;
    Desclist *dptr;
    
    orig_pc = reg(SYSREGNO(IAR));
    if (ptrtofuncaddr) {
       addr = ptrtofuncaddr;
    }
    else {
       addr = prolloc(p);
    }
    if (textobj(addr) != (local_module = textobj(orig_pc))) {
	for (dptr = p->symvalue.funcv.fcn_desc;
	     (dptr != nil) && (textobj(dptr->descaddr) != local_module); 
	     dptr = dptr->next_desc);
#ifdef K_THREADS
	/* if addr_dbsubn we are trying to hold the running thread */
	/* the function __funcblock_np of libpthreads is not       */
	/* accessible with "linkage symbol" if the library is      */
	/* shared. In this case we can call directly __funcblock_np*/
	/* the registers are saved and restored after. We can      */
	/* use the same toc.                                       */
        if (dptr == nil && !addr_dbsubn)
#else
	if (dptr == nil) 
#endif
	   error( catgets(scmc_catd, MS_execute, MSG_539,
		    "Unable to locate linkage symbol for out-of-module call"));
#ifdef K_THREADS
        if (!addr_dbsubn || dptr != nil)
#endif
	addr = dptr->descaddr;
        n = identname("__dbsubg",true);
    } else {
	/* Linkage routines (GLUE codes) can be in the local module. */
	/* Need __dbsubg to restore the TOC for them also.     */
        if (p->symvalue.funcv.islinkage && !ptrtofuncaddr)
          n = identname("__dbsubg",true);
        else
          n = identname("__dbsubc",true);
    }
    find(callpt, n) where isroutine(callpt) endfind(callpt);
    if (callpt == nil) {
	error( catgets(scmc_catd, MS_execute, MSG_162,
		     "Failed establishing calling point (libg.a not linked)"));
    }
    pc = prolloc(callpt);
    setreg(SYSREGNO(IAR), pc);
    if (just_started) setreg(FRP, reg(STKP));
#if defined(_IBMRT) && !defined(KDBXRT)
/*
    dwrite((char *)&addr,dbargs_addr-4,4); 
    setreg(GPR0, dbargs_addr-4);
*/
    setreg(GPR0, addr);
#else
    setreg(SYSREGNO(LR), addr);
#endif
    pstep(process, DEFSIG);
    pc = reg(SYSREGNO(IAR));
    if (not isbperr()) {
	printstatus();
    }
}

/*
 * Print the status of the process.
 * This routine does not return.
 */

public printstatus ()
{
    if ((process->status == FINISHED) && (!isXDE)) {
        exit(0);
    } else if ((process->status == FINISHED) &&
               (isXDE) && (runfirst) &&
               (action_mask & EXECUTION) &&
               (action_mask & EXECUTION_COMPLETED)) {
        exit(0);
    } else if ((process->status == FINISHED) &&
               (isXDE) && (runfirst) &&
               !(action_mask & EXECUTION)) {
        (*rpt_open)();
        action_mask |= DBX_TERMINATED;
        longjmp( envptr, 1 );
    } else {
	if (runfirst) {
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_execute, MSG_163,
						 "\nEntering debugger ...\n"));
	    if (isXDE) (*rpt_open)();
	    printheading();
	    init();
	}
	setcurfunc(whatblock(pc));
	getsrcpos();
	if (process->signo == SIGINT) {
	    isstopped = true;
	    printerror();
	} 
        else if (isbperr() and isstopped) 
	{
            (*rpt_output)(stdout, "stopped ");
            printloc();
            (*rpt_output)(stdout, "\n" );

	    if (curline > 0) 
	    {
		   printlines(curline, curline);
	    } 
	    else
            {
	          printinst(pc, pc);
	    }
	    if (call_command)
	    {
		reset_subarray_vars();
		bp_skips = 0;
		if (initdone)
		{
		   gobble();
		   longjmp( envptr, 1);
		}
	    }
	    else {
		extern char * process_char;
	
		if (*process_char != ';')
	        erecover();
	    }
	} else {
	    fixintr();
	    isstopped = true;
	    printerror();
	}
    }
}

/*
 * Print out the current location in the debuggee.
 */

public printloc ()
{
    extern Boolean heat_shrunk;
    struct Frame frp;
    int frame_found = false;

    (*rpt_output)(stdout, "in ");
    if (isinline(curfunc))
	(*rpt_output)(stdout, "unnamed block ");

    /*  if this is a heat_shrunk executable and the function
          name is in the traceback table  */
    if (heat_shrunk && (frame_found = (getcurframe(&frp) == 0))
     && (frp.tb.name_present && frp.name))
    {
      /*  print the function name found in the traceback table  */
      (*rpt_output)(stdout, "%s", frp.name);
    }
    else
      printname(rpt_output, stdout, curfunc, true);

    (*rpt_output)(stdout, " ");
    if (curline > 0 and not useInstLoc) {
	printsrcpos();
    } else {
	useInstLoc = false;
	curline = 0;
	(*rpt_output)(stdout, "at 0x%x", pc);

        /*  if the current location is a fdpr relocated address      */
        if (frame_found && (frp.save_pc != frp.orig_loc))
        {
          /*  print the original location of this location  */
          /*  NOTE : frp.orig_loc was set by getcurframe    */
          (*rpt_output)(stdout, " = fdpr[0x%x]", frp.orig_loc);
        }
    }
#if defined (CMA_THREAD) || defined (K_THREADS)
    /* indicate what thread is running if needed... */
    if (current_thread)
        (*rpt_output)(stdout, " (%s)", symname(current_thread));
#endif /* CMA_THREAD || K_THREADS */
}

/*
 * Predicate to test if the reason the process stopped was because
 * of a breakpoint.  If so, as a side effect clear the local copy of
 * signal handler associated with process.  We must do this so as to
 * not confuse future stepping or continuing by possibly concluding
 * the process should continue with a SIGTRAP handler.
 */

public boolean isbperr ()
{
    Process p;
    boolean b;

    p = process;

    /*  if we stopped because of a breakpoint  */
    if (p->status == STOPPED && p->is_bp) {
	b = true;
	p->sigstatus = 0;
    } else {
	b = false;
    }
    return b;
}

/*
 * Some functions for testing the state of the process.
 */

public Boolean notstarted (p)
Process p;
{
    return (Boolean) (p->status == NOTSTARTED);
}

public Boolean isfinished (p)
Process p;
{
    return (Boolean) (p->status == FINISHED);
}

/*
 * Return the signal number that stopped the process.
 */

public int errnum (p)
Process p;
{
    return p->signo;
}

/*
 * Return the signal code associated with the signal.
 */

public int errcode (p)
Process p;
{
    return p->sigcode;
}

/*
 * Return the termination code of the process.
 */

public int exitcode (p)
Process p;
{
    return p->exitval;
}

/*
 * These routines are used to access the debuggee process from
 * outside this module.
 *
 * They invoke "pio" which eventually leads to a call to "ptrace".
 * The system generates an I/O error when a ptrace fails.  During reads
 * these are ignored, during writes they are reported as an error, and
 * for anything else they cause a fatal error.
 */

extern Intfunc onsyserr();

private badaddr;
public boolean badIO;

/*
 * Read from the process' instruction area.
 */

public iread (buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    Intfunc f;

    badIO = false;
    f = onsyserr(EIO, read_err);
    badaddr = addr;
    if (coredump) {
	coredump_readtext(buff, addr, nbytes);
    } else if (noexec) {
	objfile_readtext(buff, addr, nbytes);
    } else {
	pio(process, PREAD, TEXTSEGM, buff, addr, nbytes);
    }
    onsyserr(EIO, f);
    return (badIO == true);
}

/* 
 * Write to the process' instruction area, usually in order to set
 * or unset a breakpoint.
 */

public iwrite (buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    Intfunc f;

    badIO = false;
    if (coredump) {
	error( catgets(scmc_catd, MS_execute, MSG_175,
						    "no process to write to"));
    }
    f = onsyserr(EIO, write_err);
    badaddr = addr;
    pio(process, PWRITE, TEXTSEGM, buff, addr, nbytes);
    onsyserr(EIO, f);
    return (badIO == true);
}

/*
 * Read for the process' data area.
 */

public dread (buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    Intfunc f;

    badaddr = addr;
    badIO = false;
    if (noexec) {
	f = onsyserr(EIO, read_err);
	objfile_readdata(buff, addr, nbytes);
	onsyserr(EIO, f);
    } else if (coredump && (fullcore || initdone)) {
	f = onsyserr(EFAULT, read_err);
	coredump_readdata(buff, addr, nbytes);
	onsyserr(EFAULT, f);
    } else {
	f = onsyserr(EIO, read_err);
	pio(process, PREAD, DATASEGM, buff, addr, nbytes);
	onsyserr(EIO, f);
    }
    return (badIO == true);
}

/*
 * Write to the process' data area.
 */

public dwrite (buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    Intfunc f;

    badIO = false;
    if (coredump) {
	error( catgets(scmc_catd, MS_execute, MSG_175,
						    "no process to write to"));
    }
    f = onsyserr(EIO, write_err);
    badaddr = addr;
    pio(process, PWRITE, DATASEGM, buff, addr, nbytes);
    onsyserr(EIO, f);
    return (badIO == true);
}

 /*
 * Trap for errors in reading or writing to a process.
 * The current approach is to "ignore" read errors and complain
 * bitterly about write errors.
 */

private read_err ()
{
     badIO = true;
    /*
     * Ignore.
     */
}

private write_err ()
{
    badIO = true;
    error(catgets(scmc_catd, MS_execute, MSG_215,
	  "cannot write to process (address 0x%x)"), badaddr);
}

/***************************************************
 *  Multi-process debugging for fork               *
 *                                                 *
 *  forks dbx and sets up new dbx if req           *
 *                                                 *
 ***************************************************/

ffork (pid)
   int pid;
{
   Node multcmd;		/* Command to enable multi-process in child */

   int nextdbx = 0, i,j,k,l,m,n;
   char buff[32];
   struct termio tty_old;     /* save the original tty  */
   int rc;

   register String dir;		/* source path string for xde call */
   char *argv[MAXNCMDARGS];	/* argv list for xde exec call */
   char *arg0;
   int z = 0;

  (*rpt_output)(stdout, catgets(scmc_catd, MS_execute, MSG_216,
       "application forked, child pid=%d, process stopped, awaiting input \n"),
									  pid);

  /**************** if dbx is running under XDE *******************/
  if (isXDE)
  {
    if ((nextdbx = fork()) == 0)   {     /* if it is the child xde */
       /* allow multiple arguments separated by single spaces */
       arg0 = graphical_debugger;
       while ( arg0 != NULL )
       {
	  argv[z++] = arg0;
	  arg0 = strchr(arg0, ' ');
	  if ( arg0 != NULL )
	    *arg0++ = '\0';
       }

       /** find the source files path and pass it along to the new xde */
       foreach (String, dir, sourcepath)
         sprintf(argv[z++]=(char *)(malloc(strlen(dir)+3)), "-I%s", dir);
       endfor

       /* Use '-A' option to reattach to the stopped debuggee */
       sprintf(argv[z++]=(char *)(malloc(strlen(dir)+3)), "-A%d", pid);

       /* Use '-child' option to alarm new xde that it is a forked child */
       sprintf(argv[z++]=(char *)(malloc(7)), "-child");
       argv[z]=nil;
       execvp(graphical_debugger,argv);    /* exec a new xde with needed info */
       _exit(1);
    }
    else {
      (*rpt_multiprocess)(pid);
      newdbg = nextdbx;
    }
  }

       /************* next xdb program when dbx forks *******/
  else if (multproc == on) {
       /****** next dbx program on fork *********/

    int fd;

    nextdbx = fork();
    if (nextdbx == 0)   {     /* if new next dbx */
      fd = screen(0);             /*  set up new virtual term   */
      if(fd == -1)  {
         char *sp, *tmp_file;
         FILE *fp;
         int dirlen = 0, i;
         char *s;
         tmp_file = mktemp("/tmp/dbxtmpfile.XXXXXX");
         fp = fopen(tmp_file, "w");
         /* for unknown reason, it needs next before detach */
         fprintf(fp, "next\ndetach\n");
         fclose(fp);

         /** find the source files path and pass it along to the new xde */
         foreach (String, dir, sourcepath)
           sprintf(argv[z++]=(char *)(malloc(strlen(dir)+3)), "-I%s ", dir);
         endfor
         for(i=0; i < z; i++) {
            dirlen += strlen(argv[i]);
         }
         s = (char *)malloc(dirlen+1);
         *s = '\0';
         for(i=0; i < z; i++) {
            strcat(s,argv[i]);
         }

         /* allocate space big enough to hold the following sprintf for sp */
         sp=(char *)(malloc(strlen(graphical_debugger)+60+dirlen));
         sprintf(sp, "%s -c %s %s -A %d  > /dev/null\n",
              graphical_debugger, tmp_file, s, newdebuggee);
         free(s);

         system(sp);
         free(sp);
         remove(tmp_file);
         exit(0);
       }

      rc = ptrace(REATT, pid, 0, 0, 0);   /* re-attach to child proc */
      if (rc < 0)           /* error   */
         fatal( catgets(scmc_catd, MS_execute, MSG_217,
			       "Could not reattach to child process on fork"));
      else {
         norun = true;      /* prevent use of run or rerun  */
         (*rpt_output)(stdout,  catgets(scmc_catd, MS_execute, MSG_218,
	   "debugging child, pid=%d, process stopped, waiting input \n"), pid);
	 process->pid = pid;
	 reset_debuggee_fd(fd);
	 reset_pgrp_info();
	 /* Enable multi-process debugging. */
	 multcmd = build(O_MULTPROC, build(O_LCON, (int) multproc));
	 eval(multcmd);
      }
    }
    else newdbg = nextdbx;
  }
  else if (multproc == child)
  {
    parent_pid new_parent_pid;

    delete_all_bps();
    rc = ptrace(REATT, pid, 0, 0, 0);   /* re-attach to child proc */
    if (rc < 0)           /* error   */
       fatal( catgets(scmc_catd, MS_execute, MSG_217,
	       "Could not reattach to child process on fork"));
    
    (*rpt_output)(stdout,  catgets(scmc_catd, MS_execute, MSG_218,
     "debugging child, pid=%d, process stopped, waiting input \n"), pid);
    /*  detach from the parent process  */
    detach(process->pid, 0);

    /*  Keep a list of parent process ids, in case they have
          not finished executing when dbx is exited.  We will
          then kill them  */
    new_parent_pid = new(parent_pid);
    new_parent_pid->pid = process->pid;
    new_parent_pid->next = parent_pid_list;
    parent_pid_list = new_parent_pid;

    eval(build(O_CATCH, build(O_LCON, SIGHUP)));
    process->pid = pid;
  } /*not isXDE*/
  else if (multproc == parent)
  {
    int temppid = process->pid;

    /*  temporarily make the child process the "current"
          process  */
    process->pid = pid;
    delete_all_bps();
    process->pid = temppid;
    
    /*  detach from the child process  */
    detach(pid, 0);
  }
}

void kill_parents()
{
  parent_pid parent_pid_ptr = parent_pid_list;
  while (parent_pid_ptr != NULL)
  {
    /*  send a kill signal to all processes we detached
          from while following the child  */
    kill(parent_pid_ptr->pid, 9);
    parent_pid_ptr = parent_pid_ptr->next;
  }
}

/***************************************************
 *  restart dbx to handle new process              *
 *     used when application program execs         *
 *                                                 *
 ***************************************************/

dbxexec(pid)
int pid;
{
   getprog(pid,true);
   just_started = true;
   cacheflush(process);
   exec_init();
   /* not reached */
}

/*
 *  Report process unload/load and update maps and tables.
 */

process_load ()
{
    int prev_ldcnt;
    struct ldinfo *prev_ldinfo;
    struct fdinfo *prev_fdinfo;
    extern int loadcnt;

    if (!dynamic_loading) {
    	dynamic_loading = true;
    }

    update_load_maps();
    action_mask |= LOADCALL;
}

/*
 *  Reorganize debuggee's address spaces and symbols after load/unload
 */

update_load_maps ()
{
    int prev_ldcnt;
    struct ldinfo *prev_ldinfo;
    struct fdinfo *prev_fdinfo;
    extern int loadcnt;
    extern String prevsource;
    int i;

/* Save previous loader information */
    prev_ldcnt = loadcnt;
    prev_ldinfo = loader_info;
    prev_fdinfo = fd_info;

/* Get new loader information. */
    getldrinfo(false);

/* Get new symbolic information and retain old information. */
    sync_load_info(prev_ldinfo,prev_fdinfo,prev_ldcnt);

/* Clean up (close) previous opened file descriptors within prev_ldinfo */
    for (i = 0; i < prev_ldcnt; i++)
    {
        extern  LDFILE **ldentry;
	extern boolean lazy; 
        int j = 0, needed = false;
			/* if ldentry has not be ldclosed */
	if (lazy || noexec || coredump)
        while (!needed && j < loadcnt) {
            if (prev_ldinfo[i].ldinfo_fd == ldentry[j]->ioptr->_file)
               needed = true;
            j++;
        }
				/* close if not needed and not input file  */
	if ((!needed) && (!(checkinput(prev_ldinfo[i].ldinfo_fd))))
            close(prev_ldinfo[i].ldinfo_fd);
    }
    prevsource = nil;          /* reset since previous fd could be deleted */
    if (srcfp) {	       /* close open File from skimsource() */
	fclose(srcfp);
	errno = 0;	/* fclose could fail due to previsous close() */
	srcfp = nil;
	}
}

/*
 * Get the signal number associated with a given name.
 * The name is first translated to upper case if necessary.
 */

public int siglookup (s)
String s;
{
    register char *p, *q;
    char buf[100];
    int i;

    p = s;
    q = buf;
    while (*p != '\0') {
	if (*p >= 'a' and *p <= 'z') {
	    *q = (*p - 'a') + 'A';
	} else {
	    *q = *p;
	}
	++p;
	++q;
    }
    *q = '\0';
    p = buf;
    if (buf[0] == 'S' and buf[1] == 'I' and buf[2] == 'G') {
	p += 3;
    }
    i = 1;
    for (;;) {
	if (i >= sizeof(signames) / sizeof(signames[0])) {
	    error(catgets(scmc_catd, MS_execute, MSG_623,
	          "signal \"%1$s\" unknown"), s);
	    i = 0;
	    break;
	}
	if (signames[i] != nil and streq(signames[i], p)) {
	    break;
	}
	++i;
    }
    return i;
}

/*
 * Print all signals being ignored by the debugger.
 * These signals are auotmatically
 * passed on to the debugged process.
 */

public printsigsignored (p)
Process p;
{
    printsigs(~p->sigset[0],~p->sigset[1]);
}

/*
 * Print all signals being intercepted by
 * the debugger for the specified process.
 */

public printsigscaught (p)
Process p;
{
    printsigs(p->sigset[0],p->sigset[1]);
}

private printsigs (lowerset, upperset)
int lowerset, upperset;
{
    int s;
    int set = lowerset;
    char separator[2];

    separator[0] = '\0';
    for (s = 1; s < sizeof(signames) / sizeof(signames[0]); s++) {
	if (s > (8*sizeof(Word))) {
		set = upperset;
	}
	if (set & setrep((s > (8*sizeof(Word)) ? s - (8*sizeof(Word)) : s))) {
	    if (signames[s] != nil) {
		(*rpt_output)(stdout, "%s%s", separator, signames[s]);
		separator[0] = ' ';
		separator[1] = '\0';
	    }
	}
    }
    if (separator[0] == ' ') {
    	 (*rpt_output)(stdout, "\n" );
    }
}

/* Return the current process id for the child. */

curpid()
{
   return process->pid;
}


/* 
 *  Stuff from here on down deals with handling non-executable object files.
 */

objfile_readin()
{
   objfile = ldopen(objname, objfile);
   if (objfile == NULL) {
	fatal(catgets(scmc_catd, MS_execute, MSG_225,
	      "cannot read \"%s\""), objname);
   }
}


objfile_readtext(buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
   if (dataaddrs) {
       objfile_readdata(buff, addr, nbytes);
       return;
   } else if (fileaddrs) {
       objfile_readbytes(buff, addr, nbytes);
       return;
   }
   if (textorg != 0)
	addr += textorg;
   if ((addr < text_scn_hdr.s_paddr) ||
			   (addr > text_scn_hdr.s_paddr + text_scn_hdr.s_size))
	error( catgets(scmc_catd, MS_execute, MSG_226,
				   "Address out of bounds for text section."));
   FSEEK(objfile, (long int) (text_scn_hdr.s_scnptr+addr-text_scn_hdr.s_paddr),
			BEGINNING);
   FREAD((void *) buff, (size_t) nbytes, (size_t) sizeof(Byte), objfile);
}


objfile_readdata(buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
   if (textaddrs) {
       objfile_readtext(buff, addr, nbytes);
       return;
   } else if (fileaddrs) {
       objfile_readbytes(buff, addr, nbytes);
       return;
   }
   if (dataorg != 0)
	addr += dataorg;
   if ((addr < data_scn_hdr.s_paddr) ||
			   (addr > data_scn_hdr.s_paddr + data_scn_hdr.s_size))
	error( catgets(scmc_catd, MS_execute, MSG_227,
				   "Address out of bounds for data section."));
   FSEEK(objfile, (long int)(data_scn_hdr.s_scnptr+addr-data_scn_hdr.s_paddr),
								    BEGINNING);
   FREAD((void *) buff, (size_t) nbytes, (size_t) sizeof(Byte), objfile);
}

objfile_readbytes(buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
   FSEEK(objfile, (long int) addr, BEGINNING);
   FREAD((void *) buff, (size_t) nbytes, (size_t) sizeof(Byte), objfile);
}



objfile_close()
{
   ldclose(objfile);
}
