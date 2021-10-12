static char sccsid[] = "@(#)48	1.112.1.47  src/bos/kernel/proc/exec.c, sysproc, bos41J, 9516B_all 4/21/95 10:51:25";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: copyargs_in
 *		copyargs_out
 *		execvex
 *		
 *   ORIGINS: 3, 26, 27, 83
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
 *
 * NOTES:
 *	This is the platform-independent part of exec[2].  It calls
 *	machine-dependent functions to create and destroy process
 *	images.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/param.h>
#include <sys/audit.h>
#include <sys/acct.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/fp_io.h>
#include <sys/ldr.h>
#include <sys/lock.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mstsave.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/pseg.h>              /* for stack size calculations */
#include <sys/seg.h>               /* for stack size calculations */
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/trchkid.h>
#include <sys/user.h>
#include <sys/lockname.h>
#include <sys/sleep.h>
#include <sys/atomic_op.h>

#include <a.out.h>
#include <sys/resource.h>

#include "swapper_data.h"
#include "exec.h"			/* local header file for exec */
#include "m_exec.h"			/* machine-dependent local header */

#define MAX_LARGE_DATA_SET_MODEL	0x80000000	/* 8 times 256M-bytes */


/*
 * function declarations
 */
static int	copyargs_in();		/* copy arguments into kernel memory */
static int	copyargs_out();		/* copy arguments to top of stack */

extern int	exec_priv();		/* privilege amplification */
extern void	bcopy();		/* block copy */
extern void	fs_exec();		/* close requested files */
extern void	execexit();		/* special syscall exit for execve() */
extern int	setregs();		/* set up user machine context */
extern void	prof_off();		/* turns off profiling */

BUGVDEF(excdbg, 0);			/* debugging control variable */


/*
 * NAME: execvex
 *
 * FUNCTION: exec[2] system call handler
 *
 *	This is the system call handler for all the variations of
 *	exec[2].  The library routines for execl(), execle(),
 *	execlp(), execv(), and execvp() all transform their arguments
 *	into the execve() form, and make this system call.
 *
 *	This is the machine-independent part of exec.  It calls
 *	machine-dependent functions in m_exec.c, which resides
 *	in the platfrom-dependent source tree.
 *
 * EXECUTION ENVIRONMENT:
 *	preemptable (except during file system calls)
 *	may page fault
 *
 * NOTES:
 *
 *	High-level flow of exec processing (when successful):
 *
 *	     *	open the exec'ed file
 *	     *	read the load module header and check the magic number
 *	     *	if "#!", open the indirect shell file and read its header
 *	     *	copy arguments and environment variables from old process image
 *	     *	load the exec'ed program
 *	     *	copy arguments and environment to new process image
 *	     *	update u-block and proc structure for new process image
 *	     *	call execexit() to start up the new program
 *
 * RETURNS:	-1 if unsuccessful, u.u_error has the reason;
 *		Does not return when successful, instead calls execexit()
 *			to start the new program at its main entry point.
 */

