/* @(#)90	1.40.1.25  src/bos/kernel/sys/signal.h, sysproc, bos411, 9435C411a 8/31/94 17:37:24 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: LOW_SIG
 *		SIGADDSET
 *		SIGDELSET
 *		SIGFILLSET
 *		SIGINITSET
 *		SIGISMEMBER
 *		SIGMASK
 *		SIGMASKHI
 *		SIGMASKLO
 *		SIGMASKSET
 *		SIGORSET
 *		SIG_PENDING
 *		SIGSETEMPTY
 *		_clronstack
 *		_setnewstyle
 *		_setoldstyle
 *		_setonstack
 *		_testonstack
 *		_teststyle
 *		sigmask
 *		signal
 *		
 *
 *   ORIGINS: 27, 71, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_SYS_SIGNAL
#define _H_SYS_SIGNAL

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *
 *      The ANSI standard requires that certain values be in signal.h.
 *	The ANSI standard allows additional signals and pointers to 
 *	undeclarable functions with macro definitions beginning with
 * 	the letters SIG or SIG_ and an upper case letter.
 *      However, it also requires that if _ANSI_C_SOURCE is defined then 
 *      no other function definitions are present
 *
 *      This header includes all the ANSI required entries.  In addition
 *      other entries for the XIX system are included.
 *
 */

#ifdef _ANSI_C_SOURCE

#ifdef	_NONSTD_TYPES

#ifndef _MBI			/* signals might be ints or voids */
#define _MBI int		/* use -D_MBI=void for void signal handlers */
#endif /* _MBI */
extern _MBI (*signal())();

#else	/* ~_NONSTD_TYPES */

#ifdef	_NO_PROTO 
extern void (*signal())();
#else	/* ~_NO_PROTO */
extern void (*signal(int, void (*)(int)))(int);
#endif	/*  _NO_PROTO */

#endif	/* _NONSTD_TYPES */

#ifdef	_NO_PROTO
extern int raise();
#else /* ~ _NO_PROTO */
extern int raise(int);
#endif /* _NO_PROTO */

typedef volatile int sig_atomic_t; /* accessable as an atomic entity (ANSI) */
/*
 * maximum signal number, 0 is not used
 */
#define SIGMAX	63
/*
 * valid signal values: all undefined values are reserved for future use 
 * note: POSIX requires a value of 0 to be used as the null signal in kill()
 */
#define	SIGHUP	   1	/* hangup, generated when terminal disconnects */
#define	SIGINT	   2	/* interrupt, generated from terminal special char */
#define	SIGQUIT	   3	/* (*) quit, generated from terminal special char */
#define	SIGILL	   4	/* (*) illegal instruction (not reset when caught)*/
#define	SIGTRAP	   5	/* (*) trace trap (not reset when caught) */
#define	SIGABRT    6	/* (*) abort process */
#define SIGEMT	   7	/* EMT intruction */
#define	SIGFPE	   8	/* (*) floating point exception */
#define	SIGKILL	   9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	  10	/* (*) bus error (specification exception) */
#define	SIGSEGV	  11	/* (*) segmentation violation */
#define	SIGSYS	  12	/* (*) bad argument to system call */
#define	SIGPIPE	  13	/* write on a pipe with no one to read it */
#define	SIGALRM	  14	/* alarm clock timeout */
#define	SIGTERM	  15	/* software termination signal */
#define	SIGURG 	  16	/* (+) urgent contition on I/O channel */
#define	SIGSTOP	  17	/* (@) stop (cannot be caught or ignored) */
#define	SIGTSTP	  18	/* (@) interactive stop */
#define	SIGCONT	  19	/* (!) continue (cannot be caught or ignored) */
#define SIGCHLD   20	/* (+) sent to parent on child stop or exit */
#define SIGTTIN   21	/* (@) background read attempted from control terminal*/
#define SIGTTOU   22	/* (@) background write attempted to control terminal */
#define SIGIO	  23	/* (+) I/O possible, or completed */
#define SIGXCPU	  24	/* cpu time limit exceeded (see setrlimit()) */
#define SIGXFSZ	  25	/* file size limit exceeded (see setrlimit()) */
#define SIGMSG    27	/* input data is in the ring buffer */
#define SIGWINCH  28	/* (+) window size changed */
#define SIGPWR    29	/* (+) power-fail restart */
#define SIGUSR1   30	/* user defined signal 1 */
#define SIGUSR2   31	/* user defined signal 2 */
#define SIGPROF   32	/* profiling time alarm (see setitimer) */
#define SIGDANGER 33	/* system crash imminent; free up some page space */
#define SIGVTALRM 34	/* virtual time alarm (see setitimer) */
#define SIGMIGRATE 35	/* migrate process */
#define SIGPRE	  36	/* programming exception */
#define SIGVIRT   37	/* AIX virtual time alarm */
#define SIGALRM1  38	/* m:n condition variables */
#define SIGWAITING 39	/* m:n scheduling */
#define SIGKAP    60    /* keep alive poll from native keyboard */
#define SIGGRANT  SIGKAP /* monitor mode granted */
#define SIGRETRACT 61   /* monitor mode should be relinguished */
#define SIGSOUND  62    /* sound control has completed */
#define SIGSAK    63	/* secure attention key */
/*
 * additional signal names supplied for compatibility, only 
 */
