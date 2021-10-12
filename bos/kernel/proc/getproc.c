static char sccsid[] = "@(#)42	1.19.1.23  src/bos/kernel/proc/getproc.c, sysproc, bos41J, 9520A_all 5/12/95 15:30:48";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: getargs
 *		getevars
 *		getprocs
 *		getthrds
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <procinfo.h>
#include <execargs.h>
#include <sys/low.h>
#include <sys/syspest.h>
#include <sys/pseg.h>
#include <sys/reg.h>
#include <sys/vmker.h>
#include <sys/malloc.h>
#include "ld_data.h"

/* GLOBAL VARIABLES */
extern int	copyin();	   /* copies from user to kernel	    */	
extern int	copyout();	   /* copies from kernel to user	    */	
extern int	subyte();	   /* set byte in user space with spec value*/
extern caddr_t	vm_att();	   /* attaches to segment		    */
extern void	vm_det();	   /* deattaches from to segment	    */
extern int	xmattach();	   /* fills out cross memory descriptor	    */
extern int	xmemin();	   /* copies data from one seg to another   */
extern int	xmdetach();	   /* decrements reference count to segment */
extern char 	**environ;	   /* exported environment variables ptr    */

/* LOCAL VARIABLES */
int		bd_rusage(struct user *uptr);
int		bd_psusage(struct user *uptr);

/*
 * NAME: getprocs()
 *
 * FUNCTION: Retrieves an image of the process table
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * NOTES: The table is not guaranteed to be consistent, though individual proc
 *        entries are consistent.  Uses max_proc as the high water mark.
 *
 * RETURN VALUES:
 *	number	active processes retrieved
 *      -1	failed, errno indicates cause of failure
 */
getprocs(struct procsinfo *procsinfo, int sizproc, struct fdsinfo *fdsinfo,
         int sizfd, pid_t *index, int count)