execvex(const char *fname, char *const argp[], char *const envp[])
/* fname	 name of file to exec */
/* argp[]	 array of ptrs to exec arguments */
/* envp[]	 array of ptrs to environment variables */
{
	struct file *fp, *fp_indirect=0;/* open file table pointer */
	int	bcount;			/* byte count from fp_read() */
	int	oflags;			/* open mode flags */
	int	rc;			/* return code from fp_read() */
	struct	rlimit alim;		/* used for setrlimit() */
	long 	newlimit; 		/* used for call to ulimit() */
	lock_t  lockt;			/* return code from lockl() */

	/* automatic structure declarations */
	struct stat		stst;	/* stat structure */
	union execunionX	hdr;	/* exec file header */
	struct xargs		xargs;	/* exec argument structure */
	char    auditName[MAXCOMLEN+1]; /* base file name */
	register struct xargs	*xp;	/* pointer to xargs */
	static int		svcnum = 0;
	int			svcrc = 0;

	/* Position of the pointer to the LIBPATH variable in the envp array */
	int			libpath_ptr; 
        /* Pointer to loader domain/LIBPATH variable string */
        char                    *ldomain_path = NULL;

	int			env_len = 0; /* Total length of env. strings */
	int			arg_len = 0; /* Total length of arg. strings */

	/*  Buffer for the error messages used by ld_emessdump() 
	 *  in case of an error during load.                              
         */
	char 			*errmsg_buf = NULL; 

	int			ipri;	 /* saved interrupt priority */
	struct thread		*t, *th; /* current thread */
	struct thread		*tzomb = NULL; /*zombie threads in the process*/
	struct proc		*p;	 /* current process */
	char			*errorp; /* current error storage */

	t = curthread;
	p = t->t_procp;
	errorp = &t->t_uthreadp->ut_error;

	/* Await death of all other threads belonging to the process */
	if (p->p_threadcount > 1 || t->t_uthreadp != &uthr0)
	{
		ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif

		/* 
	 	 * fork, exec, and exit are serialized by the forkstack.  If
	 	 * we have gotten this far, then a pending fork/exit in the
	 	 * same process will be negated by setting STERM. 
	 	 */
		p->p_flag |= (SEXECING);
		p->p_int |= (STERM);

		if (p->p_active > 1)
		{
			/* The process may be partially stopped/suspended */
			cont(p);

			/* The process may be partially swapped out */
			schedsig(p);

			t->t_flags |= TTERM;

			e_wakeup((int *)&p->p_synch);	/* STERM set */
			e_wakeup((int *)&t->t_synch);	/* TTERM set */

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

			/* Wait for the other threads to terminate themselves */
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
			}	
			p->p_suspended--;

			ASSERT(p->p_active == 1);
		}

		/* Orphan all other threads belonging to the process */
		if (p->p_threadcount > 1) 
		{
			/* Keep anchors */
			p->p_threadlist = t;
			tzomb = t->t_nextthread;

			/* Remove oneself from the fellow zombies */
			tzomb->t_prevthread = t->t_prevthread;
			t->t_prevthread->t_nextthread = tzomb;

			/* Remove the zombies from the threadlist */
			t->t_nextthread = t->t_prevthread = t;

#ifdef DEBUG
			/* Check the zombies */
			th = tzomb;
			do {
				switch (th->t_state) {
				case TSZOMB:
				case TSIDL:
					p->p_threadcount--;
					break;
				default :
					assert(FALSE);
				}
				th = th->t_nextthread;
			} while (th != tzomb);
			assert(p->p_threadcount == 1);
#else
			p->p_threadcount = 1;
#endif
		}

		/* Switch to the standard uthread area if not already there */
		if (t->t_uthreadp != &uthr0)
		{
			/* copy the current uthread */
			bcopy(t->t_uthreadp, &uthr0, sizeof(struct uthread));
			/* switch */
			t->t_uthreadp = &uthr0;
			SET_CSA(&uthr0.ut_save);
		}

                /* Reset so that subsequent sleeps will not return early. */
		p->p_int &= ~STERM;
		t->t_flags &= ~TTERM;

#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
		unlock_enable(ipri, &proc_base_lock);
	}
	else {
		p->p_flag |= (SEXECING);
	}

	/* Release uthread table control block and kernel stacks segment. */
	pm_release(&U.U_uthread_cb);
	if (p->p_kstackseg != NULLSEGVAL)
	{
                pm_release(&U.U_cancel_cb);
		vms_delete(SRTOSID(p->p_kstackseg));
		p->p_kstackseg = NULLSEGVAL;
		uthr0.ut_kstack = (char *)&__ublock;
	}

	/* Reallocate uthread table control block */
	(void)pm_init(	&U.U_uthread_cb,			  /* zone  */
			(char *)&__ublock.ub_uthr,		  /* start */
			(char *)&__ublock + sizeof(struct ublock),/* end   */
			sizeof(struct uthread),			  /* size  */
			(char *)&uthr0.ut_link - (char *)&uthr0,  /* link  */
			UTHREAD_ZONE_CLASS,			  /* class */
			p-proc,					  /* occur */
			PM_FIFO);				  /* flags */

	/*
	 * Clean up the zombies.
	 *
	 * Note: While all other state transitions are protected by the
	 * proc_int_lock, TSZOMB->TSNONE is protected by the proc_tbl_lock
	 * for the scheduler.
	 */
	if (tzomb != NULL) {
		struct thread *thr;
		simple_lock(&proc_tbl_lock);
		th = tzomb;
		do {
			thr = th;
			th = th->t_nextthread;
			freethread(thr);
		} while (th != tzomb);
		simple_unlock(&proc_tbl_lock);
	}

	if (audit_flag)
		svcrc = audit_svcstart("PROC_Execute", &svcnum, 3,
			U.U_cred->cr_uid, U.U_cred->cr_gid, U.U_cred->cr_epriv);

	if(svcrc){

		if(fname){
                        char *ptr;
                        int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                                *errorp = ENOMEM;
				goto bad;
                        }
                        if(copyinstr(fname, ptr, MAXPATHLEN, &len)){
                                *errorp = EFAULT;
                                goto bad;
                        }
                       	audit_svcbcopy(ptr, len);
                       	free(ptr);
		}
	}

	/* initialize accounting field - executed by superuser */
	if (!privcheck(SET_PROC_RAC))
		U.U_acflag |= ASU;

	/* initialize exec argument structure */
	xp = &xargs;
	xp->fname = (char *)fname;
	xp->argp = (char **)argp;
	xp->envp = (char **)envp;
	xp->libpath = NULL;
	xp->buf = NULL;
	xp->ebuf = NULL;
	xp->indir = 0;
	xp->loaderror = 0;

	/* come back to try to exec /etc/execerror after execload fails*/
      eagain:	

	/*
	 * Open the exec file.  The fp_xopen() service returns u.u_error.
	 * If tracing, we require read permission as well as execute.
	 * The basename of the resolved path is returned in xp->cfname.
	 */
	oflags = O_EXEC | ((p->p_flag & STRC) ? O_RDONLY : O_NONE);
	*errorp = 0;
	if ((*errorp = fp_xopen(xp->fname, xp->loaderror?FP_SYS:FP_USR, oflags,
			  xp->cfname, sizeof(xp->cfname), &fp)) != 0) {
		fp = NULL;
		goto bad;
	}

	/* save important attributes of the original exec file */
	if (fp_fstat(fp, &stst, sizeof(struct stat), FP_SYS) == -1) {
		*errorp = EACCES;
		goto bad;
	}
	
	/* not a regular file - its a directory or other bad thing*/
	if (!S_ISREG(stst.st_mode)) {
		*errorp = EACCES;	/* not a regular file - it is */
		goto bad;		/* a dir or other bad thing.  */
	}	

	/* branch back here after opening an indirect file */
      again:

	/*
	 * If privileged, the access checking in open will
	 * have succeeded, but we still don't want to run
	 * programs without execute permission for at least someone.
	 */
	if ((*errorp = fp_access(fp, IEXEC)) != 0)
		goto bad;

	/*
	 * Read in first few bytes of file for segment sizes, magic number.
	 * Also an ASCII line beginning with #! is the file name of a
	 * ``shell'' (or other interpreter) and an argument may be prepended
	 * to the argument list if given there.
	 */
	hdr.u_exshell[0] = '\0';	/* for zero length files */
	rc = fp_read(fp, (char *)&hdr, sizeof(hdr), 0, UIO_SYSSPACE, &bcount);
	if (rc) {
		*errorp = rc;
		goto bad;
	}

	if ((bcount < sizeof(hdr))
	    && (hdr.u_exshell[0] != '#'))  { /* not enough read? */
		*errorp = ENOEXEC;	/* invalid load module format */
		goto bad;
	}

	/*
	 * clean this up for machine dependencies and add a hook for
	 * dealing with old load module formats and other machine
	 * architectures.
	 */
	switch ((int)hdr.u_xcoffhdr.filehdr.f_magic) {

		register char	*cp;	/* pointer for scanning command name */
		register char	*indname; /* indirect file name pointer */
                register char   *indbase; /* indirect file basename pointer */
		char		*tail ; /* end of interpretor line */

	      case XCOFF_MAGIC:		/* XCOFF magic for this machine */

		if (hdr.u_xcoffhdr.aouthdr.o_tsize == 0) {
			*errorp = ENOEXEC;
			goto bad;
		}
		break;

	      default:

		if (hdr.u_exshell[0] != '#' || /* not an indirect file? */
		    hdr.u_exshell[1] != '!' ||
		    xp->indir) {		/* already indirect? */
			/*
			 * ADD CODE TO CALL A PLATFORM-DEPENDENT MODULE FOR
			 * HANDLING OLD BINARIES, OTHER ARCHITECTURES, ETC.
			 */
			*errorp = ENOEXEC;
			goto bad;
		}

		/* find end of line; turning tabs into blanks */
		tail = &hdr.u_exshell[2];		/* skip "#!" */
		while (tail < &hdr.u_exshell[INTERPRET]) {
			if (*tail == '\t')
				*tail = ' ';
			else if (*tail == '\n') {
				*tail = '\0';
				break;
			}
			tail++;
		}
		if (*tail++ != '\0') {	/* newline not found? */
			*errorp = ENOEXEC;
			goto bad;
		}

		/* scan for "shell" file name */
		cp = &hdr.u_exshell[2];
		while (*cp == ' ')	/* skip leading white space */
			cp++;
		indname = indbase = cp;	/* start of indirect file name */
		while (*cp && *cp != ' ') { /* find end of file name */
                        if ( *cp == '/' )
                                indbase = cp +1;
			cp++;
		}

		xp->cfarg[0] = '\0';	/* in case there's no arg */
		if (*cp) {		/* not at end of line? */
			*cp++ = '\0';	/* mark end of file name string */
			while (*cp == ' ')
				cp++;	/* skip white space */
			if (*cp)	/* is there an arg? */
				bcopy((caddr_t)cp, (caddr_t)xp->cfarg, tail-cp);
		}

		if(audit_flag)
		{
			fp_indirect = fp ;
			bcopy((caddr_t)xp->cfname,(caddr_t)auditName,MAXCOMLEN);
		}
		else
			(void)fp_close(fp);

		xp->indir = 1;		/* set file indirection flag */

		/*
		 * Open the interpreter file.  Saving its basename in
		 * xp->cfname seems strange, but that's the way it has
		 * always worked.
		 */
		if ((*errorp = fp_xopen(indname, FP_SYS, oflags,
				  NULL, 0, &fp)) != 0){
			fp=NULL;
			goto bad;		/* open failed */
		}

		if (fp_fstat(fp, &stst, sizeof(stst), FP_SYS) == -1) {
			*errorp = EACCES;
			goto bad;
		}
	
		if (!S_ISREG(stst.st_mode)){
			*errorp = EACCES;	/* not a regular file - it is */
			goto bad;		/* a dir or other bad thing.  */
		}

                copyname(xp->cfname, indbase, sizeof(xp->cfname));

		goto again;
	}

	/* 
	 * Copy arguments from the user memory into kernel memory.
	 * If there was an error during load, then the arguments and
	 * the env. variables would be in kernel memory and we don't
	 * have to do copyargs_in().
	 *
	 * Pass the address of the libpath_ptr as an argument to copyargs_in.  
         * copyargs_in copies the arguments and environment variables to 
         * kernel memory.  The LIBPATH environment variable is not copied 
	 * into the buffer. Instead, copyargs_in returns the position of 
	 * the LIBPATH variable in envp[] array.  'libpath_ptr' can be used 
	 * to retrieve the LIBPATH after the privileges are determined with 
	 * the exec_priv() function.  This needs to be done before exec_priv, 
	 * because the latter may alter the privilege of the current process 
         * and this function may fail. copyargs_in() also returns the total 
	 * length of the environment strings in the buffer.  This is needed to 
	 * retrieve the env. strings from the buffer in case of an error 
	 * during load. 'env_len' is used to save the length of env. strings.     
	 */

	if (!xp->loaderror){
	   /* allocate a pageable buffer to hold the strings */
	   if ((xp->buf = xmalloc(NCARGS, PGSHIFT, kernel_heap)) == NULL) {
		*errorp = ENOMEM;
		goto bad;
	   }
	   if (*errorp = copyargs_in(xp, &libpath_ptr, &env_len))
		goto bad;
	   
	   /*  Save the total length of arguments so that the env.
	    *  strings can be retrieved in case of an error during load.
	    */
	   arg_len = xp->nc - env_len;
	}

	/*
	 * Let the new process acquire/inherit privileges.  There are 3 
	 * return values:
	 *
         *   1   the process has had it's privileges amplified
         *       in this case don't let the user specify an alternate
         *       libpath.  we also need to remove LIBPATH from the
         *       user's environment,  to prevent subsequent exec's
         *       from using LIBPATH.  We still need to copy in LIBPATH
         *       so that a loader domain name can be retrieved if
         *       necessary.  xp->libpath is set to NULL.
         *
         *   0   the process has not had it's privileges amplified
         *       (normal case).  In this case, we have to keep the
         *       LIBPATH.  This is done by retrieving the LIBPATH
         *       from xp->envp with the help of libpath_ptr (returned
         *       by copyargs_in).  xp->envp+libpath_ptr points to the
         *       LIBPATH. The LIBPATH is then appended to xp->buf.
         *
         *  -1   the process is operating on the Trusted Path and
         *       the file to be exec'd does not have the S_ITP bit
         *       bit set in the mode (i.e. the file is not "trusted").
         *       In this case the exec fails...
         *
         * After this there is no turning back.
         */

        rc = exec_priv(fp, &stst);
        if (rc == -1) {
                (void)fp_close(fp);     /* close load module file */
                fp = NULL;
                *errorp = ENOTRUST;
                goto kill;
        }
        else if (rc == 0 || rc == 1) {
                /*
                 * In case of error during load, LIBPATH already exists
                 * in the buffer and we don't have to copy it in from
                 * the user memory.
                 * If LIBPATH variable exists in the environment, then
                 * copy into the buffer.  copyargs_in sets xp->libpath
                 * if it exists in the envp[] array.  If the xp->libpath
                 * is not set, then LIBPATH did not exist at all and there
                 * is nothing to copy.  If the xp->libpath is set, then
                 * xp->env+libpath_ptr points to the LIBPATH.
                 *
                 * In the case where rc == 1,  we still copy the LIBPATH
                 * from user to kernel space.  This is done so that any
                 * loader domain specified can be passed to the process.
                 * However,  the variables which describe the environment
                 * are not updated to reflect LIBPATH.  Therefore,  it
                 * will not be copied out(from kernel to user).
                 */
                if (xp->libpath && !xp->loaderror) {

                   register caddr_t ap; /* Pointer to LIBPATH */
                   register char *cp;   /* Pointer to end of buffer */
                   uint     len;        /* Length of LIBPATH */

                   /*  Retrieve the LIBPATH from user memory */
                   ap = (caddr_t) fuword((caddr_t)(xp->envp+libpath_ptr));
                   cp = xp->buf+xp->nc; /* Append the LIBPATH to the buffer */

                   if (*errorp = copyinstr(ap,cp,(unsigned)NCARGS-xp->nc, &len))
                      goto kill;
                   if (rc == 0) { /* case where privileges are not amplified */
                      xp->ne++;         /* Increment no. of env strings */
                      xp->nc += len;    /* Increment no. of chars in buffer */
                      xp->libpath = cp+8;  /* Set the LIBPATH */
                      ldomain_path = cp+8; /* Set loader domain path */
                      env_len += len;   /* Increment total length of env strgs*/
                   }
                   else {   /*  case where privileges are amplified */
                      ldomain_path = cp+8; /* Set loader domain path */
                      xp->libpath = NULL;
                   }
                }
        }

        /* round number of characters up to an even number of words */
        xp->nc  = (xp->nc + NBPW-1) & ~(NBPW-1);

        /* get rid of old text etc - we are now committed to execing
         * N.B. if more than one loader is supported, this call must be
         * based on the loader type of the old module, not the new one. */
        ld_usecount(-1);

	/* zero most of the data area - this is a machine dependent operation
	 * since the optional strategy depends on details of the memory
	 * management system.
	 * The simple lock is used to protect the adspace manipulations
	 * and prevent getproc() from xmattach'ing the process private segment
	 * during VMM operations.
	 */
	simple_lock(&p->p_lock);
        rc = vm_cleardata(&U.U_ufd[U.U_maxofile + 1]);
        simple_unlock(&p->p_lock);
        if (rc) 
	  	goto kill;

	/* identify user segments that can be deleted */ 
	freeuspace(NULL);

	/* if process was profiling, free profiling resources */
	if (U.U_prof.pr_scale)
		prof_off();

	/* if memory was locked, unpin locked text or data */
	if (U.U_lock & (PROCLOCK|TXTLOCK|DATLOCK))
		plock(UNLOCK);

	*errorp = 0;

	/* release mmap resources -- must be done before shmex() */
	if (U.U_map) 
	{
		vm_map_deallocate(U.U_map);
		U.U_map = NULL;
	}
	shmex();			/* release shared memory segments */
	sigexec();			/* update signal state 	*/
	fs_exec();			/* close files that were requested */

	/* If user specified a compile-time max stack size, give it    */
	/* to him with a setrlimit() call. If setrlimit() fails for whatever */
	/* reason, abort the exec process. Need to convert the desired */
	/* stack size to an absolute address for setrlimit() to handle    */
	if(hdr.u_xcoffhdr.aouthdr.o_maxstack)
	{
		simple_lock(&U.U_handy_lock);
		alim.rlim_max = u.u_rlimit[RLIMIT_STACK].rlim_max;
		alim.rlim_cur = hdr.u_xcoffhdr.aouthdr.o_maxstack;
		rc = xsetrlimit(RLIMIT_STACK, &alim);
		simple_unlock(&U.U_handy_lock);
		if (rc == -1)
			goto kill;
	}

	/* adjust data limit from o_maxdata similiarly */
	if (hdr.u_xcoffhdr.aouthdr.o_maxdata)
	{
		simple_lock(&U.U_handy_lock);
		alim.rlim_max = u.u_rlimit[RLIMIT_DATA].rlim_max;
		alim.rlim_cur = BDATAORG - PRIVORG +
			(( hdr.u_xcoffhdr.aouthdr.o_maxdata >
			MAX_LARGE_DATA_SET_MODEL ) ? 
			MAX_LARGE_DATA_SET_MODEL :
			hdr.u_xcoffhdr.aouthdr.o_maxdata) ;
		rc = xsetrlimit(RLIMIT_DATA, &alim);
		simple_unlock(&U.U_handy_lock);
		if (rc == -1)
			goto kill;
	}

	/* Establish process private segment early paging space		*/
	/* allocation state.  Shrink heap so paging space requirements	*/
	/* will not be overestimated.					*/
	if (p->p_flag & SPSEARLYALLOC)
	{
		U.U_dsize = 0;
		sbreak(PRIVORG + 1);
		if (*errorp = vms_psearlyalloc(SRTOSID(p->p_adspace)))
			goto kill;
	}
	else
		vms_pslatealloc(SRTOSID(p->p_adspace));

        if (!(xp->ep = ld_execload(fp,xp->libpath,ldomain_path)))
        {       /* loader failed? */
		register char	**arg;	/* pointer to error messages */
		int     errmsg_len = 0; /* Total length of error strings */
		int     errmsg_ctr = 0;	/* Total no. of error messages */
	        char    *cp;		/* Buffer for argument & env. strings */
		int 	len;		
		int     offset;

		(void)fp_close(fp);	/* close load module file */
		fp = NULL;
		if (xp->loaderror|(p->p_flag&STRC) )
			goto kill;		/* kill user process */
		xp->loaderror = 1;
		xp->indir = 0;
		xp->fname = "/usr/sbin/execerror";
		xp->na    = 0;
		xp->nc    = 0;
                xp->libpath = NULL;
                ldomain_path = NULL;

		/*  Allocate a buffer for the error messages to be
	         *  passed to ld_emessdump().
		 */
		if ((errmsg_buf = xmalloc(NCARGS, PGSHIFT, kernel_heap))== NULL)
		   goto kill;
		/* emessdump fills buf with a vector of char pointers followed
		 * by the strings they point to.  The vector is NULL terminated,
		 * just like any other parameter list */
		ld_emessdump(errmsg_buf,NCARGS, SYS_ADSPACE);
		arg      = (void *)errmsg_buf;
		/* Get the number of error messages and the length */
		while(*arg != NULL)
		{
		   errmsg_len += strlen(*arg)+1;
		   errmsg_ctr++;
		   arg++;
		}

		/* Allocate a buffer for the error messages & env. strings to be
		 * passed to execerror.
		 */
		if ((xp->ebuf = xmalloc(NCARGS, PGSHIFT, kernel_heap)) == NULL)
		   goto kill;
		
		/* Put the arguments and the env.strings in xp->ebuf */
		cp = xp->ebuf;
		if (copystr("execerror", cp, (unsigned)NCARGS, &len))
	           goto kill;
		cp     += len;
		xp->nc += len;
		xp->na++;

		/*  Copy the filename from xp->buf */
		if (copystr(xp->buf, cp, (unsigned)NCARGS-xp->nc, &len))
	           goto kill;
		cp     += len;
		xp->nc += len;
		xp->na++;

		/*  Copy the error messages from errmsg_buf */
		if (errmsg_len > NCARGS-xp->nc){
		   *errorp = ENOMEM;
		   goto kill;
		}
		offset = (errmsg_ctr+1)*sizeof(void *);
		bcopy(errmsg_buf+offset, cp, errmsg_len);
		xp->nc += errmsg_len;
		xp->na += errmsg_ctr;

		TRCGENT(0,HKWD_SYSC_EXECVE,1,xp->nc,xp->ebuf);

		/*  Compute remaining character count */
		if (env_len > (NCARGS-xp->nc)){
		   *errorp = ENOMEM;
		   goto kill;
		}

		/*  Append the env. variables in xp->ebuf */
		cp = xp->ebuf+xp->nc;
		bcopy(xp->buf+arg_len, cp, env_len);
		xp->nc += env_len;

		/*  Free the buffer containing the error messages */
		xmfree(errmsg_buf, kernel_heap);
		errmsg_buf = NULL;
		goto eagain;
	}

	if(fp_indirect)
	{
		lockt = lockl(&kernel_lock, LOCK_SHORT);

		bcopy((caddr_t) auditName, (caddr_t)U.U_comm, MAXCOMLEN);
		aud_vn_exec(fp_indirect->f_vnode);
		(void)fp_close(fp_indirect);
		fp_indirect = NULL;

		if (lockt != LOCK_NEST) unlockl(&kernel_lock);
	}

	/* save file basename for accounting */
	bcopy((caddr_t)xp->cfname, (caddr_t)U.U_comm, MAXCOMLEN);

	if(audit_flag) {
		lockt = lockl(&kernel_lock, LOCK_SHORT);
		aud_vn_exec(fp->f_vnode);
		if (lockt != LOCK_NEST) unlockl(&kernel_lock);
	}

	(void)fp_close(fp);		/* close load module file */
	fp = NULL;

	/* set up machine-dependent user stack context for main() */
	if (*errorp = setregs(xp))
		goto kill;		/* kill user process */

	/* copy arglist back to top of user stack. */
	if (*errorp = copyargs_out(xp))
		goto kill;		/* kill user process */

	xmfree(xp->buf, kernel_heap);	/* free the argument buffer */
	xp->buf = NULL;

	/*  In case of an error during load, free xp->ebuf */
	if (xp->loaderror){
	   xmfree(xp->ebuf, kernel_heap); /* free the argument buffer */
	   xp->ebuf = NULL;
	}

	/* Execve has succeeded. It is now safe to reset errnop and userdata */
	errnop = &errno;
	t->t_uthreadp->ut_errnopp = &errnop;
	t->t_userdata = 0;

	sysinfo.sysexec++;		/* count number of exec's */
	cpuinfo[CPUID].sysexec++;
	U.U_acflag &= ~AFORK;           /* reset the accounting flag */

	p->p_size = btoc(U.U_dsize + U.U_tsize);

	p->p_flag |= SEXECED;		/* process has exec'd */

	if (p->p_flag & STRC)
	{
		/* if tracing, then signal curthread to stop it */
		pidsig(p->p_pid, SIGTRAP);

		/* private copy text, shlib */
		assert(ld_ptrace(p->p_pid) == 0);

		/* if debugging across exec's, set stopped in exec flag */
		if (p->p_flag & SMPTRACE)
			fetch_and_or((atomic_p)&p->p_atomic, SEWTED);
	}

	BUGLPR(excdbg, BUGACT,
	       ("exec: completed sucessfully\n"));

	/* 
	 * Commit audit record
	 */

	if(svcrc){

		/* 
		 * Must do an immediate commit now since state is
		 * not saved
		 * u.u_error should be zero, but is EINVAL 
		 */
		 
		*errorp = 0;
		audit_svcfinis();
		auditscall();

	}

	/*
	 * We need to release the special fork stack before leaving.
	 * However we know for a fact that we are a monothreaded process,
	 * therefore we don't have to protect the resetting of SFORKSTACK.
	 * we don't have to wake up anybody and we don't have to actually 
	 * give up the special fork stack, we can stay on it until
	 * the end of the system call.
	 */
	p->p_flag &= ~(SFORKSTACK|SEXECING);

	/*
	 * The execve() system call is special, because it does not
	 * resume the state that had been saved on the user stack
	 * on entry.  This stack no longer exists.  We could fake
	 * out the contents of the new stack to look as if it had
	 * done a system call with the appropriate resume state saved,
	 * but it seems simpler just to pass the new user context to
	 * a special system call handler function, execexit().
	 */
	execexit(xp->ucp);

	/* NEVER RETURNS */

	/*
	 * errors that occur after the "point of no return" result in
	 * killing the new partially-constructed process image.
	 */
      kill:

	xp->loaderror = 1;	/* force kill */


	/*
	 * if any error occurred during exec, free resources and return
	 */
      bad:

	/* 
	 * Commit audit record 
	 * Don't wait for svc handler (low.s)
	 */

	if(svcrc){

		audit_svcfinis();
		auditscall();

	}

	if (xp->buf)
		xmfree(xp->buf, kernel_heap);
	if (xp->ebuf)
		xmfree(xp->ebuf, kernel_heap);
	if (errmsg_buf)
		xmfree(errmsg_buf, kernel_heap);

	if (fp)
		(void)fp_close(fp);
	if (fp_indirect)
		(void)fp_close(fp_indirect);

	if (xp->loaderror){
		BUGLPR(excdbg, BUGNFO,
		       ("exec: process image killed.\n"));

		kthread_kill(-1, SIGKILL);
	}

	BUGLPR(excdbg, BUGNFO,
	       ("exec: u.u_error = %d.\n", (int) *errorp));

	/*
	 * We know for a fact that we are a monothreaded process,
	 * therefore we don't have to protect the resetting of STERM,
	 * and we don't have to wake up anybody.
	 */
	p->p_flag &= ~(SEXECING|SFORKSTACK);

	return(-1);
}

