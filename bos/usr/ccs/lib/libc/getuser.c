static char sccsid[] = "@(#)29	1.5  src/bos/usr/ccs/lib/libc/getuser.c, libcproc, bos411, 9428A410j 6/22/94 18:04:14";
/*
 *   COMPONENT_NAME: LIBCPROC
 *
 *   FUNCTIONS:  getuser
 *
 *   ORIGINS: 27
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/resource.h>
#include <procinfo.h>
#include <sys/limits.h>
#include <uinfo.h>
#include <sys/seg.h>
#include <sys/signal.h>
#include <a.out.h>
#include <sys/mstsave.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/cred.h>
#include <sys/sem.h>
#include <sys/xmem.h>
#include <sys/uprintf.h>


/* LOCAL VARIABLES */

/*
 * command limits for exec files for the 325 user block:
 *	SHSIZE is the limit for the #! line in an interpreted file
 *	MAXCOMLEN is the size limit for the base name of the exec file
 *		that is saved in the u-block.
 */
#define SHSIZE		32		/* short form exec size */
#define	MAXCOMLEN	32		/* <= MAXNAMLEN, >= sizeof(ac_comm) */

#define    GETUSER_NONE       0    /* invalid argument                      */
#define    GETUSER_USERINFO   1    /* getuser() returns userinfo structure  */
#define    GETUSER_USER_V3    2    /* getuser() returns v3 user structure   */
#define    GETUSER_USER_V4    3    /* getuser() returns v4 user structure   */
#define    GETUSER_UICRED     4    /* getuser() returns user     structure  */
#define    NANOTOMICRO        1000 /* converts nanoseconds to microseconds  */

#define    UPDATE_RUSAGE( m, c )                                  \
                     (m)->ru_majflt = procinfo->pi_majflt;        \
                     (m)->ru_minflt = procinfo->pi_minflt;        \
                     (m)->ru_utime.tv_usec /= NANOTOMICRO;        \
                     (m)->ru_stime.tv_usec /= NANOTOMICRO;        \
                     (c)->ru_utime.tv_usec /= NANOTOMICRO;        \
                     (c)->ru_stime.tv_usec /= NANOTOMICRO

/*
 * NAME: getuser()
 *
 * FUNCTION: Return public information about processes found via getprocs()
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * NOTES: The table is not guaranteed to be consistent
 *
 * RETURN VALUES:
 *     0    process information retrieved
 *    -1    failed, errno indicates cause of failure
 */
