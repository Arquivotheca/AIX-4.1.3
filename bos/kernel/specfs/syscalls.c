static char sccsid[] = "@(#)76	1.17.2.12  src/bos/kernel/specfs/syscalls.c, sysspecfs, bos411, 9428A410j 5/16/94 13:11:48";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: pipe
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "sys/types.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "jfs/inode.h"
#include "sys/lockl.h"
#include "sys/trchkid.h"
#include "sys/errno.h"
#include "sys/id.h"
#include "sys/sleep.h"
#include "sys/malloc.h"
#include "sys/specnode.h"
#include "sys/uio.h"
#include "sys/gpai.h"
#include "sys/fs_locks.h"
#include "sys/lockname.h"
#include "sys/lock_alloc.h"

extern struct vnodeops fifo_vnops;
extern struct fileops vnodefops;
extern int    spec_generation;

caddr_t *gpai_alloc(struct galloc *);
void     gpai_free(struct galloc *, caddr_t *);
extern struct galloc specnode_pool;
extern struct galloc fifonode_pool;

/*
 * NAME:  pipe
 *
 * FUNCTION:  implements unix pipe ipc mechanism
 *
 * EXECUTION ENVIRONMENT:
 *      It can page fault.
 *      This routine is a system call entry point and may have
 *	intra or inter kernel invokations.
 *
 * DATA STRUCTURES:  inode, file
 *
 * RETURN VALUE DESCRIPTION:	2 user file descriptors are allocated
 *				the 1st is open for reading
 *				the 2nd is open for writing
 */