/*
 * NAME: copyargs_in
 *
 * FUNCTION: Copy arguments into kernel memory.
 *
 *	Allocates a kernel buffer to hold the user argument strings,
 *	then copies the data into kernel memory.
 *
 * RETURNS:
 *	zero if successful; errno value otherwise.
 *      The position of LIBPATH in the envp[] array is returned
 *      in libpath_ptr.  
 *	The total length of environment strings is returned in
 *	env_lenp.
 */

static int
copyargs_in(register struct xargs *xp, int *libpath_ptr, int *env_lenp)
/* xp		 pointer to exec argument structure */
/* libpath_ptr   position of LIBPATH in the envp[] array */
/* env_lenp	 total length of the environment strings */
{
	register char	*cp = xp->buf;	/* next character in buffer */
	register int	cc = NCARGS;	/* remaining character count */
	register char	*ap;		/* argument pointer */
	register char	**argp = xp->argp;
	register char	**envp = xp->envp;
	register int	error;
	uint    len;			/* length of string copied */

	xp->na = xp->nc = xp->ne = 0;

	U.U_procp->p_flag &= ~SPSEARLYALLOC;

	if (argp == NULL) {
		TRCGENT(0,HKWD_SYSC_EXECVE,1,strlen(xp->fname),xp->fname);
		return(0);
	}

	if (xp->indir) {
		ap = xp->cfname;	/* indirect file name */
		argp++;			/* ignore argv[0] */
		xp->na++;
		if (error = copystr(ap, cp, (unsigned)cc, &len))
			return(error);
		cp += len;
		cc -= len;

		if (xp->cfarg[0]) {
			ap = xp->cfarg;/* indirect arg is next */
			xp->na++;
			if (error = copystr(ap, cp, (unsigned)cc, &len))
				return(error);
			cp += len;
			cc -= len;
		}

		ap = xp->fname;		/* exec'ed file name next */
		xp->na++;
		if (error = copyinstr(ap, cp, (unsigned)cc, &len))
			return(error);
		cp += len;
		cc -= len;
	}

	for (;;) {
		if ((ap = (char *)fuword(argp)) == (char *)-1)
			return(EFAULT);
		else if (ap == NULL)
			break;
		argp++;			/* next arg pointer */
		xp->na++;
		if (error = copyinstr(ap, cp, (unsigned)cc, &len))
			return(error);
		cp += len;
		cc -= len;
	}

	TRCGENT(0,HKWD_SYSC_EXECVE,1,cp-xp->buf,xp->buf);

	if (envp == NULL) {
		xp->nc = NCARGS - cc;
		return(0);
	}

	for(;;) {
		if ((ap = (char *)fuword(envp)) == (char *)-1)
			return(EFAULT);
		else if (ap == NULL)
			break;
		envp++;			/* next env pointer */
		xp->ne++;
		if (error = copyinstr(ap, cp, (unsigned)cc, &len))
			return(error);

		/* check for meaningful environment variable */
		switch (*cp) {
		case 'L': /* check for LIBPATH environment variable */
			if (memcmp(cp,"LIBPATH=",8) == 0) {
				len = 0;	/* don't copy in */
				xp->ne--;

				/* Save the position of LIBPATH in envp */
				*libpath_ptr = xp->ne;

				/* Set xp->libpath as an indication that
				 * LIBPATH exists in envp[]
				 */
				xp->libpath = cp+8;
			}
			break;
		case 'P': /* check for PSALLOC environment variable */
			if (memcmp(cp,"PSALLOC=early",14) == 0)
				U.U_procp->p_flag |= SPSEARLYALLOC;
			break;
		default:
			break;
		}

		cp += len;
		cc -= len;      
		*env_lenp += len;
	}

	xp->nc = NCARGS - cc;	/*  Store number of characters in the buffer */

	return(0);
}

