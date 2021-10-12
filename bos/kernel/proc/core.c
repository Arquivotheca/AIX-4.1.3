static char sccsid[] = "@(#)45	1.22.3.4  src/bos/kernel/proc/core.c, sysproc, bos41J, 9520A_a 5/17/95 17:17:48";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: core
 *              cwrite
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/shm.h>
#include <sys/user.h>
#include <sys/pseg.h>
#include <sys/file.h>
#include <sys/fp_io.h>
#include <stdio.h>
#include <sys/ldr.h>
#include <sys/core.h>
#include <sys/malloc.h>
#include <sys/id.h>
#include <sys/errids.h>
#include <sys/acct.h>
#include <sys/stat.h>
#include <sys/lockl.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/machine.h>
#include <sys/atomic_op.h>
#include <sys/reg.h>
#include "swapper_data.h"

extern uid_t getuidx(int);
extern gid_t getgidx(int);
extern int vstat();
extern int lockl();
extern void unlockl();

static cwrite();

struct errstruct {
	struct err_rec0	hdr;
	int signo;
	pid_t pid;
	int fs_sn;
	int inode;
	char basename[MAXCOMLEN+1];
} cderr = { ERRID_CORE_DUMP, "SYSPROC", 0, 0, -1, -1, "" };

/*
 * NAME: core()
 *
 * FUNCTION: This function places the necessary information from a running
 *	process into the file "core" to allow a debugger to reconstruct the
 *	current state of that process.  This process is usually terminated
 *	upon completion of this call.  The minimum usable dump with header
 *	is written first, so it is usable even if the dump is not completed.
 *
 * DATA STRUCTURES: The user area block, user stack, loaded file information
 *	and optional user data are read and copied to the core file.
 *	No process data is modified by this procedure.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	 0	= successful completion, no core
 *	 1	= successful completion, core file written in current directory
 *	-1	= failed, partial core dump, CORE_TRUNC set in c_flag
 */ 
