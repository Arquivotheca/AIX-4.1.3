static char sccsid[] = "@(#)50	1.14  src/bos/kernel/lfs/lockrlse.c, syslfs, bos411, 9428A410j 4/19/94 16:15:35";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: lockrelease
 *
 * ORIGINS: 3, 24, 27
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

/* vfs_io.c	1.5 86/12/30 NFSSRC */
/* vfs_io.c 1.1 86/09/25 SMI	*/

#include <sys/user.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/fs_locks.h>
#include <sys/flock.h>
#include <sys/syspest.h>


/*
 * DEBUG
 */
BUGVDEF(lkrele, 0x0);

/*
 * NAME:	lockrelease (fp)
 *
 * FUNCTION:	Removes any locks this process has on the object
 *		referred to by the file pointer argument.
 *
 * PARMETERS:	Fp is a pointer to a struct file.
 *
 * RETURN VALUE:	The return code from the VNOP_LOCKCTL
 *			call is passed back.
 */
int
lockrelease(struct file *fp)
{
	/*
	 * Only do extra work if the process has done record-locking.
	 */

	int rc = 0;

	if (U.U_lockflag)
	{
		register struct vnode *vp;
		register struct file *ufp;
		register int i;
		register int locked;
		struct eflock ld;
		int tlock;

		if (tlock = (U.U_procp->p_active > 1))
			U_FD_LOCK();	/* lock the fd table during search */
		locked = FALSE; 	/* innocent until proven guilty */
		U.U_lockflag = 0;	/* reset process flag */
		vp = fp->f_vnode;
		/*
		 * Check all open files to see if there's a lock
		 * possibly held for this vnode.
		 */
		for (i = 0; i < U.U_maxofile; i++ ) {
			if ( (U.U_pofile(i) & UF_FDLOCK)
			    && ((ufp = U.U_ofile(i)) != NULL)
			    && (ufp->f_type == DTYPE_VNODE))
			{
				/* the current file has an active lock */
				if (ufp->f_vnode == vp) {
					/* release this lock */
					locked = TRUE;	/* (later) */
					U.U_pofile(i) &= ~UF_FDLOCK;
				}
				else
				{
					/* another file is locked */
					U.U_lockflag = 1;
				}
			}
		} /*for all files*/
		if (tlock)
			U_FD_UNLOCK();

		/*
		 * If 'locked' is set, release any locks that this process
		 * is holding on this file.  If record-locking on any other
		 * files was detected, the process was marked (U.U_lockflag)
		 * to run thru this loop again at the next file close.
		 */
		 if (locked) {

			ld.l_type = F_UNLCK;	/* set to unlock entire file */
			ld.l_whence = 0;	/* unlock from start of file */
			ld.l_start = 0;
			ld.l_len = 0;		/* do entire file */
			ld.l_vfs = vp->v_vfsp->vfs_type;
			ld.l_pid = U.U_procp->p_pid;
			ld.l_sysid = 0;

			rc = VNOP_LOCKCTL(vp, 0, &ld, SETFLCK,
						0, 0, fp->f_cred);
		 }
	}
	return (rc);
}