#define SIGIOINT SIGURG	/* printer to backend error signal */
#define SIGAIO	SIGIO	/* base lan i/o */
#define SIGPTY  SIGIO	/* pty i/o */
#define SIGIOT  SIGABRT /* abort (terminate) process */ 
#define	SIGCLD	SIGCHLD	/* old death of child signal */
#define SIGLOST	SIGIOT	/* old BSD signal ?? */
/*
 * valid signal action values; other values => pointer to handler function 
 */
#ifndef	_NONSTD_TYPES		
#define	SIG_DFL		(void (*)(int))0
#define	SIG_IGN		(void (*)(int))1
#define SIG_HOLD	(void (*)(int))2	/* not valid as argument 
						   to sigaction or sigvec */
#define SIG_CATCH	(void (*)(int))3	/* not valid as argument 
						   to sigaction or sigvec */
#define SIG_ERR		(void (*)(int))-1
#else	/* _NONSTD_TYPES */
#define	SIG_DFL		(_MBI  (*)())0
#define	SIG_IGN		(_MBI  (*)())1
#define SIG_HOLD	(_MBI  (*)())2	
#define SIG_CATCH	(_MBI  (*)())3
#define SIG_ERR		(_MBI  (*)())-1
#endif	/* _NONSTD_TYPES */

/*
 * values of "how" argument to sigprocmask() call
 */
#define SIG_BLOCK	0
#define SIG_UNBLOCK	1
#define SIG_SETMASK	2

#endif /* _ANSI_C_SOURCE */

/*
 *   The following are values that have historically been in signal.h.
 *
 *   They are a part of the POSIX defined signal.h and therefore are
 *   included when _POSIX_SOURCE is defined.
 *
 */

#ifdef _POSIX_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>		/* Cannot be in ANSI - name space pollutant */
#endif

/*
 * sigaction structure used in sigaction() system call 
 */
struct sigaction {

#ifdef	_NONSTD_TYPES
	_MBI	(*sa_handler)();
#else	/* ~_NONSTD_TYPES */
#ifdef	_NO_PROTO
	void	(*sa_handler)();	/* signal handler, or action value */
#else	/* ~NO_PROTO */
	void	(*sa_handler)(int);	/* signal handler, or action value */
#endif	/* NO_PROTO */
#endif	/* _NONSTD_TYPES */

	sigset_t sa_mask;		/* signals to block while in handler */
	int	sa_flags;		/* signal action flags */
};

/*
 * valid flag define for sa_flag field of sigaction structure - POSIX
 */
#define SA_NOCLDSTOP	0x00000004	/* do not set SIGCHLD for child stops*/

#endif	/* _POSIX_SOURCE */

#ifdef _ALL_SOURCE
#include <sys/context.h>

/*
 * sigevent structure referred to (but not used) in asynchronous i/o
 */
struct sigevent {
	void	       *sevt_value;
	signal_t	sevt_signo;
};

/*
 * sigstack structure used in sigstack() system call 
 */
struct	sigstack {
	char	*ss_sp;			/* signal stack pointer */
	int	ss_onstack;		/* current status */
};
/*
 * valid signal action values; other values => pointer to handler function 
 */
#define BADSIG		SIG_ERR
/*
 * valid flags define for sa_flag field of sigaction structure 
 */
