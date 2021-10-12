/* @(#)86	1.57.2.17  src/bos/kernel/sys/proc.h, sysproc, bos41J, 9512A_all 3/20/95 15:52:53 */
/*
 *   COMPONENT_NAME: SYSPROC 
 *
 *   FUNCTIONS: EXTRACT_NICE
 *              MAYBE_PID
 *              PROCMASK
 *              PROCPTR
 *              SET_NICE
 *              VALIDATE_PID
 *
 *   ORIGINS: 3, 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_PROC
#define _H_PROC

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/timer.h>
#include <sys/resource.h>
#include <sys/m_param.h>
#include <sys/pri.h>
#include <sys/lockl.h>
#include <sys/limits.h>
#include <sys/var.h>
#include <sys/thread.h>

/*
 *  One structure allocated per active process.  Entries that are
 *  in use are pinned to avoid page faults in kernel critical sections.
 *
 *  This structure contains per-process information that is needed
 *  when the process is swapped out.  The per-process data
 *  that is not needed when the process is swapped out is located
 *  in the processes u-block, <sys/user.h>.
 *  If threads are enabled, per-thread data is located in the thread and
 *  uthread structures, <sys/thread.h> and <sys/uthread.h>.
 *
 *  When changes are made here, be sure to update the assembler version
 *  in kernel/ml/proc.m4, too.
 */

#define max_proc ((struct proc *)v.ve_proc)

struct	proc {
	char		p_stat;		/* process state		*/
	char		pad1;		/* padding			*/
        short           p_xstat;        /* exit status for wait         */
	unsigned long	p_flag;		/* process flags		*/
	unsigned long	p_int;		/* process flags		*/
	unsigned long	p_atomic;	/* process flags                */ 

	/* main process link pointers	*/
	struct proc	*p_child;	/* head of list of children	*/
	struct proc	*p_siblings;	/* NULL terminated sibling list */
	struct proc	*p_uidl;	/* processes with same p_uid	*/
	struct proc	*p_ganchor;	/* anchor for pgrpl		*/

	/* thread fields */
	struct thread	*p_threadlist;	/* head of list of threads	*/
	unsigned short	p_threadcount;	/* number of threads		*/
	unsigned short	p_active;	/* number of active threads	*/
	unsigned short	p_suspended;	/* number of suspended threads	*/
	unsigned short	p_terminating;	/* number of terminating threads*/
	unsigned short	p_local;	/* number of "local" threads	*/

	/* scheduler fields */
	char		p_nice;		/* nice for cpu usage		*/
					/* NOTE: PUSER is added into	*/
					/* this field for processes that*/
					/* do not have a fixed priority */
#define P_NICE_DEFAULT	NZERO+PUSER	/* default value for p_nice	*/
#define P_NICE_MAX	40		/* maximum nice value		*/
#define P_NICE_MIN	0		/* minimum nice value		*/
	char		p_sched_pri;	/* most favored swapped thrd pri*/

	/* dispatcher fields */
	unsigned long	p_pevent;	/* pending events 		*/
	struct thread	*p_synch;	/* event list for threads waiting
					   for this process to be suspended */
	/* identifier fields */
	uid_t 		p_uid;		/* real user identifier		*/
	uid_t 		p_suid;		/* set user identifier		*/
	pid_t		p_pid;		/* unique process identifier	*/
	pid_t		p_ppid;		/* parent process identifier	*/
	pid_t		p_sid;		/* session identifier           */
	pid_t		p_pgrp;		/* process group leader pid	*/

	/* miscellaneous */
	Simple_lock	p_lock;		/* proc lock			*/
	unsigned long	p_kstackseg;	/* segment for more kstacks	*/
	unsigned long	p_adspace;	/* process address space	*/

	struct proc	*p_pgrpl;	/* circular list of process in  */
					/* the same process group.	*/
					/* NOTE: this field is valid, 	*/
					/* only if p_pgrp != 0          */
	struct proc	*p_ttyl;	/* circular list of process groups   */
					/* in the same session (p_sid).      */
					/* If the session has a controlling  */
					/* tty (u.u_ttyp), the processes     */
					/* (process groups) in the session   */
					/* have the same controlling tty.    */	
					/* NOTE: p_ttyl links one process of */
					/* each process group in the session */

