/* @(#)93	1.51.3.25  src/bos/kernel/sys/user.h, sysproc, bos41J, 9507A 2/8/95 14:22:07 */
/*
 *   COMPONENT_NAME: SYSPROC
 * 
 *   FUNCTIONS:
 *
 *   ORIGINS: 26, 27, 3, 9, 83
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


#ifndef _H_USER
#define _H_USER

/*
 *			User Structure (u-block)
 *
 * FUNCTION:
 *	The user structure is also called the user area or u-block.
 *	There is one allocated per process, including kernel processes.
 *	It is protected to disallow any access to it by user-mode code.
 *
 *	The u-block contains information about the process that
 *	need not be in memory when the process is swapped out.
 *	It is pinned when the process is swapped into memory, and
 *	unpinned out when the process is swapped out.
 *
 * NOTES:
 *	The size of the u-block directly affects the addresses of
 *	`u', `errno', and `environ' in sys/ipl.exp; the addresses
 *	of `u_lo', `k_stk_lo', and `u_stk_lo' in ml/user.m4; and the
 *	#define's in include/sys/pseg.h.  If the size of the u-block
 *	is changed, these values should be checked to see if they need
 *	to be updated.  Only the first few sections are mapped in
 *	ml/user.m4, changes to the beginning of the user structure
 *	must be refected there, too.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/limits.h>
#include <sys/dir.h>
#include <uinfo.h>
#include <sys/seg.h>
#include <sys/signal.h>
#include <a.out.h>
#include <sys/mstsave.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/cred.h>
#include <sys/sem.h>
#include <sys/xmem.h>
#include <sys/uprintf.h>
#include <sys/low.h>
#include <sys/pmzone.h>
#include <sys/uthread.h>
#include <sys/uio.h>

/*
 * command limits for exec files:
 *	SHSIZE is the limit for the #! line in an interpreted file
 *	MAXCOMLEN is the size limit for the base name of the exec file
 *		that is saved in the u-block.
 */
#define SHSIZE		32		/* short form exec size */
#define	MAXCOMLEN	32		/* <= MAXNAMLEN, >= sizeof(ac_comm) */

struct user {

	/* swappable process context */
	struct pm_heap	U_uthread_cb;	/* uthread table control block */
        struct pm_heap  U_cancel_cb;    /* cancelation control block */
	struct proc	*U_procp;	/* pointer to proc structure */
	Simple_lock	U_handy_lock;	/* self preservation lock */

	/* signal management */
	void		(*U_signal[NSIG])(int);	/* disposition of sigs */
	sigset_t	U_sigmask[NSIG];	/* sig's to be blocked */
	char		U_sigflags[NSIG];	/* sig action flags */

	/* user-mode address space mapping */
	adspace_t	U_adspace;	/* user-mode address space */
	struct segstate U_segst[NSEGS]; /* info on use of each segment */
	struct vmmlock {
		int	lock_word;	/* per-process U_adspace lock */
		struct thread	*vmm_lock_wait;	/* wait list for lock waiters */
	} U_adspace_lock;

#define	U_lock_word	U_adspace_lock.lock_word
#define U_vmm_lock_wait	U_adspace_lock.vmm_lock_wait

	/* auditing stuff */
	int		U_auditstatus;	/* auditing RESUME or SUSPEND */

	/* CHANGES FROM HERE ON DO NOT AFFECT ASSEMBLER CODE (ml/user.m4) */

	/* address map (mmap) */
	char		*U_map;

	/* current exec file information */
 	char		U_comm[MAXCOMLEN+1]; /* basename of exec file */
	char		*U_tstart;	/* start of text */
	short		U_lock; 	/* process/text locking flags */
	short		U_pad1;

	/* user identification and authorization */
	Simple_lock	U_cr_lock;	/* credentials lock */
        struct ucred    *U_cred;        /* user credentials (uid, gid, etc) */

#define GETEUID()	U.U_cred->cr_uid
#define GETRUID()	U.U_cred->cr_ruid

#define U_uid		U_cred->cr_uid
#define U_ruid		U_cred->cr_ruid
#define U_suid		U_cred->cr_suid
#define U_luid		U_cred->cr_luid
#define U_acctid	U_cred->cr_acctid
#define U_gid		U_cred->cr_gid
#define U_rgid		U_cred->cr_rgid
#define U_sgid		U_cred->cr_sgid
#define U_groups	U_cred->cr_groups
#define U_epriv		U_cred->cr_epriv
#define U_ipriv		U_cred->cr_ipriv
#define U_bpriv		U_cred->cr_bpriv
#define U_mpriv		U_cred->cr_mpriv

	uinfo_t 	U_uinfo;	/* usrinfo() buffer */
	int		U_compatibility; /* compatibility/user mode bit masks */

	/* defines for u.u_compatibility bit mask */
#define PROC_RAWDIR	1	/* read directories raw mode, no */
				/* translation to dirents 	 */

	struct sem_undo	*U_semundo;	/* semaphore undo struct pointer */

