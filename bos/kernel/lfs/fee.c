static char sccsid[] = "@(#)43	1.10.1.8  src/bos/kernel/lfs/fee.c, syslfs, bos411, 9428A410j 6/9/94 07:38:08";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: fs_fork, fs_exec, fs_exit
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/lockf.h"
#include "sys/syspest.h"
#include "sys/proc.h"
#include "sys/fs_hooks.h"
#include "sys/lock_alloc.h"
#include "sys/lockname.h"
#include "sys/sleep.h"

/*
* This file contains routines that are used by the fork, exec, and
* exit system calls to perform file system specific functions.
*/

/*
* fsfork - Perform file system specific tasks resulting from the
* fork system call.  This amounts to bumping the reference count
* for each open file and current and root directories.
*/

void
fs_fork(struct user *uchild)
{
	int n, maxo;
	struct file *fp;
        register int klock_rc;
	int tlock;	/* is multi-thread locking required? */

        klock_rc = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Do the initialization of the child process u-block. */

	/* Initialize the file system locks. */
	lock_alloc(&uchild->U_fd_lock,LOCK_ALLOC_PAGED,U_FD_CLASS,
			uchild->U_procp-(struct proc *)&proc);
	simple_lock_init(&uchild->U_fd_lock);
	lock_alloc(&uchild->U_fso_lock,LOCK_ALLOC_PAGED,U_FSO_CLASS,
			uchild->U_procp-(struct proc *)&proc);
	simple_lock_init(&uchild->U_fso_lock);
	/* Initialize the file system event anchor. */
	uchild->U_fdevent = EVENT_NULL;
	/* Initialize the indicator that file locks have been taken. */
	uchild->U_lockflag = 0;

	/*
	 * Walk parent's file descriptor table for open files known to
	 * parent & increment the global file table reference count for
	 * each file.  Copy the current state of the file descriptors
	 * to the child's file descriptor table.
	 * Increment use counts for any open file which has been mapped
	 * for fshmat.
	 */

	/* We have to have the U_FD_LOCK in the parent ublock in order
	 * to get a consistent view of the file descriptor table, and
	 * of individual descriptors.
	 */
	if (tlock = (U.U_procp->p_active > 1))
		U_FD_LOCK();
	maxo = uchild->U_maxofile = U.U_maxofile;
	for (n=0; n < maxo; n++)
	{
		if (((fp = U.U_ufd[n].fp) != NULL) &&
			(!(U.U_ufd[n].flags & UF_CLOSING)))
		{
			/* increment the use count on the file pointer */
			fp_hold(fp);
			if (U.U_ufd[n].flags & UF_FSHMAT)
				fmapfork(n);
			/* copy the file descriptor to the child ublock */
			uchild->U_ufd[n].fp = U.U_ufd[n].fp;
			uchild->U_ufd[n].flags = U.U_ufd[n].flags;
			uchild->U_ufd[n].count = 0;
		}
		else
		{
			/* remove all unwanted state in the child table */
			uchild->U_ufd[n].fp = NULL;
			uchild->U_ufd[n].flags = 0;
			uchild->U_ufd[n].count = 0;
		}
	}
	if (tlock)
		U_FD_UNLOCK();

	/*
	 * Increment the reference counts for the current directory
	 * and the root directory.
	 */
	VNOP_HOLD(uchild->U_cdir);
	if (uchild->U_rdir)
		VNOP_HOLD(uchild->U_rdir);

        if (klock_rc != LOCK_NEST)
                FS_KUNLOCK(&kernel_lock);
}

/*
 * NAME: fs_exec
 *
 * FUNCTION: Perform file system specific processing associated with
 * the exec system call.  This consists of closing all files currently
 * marked as close on exec. Additionally, audit read and write flags
 * to keep read and write state are turned off if on.
 *
 * NOTES:
 *	unmap of normal mapped files is done in shmexec
 *	unmap of fshmat mapped files is initiated here
 */

void
fs_exec()
{
	register int	klock_rc;
	register int	i;
	int		error;		/* saved u_error value */
	struct file *	fp;
	struct fs_hook *fs_exech;

	klock_rc = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	error = u.u_error;


	/* NOTE:
	 * We don't need to take the U_FD_LOCK since we are guaranteed
	 * to be single-threaded at this point.
	 */

	for (i=0; i < U.U_maxofile; i++)
	{
		/*
		 * Turn off audit state flags
		 */
		U.U_ufd[i].flags &= ~(UF_AUD_READ|UF_AUD_WRITE);

		/*
		 * Call all registered fs_exec hooks
		 */
		if ((fp = U.U_ufd[i].fp) != NULL)
			for (fs_exech = fs_exech_anchor;
			     fs_exech;
			     fs_exech = fs_exech->next)
				(fs_exech->hook)(i, fp);

		/*
		 * Call the common code for closing a file descriptor
		 * if it's marked close on exec.
		 */
		if ((U.U_ufd[i].flags & FD_CLOEXEC)
		&& ((fp = U.U_ufd[i].fp) != NULL))
			closefd(i, 1);

		if ((U.U_ufd[i].fp != NULL) &&
		    (U.U_ufd[i].flags & UF_FSHMAT))
			rmfmap(i);

	}

	u.u_error = error;

	if (klock_rc != LOCK_NEST)
		FS_KUNLOCK(&kernel_lock);

}

/*
 * NAME: fs_exit
 *
 * FUNCTION: Perform file system specific processing associated with
 * the exit system call.  This consists of closing all open files &
 * releasing the process's current and root directory
 */
void
fs_exit()
{
	struct file *fp;
	register int i;
        register int klock_rc;

	klock_rc = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* NOTE:
	 * We don't need to take the U_FD_LOCK since we are guaranteed
	 * to be single-threaded at this point.
	 */

	/* Close all open files */
	for (i = 0; i < U.U_maxofile; i++)
		if ( (fp = U.U_ufd[i].fp) != NULL )
		{
			assert(U.U_ufd[i].count == 0);
			closefd(i, 0);
		}

	/* 
	 * Free both the file descriptor lock and the fso lock
	 * when the process is exiting.
	 */
	lock_free(&U.U_fd_lock);
	lock_free(&U.U_fso_lock);

	/* release current directory, if any; kprocs may not have one */
	if (U.U_cdir)
		VNOP_RELE(U.U_cdir);

	/* release root directory, if any */
	if (U.U_rdir)
		VNOP_RELE(U.U_rdir);

        if (klock_rc != LOCK_NEST)
                FS_KUNLOCK(&kernel_lock);
}
