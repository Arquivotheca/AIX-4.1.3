static char sccsid[] = "@(#)54	1.17.1.13  src/bos/usr/ccs/lib/libdbx/library.c, libdbx, bos41J, 9520A_a 5/17/95 17:37:03";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: alloc, back, backv, beginerrmsg, bset0, call, callv, catcherrs,
 *	      cmp, dispose, enderrmsg, errmsg, error, fatal, fswap, get, index,
 *	      initErrInfo, ischild, isptraced, mov, nil, nocatcherrs, 
 *	      numerrors, numwarnings, onsyserr, ord, panic, pfind,
 *	      ptraced, put, pwait, rindex, shell, strdup, streq, syserr,
 *	      unptraced, warning
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

/*
 * General purpose routines.
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/termio.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "defs.h"
#include "main.h"
#include "process.h"

extern fork_type multproc;
extern int newdbg;
int newdebuggee;    /* pid of the forked debuggee process */

#undef FILE

#include "dbx_msg.h"
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

String cmdname;			/* name of command for error messages */
String prompt;			/* prompt string - usually cmdname */
Filename errfilename;		/* current file associated with error */
short errlineno;		/* line number associated with error */
boolean forkpending = false;	/* Stopped due to fork */
boolean execpending = false;	/* Stopped due to exec */
boolean loadpending = false;	/* Stopped due to load */
extern void (*dpi_update_proc)();
extern int  dpi_update_count;
extern int  dpi_update_delay;

/*
 * Definitions for doing memory allocation.
 */

extern char *malloc();
extern void free();

#define alloc(n, type)	((type *) malloc((unsigned) (n) * sizeof(type)))
#define forking		(pnum == pid)

typedef void (*VoidFunc)();

extern VoidFunc onsyserr();

typedef struct {
    VoidFunc func;
} ErrInfo;

#define ERR_IGNORE ((VoidFunc) 0)
#define ERR_CATCH  ((VoidFunc) 1)

/*
 * Call a program.
 *
 * Four entries:
 *
 *	call, callv - call a program and wait for it, returning status
 *	back, backv - call a program and don't wait, returning process id
 *
 * The command's standard input and output are passed as FILE's.
 */


#define MAX_NARGS 1000   /* unchecked upper limit on max num of arguments */
#define BADEXEC 127	/* exec fails */

#define ischild(pid)    ((pid) == 0)

/* VARARGS3 */
public int call(name, in, out, args)
String name;
File in;
File out;
String args;
{
    String *ap, *argp;
    String argv[MAX_NARGS];

    argp = &argv[0];
    *argp++ = name;
    ap = &args;
    while (*ap != nil) {
	*argp++ = *ap++;
    }
    *argp = nil;
    return callv(name, in, out, argv);
}

/* VARARGS3 */
public int back(name, in, out, args)
String name;
File in;
File out;
String args;
{
    String *ap, *argp;
    String argv[MAX_NARGS];

    argp = &argv[0];
    *argp++ = name;
    ap = &args;
    while (*ap != nil) {
	*argp++ = *ap++;
    }
    *argp = nil;
    return backv(name, in, out, argv);
}

public int callv(name, in, out, argv)
String name;
File in;
File out;
String *argv;
{
#ifndef _NO_PROTO
    typedef void (*Operation)(int);
#else
    typedef void (*Operation)();
#endif
    Operation quit_catcher;
    Operation intr_catcher;
    int pid, status;

    pid = backv(name, in, out, argv);
    quit_catcher = signal(SIGQUIT, SIG_DFL);   /* Store addr. of old signal  */
    intr_catcher = signal(SIGINT,  SIG_DFL);   /* catching routines.         */
    signal(SIGINT,  SIG_IGN);		       /* Ignore quits and interupts */
    signal(SIGQUIT, SIG_IGN);		       /* during child execution.    */
    pwait(pid, &status);
    signal(SIGINT,  intr_catcher);	       /* Restore to original state. */
    signal(SIGQUIT, quit_catcher);
    return status;
}

/*
 * Swap file numbers so as to redirect standard input and output.
 */

public fswap(oldfd, newfd)
int oldfd;
int newfd;
{
    if (oldfd != newfd) {
	close(oldfd);
	dup(newfd);
	close(newfd);
    }
}