	/* accounting and profiling data */
	time_t		U_start;
	time_t		U_ticks;
	struct profdata {		/* profile arguments */
	    short	*pr_base;	/* buffer base */
	    unsigned	pr_size;	/* buffer size */
	    unsigned	pr_off; 	/* pc offset */
	    unsigned	pr_scale;	/* pc scaling */
	} U_prof;
	short	U_acflag;		/* accounting flag */
	struct rusage	U_ru;		/* this process resource usage value */
	struct rusage	U_cru;		/* accumulated children's resources */

#define U_utime		U_ru.ru_utime.tv_sec	/* this process user time */
#define U_stime		U_ru.ru_stime.tv_sec	/* this process system time */
#define U_cutime	U_cru.ru_utime.tv_sec	/* sum of children's utimes */
#define U_cstime	U_cru.ru_stime.tv_sec	/* sum of children's stimes */

	/* resource limits and counters */
	unsigned	U_tsize;		/* text size (bytes) */
	struct rlimit	U_rlimit[RLIM_NLIMITS];	/* resource limits */

#define U_smax          U_rlimit[RLIMIT_STACK].rlim_cur /* soft limit max stack */
/* since vmm does not maintain current stack size we use the max */
#define U_ssize         U_rlimit[RLIMIT_STACK].rlim_cur /* current stack size */
#define U_dmax          U_rlimit[RLIMIT_DATA].rlim_cur  /* soft limit max data */
#define U_limit         U_rlimit[RLIMIT_FSIZE].rlim_cur /* max file size */
#define U_minflt        U_ru.ru_minflt          /* minimum page fault count */
#define U_majflt        U_ru.ru_majflt          /* major page fault count */
#define U_ior		U_ru.ru_inblock		/* block read count */
#define U_iow           U_ru.ru_oublock         /* block write count */

	long		U_ioch; 		/* I/O character count */

	/* timers */
	Simple_lock	U_timer_lock;
	struct trb	*U_timer[NTIMERS];	/* per process timers */

	/* controlling tty info */
  	pid_t		*U_ttysid;	/* ptr to session leader id in tty */
	pid_t		*U_ttyp;	/* ptr to controlling tty pgrp field */
	dev_t		U_ttyd; 	/* controlling tty dev */
	off_t		U_ttympx;	/* mpx value for controlling tty */
	unsigned	*U_ttys;	/* pointer to t_state in tty struct */
	int		U_ttyid;	/* tty id */
	int		(*U_ttyf)();	/* tty query function pointer */

	struct upfbuf	*U_message;	/* uprintf buffer pointer */
	int		U_dsize;	/* current break value */
	int		U_sdsize;	/* data size from shared lib */
	struct pinprof *U_pprof;	/* pinned user profiling buffer - struct
					   pinprof defined in mon.h */
	struct xmem	*U_dp;		/* memory descriptor for pinned prof
					   buffer */

	/* file system state */
	struct vnode	*U_cdir;	/* current directory of process */
	struct vnode	*U_rdir;	/* root directory of process */
	short		U_cmask;	/* mask for file creation */
	Simple_lock	U_fso_lock;	/* other file system fields lock */
	long		U_lockflag;	/* process has done file locks */
	long		U_fdevent;	/* fd lock event list */

#ifndef _LONG_LONG
        unsigned long   U_irss[2];      /* accumulator for memory integral */
#else
        long long       U_irss;         /* accumulator for memory integral */
#endif
	struct pinu_block *U_pinu_block;/* list of control structs for pinu */
	tid_t		U_ulocks;	/* event list for user locks */

/*	long		U_spare[1];	!!! missing line !!!		      */

	/* cache line boundary */
	long		U_loader[64];	/* loader area */
	
	short		U_maxofile;	/* maximum u_ofile index in use */
	Simple_lock	U_fd_lock;	/* file descriptor lock */

	/* this part of user structure is pageable, and should go at the end */
	struct ufd {
		struct file *	fp;
		unsigned short	flags;
		unsigned short	count;
	} U_ufd[OPEN_MAX];		/* User's file descriptor table */
};

#define U_ofile(x)	U_ufd[(x)].fp
#define U_pofile(x)	U_ufd[(x)].flags

#ifdef	_KERNEL
#define UBLOCK_OFFSET	0x400		/* see ipl.exp */
#define UBPAD_OFFSET	((UBLOCK_OFFSET +		\
			  sizeof(struct uthread) +	\
			  sizeof(struct user)		\
			 ) & (PAGESIZE-1))
#define UBPAD		(PAGESIZE - UBPAD_OFFSET)

struct ublock {
	struct uthread	ub_uthr0;		/* default uthread structure  */
	struct user	ub_user;		/* user structure	      */
	char		ub_pad[UBPAD];		/* pad to page boundary       */
	struct uthread	ub_uthr[MAXTHREADS];	/* additional uthread struct  */
};

extern struct ublock	__ublock;	/* fixed place in every address space */