/* struct procsinfo *procsinfo;	pointer to array of procsinfo struct */
/* int sizproc;			size of expected procsinfo structure */
/* struct fdsinfo *fdsinfo;	pointer to array of fdsinfo struct   */
/* int sizfd;			size of a single fdsinfo structure   */
/* pid_t *index;		pid of the first requested entry     */
/* int count;			number of entries requested	     */
{

	register struct proc *p;	/* pointer to kernel proc table	*/
	struct procsinfo pinfo;		/* temp storage, proc entries	*/
	struct fdsinfo finfo;
	register int num_act = 0;	/* counter of active processes	*/
	unsigned long tsz;		/* virtual size of text segment */ 
	unsigned long dsz;		/* virtual size of data segment */ 
	unsigned long tvm;		/* total size of text           */
	unsigned long dvm;		/* total size of data           */
        int machine_frames;		/* total num of page frames on machine*/
        int proc_frames;   		/* total page frames used by process  */
	struct user *Up;		/* pointer to the uarea 	*/
	register struct proc *first;	/* where to start in proc table */
	pid_t pid;			/* temporary pid for translation*/
	struct xmem dp;			/* ptr to xmem descriptor	*/
	int xm;				/* xmem result			*/
	char *errorp;			/* current u.u_error		*/

	assert(csa->prev == NULL);	/* make sure this is a process	*/
	errorp = &curthread->t_uthreadp->ut_error;

	if (((procsinfo != NULL) && (sizproc != sizeof(struct procsinfo))) ||
	    ((fdsinfo   != NULL) && (sizfd   != sizeof(struct fdsinfo)))   ||
	    (count <= 0))
	{
		*errorp = EINVAL;
		return(-1);
	}

	if (copyin((caddr_t)index, (caddr_t)&pid, sizeof(pid_t)))
	{
		*errorp = EFAULT;
		return (-1);
	}

	/*
	 * Note: We don't use VALIDATE_PID because the pid may not refer to
	 * a valid entry and is only used as an index in the proc table.
	 */
	if (!MAYBE_PID(pid) || (PROCMASK(pid) >= NPROC))
	{
		*errorp = EINVAL;
		return(-1);
	}

	first = PROCPTR(pid);

	/* walk through the proc table till max_proc high water mark	*/
	for (p = first; p < max_proc; p++)
	{
		if ((p->p_stat != SNONE) && (++num_act <= count))
		{
		    if (procsinfo)
		    {
			bzero(&pinfo, sizproc);

			pinfo.pi_pid = p->p_pid;	/* process ID	*/
			pinfo.pi_ppid = p->p_ppid;	/* parent ID	*/
			pinfo.pi_sid = p->p_sid;	/* session ID	*/
			pinfo.pi_pgrp = p->p_pgrp;	/* process grp	*/
			pinfo.pi_uid = p->p_uid;	/* real user ID	*/
			pinfo.pi_suid = p->p_suid;	/* save user ID	*/

			pinfo.pi_nice = EXTRACT_NICE(p);
			pinfo.pi_state = p->p_stat;
			pinfo.pi_flags = p->p_flag | p->p_atomic | p->p_int;

			pinfo.pi_thcount = p->p_threadcount;

			pinfo.pi_adspace = p->p_adspace;
			pinfo.pi_majflt = p->p_majflt;
			pinfo.pi_minflt = p->p_minflt;
			pinfo.pi_repage = p->p_repage;
			pinfo.pi_size = p->p_size;

			if (p->p_stat == SZOMB)
			{
			  pinfo.pi_utime = p->xp_utime;
			  pinfo.pi_stime = p->xp_stime;
			  pinfo.pi_ru = p->p_ru;
			}

			pinfo.pi_sig =  p->p_sig;

			/* check to see if the process is active */
			if ((p->p_stat != SNONE) && (p->p_stat != SIDL) &&
							   !(p->p_flag & SEXIT))
			{
		          /*
                           * The process private segment in which the user
                           * area is located may disappear. We need to increment
                           * its use count. Therefore we
			   *	- get the proc_tbl_lock to hold the segment.
			   *	- get the p_lock to lockout vm_cleardata.
                           *    - vm_att to load the segment register (no check)
                           *    - xmattach to bump its use count.
			   *	- release the p_lock.
			   *	- release the proc_tbl_lock.
			   *	- copy whatever we need.
                           *    - xmdetach to decrement the use count.
                           *    - vm_det to free the segment register (no check)
                           */
			  Up = NULL;
			  xm = XMEM_FAIL;
			  simple_lock(&proc_tbl_lock);
			  if (p->p_adspace != NULLSEGVAL)
			  {
			  	simple_lock(&p->p_lock);
				Up = (struct user *)vm_att(p->p_adspace, &U);
				dp.aspace_id = XMEM_INVAL;
				xm = xmattach(Up, sizeof(int),&dp, SYS_ADSPACE);
			  	simple_unlock(&p->p_lock);
			  }
			  simple_unlock(&proc_tbl_lock);
			  if (xm == XMEM_SUCC)
			  {
			    bcopy(Up->U_cred, &pinfo.pi_cred,
							  sizeof(struct ucred));
			    pinfo.pi_start = Up->U_start;
			    pinfo.pi_cru = Up->U_cru;
			    bcopy(&Up->U_rlimit, &pinfo.pi_rlimit,
					    RLIM_NLIMITS*sizeof(struct rlimit));
			    pinfo.pi_tsize = Up->U_tsize;
			    pinfo.pi_ttyp = (ulong)Up->U_ttyp;
			    pinfo.pi_ttyd = (ulong)Up->U_ttyd;
			    pinfo.pi_ttympx = (ulong)Up->U_ttympx;
			    pinfo.pi_dsize = Up->U_dsize;
			    pinfo.pi_sdsize = Up->U_sdsize;
			    bcopy(&Up->U_comm, &pinfo.pi_comm,
						    (MAXCOMLEN+1)*sizeof(char));
			    bcopy(&Up->U_signal, &pinfo.pi_signal,
						    NSIG*sizeof(void (*)(int)));
			    bcopy(&Up->U_sigflags, &pinfo.pi_sigflags,
							     NSIG*sizeof(char));
			    pinfo.pi_maxofile = (ulong)Up->U_maxofile;
			    pinfo.pi_cdir = (ulong)Up->U_cdir;
			    pinfo.pi_rdir = (ulong)Up->U_rdir;
			    pinfo.pi_utime = (ulong)Up->U_ru.ru_utime.tv_sec;
			    pinfo.pi_stime = (ulong)Up->U_ru.ru_stime.tv_sec;
			    bcopy(&Up->U_ru,&pinfo.pi_ru,sizeof(struct rusage));

			    dsz = vms_rusage(Up->U_adspace.srval[PRIVSEG]);
			    tsz = vms_rusage(Up->U_adspace.srval[TEXTSEG]);
			    dvm = vms_psusage(Up->U_adspace.srval[PRIVSEG]);
			    tvm = vms_psusage(Up->U_adspace.srval[TEXTSEG]);
			    /* adjust for large data programs */
			    dsz += bd_rusage(Up);
			    dvm += bd_psusage(Up);
			    /* check for per-process library data segment */
			    if (Up->U_adspace.alloc &
			    	((unsigned)0x80000000 >> SHDATASEG))
			    {
				dsz +=
				  vms_rusage(Up->U_adspace.srval[SHDATASEG]);
				dvm +=
				  vms_psusage(Up->U_adspace.srval[SHDATASEG]);
			    }
			    /* Check for an overflow segment */
			    if (OVFL_EXISTS((struct loader_anchor *)
							Up->U_loader))
			    {
				dsz += vms_rusage(((struct loader_anchor *)
					(Up->U_loader))->la_ovfl_srval);
				dvm += vms_psusage(((struct loader_anchor *)
					(Up->U_loader))->la_ovfl_srval);
			    }
				
			    pinfo.pi_size = btoc((dvm + tvm)*PAGESIZE);
			    pinfo.pi_drss = dsz;
			    pinfo.pi_trss = tsz;
			    pinfo.pi_dvm = dvm;

   			    /* get percentage of active real memory */
   			    machine_frames = vmker.nrpages
						- vmker.badpages - vmker.numfrb;
			    proc_frames = dsz + tsz;
   			    pinfo.pi_prm = (machine_frames == 0) ? 0 :
                    	 	(proc_frames * 1000 / machine_frames + 5) / 10;

			    xmdetach(&dp);
			  }
			  if (Up)
			  	vm_det((caddr_t)Up);
			}

		    }

		    if (fdsinfo)
		    {
			bzero(&finfo, sizfd);

                        /* check to see if the process is active */
                        if ((p->p_stat != SNONE) && (p->p_stat != SIDL) &&
                                                           !(p->p_flag & SEXIT))
                        {
			  Up = NULL;
			  xm = XMEM_FAIL;
			  simple_lock(&proc_tbl_lock);
			  if (p->p_adspace != NULLSEGVAL)
			  {
			  	simple_lock(&p->p_lock);
				Up = (struct user *)vm_att(p->p_adspace, &U);
				dp.aspace_id = XMEM_INVAL;
				xm = xmattach(Up, sizeof(int),&dp, SYS_ADSPACE);
			  	simple_unlock(&p->p_lock);
			  }
			  simple_unlock(&proc_tbl_lock);
			  if (xm == XMEM_SUCC)
                          {
				bcopy(&Up->U_ufd, &finfo, sizfd);
				xmdetach(&dp);
			  }
			  if (Up)
			  	vm_det((caddr_t)Up);
			}
		    }

		    if (procsinfo)
		    {
			/* protected copy to user space, checks perm	*/
			if (copyout((caddr_t)&pinfo, (caddr_t)procsinfo++,
								       sizproc))
			{
	       			*errorp = EFAULT;
				return(-1);
			}
		    }

		    if (fdsinfo)
		    {
			/* protected copy to user space, checks perm	*/
			if (copyout((caddr_t)&finfo, (caddr_t)fdsinfo++, sizfd))
			{
	       			*errorp = EFAULT;
				return(-1);
			}
		    }

		}
		else
		{
			if (num_act > count) {
				/* we got 'count' entries, get out of loop */
				num_act--;
				break;
			}
		}
	}

	/*
	 * We cannot be sure that p still points to a valid entry,
	 * however, we only need to return a pid which shares the
	 * same slot, therefore we fake one.
	 */
	pid = (pid_t)((p - &proc[0])<<PGENSHIFT);
		
	if (copyout((caddr_t)&pid, (caddr_t)index, sizeof(pid_t)))
	{
		*errorp = EFAULT;
		return(-1);
	}

	return(num_act);	/* successful, return no. active entries */
}