public int backv(name, in, out, argv)
String name;
File in;
File out;
String *argv;
{
    int pid;

    fflush(stdout);
    if (ischild(pid = fork())) {
	fswap(0, fileno(in));
	fswap(1, fileno(out));
	onsyserr(EACCES, ERR_IGNORE);
	execvp(name, argv);
	_exit(BADEXEC);
    }
    return pid;
}

/*
 * Invoke a shell on a command line.
 */

#define DEF_SHELL	"sh"

public shell(s)
String s;
{
    extern String getenv();
    String sh;
    Ttyinfo savein, saveout;

    savetty(stdin, &savein);
    savetty(stdout, &saveout);
    if ((sh = getenv("SHELL")) == nil) {
	sh = DEF_SHELL;
    }
    if (s != nil and *s != '\0') {
	call(sh, stdin, stdout, "-c", s, 0);
    } else {
	call(sh, stdin, stdout, 0);
    }
    restoretty(stdin, &savein);
    restoretty(stdout, &saveout);
}

/*
 * Wait for a process the right way.  We wait for a particular
 * process and if any others come along in between, we remember them
 * in case they are eventually waited for.
 *
 * This routine is not very efficient when the number of processes
 * to be remembered is large.
 *
 * To deal with a kernel idiosyncrasy, we keep a list on the side
 * of "traced" processes, and do not notice them when waiting for
 * another process.
 */

typedef struct pidlist {
    int pid;
    int status;
    struct pidlist *next;
} Pidlist;

private Pidlist *pidlist, *ptrclist, *pfind();

public ptraced(pid)
int pid;
{
    Pidlist *p;

    p = alloc(1, Pidlist);
    p->pid = pid;
    p->next = ptrclist;
    ptrclist = p;
}

public unptraced(pid)
int pid;
{
    register Pidlist *p, *prev;

    prev = nil;
    p = ptrclist;
    while (p != nil and p->pid != pid) {
	prev = p;
	p = p->next;
    }
    if (p != nil) {
	if (prev == nil) {
	    ptrclist = p->next;
	} else {
	    prev->next = p->next;
	}
	dispose(p);
    }
}

private boolean isptraced(pid)
int pid;
{
    register Pidlist *p;

    p = ptrclist;
    while (p != nil and p->pid != pid) {
	p = p->next;
    }
    return (boolean) (p != nil);
}

#define STOPPED 0x7f
#define FORK 0x7e
#define EXEC 0x7d
#define LOAD_OR_UNLOAD 0x7c


/*
 * Signal handler for the SIGALRM.  This is only called when there is a
 * dpi_update_proc registered.  It is also only called when we are
 * waiting for the process to finish in pwait().
 *
 * This was added to allow dpi debuggers to update their interfaces while
 * we are waiting for the child to do something interesting.
 */
int call_update_proc(sig)
{
    (*dpi_update_proc)();
    dpi_update_count = 0;
    signal(SIGALRM, call_update_proc);
    ualarm(dpi_update_delay * 1000000, 0);
}