int
core (char signo, struct sigcontext *sigctx)
/* signo	 current signal number being processed */
/* sigctx	 pointer to temp register save area	*/
{
	struct core_dump *hdrp;		/* core dump header pointer */
	struct ld_info	*ldorgp, *ldp;	/* pointer to loader info */
	char		*datap;		/* pointer to data area */
	struct file	*fp;		/* core file read/write pointer */
	off_t		offset;		/* offset of core tables for I/O */
	uint		stack_top;      /* top of stack, where stack begins */
	int		count;		/* loader entry counter */
	int		length;		/* length of data to write */
	int		status = 0;	/* write file status, 0 is good */
	int		status2 = 0;	/* temporary status */
	int		maxfile;	/* maximum core file size */
	int		mstoffset;	/* offset to mst area */
	int		stacksiz = 0;	/* stack size */
	uint		sreg;           /* sreg for stack segment */
	struct stat	core_stat;	/* statistics of core file */
	int		lock_t;		/* lock return code */
	struct thread	*t;		/* thread block pointer */
	struct thread	*th;		/* another thread block pointer */
	struct proc	*p;		/* current process */
	struct vm_info	*vmmp;		/* pointer to vm_info table */
	struct vm_info	*vptr;		/* tmp pointer */
	int		mmregions;	/* number of mmaped regions */
	int		mmoffset;	/* offset to mmaped regions */
	int		ipri;		/* saved interrupt priority */
	int		npgs_needed;	/* estimate of pages needed */
	struct sigcontext *scp;		/* signal context pointer */
	struct sigcontext sc_buf;	/* local sigctx copy */
	struct sigcontext tempsc;	/* local sigctx structure scratch */
        uint		shmat_stack_top; /* shared memory stack pointer */
        uint		default_stack_top; /* process private stack pointer */
	uint		segsize;	/* size of shmat'd segment */
	struct mstsave	*def_mstptr=NULL; /* default sigctx mst pointer */
	struct mstsave  *mstptr;	/* machine state save area pointer */
        int             active_threads; /* number of active threads */
	int		ldsize = 0;	/* size of loader entry table */
        struct ucred    *crp;		/* users credential pointer */
	int		rc;		/* temp return code */
	vmhandle_t	def_srval;	/* default sigctx srval for stk ptr */
	uint		def_sreg;	/* default sigctx stk ptr segment */
	uint		stack_floor;	/* bottom of user stack region */
	uint		data_limit;	/* data region in seg 2 */ 

	t = curthread;
	p = t->t_procp;

	/* Quiesce system by suspending all threads */
        if (p->p_active > 1) {

		ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif

                /*
                 * Can't core dump if exit or exec is in process.
                 * Also don't allow concurrent core dumps.
                 */
		if (p->p_int & (SSUSPUM|STERM)) {
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
			unlock_enable(ipri, &proc_base_lock);
			return(-1);
		}

		/* 
		 * Request suspension in user mode. If signals are checked
		 * before sleeping, then it will appear as if SIGKILL has
		 * been sent.   
		 */
		p->p_int |= SSUSPUM;

		/* The process may be partially swapped out */
		schedsig(p);

		/* The process may be partially stopped in kernel mode */
		cont(p);

                /* Wake up the threads waiting for me */
		t->t_flags |= TTERM;
		e_wakeup((int *)&t->t_synch);			/* TTERM set */

                /* Wake up the threads sleeping interruptibly */
                th = t->t_nextthread; /* skip oneself */
                while (th != t) {
			switch (th->t_wtype) {
			case TWCPU :
			case TNOWAIT :
				break;
			case TWPGIN :
				vcs_interrupt(th);
				break;
			default :
				if (th->t_flags & TWAKEONSIG) {
                       		     	if (th->t_state == TSSLEEP)
                                	      setrq(th, E_WKX_PREEMPT, RQHEAD);
                            	     	else 
					      th->t_wtype = TNOWAIT;
				}
				break;
			}
                        th = th->t_nextthread;
                }

		p->p_suspended++;
		while (p->p_suspended < p->p_active) {
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
			e_sleep_thread((int *)&p->p_synch,
				&proc_base_lock, LOCK_HANDLER);
#ifdef _POWER_MP
			simple_lock(&proc_int_lock);
#endif
			if (p->p_int & STERM) {
				p->p_suspended--;
				p->p_int &= ~SSUSPUM;
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
#endif
				unlock_enable(ipri, &proc_base_lock);
				thread_terminate();
				/* Does not return */
			}
		}
		p->p_suspended--;

                /* Reset so that subsequent sleeps will not return early. */
		p->p_int &= ~SSUSPUM;
		t->t_flags &= ~TTERM;

#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
		unlock_enable(ipri, &proc_base_lock);
        }

	/* create entry in error log file */
	if ( signo != SIGQUIT )
	{
		/* synchronize access - critical section */
		lock_t = lockl(&kernel_lock, LOCK_SHORT);

        	/* Get the user's credentials */
        	crp = crref();

		/* get current directory vnode */
		if (vstat(U.U_cdir, &core_stat, crp) == 0)  	   
		{
			cderr.fs_sn = core_stat.st_vfs; /* filesystem ser num */
			cderr.inode = core_stat.st_ino; /* current dir inode  */
		}

        	crfree(crp);

		if (lock_t != LOCK_NEST)
			unlockl(&kernel_lock);

		cderr.pid = p->p_pid;			/* process id number  */
		cderr.signo = (int)signo;		/* signal number      */
		cderr.basename[MAXCOMLEN] = '\0';
		(void)strncpy(cderr.basename,t->t_userp->U_comm,MAXCOMLEN);
		errsave(&cderr, sizeof(cderr));
	}

	u.u_error = 0;

	maxfile = (U.U_sigflags[signo] & SA_NODUMP) ? 0 : CCORE_SIZE;
	
	/* check permissions and minimum size */
	if ((getuidx(ID_SAVED) != getuidx(ID_REAL)) ||
	    (getgidx(ID_SAVED) != getgidx(ID_REAL)) ||
	    (CHDRSIZE > maxfile))
		return(0);

	/* open file with user's default permissions and write access */
	if (fp_open(COREFILE, O_TRUNC|O_CREAT|O_RDWR, 0666, 0, FP_SYS, &fp))
		return(0);	/* don't set u.u_error	*/

	/* set core header to the minimum dump possible */
	hdrp = (struct core_dump *)xmalloc(CHDRSIZE, 2, kernel_heap);
	if(hdrp == NULL)
	{
		(void)fp_close(fp);
		return(-1);
	}

	/* Guess how many paging space blocks will be used by this routine */
	npgs_needed = ((p->p_active * sizeof(struct mstsave)) + 
			CHDRSIZE + CTABSIZE) / PAGESIZE;

	/*
	 * if below initial warning for low pg space, acquire
	 * lock in write mode; else acquire lock in read mode.
	 */
again:  if ((psdanger(SIGDANGER) - npgs_needed) < 0) 
	{
		rc = lock_try_write(&core_lock);
		if (rc == FALSE)
		{
	                delay(200);
	                goto again;
		}
        } else if (lock_try_read(&core_lock) == FALSE) {
                delay(200);
                goto again;
        }

        hdrp->c_signo = signo;
        hdrp->c_flag = CORE_VERSION_1;
	if (U.U_sigflags[signo] & SA_FULLDUMP || v.v_fullcore == 1)
		hdrp->c_flag |= FULL_CORE;
	hdrp->c_entries = 0;
        hdrp->c_tab = NULL;
        hdrp->c_stack = NULL;
	hdrp->c_size = 0;
        hdrp->c_u = U;
        hdrp->c_nmsts = 0;
	hdrp->c_msts = NULL;
        hdrp->c_datasize = 0;
        hdrp->c_data = NULL;
        hdrp->c_vmregions = 0;
        hdrp->c_vmm = NULL;

	/* store current thread mst */ 
	if (copyin(sigctx, &sc_buf, sizeof(struct sigcontext)) == 0)
	{
		/*
		 * If copyin() fails, def_mstptr is NULL.
		 * This prevents a DSI below where the default
		 * signal context is deferenced.
		 */
		def_mstptr = &sc_buf.sc_jmpbuf.jmp_context;
		hdrp->c_mst = *def_mstptr;
		hdrp->c_mst.curid = t->t_tid;
		hdrp->c_flag |= UBLOCK_VALID;

		def_sreg = def_mstptr->gpr[1] >> SEGSHIFT;
		def_srval = def_mstptr->as.srval[def_sreg];
	}
	else {
		def_srval = p->p_adspace;
	}

       	status |= cwrite(fp, (caddr_t)hdrp, CHDRSIZE, UIO_SYSSPACE, 0);

	offset = CHDRSIZE;

	/* walk through loaded files, write info */
        if ((offset + CTABSIZE) <= maxfile)
	{

		length = CTABSIZE * 10;	/* starting guess size */
		while (1) {
	                if ( (ldorgp = (struct ld_info *)
				xmalloc(length, 2, kernel_heap)) == NULL )
			{
				xmfree(hdrp, kernel_heap);
				(void)fp_close(fp);
				lock_done(&core_lock);
				return(-1);
			}

	                if (ld_getinfo(p->p_pid, length, ldorgp) != 0)
			{
				/* out with the old */
                        	xmfree(ldorgp, kernel_heap);
                        	length *= 2;
			}
			else
				break;
                }

		/* get count; set ldinfo_core to NULL */
		for (ldp = ldorgp, count = 1; ldp->ldinfo_next; count++)
		{
			ldp->ldinfo_core = NULL;
			ldp = (struct ld_info *) ((char *)ldp+ldp->ldinfo_next);
		}
		ldp->ldinfo_core = NULL;
			
		ldsize = MIN((char *)ldp - (char *)ldorgp + CTABSIZE, length);

		/* 
		 * if too big, store first ld_info entry 
		 * (contains program name) 
		 */
		if ((offset + ldsize) > maxfile) {
			ldsize = CTABSIZE;
			count = 1;
		} 

		status2 = cwrite(fp, (caddr_t)ldorgp, ldsize, 
				UIO_SYSSPACE, offset);
		if (!status2) {
			hdrp->c_entries = count;
			hdrp->c_tab = (struct ld_info *)offset;
                	hdrp->c_flag |= LE_VALID;
			offset += ldsize;
		}
		status |= status2;
	}
	else {
		hdrp->c_flag |= CORE_TRUNC;
	}

        /*
         * Determine size of default user stack.  Assume that it resides
         * in a single segment.  core doesn't support multiple shmatted
         * segments, since there is only one stack pointer in header.
         */
	data_limit = (U.U_dsize < SEGSIZE ? U.U_dsize : U.U_sdsize) + PRIVORG;
	stack_floor = USRSTACK - U.U_smax;
	if ((stack_floor < data_limit) || (stack_floor > USRSTACK))
		stack_floor = data_limit;

	mstoffset = offset;
        shmat_stack_top = 0xffffffff;
        default_stack_top = 0xffffffff;
        th = t;
        active_threads = 0;
        do {
                mstptr = (struct mstsave *) &(tempsc.sc_jmpbuf.jmp_context);
                if (th->t_state == TSIDL || th->t_state == TSZOMB)
                {
                        mstptr = NULL;
                }
                else if ((scp = th->t_scp) != NULL)
                {
                        copyin(scp, &tempsc, sizeof(struct sigcontext));
                }
                else 
                {
                        bcopy(&(th->t_uthreadp->ut_save),
                              &(tempsc.sc_jmpbuf.jmp_context),
                              sizeof(struct mstsave));
                }

                if (mstptr != NULL) {

                        active_threads++;
                        mstptr->curid = th->t_tid;

			/* Faulting mst goes into the header */
                        if (th != t) {  
				if (offset + sizeof(struct mstsave) <= maxfile)
				{
                                	status2 = cwrite(fp, (caddr_t)mstptr,
                 	                         sizeof(struct mstsave),
                                         	 UIO_SYSSPACE, offset);
					if (!status2) {
              					hdrp->c_msts =(struct mstsave *)
							mstoffset;
                        			hdrp->c_nmsts++;
                        			hdrp->c_flag |= MSTS_VALID;
                                		offset +=sizeof(struct mstsave);
					}
					status |= status2;
                        	}
				else {
					hdrp->c_flag |= CORE_TRUNC;
				}
			}

                        /* determine segment of user stack */
                        stack_top = (uint)round_down(mstptr->gpr[1], PAGESIZE);
                        sreg = stack_top >> SEGSHIFT;

                        /* 
			 * If the current stack pointer is within the
			 * the segment containing the faulting mst, then
			 * calculate its low water mark. 
			 */
                        if ((U.U_segst[sreg].segflag & SEG_SHARED) &&
			    (U.U_adspace.srval[sreg] == def_srval))
			{
                                if (stack_top < shmat_stack_top) {
                                        shmat_stack_top = stack_top;
                                }
                        }
			/* 
			 * Otherwise if the current stack is within the user
			 * stack region, then calc its low water mark.
			 */
                        else if ((stack_top <= USRSTACK) &&
                                 (stack_top > stack_floor))
			{
                                if (stack_top < default_stack_top)
                                        default_stack_top = stack_top;
                        }
                }

		th = th->t_nextthread;

	} while (th != t);

 	ASSERT(active_threads == p->p_active);

	/*
	 * Try to dump shared memory segment stack.
	 */
	if ((shmat_stack_top != 0xffffffff) && (checksrval(def_srval) == 0)) 
	{
		stack_top = shmat_stack_top;
		segsize = U.U_segst[sreg].shm_ptr->shm_segsz;
		stacksiz = (sreg<<SEGSHIFT|segsize) - stack_top;
		if ((offset + stacksiz) > maxfile)
			stacksiz = 0;
	}

	/*
	 * If we haven't identified the stack yet, try to dump default 
	 * user stack based on the lowest stack pointer.
	 */
	if (stacksiz == 0) 
	{
		if (default_stack_top != 0xffffffff) {
			stack_top = default_stack_top;
			stacksiz = USRSTACK - stack_top;

			/* If it doesn't fit in its entirety, then reset */
			if ((offset + stacksiz) > maxfile)
				stacksiz = 0;

		}
	}

	/*
	 * If we still haven't dumped stack, try the VMM low water mark
	 * in the stack region.
	 */
        if (stacksiz == 0) 
	{
            	int mindown, pages;

            	if ((mindown = vm_get_ssize(p->p_adspace)) != 0) {
                	pages = SEGSIZE/PAGESIZE - mindown;
                	stack_top = PRIVORG + SEGSIZE - (PAGESIZE * pages);
                	if (stack_top <= USRSTACK && stack_top > stack_floor)
                    		stacksiz = USRSTACK - (int)stack_top;
			if ((offset + stacksiz) > maxfile)
				stacksiz = 0;
            	}
        }

	/*
	 * If we still haven't dumped a stack, then get the last page
	 * of the faulting stack.  dbx requires a stack, even one that
	 * it can't make sense.  This could pick up a stack in
	 * the data region.
	 */
	if (stacksiz == 0) 
	{
       		stack_top = (uint)round_down(def_mstptr->gpr[1], PAGESIZE);
	 	stacksiz = PAGESIZE;
	}
		

	/* dump user stack, if there is room */
	if ((stacksiz != 0) && (offset + stacksiz) <= maxfile)
	{
		status2 = cwrite(fp, (caddr_t)stack_top, stacksiz, 
				UIO_USERSPACE, offset);
		if (!status2) {
			hdrp->c_flag |= USTACK_VALID;
			hdrp->c_stack = (char *)offset;
			hdrp->c_size = stacksiz;
			offset += stacksiz;
		}
		status |= status2;
	}
	else 
	{
		hdrp->c_flag |= CORE_TRUNC;
	}

	/*
	 * full dump:
	 *
	 *	dump data areas
	 *	dump mmap'd regions
	 *	dump vm_info structs
	 */
	if (hdrp->c_flag & FULL_CORE)
	{
		caddr_t data_org=(caddr_t)PRIVORG;
		uint    data_length=U.U_dsize; 

		if (data_length > SEGSIZE) /* big data? */
		{
			data_length -= SEGSIZE; 
			data_org = (caddr_t)BDATAORG;
			hdrp->c_flag |= CORE_BIGDATA;
		}

		/* check if data area fits */
		if (offset + data_length <= maxfile) 
		{
			status2 = cwrite(fp, data_org, data_length, 
					 UIO_USERSPACE, offset);
			if (!status2) {
        			hdrp->c_datasize = data_length;
				hdrp->c_data = (char *)offset;
				offset += data_length;
			}
			status |= status2;
		}
		else {
			hdrp->c_flag |= CORE_TRUNC;
		}

		/*
		 * dump remaining data areas
		 */
		if (hdrp->c_flag & LE_VALID)
		{
		    for (ldp = ldorgp, count = 1;
			 count <= hdrp->c_entries;
			 ldp = (struct ld_info *)((char *)ldp+ldp->ldinfo_next),
			 count++)
		    {
			length = ldp->ldinfo_datasize;

			/* data area not within default data area */
			if (length > 0) 
			{
			    if (offset + length <= maxfile) 
			    {
				 status2 = cwrite(fp, 
						 (caddr_t)ldp->ldinfo_dataorg, 
						 length, UIO_USERSPACE, offset);
				 if (!status2) {
					ldp->ldinfo_core = offset;
					offset += length;
				 }
				 status |= status2;
			    } else 
			    {
			         hdrp->c_flag |= CORE_TRUNC;
			    }
			}
		    }

		    /* update all ld_info structures */
		    status |= cwrite(fp, (caddr_t)ldorgp, ldsize,
					UIO_SYSSPACE, hdrp->c_tab);
		}

		/* 
		 * dump memory mapped regions and vm_info structs
		 */
		if (U.U_map) 
		{
		    mmregions = vm_map_core(U.U_map,(struct vm_info *)NULL);

		    if (mmregions != 0) 
		    {
                   	length = mmregions * sizeof(struct vm_info);

		    	if (offset + length <= maxfile)
		    	{
			     vmmp = (struct vm_info *)
				  xmalloc((uint)length, (uint)2, kernel_heap);
	
			     if (vmmp == NULL)
			     {
				  hdrp->c_flag |= CORE_TRUNC;
				  goto exit;
			     }

			     /* get mmap region info */
			     vm_map_core(U.U_map, vmmp);	

			     /* 
			      * Save offset of vm_info structures.
			      * Write the mmap regions next.
			      */
		    	     mmoffset = offset;
                             offset += length;
			     vptr = (struct vm_info *)vmmp;

			     for (count = 0; count < mmregions; count++)
			     {
				 vptr->vminfo_offset = 0;
				 if (offset + vptr->vminfo_size <= maxfile)
				 {
				     /* 
				      * write one region at a time
				      */
				     status2 = cwrite(fp, 
					          (caddr_t)vptr->vminfo_addr, 
						  vptr->vminfo_size, 
						  UIO_USERSPACE, offset);
				     /*
				      * The write may succeed, or it may 
				      * terminate due to hitting a red zone
				      * while attempting to dump a thread's
				      * stack.  We accept this later case, 
				      * and continue.
				      */
				     if (!status2 || EPERM == status2) {
					  vptr->vminfo_offset = offset;
					  offset += vptr->vminfo_size;
				     }
				     else {
				     	  status |= status2;
				     }
				 } 
				 else {
				     hdrp->c_flag |= CORE_TRUNC;
				 }
				 vptr++;
			     }

			     /* now write vm_info tables */
			     status2 = cwrite(fp, (caddr_t)vmmp, length, 
						UIO_SYSSPACE, mmoffset);
			     if (!status2) {
		    	     	 hdrp->c_vmregions = mmregions;
				 hdrp->c_vmm = (struct vm_info *)mmoffset;
		             }	
			     status |= status2;

			     xmfree((void *)vmmp, kernel_heap);
		    } 
		    else {
		        hdrp->c_flag |= CORE_TRUNC;
		    }
		}
	    }
	} 	/* end full dump */
exit:

	if (status)
		hdrp->c_flag |= CORE_TRUNC;
	if (hdrp->c_flag & CORE_TRUNC)
		hdrp->c_flag &= ~FULL_CORE;

	/* rewrite core header */
	status |= cwrite(fp, (caddr_t)hdrp, CHDRSIZE, UIO_SYSSPACE, 0);

	lock_done(&core_lock);

	if (hdrp->c_flag & LE_VALID)
		xmfree((void *)ldorgp, kernel_heap);
	xmfree((void *)hdrp, kernel_heap);

	(void)fp_close(fp);
	if (status != 0)
		return(-1);	

	/* set accounting bit - core was dumped */
	U.U_acflag |= ACORE;

	return(1);
}

/*
* cwrite - write to a core file
*
* Arguments:	fp - file pointer to the core file
*		addr - address of the buffer
*		count - number of bytes to write
*		adspace - address space identifier
*		offset - offset into the file
*
* Return value:	0 implies success, !0 implies failure.
*/

static
cwrite(	struct file *	fp,
	caddr_t		addr,
	int		count,
	int		adspace,
	off_t		offset)
{
	int rc, wcount;

	(void) fp_lseek(fp, offset, SEEK_SET);	
	rc = fp_write(fp, addr, count, 0, adspace, &wcount);
	return rc != 0 || wcount != count;
}