/*
 * NAME: getthrds()
 *
 * FUNCTION: Retrieves an image of the threads table
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * NOTES: The table is not guaranteed to be consistent, though individual proc
 *        entries are consistent.  Uses max_proc as the high water mark.
 *
 * RETURN VALUES:
 *	number	active processes retrieved
 *      -1	failed, errno indicates cause of failure
 */
getthrds(pid_t pid, struct thrdsinfo *thrdsinfo, int sizthrd,
	 tid_t *index, int count)
/* pid_t pid;			 process to which the threads belong	*/
/* struct  thrdsinfo *thrdsinfo; pointer to array of thrdsinfo struct	*/
/* int	sizthrd;		 size of expected thrdsinfo structure	*/
/* tid_t *index;		 tid of the first requested entry 	*/
/* int count;			 number of entries requested 		*/
{

	register struct thread *t;	/* pointer to kernel thread table */
	struct thrdsinfo thinfo;	/* temp storage, thread entries	  */
	register int num_act = 0;	/* counter of active processes	  */
	register struct thread *first;	/* where to start in thread table */
	tid_t tid;			/* temporary tid for translation  */
	char *errorp;			/* current u.u_error		  */

	assert(csa->prev == NULL);	/* make sure this is a process */
	errorp = &curthread->t_uthreadp->ut_error;

	if (((thrdsinfo != NULL) && (sizthrd != sizeof(struct thrdsinfo))) ||
	    (count <= 0))
	{
		*errorp = EINVAL;
		return(-1);
	}

	if (copyin((caddr_t)index, (caddr_t)&tid, sizeof(tid_t)))
	{
		*errorp = EFAULT;
		return (-1);
	}

	/*
	 * Note: We don't use VALIDATE_TID because the tid may not refer to
	 * a valid entry and is only used as an index in the thread table.
	 */
	if (tid && (!MAYBE_TID(tid) || (THREADMASK(tid) >= NTHREAD)))
	{
		*errorp = EINVAL;
		return(-1);
    	}

	first = THREADPTR(tid);

	if ((pid != (pid_t)-1) && !(VALIDATE_PID(pid)))
	{
		*errorp = ESRCH;
		return(-1);
	}

	/* walk through the thread table till high water mark	*/
	for (t = first;  t < (struct thread *)v.ve_thread; t++)
	{
		if ((t->t_state != TSNONE) &&
		    ((pid == (pid_t)-1) || (t->t_procp->p_pid == pid)) &&
		    (++num_act <= count))
		{
		    if (thrdsinfo)
		    {
			thinfo.ti_tid = t->t_tid;
			thinfo.ti_pid = t->t_procp->p_pid;

			thinfo.ti_policy = t->t_policy;
			thinfo.ti_pri = t->t_pri;
			thinfo.ti_state = t->t_state;
			thinfo.ti_flag = t->t_flags | t->t_atomic;
			thinfo.ti_scount = t->t_suspend;
			thinfo.ti_wtype = t->t_wtype;
			thinfo.ti_wchan = t->t_wchan;
			thinfo.ti_cpu = t->t_cpu;
			thinfo.ti_cpuid = t->t_cpuid;

			thinfo.ti_sigmask = t->t_sigmask;
			thinfo.ti_sig = t->t_sig;
			thinfo.ti_cursig = t->t_cursig;

			thinfo.ti_code = 0; /* Always 0, not read */
			thinfo.ti_scp = t->t_scp;

			thinfo.ti_ticks = t->t_ticks;
			thinfo.ti_dispct = t->t_dispct;
			thinfo.ti_fpuct = t->t_fpuct;
			thinfo.ti_ustk = t->t_stackp;

			/* protected copy to user space, checks perm    */
			if (copyout((caddr_t)&thinfo, (caddr_t)thrdsinfo++,
								       sizthrd))
			{
	       			*errorp = EFAULT;
				return(-1);
			}
		    }

		}
		else
		{
			if (num_act > count)
			{
				/* we got 'count' entries, get out of loop */
				num_act--;
				break;
			}
		}
	}

	/*
	 * We cannot be sure that p still points to a valid entry,
	 * however, we only need to return a pid which shares the
	 * same slot, therefore we fake one.
	 */
	tid = (tid_t)(((t - &thread[0])<<TGENSHIFT) + 1);

	if (copyout((caddr_t)&tid, (caddr_t)index, sizeof(tid_t)))
	{
		*errorp = EFAULT;
		return(-1);
	}

	return(num_act);	/* successful, return no. active entries */
}

