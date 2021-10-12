/* @(#)19       1.24  src/bos/usr/include/procinfo.h, sysproc, bos411, 9428A410j 6/24/94 10:14:47 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef	_H_PROCINFO
#define	_H_PROCINFO

#include <sys/proc.h>
#include <sys/user.h>


struct	procinfo
{
	/* identification/authentication */
	unsigned long	pi_pid;		/* process ID */
	unsigned long	pi_ppid;	/* parent process ID */
	unsigned long	pi_sid;		/* session identifier */
	unsigned long	pi_pgrp;	/* process group */
	unsigned long	pi_uid;		/* real user ID	*/
	unsigned long	pi_suid;	/* saved user ID */

	/* scheduler information */
	unsigned long	pi_pri;		/* priority, 0 high, 31 low */
	unsigned long	pi_nice;	/* nice for priority, 0 to 39 */
	unsigned long	pi_cpu;		/* processor usage, 0 to 80 */

	/* process states are defined in <sys/proc.h>: */
	unsigned long	pi_stat;	/* process state */

	/* process flags are defined in <sys/proc.h>: */
	unsigned long	pi_flag;

	/* dispatcher fields */
	char		*pi_wchan;	/* wait channel */
	unsigned long	pi_wtype;	/* the wait type */

	/* miscellaneous */
	unsigned long	pi_adspace;	/* process address space */
	unsigned long	pi_majflt;	/* i/o page faults	 */
	unsigned long	pi_minflt;	/* non i/o page faults   */

	/* valid when the process is a zombie only */
	unsigned long	pi_utime;	/* this process user time */
	unsigned long	pi_stime;	/* this process system time */

	/* process statistics */
	unsigned long	pi_size;	/* size of image (pages) */
};

struct	userinfo
{
	/* credentials information */
	unsigned long	ui_luid;	/* login user id */
	unsigned long	ui_uid;		/* effective user identifier */
	unsigned long	ui_gid;		/* effective group identifier */

	/* accounting and profiling data */
	unsigned long	ui_start;	/* time at which process began */
	struct rusage	ui_ru;		/* this process' rusage info */
	struct rusage	ui_cru;		/* children's rusage info */

	/* resource limits info */
	struct rlimit	ui_rlimit[RLIM_NLIMITS];	/* resource limits */

	/* size of text */
	unsigned long	ui_tsize;	/* size of text */

	/* controlling tty info */
	unsigned long	ui_ttyp;	/* has a controlling terminal */
	unsigned long	ui_ttyd;	/* controlling terminal */
	unsigned long	ui_ttympx;	/*     "          "     channel */
	char	ui_comm[ MAXCOMLEN+1 ];		/* (truncated) program name */

	/* memory usage info */
	unsigned long	ui_drss;	/* data resident set size */
	unsigned long	ui_trss;	/* text resident set size */
	unsigned long	ui_dvm;		/* data virtual memory size */
	unsigned long	ui_prm;		/* percent real memory usage */
};

struct uicredinfo
{
	struct userinfo  uici_ui;	/* userinfo structure */
	struct ucred     uici_cred;	/* cred structure */
};


#define SSLEEP		1
#define SRUN		3

#define SNOWAIT         0
#define SWEVENT         1
#define SWLOCK          2
#define SWTIMER         3
#define SWCPU           4
#define SWPGIN          5
#define SWPGOUT         6
#define SWPLOCK         7
#define SWFREEF         8
#define SWMEM           9

struct procsinfo
{
	/* identification/authentication */
	unsigned long	pi_pid;		/* process ID */
	unsigned long	pi_ppid;	/* parent process ID */
	unsigned long	pi_sid;		/* session identifier */
	unsigned long	pi_pgrp;	/* process group */
	unsigned long	pi_uid;		/* real user ID */
	unsigned long	pi_suid;	/* saved user ID */

	/* scheduler information */
	unsigned long	pi_nice;	/* nice for priority */
	unsigned long	pi_state;	/* process state -- from proc.h */
	unsigned long	pi_flags;	/* process flags -- from proc.h */
	unsigned long	pi_thcount;	/* thread count */