/*
 * NAME: copyargs_out
 *
 * FUNCTION: Copy arguments out to top of user stack.
 *
 *	Allocates a kernel buffer to hold the user argument strings,
 *	then copies the data into kernel memory.
 *
 * RETURNS:
 *	zero if successful; errno value otherwise.
 */

static int
copyargs_out(register struct xargs *xp)
/* xp       pointer to exec argument structure */
{
	register caddr_t ap = xp->ap;	/* argument area pointer */
	register char	*cp;	        /* string buffer pointer */
	register char	*buf;        	/* string buffer pointer */
	register caddr_t	usp;	/* user stack string pointer */
	register int	na = xp->na;	/* number of arg characters */
	register int	ne = xp->ne;	/* number of env characters */
	caddr_t		strp;		/* user stack string area */

	/*  If there was an error during load, then the 
	 *  xp->ebuf points to the string buffer; else 
	 *  xp->buf contains the string buffer.
	 */
	if (xp->loaderror)
	   cp = buf = xp->ebuf;
	else
	   cp = buf = xp->buf;

	/* compute and remember address of string area */
	usp = strp = ap + XA_ARGSIZE(xp) + XA_ENVSIZE(xp);

	/* store the argument list pointers in the argument area */
	while (na--) {
		if (suword(ap, usp))
		    	return(EFAULT);
		while(usp++, *cp++)	/* scan past end of string */
			;
		ap += NBPW;
	}

	/* store a null pointer at the end of the argument list */
	if (suword(ap, NULL))
		return(EFAULT);
	ap += NBPW;

	/* store the environment list pointers in the argument area */
	while (ne--) {
		if (suword(ap, usp))
		    	return(EFAULT);
		while(usp++, *cp++)	/* scan past end of string */
			;
		ap += NBPW;
	}

	/* store a null pointer at the end of the environment list */
	if (suword(ap, NULL))
		return(EFAULT);
	ap += NBPW;

	/* copy the whole string area in one move */
	if (copyout(buf, strp, xp->nc))
		return(EFAULT);

	return(0);
}