/*
 * NAME: getargs()
 *
 * FUNCTION: Retrieves the arguments for a process found via getproc()
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * NOTES: The table is not guaranteed to be consistent
 *
 * RETURN VALUES:
 *	 0	active processes retrieved
 *	-1	failed, errno indicates cause of failure, the first byte
 *	  	of args is set to NULL.
 */
getargs(struct procinfo *procinfo, int plen, char *args, int alen)
/* struct  procinfo *procinfo;	 pointer to array of procinfo struct	*/
/* int	plen;			 size of expected procinfo struct	*/
/* char	*args;			 pointer to user array for arguments	*/
/* int	alen;			 size of expected argument array	*/
{
	struct proc 	*p;		/* process struct pointer	*/
	int 		cnt = 2;	/* char count for transfers	*/
	int 		argc;		/* number of arguments		*/
	char 		**argv;		/* argv for user process	*/
	char 		*argvp;		/* char pointer array pointer	*/
	caddr_t 	dptr;		/* ptr to the specified uarea   */
	int 		rc = XMEM_SUCC;	/* return flag 			*/
	struct xmem	dp;		/* cross memory descriptor      */
	char 		c;		/* trusted dest for user data   */
	pid_t		*pidp, pid;
	int		xm;		/* xmem result			*/
	char		*errorp;	/* current u.u_error		*/

	assert(csa->prev == NULL);	/* make sure this is a process	*/
	errorp = &curthread->t_uthreadp->ut_error;

	/* check parameters for valid struct sizes:  plen, alen		*/
	/* accept either size, for compatibility */
	if (((plen != sizeof(struct procsinfo)) &&
	     (plen != sizeof(struct procinfo))) ||
            (alen < 2))
	{
		*errorp = EINVAL;
		return(-1);
	}

	/* 
	 * Initialize the output variable, "args", which is composed of
	 * multiple strings.  Each of which is terminated by a NULL 
	 * character as is the entire set of strings.
	 */
	if (subyte(args, NULL) != 0)
	{
       		*errorp = EFAULT;
		return(-1);
	}
	if (subyte(args+1, NULL) != 0)
	{
       		*errorp = EFAULT;
		return(-1);
	}

	/* get the pid from the procinfo structure */
	if (plen == sizeof(struct procsinfo))
		pidp = (pid_t *)&((struct procsinfo *)procinfo)->pi_pid;
	else
		pidp = (pid_t *)&procinfo->pi_pid;
	if (copyin((caddr_t)pidp, (caddr_t)&pid, sizeof(pid_t)))
	{
		*errorp = EFAULT;
		return(-1);
	}
	
	/* check to see if the process is still active & the same pid	  */
	if ((p = VALIDATE_PID(pid)) &&
	    !(p->p_stat == SNONE || p->p_stat == SIDL || p->p_stat == SZOMB) && 
	    !(p->p_flag & (SEXIT | SKPROC)) )
	{
		/*
		 * Attach to user segment, setup cross-memory descriptor,
		 * then detach from segment.  This segment can be detached,
		 * because xmemin() attaches and detaches each time it 
		 * copies from the source segment.  The cross-memory services
		 * are used because they provide exception handling, and
		 * because the source segment is not deduced from the current 
		 * processes address space. 
		 * We have to hold the proc_tbl_lock to lock p_adspace.  
		 * We have to hold the p_lock across the xmattach to 
		 * the target process from entering vm_cleardata.
		 */
		dptr = NULL;
		xm = XMEM_FAIL;
		simple_lock(&proc_tbl_lock);
		if (p->p_adspace != NULLSEGVAL)
		{
			simple_lock(&p->p_lock);
			dptr = vm_att(p->p_adspace, 0);
			dp.aspace_id = XMEM_INVAL;
			xm = xmattach(dptr, sizeof(int), &dp, SYS_ADSPACE);
			simple_unlock(&p->p_lock);
		}
		simple_unlock(&proc_tbl_lock);
		if (dptr)
			vm_det(dptr);
		if (xm == XMEM_FAIL) {
			*errorp = EFAULT;
			return(-1);
		}

		/* 
		 * Copy applications main(argc, argv) parameters from user 
		 * stack, which is passed to the application in register 4, 
		 * and which is saved at the top of the stack.  
		 */
		if (xmemin(ARGC_value, &argc, sizeof(argc), &dp) != XMEM_SUCC)
		{
			*errorp = EFAULT;
			(void)xmdetach(&dp);
			return(-1); 
		}
		if (xmemin(ARGS_loc, &argv, sizeof(argv), &dp) != XMEM_SUCC)
		{
			*errorp = EFAULT;
			(void)xmdetach(&dp);
			return(-1); 
		}
		rc = xmemin(argv++, &argvp, sizeof(argvp), &dp);

		/* 
		 * Fetch a byte at a time from the specified process. 
		 * xmemin truncates the top four bits, so the offsets are
		 * recomputed relative to the vmattach()ed segment.
		 */
		while (  argvp != NULL 			&& 
			 argc-- > 0			&&
			(cnt < alen && cnt < NCARGS) 	&& 
			 rc == XMEM_SUCC  )
		{
			do
			{
				/* overflow user area or exceed sys max	  */
				if (cnt == alen || cnt == NCARGS)
				{
					if (subyte(args++, '\0') != 0)
					{
       						rc = XMEM_FAIL;
					}
					cnt++;
					break;
				}
				if ((rc = xmemin(argvp++,&c,sizeof(c),&dp)) == 
					XMEM_SUCC)
				{
					if (subyte(args++, c) != 0)
					{
       						rc = XMEM_FAIL;
						break;
					}
					cnt++;
				}
			}
			while (c != '\0' && rc == XMEM_SUCC);
			if ( rc == XMEM_SUCC )
			{
				rc = xmemin(argv++, &argvp, sizeof(argvp), &dp);
			}
		}
		if (subyte(args, '\0') != 0)
		{
       			rc = XMEM_FAIL;
		}
		if (rc != XMEM_SUCC)
		{
			rc = EFAULT; 
		}
		(void)xmdetach(&dp);
	}
	else 
	{
		/* if process does not exist */
		if (p->p_stat == SNONE || p->p_pid != pid)
		{
			rc = EBADF;
		}
	}

	if (rc != XMEM_SUCC)
	{
		*errorp = rc;
		return(-1);
	}
	return(0);
}