int
pipe(pp)
int	*pp;
{
	struct file *	wf;
	struct file *	rf = NULL;
	struct vnode *	vp = NULL;
	struct specnode *snp;
	struct gnode *	gp;
	int		rc = 0;
	int		pfd[2];
	extern int	kernel_lock;	/* global kernel lock		*/
	extern int	gn_reclk_count; /* occurrence number of rec lock */
	int		lockt;		/* previous state of kernel lock */
	int tlock;			/* is multi-thread locking required? */
	extern struct vfs pipevfs;	/* vfs to hang pipe vnodes off of */
	struct ucred *	crp;		/* pointer to process credentials */
	struct timestruc_t t;

	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* allocate our specnode */
	if ((snp = (struct specnode *)gpai_alloc(&specnode_pool)) == NULL)
	{
		rc = ENOMEM;
		goto errout;
	}

	/* initialize the specnode fields */
	snp->sn_next =		NULL;
	snp->sn_count = 	1;
	snp->sn_gen =		fetch_and_add(&spec_generation,1);
	bzero(&snp->sn_gnode, sizeof(struct gnode));
	snp->sn_pfsgnode = 	NULL;

	/* allocate and initialize the specnode attributes */
	snp->sn_attr = (struct icattr *)malloc(sizeof(*snp->sn_attr));
	if (snp->sn_attr == NULL)
	{
		gpai_free(&specnode_pool, snp);
		rc = ENOMEM;
		goto errout;
	}
	bzero(snp->sn_attr, sizeof(*snp->sn_attr));	/* XXX */

	curtime(&t);
	/* fill in the specnode attribute fields */
	snp->sn_mode =		IFIFO;
	snp->sn_atime =		t;
	snp->sn_mtime =		t;
	snp->sn_ctime =		t;
	snp->sn_acl =		NULL;

	/* allocate and initialize the fifonode */
	snp->sn_fifonode = (struct fifonode *)gpai_alloc(&fifonode_pool);
	if (snp->sn_fifonode == NULL)
	{
		free(snp->sn_attr);
		gpai_free(&specnode_pool, snp);
		rc = ENOMEM;
		goto errout;
	}
	bzero(snp->sn_fifonode, sizeof(*snp->sn_fifonode));	/* XXX */

	snp->sn_wevent =	EVENT_NULL;
	snp->sn_revent =	EVENT_NULL;

	/* initialize the gnode fields */
	gp = STOSGP(snp);
	gp->gn_type =		VFIFO;
	gp->gn_data =		(caddr_t)snp;
	gp->gn_ops =		&fifo_vnops;
	gp->gn_seg =		-1;
	gp->gn_rdev =		NODEVICE;
	gp->gn_reclk_event =	EVENT_NULL;
	gp->gn_filocks =	NULL;
	lock_alloc(&gp->gn_reclk_lock, LOCK_ALLOC_PAGED,
		   RECLK_LOCK_CLASS, gn_reclk_count++);
	simple_lock_init(&gp->gn_reclk_lock);

	/* set up read and write counts for pipe */
	snp->sn_rcnt =		1;
	snp->sn_wcnt =		1;

	/* Indicate there is a writer on the pipe */
	snp->sn_flag =		FIFO_WWRT;

	/* get a held vnode from the pipe vfs */
	if (rc = vn_get(&pipevfs, STOSGP(snp), &vp))
	{
		gpai_free(&fifonode_pool, snp->sn_fifonode);
		free(snp->sn_attr);
		gpai_free(&specnode_pool, snp);
		goto errout;
	}

	/* set vnode flag to indicate a specfs vnode */
	vp->v_flag |= V_SPEC;

	/* There is no underlying PFS object */
	vp->v_pfsvnode = NULL;

	/* allocate our read file pointer */
	if (rc = ufdalloc(0, &pfd[0]))
		goto errout;
	if (rc = fpalloc(vp, FREAD, 0, NULL, &rf)) {
		ufdfree(pfd[0]);
		goto errout;
	}

	/* allocate our write file pointer */
	if (rc = ufdalloc(0, &pfd[1]))
		goto errout;
	if (rc = fpalloc(vp, FWRITE, 0, NULL, &wf)) {
		ufdfree(pfd[1]);
		goto errout;
	}

	/* fill in other file struct fields */
	rf->f_type = wf->f_type = DTYPE_VNODE;
	rf->f_ops  = wf->f_ops  = &vnodefops;

	/* get cred pointer and save in the file structs; 
	 * do an extra hold because it is used in two places.
	 */
	crp = crref();
	crhold(crp);
	rf->f_cred = wf->f_cred = crp;

	/* initialize the uid and gid in the specnode from the cred struct */
	snp->sn_uid =		crp->cr_uid;	
	snp->sn_gid =		crp->cr_gid;

	VNOP_HOLD(vp);
	vp = NULL;

        /*
	 * LOCKING NOTE:  up to this point there has been no locking
         * because the specnode has not been accessible via any pointers
         * external to this function.  Once a pointer is stuck into the
         * file table this is no longer true.  At this point we could
         * grab the specnode lock, but there are no more references to
	 * it so that doesn't seem to be necessary.
         */

	/* The vnode is held once already so we don't need to bump
	 * its hold count.  Just attach the file pointer to the
	 * file descriptor.
	 */
	U.U_ofile(pfd[0]) = rf;

	/* The vnode is now attached to the write file pointer, so hold
	 * the vnode and forget it.  Also attach the file pointer to the
	 * file descriptor.
	 */
	U.U_ofile(pfd[1]) = wf;

	if (copyout(pfd, pp, sizeof(pfd))) {
		close(pfd[0]);
		close(pfd[1]);
		rc = EFAULT;
		goto errout;
	}

	TRCHKL2T(HKWD_SYSC_PIPE, pfd[0], pfd[1]);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	return 0;

errout:
	if (vp)
	{
		/* reset open counts before releasing */
		VTOSP(vp)->sn_rcnt = 0;
		VTOSP(vp)->sn_wcnt = 0;
		VNOP_RELE(vp);
		if (rf)
		{
			ffree(rf);
			ufdfree(pfd[0]);
		}
	}

	u.u_error = rc;

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	return -1;
}