public pwait(pid, statusp)
int pid, *statusp;
{
    Pidlist *p;
    int pnum = 0, status;
    int statflag;   /* The low order byte of the status */
    boolean forkpend;     /* Fork is pending */
    boolean loadpend;     /* load or unload is pending */
    static  int  alarm_left = 0;
    struct sigaction alrm_action;

    p = pfind(pid);
    if (p != nil) {
	*statusp = p->status;
	dispose(p);
    } else {
	if (dpi_update_proc)
	{
	    /*
	     * dpi_update_count lets us know if the dpi_update_proc has
	     * been called or not.  This is necessary since if the user
	     * does a 'trace; run', then this routine will be called
	     * lots of times, and the timer will never go off unless we
	     * adjust for that.
	     */
	    if (dpi_update_count++)
	    {
		if (alarm_left <= 0)
		{
		    dpi_update_count = 0;
		    (*dpi_update_proc)();
		    alarm_left = dpi_update_delay * 1000000;
		}
	    }
	    else
		alarm_left = dpi_update_delay * 1000000;
		
	    signal(SIGALRM, call_update_proc);
	    ualarm(alarm_left, 0);
	}
	pnum = wait(&status);

        /* Need to continue the wait if the wait was interrupted and failed */
        while ((pnum == -1) && (errno == EINTR)) {
            pnum = wait(&status);
        }

	if (dpi_update_proc)
	{
	    /* turn off the alarm	*/
	    signal(SIGALRM, SIG_IGN);
	    alarm_left = ualarm(0, 0);
	    
	    /*
	     * round down some incase we are doing step by instruction
	     * otherwise it will never stop.  This is not even close to
	     * accurate, but to get accuracy would require even more
	     * system calls to get/set timers, which is way too expensive.
	     */
	    alarm_left &= ~0x3fff;
	}

	statflag = status & 0xff;	/* Get appropriate part of status */
	forkpend = (boolean) ((multproc != off) && (statflag == FORK));
	loadpend = (boolean) (statflag == LOAD_OR_UNLOAD);
	forkpending = forkpend;
	loadpending = loadpend;

	/* This gets compilcated here.  If we are in multiprocess debugging
	 * mode, then we need to be careful.  If we are woken up as the result
	 * of a fork, then we may be woken up by either the child of the
	 * originally debugged process, or by the original debugged process.
	 * So, we must wait until we have received both.  If we have woken up
	 * as the result of an exec, then the process is still the same, but
	 * we need to start over.
	 */

	while ((pnum != pid && pnum >= 0) || (forkpend)) {
	    statflag = status & 0xff;
	    if (pnum == pid) {    /* Parent forked debuggee */
		while (forkpend) {
		     pnum = wait(&status);
		     statflag = status & 0xff;
		     if (statflag == FORK) {
		       if (pnum != pid) {  /* Can this not happen? */
		         newdebuggee = pnum;
			 forkpend = false;
			 pnum = pid;  /* Reset to fall out of loop */
		       }
		     } else {
                         if ((not isptraced(pnum)) && (pnum != newdbg)) {
                            p = alloc(1, Pidlist);
                            p->pid = pnum;
                            p->status = status;
                            p->next = pidlist;
                            pidlist = p;
                         }
		     }
		}      /* Parent of forked debuggee found first */
	     } else if (forkpend) {    /* Child forked debuggee */
		     newdebuggee = pnum;
                     pnum = wait(&status);
		     while (pnum != pid) {
                         if (pnum < 0)
			     break;
                         if ((not isptraced(pnum)) && (pnum != newdbg)) {
                            p = alloc(1, Pidlist);
                            p->pid = pnum;
                            p->status = status;
                            p->next = pidlist;
                            pidlist = p;
                         }
			 pnum = wait(&status);
		     }
		     forkpend = false;
	     } else {		/* Some stray child. */
                     if ((not isptraced(pnum)) & (pnum != newdbg)) {
                        p = alloc(1, Pidlist);
                        p->pid = pnum;
                        p->status = status;
                        p->next = pidlist;
                        pidlist = p;
                     }
		     pnum = wait(&status);
		     statflag = status & 0xff;
		     if (!forkpend) {
		         forkpend = (boolean) ((multproc != off) 
                                            && (statflag == FORK));
		         forkpending = forkpend;
		     }
		     if (!loadpend) {
		         loadpend = (boolean) (statflag == LOAD_OR_UNLOAD);
		         loadpending = loadpend;
		     }
	     }
	}
	if (pnum < 0) {
	    p = pfind(pid);
	    if (p == nil) {
	/*     panic("pwait: pid %d not found", pid); */
	       *statusp = 0;
	    }
	    else {
	       *statusp = p->status;
	       dispose(p);
	    }
	} else {
            switch(statflag)
            {
                case FORK:           /* application forked */
                   if (multproc != off) {
                     ffork(newdebuggee);
		   }
		   status = 0x500 | STOPPED;
                   break;
                case EXEC:           /* application execed    */
                   if (multproc != off) {
                     dbxexec(pnum);
		   }
		   status = 0x500 | STOPPED;
                   break;
                case LOAD_OR_UNLOAD:           /* application execed    */
		   status = 0x500 | STOPPED;
		   process_load();
                   break;
                default:             /* normal stop     */
                   break;
            }
	    *statusp = status;
	}
    }
}

/*
 * Look for the given process id on the pidlist.
 *
 * Unlink it from list if found.
 */