/*
 * NAME: xmatt_segnum()
 *
 * FUNCTION: xmattaches the segment for an environment string
 *
 * EXECUTION ENVIRONMENT: This routine may only be called locally
 *
 * RETURN VALUES:
 *	 0	xmattach succeeded, the xmem descriptor is returned
 *	-1	failed
 */
static int
xmatt_segnum(ulong segnum, struct xmem *adp, struct proc *p,struct user *localU)
/* ulong        segnum;	segment number of needed segment	*/
/* struct xmem *adp;	pointer to cross memory descriptor 	*/
/* struct proc *p;	pointer to process structure		*/
/* struct user *localU; local copy of process U area		*/
{
	caddr_t		dptr;	/* ptr to addr space of proc	*/
	int		xm;	/* xmem result			*/

	/*
	 * Attach to user segment, setup cross-memory descriptor,
	 * then detach from segment.  This segment can be detached,
	 * because xmemin() attaches and detaches each time it 
	 * copies from the source segment.
	 * We have to hold the proc_tbl_lock to lock segments PRIVSEG
	 * BDATASEG-BDATASEGMAX, SHDATASEG, which are the only valid
	 * segments we may attach to.
	 * We have to hold the p_lock to prevent vm_cleardata 
	 * from running while we attach to the segment.
	 */
	simple_lock(&proc_tbl_lock);
	if (localU)
	{
		dptr = localU->U_adspace.srval[segnum];
	}
	else
	{
		ASSERT(segnum == PRIVSEG);
		dptr = p->p_adspace;
	}
	if (dptr == NULLSEGVAL)
	{
		simple_unlock(&proc_tbl_lock);
		return(-1);
	}
	dptr = vm_att(dptr, 0);
	adp->aspace_id = XMEM_INVAL;
	simple_lock(&p->p_lock);
	xm = xmattach(dptr, sizeof(int), adp, SYS_ADSPACE);
	simple_unlock(&p->p_lock);
	simple_unlock(&proc_tbl_lock);
	vm_det(dptr);  
	if (xm == XMEM_FAIL)
		return(-1);
	return(0);
}