	struct ptipc	*p_ipc;		/* ipc when being debugged	*/
	struct proc	*p_dblist;	/* processes being debugged	*/
	struct proc	*p_dbnext;	/* next in p_dblist		*/

	/* signal information */
	sigset_t	p_sig;		/* pending signals		*/
	sigset_t	p_sigignore;	/* signals being ignored	*/
	sigset_t	p_sigcatch;	/* signals being caught		*/

	/* zombie process information */
	struct rusage	p_ru;		/* Rusage structure for exit()  */

	/* process statistics */
	unsigned long	p_size;		/* size of image (pages)        */
	unsigned long   p_pctcpu;       /* cpu percentage               */
	unsigned long	p_auditmask;	/* Auditing stuff		*/
	unsigned long	p_minflt;	/* page fault count - no I/O    */
	unsigned long	p_majflt;	/* page fault count - I/O needed*/

	/* additional scheduler fields */
	int		p_repage;	/* repaging count		*/
	int	 	p_sched_count;	/* watchdog suspension count 	*/ 
	struct proc	*p_sched_next;	/* next process in swap queues  */
	struct proc	*p_sched_back;	/* previous process in swap q   */
	short		p_cpticks;	/* ticks of cpu time in last sec*/

	short		p_msgcnt;	/* uprintf message count	*/
	unsigned long	p_majfltsec;	/* maj flts in the last sec     */

	long		p_extra[1];	/* padding to 32 byte boundary	*/
};

#define p_link		p_child		/* any field not used when free */

#define xp_stat		p_xstat
#define xp_ru		p_ru
#define xp_utime	p_ru.ru_utime.tv_sec
#define xp_stime	p_ru.ru_stime.tv_sec

#define p_sigmask	p_threadlist->t_sigmask	/* to be removed (NFS) */

/*
 * NOTE: process flag values are or'd together for binary compatibility
 */

/*
 * process flags, p_flag
 *
 * This field can be updated under the process only.  If the process
 * is single threaded, then the update can be made at base level.  Otherwise
 * interrupts need to be disabled and the proc_int_lock held.
 */
#define SLOAD		0x00000001	/* user and uthread struct. pinned */
#define SNOSWAP		0x00000002	/* process can't be swapped out	   */
#define SFORKSTACK	0x00000004	/* special fork stack is allocated */
#define STRC		0x00000008	/* process being traced		   */
#define SFIXPRI		0x00000100	/* fixed priority, ignoring p_cpu  */
#define SKPROC		0x00000200	/* Kernel processes		   */
#define SSIGNOCHLD	0x00000400	/* do send SIGCHLD on child's death*/
#define SSIGSET		0x00000800	/* process uses the SVID sigset int*/
#define SLKDONE		0x00002000	/* this process has "done" locks   */
#define STRACING	0x00004000	/* process is a debugger	   */
#define SMPTRACE	0x00008000	/* multi-process debugging	   */
#define SEXIT		0x00010000	/* process is exiting		   */
#define SORPHANPGRP	0x00040000	/* orphaned process group	   */
#define SNOCNTLPROC	0x00080000	/* session leader relinquished	   */
					/* the controlling terminal	   */
#define SPPNOCLDSTOP	0x00100000	/* Do not send parent process	   */
					/*  SIGCHLD when a child stops	   */
#define SEXECED		0x00200000	/* process has exec'd		   */
#define SJOBSESS	0x00400000	/* job control used in session	   */
#define SJOBOFF		0x00800000      /* free from job control           */
#define SEXECING	0x01000000	/* process is execing		   */
#define SPSEARLYALLOC	0x04000000	/* allocates paging space early	   */

/*
 * process flags, p_int
 *
 * This field can be updated at the interrupt level, under the process,
 * or from another process provided that interrupts are disabled and the
 * proc_int_lock is held. 
 */
#define SJUSTBACKIN	0x00020000	/* process recently restarted	   */
#define SPSMKILL	0x02000000	/* paging space mgr chose me to die*/
#define STERM		0x10000000	/* process should be terminated	   */
#define SSUSP		0x20000000	/* suspend process in kmode/umode  */
#define SSUSPUM		0x40000000	/* suspend process in umode        */
#define SGETOUT		0x80000000	/* process should be swapped out   */

/*
 * This flag triggers a call to sig_slih.  When changing these flags,
 * one must also change proc.m4.  Moreover, the bits have to be consecutive
 * to work with rlinm.
 */
