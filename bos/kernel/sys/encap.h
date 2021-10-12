/* @(#)28	1.3  src/bos/kernel/sys/encap.h, sysproc, bos411, 9428A410j 10/21/93 12:04:22 */
/*
 *   COMPONENT_NAME: SYSPROC
 * 
 *   FUNCTIONS:
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_ENCAP
#define _H_ENCAP

#ifdef _KERNEL

#include <sys/types.h>

/* Structure used by getctty/setctty routines */
struct ttyinfo
{
        pid_t           ti_ttysid;      /* pid of session leader */
        pid_t           ti_ttyp;        /* pid of process group leader */
        dev_t           ti_ttyd;        /* controlling tty device */
        off_t           ti_ttympx;      /* mpx value for controlling tty */
        unsigned        *ti_ttys;       /* ptr to t_state in tty struct */
        int             ti_ttyid;       /* tty id */
        int             (*ti_ttyf)();   /* tty query function pointer */
};

/* flags for rusage_incr function */
#define RUSAGE_MSGSENT	1
#define RUSAGE_MSGRCV	2
#define RUSAGE_INBLOCK	3
#define RUSAGE_OUTBLOCK	4

/* encapsulations routines */
#ifdef _NO_PROTO
void	  getctty();
pid_t	  getsid();
boolean_t is_blocked();
boolean_t is_caught();
boolean_t is_ignored();
int	  is_orphan();
boolean_t is_pgrp();
boolean_t is_pgrpl();
boolean_t is_sessl();
void	  limit_sigs();
void	  sigsetmask();
void	  rusage_incr();
void	  setctty();
#else /* _NO_PROTO */
void	  getctty(struct ttyinfo *);
pid_t	  getsid(pid_t);
boolean_t is_blocked(int);
boolean_t is_caught(int);
boolean_t is_ignored(int);
int	  is_orphan(pid_t);
boolean_t is_pgrp(pid_t);
boolean_t is_pgrpl(pid_t);
boolean_t is_sessl(pid_t);
void	  limit_sigs(sigset_t *, sigset_t *);
void	  sigsetmask(sigset_t *);
void	  rusage_incr(int, int);
void	  setctty(struct ttyinfo *);
#endif /* _NO_PROTO */

#endif

#endif /* _H_ENCAP */