/*
 * NAME: getevars()
 *
 * FUNCTION: Retrieves the environment for a process found via getproc()
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * NOTES: The table is not guaranteed to be consistent
 *
 * RETURN VALUES:
 *	 0	active processes retrieved
 *	-1	failed, errno indicates cause of failure, the first byte
 *	  	of user_envp is set to NULL.
 */
int
getevars(struct procinfo *procinfo, int plen, char *user_envp, int alen)
/* struct  procinfo *procinfo;	pointer to array of procinfo struct	*/
/* int	plen;			size of expected procinfo struct	*/
/* char	*user_envp;		pointer to user array for environment	*/
/* int	alen;			size of expected environment array	*/
{
	char 		**env;		/* env for user process		*/
	char 		*envp;		/* char pointer array pointer	*/
	ulong 		saveenvpseg;	/* char pointer array segnum	*/
	struct proc 	*p;		/* process struct pointer	*/
	int 		cnt = 2;	/* char count for transfers	*/
	int 		rc = XMEM_SUCC;	/* return value flag		*/
	struct xmem	*adp;		/* &cross memory descriptor     */
	char 		c;		/* trusted dest for user data   */
	pid_t		*pidp, pid;
	char		*errorp;	/* current u.u_error		*/
	struct xmem	dpseg[NSEGS];
	int		valid[NSEGS];
	struct user	localU;
	int		i;

	assert(csa->prev == NULL);	/* make sure this is a process	*/
	errorp = &curthread->t_uthreadp->ut_error;

	/* check parameters for valid struct sizes:  plen, alen		*/
	/* accept either size, for compatibility */
	if (((plen != sizeof(struct procsinfo)) &&
	     (plen != sizeof(struct procinfo))) ||
            (alen < 2))
	{
		*errorp = EINVAL;
		return(-1);
	}
	/* 
	 * Initialize the output variable, "args", which is composed of
	 * multiple strings.  Each of which is terminated by a NULL 
	 * character as is the entire set of strings.
	 */
	if (subyte(user_envp, NULL) != 0)
	{
       		*errorp = EFAULT;
		return(-1);
	}
	if (subyte(user_envp+1, NULL) != 0)
	{
       		*errorp = EFAULT;
		return(-1);
	}

	/* get the pid from the procinfo structure */
	if (plen == sizeof(struct procsinfo))
		pidp = (pid_t *)&((struct procsinfo *)procinfo)->pi_pid;
	else
		pidp = (pid_t *)&procinfo->pi_pid;
	if (copyin((caddr_t)pidp, (caddr_t)&pid, sizeof(pid_t)))
	{
		*errorp = EFAULT;
		return(-1);
	}
	
	/* check to see if the process is still active & the same pid	*/
	if ((p = VALIDATE_PID(pid)) &&
	     !(p->p_stat == SIDL || p->p_stat == SZOMB || p->p_stat == SNONE) &&
	     !(p->p_flag & (SEXIT | SKPROC)))
	{
		/*
		 * Initialize control variables for the xmattach logic.
		 *
		 * PRIVSEG, BDATASEG-BDATASEGMAX, and SHDATASEG are the only
		 * valid segments.
		 */
		for (i = 0; i < NSEGS; i++)
		{
			dpseg[i].aspace_id = XMEM_INVAL;
			valid[i] = FALSE;
		}
		valid[PRIVSEG] = TRUE;

		/* 
		 * Setup cross-memory descriptor for user segment for use
		 * by cross-memory services.  The cross-memory services
		 * are used because they provide exception handling, and
		 * because the source segment is not deduced from the current 
		 * processes address space. 
		 */
		rc = xmatt_segnum(PRIVSEG, &dpseg[PRIVSEG], p, NULL);
		if (rc == -1)
		{
			*errorp = EFAULT;
			return(-1);
		}
		rc = xmemin(&U, &localU, sizeof(U), &dpseg[PRIVSEG]);
		if (rc == -1)
		{
			xmdetach(&dpseg[PRIVSEG]);
			*errorp = EFAULT;
			return(-1);
		}
		for (i = BDATASEG; i <= BDATASEGMAX; i++)
		{
			if(!(localU.U_segst[i].segflag & SEG_WORKING))
				break;
			valid[i] = TRUE;
		}
		if (localU.U_adspace.alloc & ((unsigned)0x80000000>>SHDATASEG))
			valid[SHDATASEG] = TRUE;
		
		/* 
		 * Copy applications main() parameter char **env from user 
		 * stack, which is passed to the application in register 5, 
		 * and which is saved in the exported variable environ.
		 */
		if ((rc = xmemin(&environ, &env, sizeof(env),
						&dpseg[PRIVSEG])) == XMEM_SUCC)
		{
                        if (((ulong)env >> SEGSHIFT) == PRIVSEG)
				rc = xmemin(env++, &envp, sizeof(envp),
							      &dpseg[PRIVSEG]);
                        else
				envp = NULL;
		}

		/* Current segment */
		adp = &dpseg[PRIVSEG];
		saveenvpseg = PRIVSEG;

		/* 
		 * Fetch a byte at a time from the specified process. 
		 * xmemin() truncates the top four bits, so the offsets are
		 * recomputed relative to the vmattach()ed segment.
		 */
                while (  envp != NULL &&
                        (cnt < alen && cnt < NCARGS) &&
                         rc == XMEM_SUCC  )
		{
		    do
		    {
			/* overflow user area or exceed sys max	  */
			if (cnt == alen || cnt == NCARGS)
			{
			    if (subyte(user_envp++, '\0') != 0)
			    {
       				rc = XMEM_FAIL;
			    }
			    break;
			}
			/* handle moving to a different segment */
			if (saveenvpseg != ((ulong)envp >> SEGSHIFT))
			{
			    saveenvpseg = (ulong)envp >> SEGSHIFT;
			    if (valid[saveenvpseg])
			    {
				if (dpseg[saveenvpseg].aspace_id == XMEM_INVAL)
				{
				    if (xmatt_segnum(saveenvpseg,
						     &dpseg[saveenvpseg],
						     p,
						     &localU) == -1)
				    {
					rc = XMEM_FAIL;
					break;
				    }
				}
				adp = &dpseg[saveenvpseg];
			    }
			    else
			    {
				if (c != '\0' )
				    if (subyte(user_envp++, '\0') != 0)
       					rc = XMEM_FAIL;
				    else
					cnt++;
				 break;
			    }
			}
			if ((rc = xmemin(envp++,&c,sizeof(c),adp)) == XMEM_SUCC)
			{
			    if (subyte(user_envp++, c) != 0)
			    {
       				rc = XMEM_FAIL;
				break;
			    }
			    cnt++;
			}
		    }
		    while (c != '\0' && rc == XMEM_SUCC);
		    if ( rc == XMEM_SUCC )
		    {
			rc = xmemin(env++, &envp, sizeof(envp),&dpseg[PRIVSEG]);
		    }
		}
		if (subyte(user_envp, NULL) != 0)
		{
       			rc = XMEM_FAIL;
		}
		if (rc != XMEM_SUCC)
		{
			rc = EFAULT; 
		}
		for (i = 0; i < NSEGS; i++)
			if (valid[i])
				if (dpseg[i].aspace_id != XMEM_INVAL)
					xmdetach(&dpseg[i]);
	}
	else 
	{
		/* if process does not exist */
		if (p->p_stat == SNONE || p->p_pid != pid)
		{
			rc = EBADF;
		}
	}

	if (rc != XMEM_SUCC)		
	{
		*errorp = rc;
		return(-1);
	}
	return(0);
}