#define	SA_ONSTACK	0x00000001	/* run on special signal stack */
#define SA_OLDSTYLE 	0x00000002	/* old "unreliable" UNIX semantics */
#define SA_RESTART	0x00000008	/* restart system calls on sigs*/
#define SA_NODUMP       0x00000010	/* termination by this sig does not cause a core file */
#define SA_PARTDUMP     0x00000020	/* create a partial dump for this signal */
#define SA_FULLDUMP     0x00000040	/* create a full dump (with data areas) */
#define SA_SIGSETSTYLE  0x00000080	/* new system V sigset type semantics */
/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to properly restore state if
 * a non-standard exit is performed.
 */
struct	sigcontext {
	int		sc_onstack;	/* sigstack state to restore */
	sigset_t	sc_mask;	/* signal mask to restore */
	int		sc_uerror;	/* u_error to restore */
	struct	jmpbuf	sc_jmpbuf;	/* process context to restore */
};
/*
 * flag bits defined for parameter to psig and sendsig; bits indicate
 *  what context (where and how much) should be saved on signal delivery
 */       
#define	NO_VOLATILE_REGS	0x0001
#define	USE_SAVE_AREA		0x0002
/* 
 * macros to manipulate signal sets
 */
#define	SIGMASKLO(__s)	( 1 << ((__s) - 1) )
#define SIGMASKHI(__s)	( 1 << (((__s)-32) - 1) )
#define LOW_SIG(__s)	((__s) <= 32 )
#define SIGMASK(__s)    (1<<(((__s)-1)&31))

#define	SIGFILLSET(__set)	\
{	(__set).losigs = ~0;	\
	(__set).hisigs = ~0;	\
}

#define SIGDELSET(__set,__s)        \
	{*((&(__set).losigs)+(((__s)-1)>>5)) &= ~(1<<(((__s)-1)&31));}

#define SIGADDSET(__set,__s)        \
	{*((&(__set).losigs)+(((__s)-1)>>5)) |= (1<<(((__s)-1)&31));}

#define SIGSETEMPTY(__set)	\
	(!((__set).losigs) && !((__set).hisigs))

#define SIGISMEMBER(__set,__s) \
	( (*((&(__set).losigs)+(((__s)-1)>>5)) >> (((__s)-1)&31) ) & 1 )

/* SIG_PENDING will report inadequate results if _THREADS */
#define SIG_PENDING(__p)						\
	((								\
	  (__p)->p_sig.losigs &						\
		~((__p)->p_sigignore.losigs | (__p)->p_sigmask.losigs)	\
	 ) || (								\
	  (__p)->p_sig.hisigs &						\
		~((__p)->p_sigignore.hisigs | (__p)->p_sigmask.hisigs)	\
	))

#define	SIGINITSET(__set)	\
{	(__set).losigs = 0;	\
	(__set).hisigs = 0;	\
}
#define SIGMASKSET(__dest, __mask) \
{	(__dest).losigs &= ~(__mask).losigs; \
	(__dest).hisigs &= ~(__mask).hisigs; \
}

#define SIGORSET(__dest, __mask) \
{	(__dest).losigs |= (__mask).losigs; \
	(__dest).hisigs |= (__mask).hisigs; \
}


/*
 * sigvec structure used in sigvec compatibility interface.
 */
struct	sigvec {
#ifdef	_NONSTD_TYPES
	_MBI	(*sv_handler)();	/* signal handler */
#else	/* ~_NONSTD_TYPES */	
#ifdef	_NO_PROTO
	void    (*sv_handler)();	/* signal handler */
#else	/* ~_NO_PROTO */
	void    (*sv_handler)(int);	/* signal handler */
#endif	/* _NO_PROTO */
#endif	/* _NONSTD_TYPES */
	int     sv_mask;        /* signal mask to apply */
	int     sv_flags;
};                           
#define	sv_onstack	sv_flags
/*
 * values in sv_flags are interpreted identically to values in
 * sa_flags for sigaction(), except SV_INTERRUPT has the opposite
 * meaning as SA_RESTART.  The following additional names are
 * names are defined for values in sv_flags to be compatible with
 * old usage of sigvec() 
 */