	/* memory */
	unsigned long	pi_adspace;	/* process address space */
	unsigned long	pi_majflt;	/* i/o page faults */
	unsigned long	pi_minflt;	/* non i/o page faults */
	unsigned long	pi_repage;	/* repaging count */
	unsigned long	pi_size;	/* size of image (pages) */

	/* valid when the process is a zombie only */
	unsigned long	pi_utime;	/* this process user time */
	unsigned long	pi_stime;	/* this process system time */

	/* credentials information */
	struct ucred	pi_cred;

	/* accounting and profiling data */
	unsigned long	pi_start;	/* time at which process began */
	struct rusage	pi_ru;		/* this process' rusage info */
	struct rusage	pi_cru;		/* children's rusage info */

	/* resource limits info */
	struct rlimit	pi_rlimit[RLIM_NLIMITS]; /* resource limits */

	/* size of text */
	unsigned long	pi_tsize;	/* size of text */

	/* controlling tty info */
	unsigned long	pi_ttyp;	/* has a controlling terminal */
	unsigned long	pi_ttyd;	/* controlling terminal */
	unsigned long	pi_ttympx;	/*	"	  "	channel */
        unsigned long   pi_dsize;       /* current break value */
        unsigned long   pi_sdsize;      /* data size from shared library */
	char		pi_comm[MAXCOMLEN+1]; /* (truncated) program name */

	/* memory usage info */
	unsigned long	pi_drss;	/* data resident set size */
	unsigned long	pi_trss;	/* text resident set size */
	unsigned long	pi_dvm;		/* data virtual memory size */
	unsigned long	pi_prm;		/* percent real memory usage */

	/* signal management */
	unsigned long	pi_signal[NSIG];/* disposition of sigs */
	char		pi_sigflags[NSIG];/* sig action flags */
	sigset_t	pi_sig;		/* pending sigs */

	/* file management */
        unsigned long   pi_cdir;        /* current directory of process */
        unsigned long   pi_rdir;        /* root directory of process */
	unsigned long	pi_maxofile;	/* maximum u_ofile index in use */
	unsigned long	pi_resvd[5];	/* reserve space for future use */
};

struct fdsinfo
{
	struct {
		struct file	*fp;
		unsigned short	flags;
		unsigned short	count;
	} pi_ufd[OPEN_MAX];		/* User's file descriptor table */
};

struct thrdsinfo
{
	/* identification */
	unsigned long	ti_tid;		/* thread ID */
	unsigned long	ti_pid;		/* process ID */

	/* scheduler information */
	unsigned long	ti_policy;	/* scheduling policy */
	unsigned long	ti_pri;		/* priority */
	unsigned long	ti_state;	/* thread state -- from thread.h */
	unsigned long	ti_flag;	/* thread flags -- from thread.h */
	unsigned long	ti_scount;	/* suspend count */
	unsigned long	ti_wtype;	/* type of thread wait */
	unsigned long	ti_wchan;	/* wait channel */
	unsigned long	ti_cpu;		/* processor usage */
	unsigned long	ti_cpuid;	/* processor on which I'm bound */

	/* signal management */
	sigset_t	ti_sigmask;	/* sigs to be blocked */
	sigset_t	ti_sig;		/* pending sigs */
	unsigned long	ti_code;	/* iar of exception */
	struct sigcontext *ti_scp;	/* sigctx location in user space */
	char		ti_cursig;	/* current/last signal taken */
	char		ti_pad1[3];	/* pad to word boundary */

	/* miscellaneous */
	unsigned long	ti_ticks;	/* # of ticks since dispatched */
	unsigned long	ti_dispct;	/* number of dispatches */
	unsigned long	ti_fpuct;	/* number of FP unavail ints. */
	unsigned long	ti_ustk;	/* user stack pointer */
	unsigned long	ti_resvd[5];	/* reserve space for future use */
};

#endif	/* _H_PROCINFO */