#define uthr0		(__ublock.ub_uthr0)	/* constant */
#define U		(__ublock.ub_user)	/* constant */

/* compatibility defines */
#ifndef u
#define u		(curthread->t_uaddress)

#define u_save		uthreadp->ut_save
#define u_error		uthreadp->ut_error
#define u_flags		uthreadp->ut_flags
#define u_oldmask	uthreadp->ut_oldmask
#define u_code		uthreadp->ut_code
#define u_sigsp		uthreadp->ut_sigsp
#define u_fstid		uthreadp->ut_fstid
#define u_ioctlrv	uthreadp->ut_ioctlrv
#define u_th_timer	uthreadp->ut_timer
#define u_loginfo	uthreadp->ut_loginfo

#define u_procp		userp->U_procp
#define u_signal	userp->U_signal
#define u_sigmask	userp->U_sigmask
#define u_sigflags	userp->U_sigflags
#define u_adspace	userp->U_adspace
#define u_segst		userp->U_segst
#define u_auditstatus	userp->U_auditstatus
#define u_map		userp->U_map
#define u_comm		userp->U_comm
#define u_lock		userp->U_lock
#define u_cred		userp->U_cred
#define u_uid		userp->U_uid
#define u_ruid		userp->U_ruid
#define u_suid		userp->U_suid
#define u_luid		userp->U_luid
#define u_acctid	userp->U_acctid
#define u_gid		userp->U_gid
#define u_rgid		userp->U_rgid
#define u_sgid		userp->U_sgid
#define u_groups	userp->U_groups
#define u_epriv		userp->U_epriv
#define u_ipriv		userp->U_ipriv
#define u_bpriv		userp->U_bpriv
#define u_mpriv		userp->U_mpriv
#define u_uinfo		userp->U_uinfo
#define u_compatibility	userp->U_compatibility
#define u_semundo	userp->U_semundo
#define u_start		userp->U_start
#define u_ticks		userp->U_ticks
#define u_prof		userp->U_prof
#define u_acflag	userp->U_acflag
#define u_ru		userp->U_ru
#define u_cru		userp->U_cru
#define u_utime         userp->U_utime
#define u_stime		userp->U_stime
#define u_cutime	userp->U_cutime
#define u_cstime	userp->U_cstime
#define u_tsize		userp->U_tsize
#define u_rlimit	userp->U_rlimit
#define u_timer		userp->U_timer
#define u_smax		userp->U_smax
#define u_ssize		userp->U_ssize
#define u_dmax		userp->U_dmax
#define u_limit		userp->U_limit
#define u_minflt	userp->U_minflt
#define u_majflt	userp->U_majflt
#define u_ior		userp->U_ior
#define u_iow		userp->U_iow
#define u_ioch		userp->U_ioch
#define u_ttysid	userp->U_ttysid
#define u_ttyp		userp->U_ttyp
#define u_ttyd		userp->U_ttyd
#define u_ttympx	userp->U_ttympx
#define u_ttys		userp->U_ttys
#define u_ttyid		userp->U_ttyid
#define u_ttyf		userp->U_ttyf
#define u_message	userp->U_message
#define u_dsize		userp->U_dsize
#define u_sdsize	userp->U_sdsize
#define u_pprof		userp->U_pprof
#define u_dp		userp->U_dp
#define u_cdir		userp->U_cdir
#define u_rdir		userp->U_rdir
#define u_cmask		userp->U_cmask
#define u_fd_lock	userp->U_fd_lock
#define u_fso_lock	userp->U_fso_lock
#define u_lockflag	userp->U_lockflag
#define u_fdevent	userp->U_fdevent
#define u_irss		userp->U_irss
#define u_loader	userp->U_loader
#define u_maxofile	userp->U_maxofile
#define u_ufd		userp->U_ufd
#define u_ofile(x)	userp->U_ofile(x)
#define u_pofile(x)	userp->U_pofile(x)
#endif	/* !u */

#define TIMER_LOCK(p, ipri)						\
	{								\
		if (p->p_active == 1)					\
			ipri = i_disable(INTTIMER);			\
		else							\
			ipri = disable_lock(INTTIMER, &U.U_timer_lock);	\
	}

#define TIMER_UNLOCK(p, ipri)						\
	{								\
		if (p->p_active == 1)					\
			i_enable(ipri);					\
		else							\
			unlock_enable(ipri, &U.U_timer_lock);		\
	}

#endif	/* _KERNEL */

/* ioflag values: Read/Write, User/Kernel, Ins/Data */
#define U_WUD 0
#define U_RUD 1
#define U_WKD 2
#define U_RKD 3
#define U_WUI 4
#define U_RUI 5

#define EXCLOSE 01

#define UF_EXCLOSE	0x1
#define UF_MAPPED	0x2
#define UF_FDLOCK	0x4
#define UF_AUD_READ	0x8
#define UF_AUD_WRITE	0x10
#define UF_FSHMAT	0x20
#define UF_CLOSING	0x40
#define UF_ALLOCATED	0x80

#endif /* _H_USER */