getuser(struct procinfo *procinfo, int plen, void *user, int ulen)
/* struct  procinfo *procinfo;             ptr to array of procinfo struct
 * int     plen;                           size of expected procinfo struct
 * void   *user;                           ptr to userinfo struct OR user struct
 * int     ulen;                           size of expected userinfo struct
 */
{
   	struct procsinfo procsinfobuf; 	/* temp storage, proc entry */
   	register struct procsinfo *pptr=&procsinfobuf; /* ptr to proc entry */
   	struct fdsinfo fdsinfobuf; 	/* temp storage, fds entries */
   	register struct userinfo *uptr=user; /* temporary userinfo struct ptr*/
   	char type;	   		/* identifies requested struct */
	int i;
	ulong pidbuf;			/* buffer for requested proc id */
	struct user *u410ptr;		/* temp pointer for v4 user struct */
	struct user325 {

		/* swappable process context */
		struct mstsave	u_save; 	/* machine state save area */
		ulong 		u_procp;	/* PID from procsinfo struct */
		/* above field used to be "struct proc	*u_procp;" */

		/* system call state */
		short		u_errcnt;	/* syscall error count */
		char		u_error;	/* return error code */
		char		u_pad;
		label_t		*u_kjmpbuf;/* top of kernel exception longjmp*/
					   /*   buffer stack, or NULL */
		long		u_iorb;	   /* I/O request block - not used */

		/* signal management */
		struct sigcontext *u_sigctx;	/* signal context structure */
		sigset_t	u_oldmask;	/* mask from before sigpause */
		int		u_code;      /* ``code'' for syscall handler */
		char		*u_sigsp;	/* special signal stack */
		void		(*u_signal[NSIG])(int);/* disposition of sigs*/
		sigset_t	u_sigmask[NSIG];  /* sig's to be blocked */
		char		u_sigflags[NSIG];	/* sig action flags */

		/* user-mode address space mapping */
		adspace_t	u_adspace;	/* user-mode address space */
		struct segstate u_segst[NSEGS];/* info on use of each segment*/

		/* auditing stuff */
		int	u_auditstatus;		/* auditing RESUME or SUSPEND*/
		struct auddata {		/* audit relevant data */
			ushort	svcnum;	/* name index from audit_klookup */
			ushort	argcnt;	/* number of arguments stored */
			int	args[10]; /* Parameters for this call */
			char	*audbuf; /* buffer for misc audit record */
			int	bufsiz;	/* allocated size of pathname buffer */
			int	buflen;	/* actual length of pathname(s) */
			ushort	bufcnt;	/* number of pathnames stored */
			ulong	status;	/* audit status bitmap */
		} u_audsvc;

	/* CHANGES FROM HERE ON DO NOT AFFECT ASSEMBLER CODE (ml/user.m4) */

		long		u_pad1[1];	/* spare word */

		/* address map (mmap) */
		char		*u_map;

		/* current exec file information */
		union execunion {			/* file header union */
			struct xcoffhdr u_xcoffhdr; /* xcoff header */
			char u_exshell[SHSIZE]; /* #! + name of interpreter */
		} u_exh;
 		char		u_comm[MAXCOMLEN+1]; /* basename of exec file*/
		short		u_lock; 	/* process/text locking flags*/
		char		u_sep;	/* flag for I and D separation */
		char		u_intflg;	/* catch intr from sys */

		/* user identification and authorization */
	
		struct ucred	*u_cred; /* user credentials (uid, gid, etc) */
	
		uinfo_t 	u_uinfo;	/* usrinfo() buffer */
		int	u_compatibility; /* compatibility/user mode bit masks*/

		/* per-process timer management */
        	struct  trb     *t_trb[NTIMERS];/* array of per-process timers  */

		struct sem_undo	*u_semundo; /* semaphore undo struct pointer */

		/* accounting and profiling data */
		time_t		u_start;
		time_t		u_ticks;
		struct profdata {		/* profile arguments */
	    		short	*pr_base;	/* buffer base */
	    		unsigned	pr_size;	/* buffer size */
	    		unsigned	pr_off; 	/* pc offset */
	    		unsigned	pr_scale;	/* pc scaling */
		} u_prof;
		short	u_acflag;		/* accounting flag */
		struct rusage	u_ru;	/* this process resource usage value */
		struct rusage	u_cru;	/* accumulated children's resources */

		/* resource limits and counters */
		unsigned	u_tsize;		/* text size (bytes) */
		struct rlimit	u_rlimit[RLIM_NLIMITS];	/* resource limits */
		long		u_ioch;		/* I/O character count */

		/* controlling tty info */
  		pid_t		*u_ttysid; /* ptr to session leader id in tty*/
		pid_t		u_ttyp; /* controlling tty pgrp field*/
		/* above field used to be "pid_t	*u_ttyp;" */
		dev_t		u_ttyd; 	/* controlling tty dev */
		off_t		u_ttympx; /* mpx value for controlling tty */
		unsigned	*u_ttys; /* pointer to t_state in tty struct */
		int		u_ttyid;	/* tty id */
		int		(*u_ttyf)(); /* tty query function pointer */

		void		*u_loginfo;	/* loginfo pointer */
		struct upfbuf	*u_message;	/* uprintf buffer pointer */
		int		u_dsize;	/* current break value */
		int		u_sdsize;	/* data size from shared lib */
		struct pinprof *u_pprof;	/* pinned user profiling buffer
					     struct pinprof defined in mon.h */
		struct xmem	*u_dp;	/* memory descriptor for pinned prof
					buffer */

		/* file system state */
		struct vnode	*u_cdir; /* current directory of process */
		struct vnode	*u_rdir;	/* root directory of process */
		ushort		u_vfs;
		struct vnode	*u_pdir;	/* vnode of parent of dirp */
		pid_t		u_epid;		/* proc id for file locks */
		int		u_sysid;	/* system id for file locks */
		char		*u_lastname;	/* lastname component */
		short		u_cmask;	/* mask for file creation */
		long		u_ioctlrv; /* return value for PSE ioctl's */
		long		_u_fsspace[7];	/* more spare room */
		long		u_loader[64];	/* loader area */
	/* this part of the  u-block is pageable, and should go at the end */
		short		u_maxofile; /* maximum u_ofile index in use */
		struct ufd {
			struct file *	fp;
			int		flags;
		} u_ufd[OPEN_MAX];	/* User's file descriptor table */
	} *u325ptr; /* temporary 325 user struct ptr*/


   	type          = (ulen == sizeof(struct userinfo)   ? GETUSER_USERINFO :
        	         ulen == sizeof(struct user325)    ? GETUSER_USER_V3 :
        	         ulen == sizeof(struct user)       ? GETUSER_USER_V4 :
			 ulen == sizeof(struct uicredinfo) ? GETUSER_UICRED :
                	                                     GETUSER_NONE ) ;

   	/* check parameters for valid struct size: plen, ulen */

   	if ( plen != sizeof(struct procinfo) || type == GETUSER_NONE )
   	{
       		errno = EINVAL;
       		return(-1);
   	}
	
	/* get the data using the getprocs system call */

	pidbuf = procinfo->pi_pid;	/* set index to proc of interest */
	if (getprocs(pptr, sizeof(struct procsinfo), &fdsinfobuf,
			sizeof(struct fdsinfo), &pidbuf, 1) == -1)
		return (-1);

	/* check for valid pid */

	if ((pptr->pi_pid != procinfo->pi_pid) || (pptr->pi_state == SZOMB) ||
	    (pptr->pi_state == SNONE) || (pptr->pi_flags & SEXIT))
	{
		errno = EBADF;
		return(-1);
	}

	/* copy the data from the procsinfo buffer to the user's buffer */

	switch(type){
	case GETUSER_USER_V3 :
		u325ptr = (struct user325 *) user;
		u325ptr->u_procp = pptr->pi_pid; /* fake procptr using PID */
		for (i=0; i<NSIG; i++)
		{
			u325ptr->u_signal[i] = (void *)pptr->pi_signal[i];
			u325ptr->u_sigflags[i] = pptr->pi_sigflags[i];
		}
		for(i = 0; i < MAXCOMLEN+1; i++)
        		u325ptr->u_comm[i] = pptr->pi_comm[i];
		u325ptr->u_start = pptr->pi_start;
        	u325ptr->u_ru     = pptr->pi_ru;
        	u325ptr->u_cru    = pptr->pi_cru;
	       	u325ptr->u_tsize  = pptr->pi_tsize;
		for(i = 0; i < RLIM_NLIMITS; i++)
        		u325ptr->u_rlimit[i] = pptr->pi_rlimit[i];
        	u325ptr->u_ttyp   = pptr->pi_ttyp;
        	u325ptr->u_ttyd   = pptr->pi_ttyd;
        	u325ptr->u_ttympx = pptr->pi_ttympx;
        	u325ptr->u_dsize = pptr->pi_dsize;
        	u325ptr->u_sdsize = pptr->pi_sdsize;
        	u325ptr->u_cdir = pptr->pi_cdir;
        	u325ptr->u_rdir = pptr->pi_rdir;
        	u325ptr->u_maxofile = pptr->pi_maxofile;
		bcopy(&fdsinfobuf, &u325ptr->u_ufd[0],
				sizeof(struct fdsinfo));
               	UPDATE_RUSAGE(&u325ptr->u_ru, &u325ptr->u_cru);
		break;
	case GETUSER_USER_V4 :
		u410ptr = (struct user *)user;
		u410ptr->U_procp = pptr->pi_pid; /* fake procptr using PID */
		for (i=0; i<NSIG; i++)
		{
			u410ptr->U_signal[i] = (void *)pptr->pi_signal[i];
			u410ptr->U_sigflags[i] = pptr->pi_sigflags[i];
		}
		for(i = 0; i < MAXCOMLEN+1; i++)
        		u410ptr->U_comm[i] = pptr->pi_comm[i];
		u410ptr->U_start  = pptr->pi_start;
        	u410ptr->U_ru     = pptr->pi_ru;
        	u410ptr->U_cru    = pptr->pi_cru;
	       	u410ptr->U_tsize  = pptr->pi_tsize;
		for(i = 0; i < RLIM_NLIMITS; i++)
        		u410ptr->U_rlimit[i] = pptr->pi_rlimit[i];
        	u410ptr->U_ttyp   = pptr->pi_ttyp;
        	u410ptr->U_ttyd   = pptr->pi_ttyd;
        	u410ptr->U_ttympx = pptr->pi_ttympx;
        	u410ptr->U_dsize  = pptr->pi_dsize;
        	u410ptr->U_sdsize = pptr->pi_sdsize;
        	u410ptr->U_cdir = pptr->pi_cdir;
        	u410ptr->U_rdir = pptr->pi_rdir;
        	u410ptr->U_maxofile = pptr->pi_maxofile;
		bcopy(&fdsinfobuf, &u410ptr->U_ufd[0],
				sizeof(struct fdsinfo));
               	UPDATE_RUSAGE(&u410ptr->U_ru, &u410ptr->U_cru);
		break;
	case GETUSER_UICRED :
		((struct uicredinfo *)uptr)->uici_cred = pptr->pi_cred;
		/* A uicredinfo also includes a userinfo, so fall thru */
	case GETUSER_USERINFO :
   		uptr->ui_luid 	= (ulong)pptr->pi_cred.cr_luid;
   		uptr->ui_uid  	= (ulong)pptr->pi_cred.cr_uid;
   		uptr->ui_gid  	= (ulong)pptr->pi_cred.cr_gid;

        	uptr->ui_start  = pptr->pi_start;
        	uptr->ui_ru     = pptr->pi_ru;
        	uptr->ui_cru    = pptr->pi_cru;

		for(i = 0; i < RLIM_NLIMITS; i++)
        		uptr->ui_rlimit[i] = pptr->pi_rlimit[i];

	       	uptr->ui_tsize  = pptr->pi_tsize;

        	uptr->ui_ttyp   = pptr->pi_ttyp;
        	uptr->ui_ttyd   = pptr->pi_ttyd;
        	uptr->ui_ttympx = pptr->pi_ttympx;
		for(i = 0; i < MAXCOMLEN+1; i++)
        		uptr->ui_comm[i] = pptr->pi_comm[i];

   		uptr->ui_drss 	= pptr->pi_drss;
   		uptr->ui_trss 	= pptr->pi_trss;
   		uptr->ui_dvm 	= pptr->pi_dvm;
   		uptr->ui_prm 	= pptr->pi_prm;
               	UPDATE_RUSAGE(&uptr->ui_ru, &uptr->ui_cru);
		break;
	}
   	return(0);
}
