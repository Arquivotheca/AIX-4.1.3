/* @(#)75    1.7.1.7  src/bos/usr/ccs/lib/libdbx/process.h, libdbx, bos411, 9428A410j 11/15/93 13:59:44 */
#ifndef _h_process
#define _h_process
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
#define DEFSIG -1
#define packedbits(n) (((n) + 31) >> 5)
#define NREGBITS packedbits(NGREG+NSYS+MAXFREG+1)
#define regword(n) (n >> 5)
#define regbit(n) (1 << (n & 037))
#ifdef KDBX
#define	LOWADDR	0
#else
#define	LOWADDR	0x10000000
#endif

/*
 * A cache of the instruction segment is kept to reduce the number
 * of system calls.  Might be better just to read the entire
 * code space into memory.
 */

#define CACHESIZE 1003

typedef struct {
    Word addr;
    Word val;
} CacheWord;

/*
 * This structure holds the information we need from the user structure.
 */

typedef struct {
    struct termio ttyinfo;
    integer fcflags;
} Ttyinfo;


struct Process {
    int pid;			/* process being traced */
#ifdef _THREADS
    tid_t tid;                  /* thread being traced */
#endif /* _THREADS */
    int mask;			/* process status word */
    unsigned valid[NREGBITS];   /* valid bits for registers */
    unsigned dirty[NREGBITS];   /* dirty bits for registers */
    Word reg[NGREG+NSYS];	/* process' registers */
    double freg[MAXFREG+1];	/* process' fp registers */
    short status;		/* either STOPPED or FINISHED */
    short signo;		/* signal that stopped process */
    short sigcode;		/* extra signal information */
    int exitval;		/* return value from exit() */
    long sigset[2];		/* bit array of traced signals */
    CacheWord word[CACHESIZE];	/* text segment cache */
    Ttyinfo ttyinfo;		/* process' terminal characteristics */
    Ttyinfo ttyinfo_in;		/* process' stdin terminal characteristics */
    Address sigstatus;		/* process' handler for current signal */
    Boolean is_bp;              /* is signal SIGTRAP a breakpoint */
};

typedef struct Process *Process;

/* ldinfo and fdinfo together reflect the memory and file usage of a process. */
struct ldinfo {
    unsigned int ldinfo_next;	/* Offset of next entry from here or 0 if end */
    int ldinfo_fd;		/* File descriptor returned by loader */
    unsigned int textorg;	/* Text origin */
    unsigned int textsize;	/* Text size */
    unsigned int dataorg;	/* Data origin */
    unsigned int datasize;	/* Data size */
};

struct fdinfo {
    char *pathname;		/* Pathname */
    char *membername;		/* Membername */
};

extern struct ldinfo *loader_info;
extern struct fdinfo *fd_info;
extern unsigned short current_hardware;	/* Used to indicate hardware we are
					 * executing on
					 */
    
Process process;
int fpregs;

extern process_init ();
extern getprog (/* pid,exec */);
extern Word reg (/* n */);
extern double fpregval (/* n */);
extern setreg (/* n, w */);
extern pstart (/* p, argv, infile, outfile */);
extern pterm (/* p */);
extern pcont (/* p, signo */);
extern psigtrace (/* p, sig, sw */);
extern setsigtrace ();
extern unsetsigtraces (/* p */);
extern sigs_off ();
extern sigs_on ();
extern getinfo (/* p, status */);
extern setinfo (/* p, signo */);
extern getsysregs (/* p, pc, cs, ics, mq */);
extern Address usignal (/* p */);
extern cacheflush (/* p */);
extern printptraceinfo ();
#endif /* _h_process */