#define SSIGSLIH 	(STERM|SSUSP|SSUSPUM|SGETOUT)	

/*
 * process flags, p_atomic 
 *
 * This field is updated through atomic primitives only.
 */
#define SWTED		0x00000010	/* stopped while traced		   */
#define SFWTED		0x00000020	/* stopped after fork while traced */
#define SEWTED		0x00000040	/* stopped after exec while traced */
#define SLWTED		0x00000080	/* stopped after load/unload while */

/*
 * process states, p_stat
 */
#define SNONE		0		/* slot is available	*/
#define SIDL		4		/* process is created	*/
#define SZOMB		5		/* process is dying	*/
#define SSTOP		6		/* process is stopped	*/
#define SACTIVE		7		/* process is active	*/
#define SSWAP		8		/* process is swapped	*/

/*
 * Defines and macros for use with process table entries
 */
#define PIDRESERVED	7		/* number of bits reserved in pid */
#define PROCSHIFT	17		/* number of bits in proc index	  */
#define PGENSHIFT	8		/* proc index offset		  */
#define PGENMASK	((1<<PGENSHIFT)-1)
#define NPROC		(1<<PROCSHIFT)
#define PIDMASK		((NPROC-1)<<PGENSHIFT)
#define PIDMAX		((NPROC<<PGENSHIFT)-1)

/* pid are even, but 1 is also a pid */
#define MAYBE_PID(pid)  (!(pid & 1) || (pid == 1))

/* mask for proc index		*/
#define PROCMASK(pid)	((((pid)&PIDMASK)>>PGENSHIFT) | ((pid)&1))

#ifdef _KERNEL
extern struct proc proc[NPROC];		/* the process table, itself      */

/* convert pid to proc pointer	*/
#define PROCPTR(pid)	(&proc[PROCMASK(pid)])

/* validate whether a process id is valid or not  */
#define	VALIDATE_PID(pid)						\
	(((PROCPTR(pid) >= (struct proc *)v.ve_proc) ||			\
	  (PROCPTR(pid)->p_pid != (pid))) ?				\
					NULL : PROCPTR(pid))

/* determine if a process is multi-threaded given a procp or threadp */
#define MTHREAD(p)	( (p)->p_active > 1 )
#define MTHREADT(t)	( (t)->t_procp->p_active > 1 )

#endif

#define EXTRACT_NICE(p)                                                      \
        (((p)->p_flag & SFIXPRI) ? (P_NICE_MAX + 1) : ((p)->p_nice - PUSER))

#define SET_NICE(p,n)                                                   \
{                                                                       \
        if (!((p)->p_flag & SFIXPRI)) {                                 \
                (p)->p_nice = (MIN(P_NICE_MAX,MAX(0,(n)))) + PUSER;     \
        }                                                               \
}

/* global locks - listed in precedence */
extern Complex_lock core_lock;
extern Simple_lock time_lock;	
extern Simple_lock proc_tbl_lock;
extern Simple_lock ptrace_lock;	
extern Simple_lock watchdog_lock;
extern Simple_lock uprintf_lock;
extern Simple_lock tod_lock;	
extern Simple_lock proc_base_lock;	
extern Simple_lock proc_int_lock;	

/* 
 * Structures for resource intialization/termination handlers
 */
struct proch 
{
	struct	proch	*next;	/* next pointer */
	void (*handler)();		/* function to be called */
};

/* Defines to pass the resource handlers */
#define	PROCH_INITIALIZE	1
#define PROCH_TERMINATE		2
#define PROCH_SWAPOUT		3   /* process is being swapped out. It's   */
				    /*  u block is still pinned. Interrupts */
				    /*  are enabled                         */
#define PROCH_SWAPIN		4   /* process is being made runnable. It's */
				    /*  u block has been pinned , but it has*/
				    /*  not been put on a 'ready to run'    */
				    /*  queue. Interrupts are enabled.      */
#define THREAD_INITIALIZE      11
#define THREAD_TERMINATE       12

/*
 * p_pctcpu is a floating point number represented as a ulong. It has 
 * a range of [(2 ** 16 ) - 1, (2 ** -16) + 1].  To translate into a 
 * floating point divide by (double)FLT_MODULO.
 */
#define FLT_MODULO		(1<<16)

#endif  /* _H_PROC */
