/* @(#)47	1.9  src/bos/usr/ccs/lib/libdbx/execute.h, libdbx, bos411, 9428A410j 10/18/93 03:54:31 */
#ifndef _h_execute
#define _h_execute
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27, 83
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
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#include "machine.h"
#include <termio.h>
#include <sys/signal.h>
#include <sys/ptrace.h>

#define MAXNCMDARGS 1000         /* maximum number of arguments to RUN   */
#define MAXCMDLNLEN 1000         /* maximum length of the command length */
#define NOTSTARTED 1
#define STOPPED 0177
#define FINISHED 0

/*
 * This magic macro enables us to look at the process' registers
 * in its user structure.
 */

#ifdef vax
#  define regloc(reg)	(ctob(UPAGES) + (sizeof(Word) * (reg)))
#endif
#ifdef mc68000
#  define regloc(reg)	(ctob(UPAGES) + (sizeof(Word) * ((reg) - PC)) - 10)
#endif
#ifdef ibm032
#  define U_OFSET(u,f)	((char *)&u.f - (char *)&u)
#endif

#define WMASK           (~(sizeof(Word) - 1))
#define cachehash(addr) ((unsigned) ((((unsigned) addr) >> 2) % CACHESIZE))

#define FIRSTSIG        SIGINT
#define LASTSIG         SIGQUIT
#define ischild(pid)    ((pid) == 0)
#define traceme()       ptrace(0, 0, 0, 0, 0)
#define setrep(n)       (1 << ((n)-1))
#define istraced(p)     ((p->signo > 32) ? p->sigset[1]&setrep(p->signo-32) \
					 : p->sigset[0]&setrep(p->signo))

/*
 * Ptrace options (specified in first argument).
 */

#ifndef _THREADS
#define U_READ   PT_READ_U       /* read from process's user structure */
#define U_WRITE  PT_WRITE_U      /* write to process's user structure */
#endif
#define I_READ   PT_READ_I       /* read from process's instruction space */
#define I_WRITE  PT_WRITE_I      /* write to process's instruction space */
#define D_READ   PT_READ_D       /* read from process's data space */
#define D_WRITE  PT_WRITE_D      /* write to process's data space */
#define CONT     PT_CONTINUE     /* continue stopped process */
#ifndef ibm032
#define SSTEP    PT_STEP         /* continue for approximately 1 instruction */
#endif
#define PKILL    PT_KILL	 /* terminate the process */


/* New PTRACE operations put in for dbx enhancements */
#define ATTACH  PT_ATTACH		/* Attach to running process. */
#define DETACH  PT_DETACH		/* Detach from process. */
#define REGSET  PT_REGSET		/* Return entire GPR set */
#define REATT   PT_REATT		/* Reattach debugger to process. */
#define NAME    PT_NAME	 		/* Return name of program. */
#define MULTI   PT_MULTI		/* Set/Clear multi-processing. */
#define S_FREE_PROC 0x7e	/* Sig sent for detach ptrace. */

#ifdef _THREADS
#   define readreg(p, r)        ptrace(PT_READ_GPR, p->pid, r, 0, 0);
#   define writereg(p, r, v)    ptrace(PT_WRITE_GPR, p->pid, r, v, 0);
#   define readflreg(p, r, a)   ptrace(PT_READ_FPR, p->pid, &(a), r, 0);
#   define writeflreg(p, r, a)  ptrace(PT_WRITE_FPR, p->pid, &(a), r, 0);
#   define getgprs(p, a)        ptrace(PT_REGSET, p->pid, &(a), 0, 0);
#   define readsprs(p, r)       ptrace(PTT_READ_SPRS, p->tid, r, 0,0);
#   define readgprs(p, r)       ptrace(PTT_READ_GPRS, p->tid, r, 0,0);
#   define readfprs(p, r)       ptrace(PTT_READ_FPRS, p->tid, r, 0,0);
#   define writesprs(p, r)      ptrace(PTT_WRITE_SPRS, p->tid, r, 0,0);
#   define writegprs(p, r)      ptrace(PTT_WRITE_GPRS, p->tid, r, 0,0);
#   define writefprs(p, r)      ptrace(PTT_WRITE_FPRS, p->tid, r, 0,0);
#else
#   define readreg(p, r)	ptrace(PT_READ_GPR, p->pid, r, 0);
#   define writereg(p, r, v)	ptrace(PT_WRITE_GPR, p->pid, r, v);
#   define readflreg(p, r, a)	ptrace(PT_READ_FPR, p->pid, &(a), r, 0);
#   define writeflreg(p, r, a)	ptrace(PT_WRITE_FPR, p->pid, &(a), r, 0);
#   define getgprs(p, a)	ptrace(PT_REGSET, p->pid, &(a), 0, 0);
#endif


/*
 * These definitions are for the arguments to "pio".
 */

typedef enum { PREAD, PWRITE } PioOp;
typedef enum { TEXTSEGM, DATASEGM } PioSeg;

typedef int (*Intfunc)();
#ifndef _NO_PROTO
typedef void (*Voidfunc)(int);
#else
typedef void (*Voidfunc)();
#endif

/*
 * Signal name manipulation.
 */

private String signames[NSIG] = {
    0, "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE", "KILL",
    "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG", "STOP", "TSTP", "CONT",
    "CHLD", "TTIN", "TTOU", "IO", "XCPU", "XFSZ", 0, "MSG", "WINCH", "PWR",
    "USR1", "USR2", "PROF", "DANGER", "VTALRM", "MIGRATE", "PRE", "VIRT", 0, 0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    "GRANT", "RETRACT", "SOUND", "SAK"
};

extern startprog (/* argv, infile, outfile */);
extern arginit ();
extern meta_expand(/* unresolved_buf, resolved_buf */);
extern newarg (/* arg, quoted */);
extern inarg (/* filename */);
extern outarg (/* filename */);
extern errarg (/* filename */);
extern run ();
extern cont (/* signo */);
extern void fixintr ();
extern resume (/* signo, last_bp */);
extern stepc ();
extern next ();
extern rtnfunc (/* f */);
extern stepover ();
extern stepto (/* addr */);
extern printstatus ();
extern printloc ();
extern boolean isbperr ();
extern Boolean notstarted (/* p */);
extern Boolean isfinished (/* p */);
extern int errnum (/* p */);
extern int errcode (/* p */);
extern int exitcode (/* p */);
extern iread (/* buff, addr, nbytes */);
extern iwrite (/* buff, addr, nbytes */);
extern dread (/* buff, addr, nbytes */);
extern dwrite (/* buff, addr, nbytes */);
extern ffork (/* pid */);
extern dbxexec(/* pid */);
extern int siglookup (/* s */);
extern printsigsignored (/* p */);
extern printsigscaught (/* p */);
extern curpid();
extern objfile_readin();
extern objfile_readtext(/* buff, addr, nbytes */);
extern objfile_readdata(/* buff, addr, nbytes */);
extern objfile_close();
#endif /* _h_execute */