private Pidlist *pfind(pid)
int pid;
{
    register Pidlist *p, *prev;

    prev = nil;
    for (p = pidlist; p != nil; p = p->next) {
	if (p->pid == pid) {
	    break;
	}
	prev = p;
    }
    if (p != nil) {
	if (prev == nil) {
	    pidlist = p->next;
	} else {
	    prev->next = p->next;
	}
    }
    return p;
}

/*
 * System call error handler.
 *
 * The syserr routine is called when a system call is about to
 * set the c-bit to report an error.  Certain errors are caught
 * and cause the process to print a message and immediately exit.
 */

extern int sys_nerr;
extern char *sys_errlist[];
 
/*
 * Before calling syserr, the integer errno is set to contain the
 * number of the error.  The routine "_mycerror" is a dummy which
 * is used to force the loader to get my version of cerror rather
 * than the usual one.
 */

extern int errno;
/*
extern _mycerror();
*/

/*
 * Initialize error information, setting defaults for handling errors.
 */

private ErrInfo *errinfo;

private initErrInfo ()
{
    integer i;

    errinfo = alloc(sys_nerr, ErrInfo);
    for (i = 0; i < sys_nerr; i++) {
	errinfo[i].func = ERR_CATCH;
    }
    errinfo[0].func = ERR_IGNORE;
    errinfo[EPERM].func = ERR_IGNORE;
    errinfo[ENOENT].func = ERR_IGNORE;
    errinfo[ESRCH].func = ERR_IGNORE;
    errinfo[EBADF].func = ERR_IGNORE;
    errinfo[ENOTTY].func = ERR_IGNORE;
}

public syserr()
{
    register ErrInfo *e;

    if (errno < 0 or errno > sys_nerr) {
	fatal("errno %d", errno);
    } else {
	if (errinfo == nil) {
	    initErrInfo();
	}
	e = &(errinfo[errno]);
	if (e->func == ERR_CATCH) {
	    fatal(sys_errlist[errno]);
	} else if (e->func != ERR_IGNORE) {
	    (*e->func)();
	}
    }
}

/*
 * Catcherrs' purpose is to initialize the errinfo table, get this module
 * loaded, and make sure my cerror is loaded (only applicable when this is
 * in a library).
 */

public catcherrs()
{
/*
    _mycerror();
*/
    initErrInfo();
}

/*
 * Turn off the error catching mechanism completely by having all errors
 * ignored.  This is most useful between a fork and an exec.
 */

public nocatcherrs()
{
    integer i;

    for (i = 0; i < sys_nerr; i++) {
	errinfo[i].func = ERR_IGNORE;
    }
}

/*
 * Change the action on receipt of an error, returning the previous action.
 */

public VoidFunc onsyserr(n, f)
int n;
VoidFunc f;
{
    VoidFunc oldf;

    if (errinfo == nil) {
	initErrInfo();
    }
    oldf = errinfo[n].func;
    errinfo[n].func = f;
    return oldf;
}


/*
 * Standard error handling routines.
 */

private short nerrs;
private short nwarnings;

/*
 * Main driver of error message reporting.
 */

/* VARARGS2 */
private errmsg(errname, shouldquit, s, a, b, c, d, e, f, g, h, i, j, k, l, m)
String errname;
boolean shouldquit;
String s;
{
    fflush(stdout);
    if (shouldquit and cmdname != nil) {
	(*rpt_error)(stderr, "%s: ", cmdname);
    }
    if (errfilename != nil) {
	(*rpt_error)(stderr, "%s: ", errfilename);
    }
    if (errlineno > 0) {
	(*rpt_error)(stderr, "%d: ", errlineno);
    }
    if (errname != nil) {
	(*rpt_error)(stderr, "%s: ", errname);
    }
    (*rpt_error)(stderr, s, a, b, c, d, e, f, g, h, i, j, k, l, m);
	(*rpt_error)(stderr, "\n" );
    fflush(stderr);
    if (shouldquit) {
	quit(1);
    }
}

/*
 * For when printf isn't sufficient for printing the error message ...
 */

public beginerrmsg()
{
    fflush(stdout);
    if (errfilename != nil) {
	(*rpt_error)(stderr, "%s: ", errfilename);
    }
    if (errlineno > 0) {
	(*rpt_error)(stderr, "%d: ", errlineno);
    }
}