#define NSIG		(SIGMAX+1)	/* maximum number of signals */
#define SIG_STK		SA_ONSTACK	/* bit for using sigstack stack */
#define SIG_STD		SA_OLDSTYLE	/* bit for old style signals */
#define SV_ONSTACK	SA_ONSTACK	/* bit for using sigstack stack */
#define SV_INTERRUPT	SA_RESTART	/* bit for NOT restarting syscalls */

#define _OLDSTYLE 	(SA_OLDSTYLE)	/* bit for old style signals */
#define _ONSTACK  	(SA_ONSTACK)	/* bit for using sigstack stack */
#define _teststyle(__n)   ((__n) & _OLDSTYLE) /** TRUE if Bell style signals. **/
#define _testonstack(__n) ((__n) & _ONSTACK)  /** TRUE if on user-sig stack.  **/
#define _setoldstyle(__n) ((__n) | _OLDSTYLE)
#define _setnewstyle(__n) ((__n) & ~_OLDSTYLE)
#define _setonstack(__n)  ((__n) | _ONSTACK)
#define _clronstack(__n)  ((__n) & ~_ONSTACK)

#ifndef _KERNEL 
/*
 * Macro for converting signal number to a mask suitable for
 * sigblock().
 */
#define sigmask(__m)	(1 << ((__m)-1))
#endif /* _KERNEL */

#ifdef _THREAD_SAFE
/**********
 for ssignal and gsignal
**********/

#ifdef _NO_PROTO
void (*ssignal_r())();
int gsignal_r();
#else
void (*ssignal_r(int, void (*)(int), void (*[])(int)))(int);
int gsignal_r(int, void (*[])(int));
#endif

#define MAXSIG	16
#define MINSIG	(-4)
#define TOT_USER_SIG (MAXSIG - MINSIG + 1)
#endif /* THREAD_SAFE */

#endif /* _ALL_SOURCE */

#ifdef _POSIX_SOURCE

#ifdef _NO_PROTO
extern int kill();
extern int sigaction();
extern int sigprocmask();
extern int sigsuspend();
extern int sigemptyset();
extern int sigfillset();
extern int sigaddset();
extern int sigdelset();
extern int sigismember();
extern int sigpending();

#ifdef _ALL_SOURCE
extern int sigwait();
extern int siglocalmask();
extern int killpg();
extern int sigignore();
extern int sigblock();
extern int sighold();
extern int siginterrupt();
extern void (*sigset())();
extern int sigpause();
extern int sigrelse();
extern int sigsetmask();
extern int sigstack();
extern int sigvec();
#endif  /* _ALL_SOURCE */
#else

/*
 * function prototypes for signal functions
 */
#ifndef _KERNEL
/* system calls */
extern int kill(pid_t, int);
extern int sigaction(int, const struct sigaction *, struct sigaction *);
extern int sigprocmask(int, const sigset_t *, sigset_t *);
extern int sigsuspend(const sigset_t *);
#ifdef _ALL_SOURCE
extern int siglocalmask(int, const sigset_t *);
extern int sigsetmask(int);
extern int sigstack(struct sigstack *, struct sigstack *);
#endif /* _ALL_SOURCE */
#endif /* _KERNEL */
/* library routines */
extern int sigemptyset(sigset_t *);
extern int sigfillset(sigset_t *);
extern int sigaddset(sigset_t *, int);
extern int sigdelset(sigset_t *, int);
extern int sigismember(const sigset_t *, int);
extern int sigpending(sigset_t *);
#ifdef _ALL_SOURCE
#if _AIX32_THREADS
/* See comments in stdlib.h on _AIX32_THREADS */
extern int sigwait(sigset_t *);
#else   /*  !_AIX32_THREADS     POSIX 1003.4a Draft 7 prototype */
extern int sigwait(const sigset_t *, int *);
#endif  /*  !_AIX32_THREADS     POSIX 1003.4a Draft 7 prototype */
extern int killpg(pid_t, int);
extern int sigignore(int);
extern int sigblock(int);
extern int sighold(int);
extern int siginterrupt(int, int);
extern void (*sigset(int, void(*)(int)))(int);
extern int sigpause(int);
extern int sigrelse(int);
extern int sigvec(int, struct sigvec *, struct sigvec *);
#endif /* _ALL_SOURCE */
#endif /* _NO_PROTO */
#endif /* _POSIX_SOURCE */

#endif /* _H_SYS_SIGNAL */