public enderrmsg()
{
	(*rpt_error)(stderr, "\n" );
    fflush(stderr);
    erecover();
}

/*
 * The messages are listed in increasing order of seriousness.
 *
 * First are warnings.
 */

/* VARARGS1 */
public warning(s, a, b, c, d, e, f, g, h, i, j, k, l, m)
String s;
{
    nwarnings++;
    errmsg( catgets(scmc_catd, MS_library, MSG_851,
           "warning"), false, s, a, b, c, d, e, f, g, h, i, j, k, l, m);
}

/*
 * Errors are a little worse, they mean something is wrong,
 * but not so bad that processing cannot continue.
 *
 * The routine "erecover" is called to recover from the error,
 * a default routine is provided that does nothing.
 */

/* VARARGS1 */
public error(s, a, b, c, d, e, f, g, h, i, j, k, l, m)
String s;
{
    extern erecover();

    nerrs++;
    errmsg(nil, false, s, a, b, c, d, e, f, g, h, i, j, k, l, m);
    erecover();
}

/*
 * Non-recoverable user error.
 */

/* VARARGS1 */
public fatal(s, a, b, c, d, e, f, g, h, i, j, k, l, m)
String s;
{
    if (attach)
      detach(process->pid, 0);
    errmsg( catgets(scmc_catd, MS_library, MSG_852,
           "fatal error"), true, s, a, b, c, d, e, f, g, h, i, j, k, l, m);
}

/*
 * Panics indicate an internal program error.
 */

/* VARARGS1 */
public panic(s, a, b, c, d, e, f, g, h, i, j, k, l, m)
String s;
{
    errmsg( catgets(scmc_catd, MS_library, MSG_853,
           "internal error"), false, s, a, b, c, d, e, f, g, h, i, j, k, l, m);
}

short numerrors()
{
    short r;

    r = nerrs;
    nerrs = 0;
    return r;
}

short numwarnings()
{
    short r;

    r = nwarnings;
    nwarnings = 0;
    return r;
}

/*
 * Recover from an error.
 *
 * This is the default routine which we aren't using since we have our own.
 *
public erecover()
{
}
 *
 */

/*
 * Default way to quit from a program is just to exit.
 *
public quit(r)
int r;
{
    exit(r);
}
 *
 */

/*
 * Compare n-byte areas pointed to by s1 and s2
 * if n is 0 then compare up until one has a null byte.
 */

public int cmp(s1, s2, n)
register char *s1, *s2;
register unsigned int n;
{
    if (s1 == nil || s2 == nil) {
	panic("cmp: nil pointer");
    }
    if (n == 0) {
	while (*s1 == *s2++) {
	    if (*s1++ == '\0') {
		return(0);
	    }
	}
	return(*s1 - *(s2-1));
    } else {
	for (; n != 0; n--) {
	    if (*s1++ != *s2++) {
		return(*(s1-1) - *(s2-1));
	    }
	}
	return(0);
    }
}

/*
 * Move n bytes from src to dest.
 * If n is 0 move until a null is found.
 */

public mov(src, dest, n)
register char *src, *dest;
register unsigned int n;
{
    if (src == nil)
	panic("mov: nil source");
    if (dest == nil)
	panic("mov: nil destination");
    if (n != 0) {
	for (; n != 0; n--) {
	    *dest++ = *src++;
	}
    } else {
	while ((*dest++ = *src++) != '\0');
    }
}

/*
public bcopy (fromaddr, toaddr, n)
char *fromaddr, *toaddr;
int n;
{
    memcpy(toaddr, fromaddr, n);
}
*/

public bset0 (addr, n)
char *addr;
int n;
{
    memset((void *) addr, 0, (size_t) n);
}

/*
 * TUP(c)  - convert c to upper case only if it is lower case
 */
#define TUP(c) (islower(c) ? ((c) -'a' + 'A') : (c))

/*
 * strcmpi(s, t) - Case-insensitive strcmp()
 */
int strcmpi(s, t)
    char *s, *t;
{
  for ( ; TUP(*s) == TUP(*t); ++s, ++t)
    if (*s == '\0')
      return 0;
  return *s - *t;
}
