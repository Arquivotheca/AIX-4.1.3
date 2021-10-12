static char sccsid[] = "@(#)96  1.1.1.4  src/bos/kernext/aio/aio_init.c, sysxaio, bos412, 9445C412a 11/1/94 09:34:37";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: aio_close_hook, remdev, aio_init, arl_init, morereqs, moredevs
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "aio_private.h"
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

struct fs_hook closeh;
struct fs_hook fs_exech;

void
aio_close_hook(int fd, struct file *fp)
{
	int rc;
	pid_t pid = getpid();
	struct gnode    *gnp = fp->f_vnode->v_gnode;	/* gnode pointer */

	/* check the device table and delete entry if found
	 */
	if ((fp->f_flag & FAIO) &&
	(fp->f_type == DTYPE_VNODE) &&
	(gnp->gn_type == VCHR)) {
		/* remove the devid from the dev table, in
		 * case the device is deleted.
		 */
		remdev(gnp->gn_rdev);
	}

	if (fp->f_flag & FAIO) {
		DBGPRINTF(DBG_CANCEL,
			  ("aio_close_hook: pid %d, fd %d\n", pid, fd));
		while ((rc = cancel_fd(pid, fp, fd, 1)) == AIO_NOTCANCELED)
			DBGPRINTF(DBG_CANCEL,
				  ("aio_close_hook: pid %d, fd %d retrying\n",
				   pid, fd));
	} else
		DBGPRINTF(DBG_CANCEL_VERBOSE,
		      ("aio_close_hook: skipping pid %d, fd %d\n", pid, fd));
}

/* remove a device table entry from the device table    */
void
remdev(dev_t dev)
{
	int             i, search = 1;	/* dev table search flag */
	struct devtab   *devtabp;	/* dev table pointer */
	struct devtab   *devtabp2;	/* dev table pointer */

	devtabp2 = devtabp = devnop;	/* initialize for the search */
	DEVTAB_LOCK();
	while ((devtabp) && (devtabp->empty)) {
		for (i = 0; i < devtabp->empty; ++i) {
			/* first find the entry in the table
			 */
			if ( devtabp->deventry[i].devid == dev ) {
				/* now find last section in the table
				 */
				while ((devtabp2->next) &&
				(devtabp2->next->empty)) {
					devtabp2 = devtabp2->next;
				}
				devtabp2->empty -= 1; /* last entry     */
				/* now move last entry onto deleted entry
				 */
				(devtabp->deventry[i].devid) =
				(devtabp2->deventry[devtabp->empty].devid);
				(devtabp->deventry[i].devtype) =
				(devtabp2->deventry[devtabp->empty].devtype);

				DEVTAB_UNLOCK();
				return;
			}
		}

		devtabp = devtabp->next;
	}
	DEVTAB_UNLOCK();
	return;
}


/***********************************************************************\
*                                                                       *
* FUNCTION:     AioInit							*
*                                                                       *
* DESCRIPTION:                                                          *
*       This function is called by the aioload program immediately	*
*	after this module (asynchronous I/O) is loaded into the kernel.	*
*	It is also called just before this module is unloaded.  The	*
*	argument cmd is -1 for an unload request, it encodes several	*
*	initialization parameters for a load request.			*
*                                                                       *
* RETURNS:                                                              *
*       int error       Non-zero on error condition.			*
*                                                                       *
\***********************************************************************/

int
aio_init(int cmd, struct uio *uiop)
{
	struct aio_cfg	aio_cfg;
	int		err = 0;
	int		i;
	int             old_state;
	server	       *sp;
	
	if ( cmd == CFG_INIT ) {

		old_state = AIO_UNCONFIGURED;
		if (!compare_and_swap(&aio_state,
				      &old_state, AIO_CONFIGURING))
			return EBUSY;

		/*  We have just loaded async i/o.              */
		if (copyin(uiop->uio_iov->iov_base, &aio_cfg, sizeof aio_cfg)){
			aio_state = AIO_UNCONFIGURED;
			return EFAULT;
		}

		/* initialize for the LVM async path    */
		if((err = arl_init()) != 0){
			aio_state = AIO_UNCONFIGURED;
			return err;
		}

		/*
		 * Initialize global variables from cfg struct.
		 */
		maxservers   = aio_cfg.maxservers;
		minservers   = aio_cfg.minservers;
		maxreqs      = aio_cfg.maxreqs;
	
		/*
		 * If the user didn't define a priority while configuring
		 * the aio kernel extension, it will be passed as NULL in
		 * the aio_cfg configuration. If it is NULL, we use the
		 * default of 39. If not, we set the priority according.
		 */
		if (aio_cfg.kprocprio)
			s_priority  = aio_cfg.kprocprio;

		/*
		 *  initialize queue table
		 */
		
		for (i = 0; i < QTABSIZ; ++i) {
			qtab[i].req_count = 0;
			qtab[i].head = NULL;
			qtab[i].tail = NULL;
			qtab[i].s_count = 0;
			lock_alloc(&qtab[i].lock,LOCK_ALLOC_PAGED, 
					AIO_QUEUE_LOCK,i);
			simple_lock_init(&qtab[i].lock);
		}

		/*
		 * Initialize global AIO simple locks
		 */
		lock_alloc(&aio_qlock, LOCK_ALLOC_PAGED, AIO_AIOQ_LOCK,-1);
		simple_lock_init(&aio_qlock);
		lock_alloc(&servers.lock,LOCK_ALLOC_PAGED, AIO_SERVERS_LOCK,-1);
		simple_lock_init(&servers.lock);
		lock_alloc(&knots.lock, LOCK_ALLOC_PIN, AIO_KNOTS_LOCK,-1);
		simple_lock_init(&knots.lock);
		lock_alloc(&suspenders.lock,LOCK_ALLOC_PIN, AIO_SUSP_LOCK,-1);
		simple_lock_init(&suspenders.lock);
		/*
		 * create initial unassigned servers
		 * we need to hold the server lock because the call
		 * to new_server() and add_server() require this lock
		 * being held.
		 */
		SERVERS_LOCK();
		for (i = 0; i < minservers; ++i)
			if (sp = new_server())
				add_server(sp);
		SERVERS_UNLOCK();		
		
		/*
		 *  Register close hook ...
		 */
		
		closeh.hook = aio_close_hook;
		fs_hookadd(&closeh, FS_CLOSEH_ADD);
		
		/*
		 *  ... and exec hook.
		 * They're the same function (but not the same
		 * fs_hook struct!) because we do the same thing
		 * at close and exec: cancel outstanding aio.
		 */
		
		fs_exech.hook = aio_close_hook;
		fs_hookadd(&fs_exech, FS_EXECH_ADD);

		old_state = AIO_CONFIGURING;
		while (!compare_and_swap(&aio_state,
			      &old_state, AIO_CONFIGURED));
		return 0;
	}
	else {
		/*
		 * We are not able to terminate the driver reliably,
		 * nor do anything other commands. We simply return
		 * EBUSY.
		 */
		return EBUSY;
	}
}

/***************************************************************************
 * NAME: arl_init
 *
 * FUNCTION: initialization routine for the LVM path
 *
 *
 * EXECUTION ENVIRONMENT: process level - this routine does not require
 *                        pinning and may page fault.
 *
 * RETURNS:
 *      0 - Successful
 *      ENOMEM - Out of memory
 *
 ***************************************************************************
 */

int
arl_init()
{
	int     i, rc;

	/* initialize the lvm_active lock         */
	lock_alloc(&lvm_lock,LOCK_ALLOC_PIN, AIO_LVM_LOCK,-1);
	simple_lock_init(&lvm_lock);

	/* initialize the dev_tab lock          */
	lock_alloc(&devtab_lock,LOCK_ALLOC_PAGED, AIO_DEVTAB_LOCK,-1);
	simple_lock_init(&devtab_lock);

	/* pin the routines that must execute at priority INTIODONE */
	rc = pincode( (int (*)()) arl_iodone);
	if (rc) {
		lock_free(&lvm_lock);
		lock_free(&devtab_lock);
		return rc;
	}

	/* Initialize the initial buf pool */
	freebufp = morebufs(STRATBUFS);
	freebufsize = STRATBUFS;    /* initial buf pool size    */

	/* Initialize the initial request pool */
	if (freebufp) {
		freereqp = morereqs(STRATREQS);
		freereqsize = STRATREQS;  /* initial request pool size    */
	}
	/* Initialize the initial device devid table */
	if (freebufp && freereqp) {
		devnop = moredevs();
	}
	/* check for out of memory      */
	if ( (!freebufp) || (!freereqp) || (!devnop) ) {
		lock_free(&lvm_lock);
		lock_free(&devtab_lock);
		(void) unpincode( (int (*)()) arl_iodone);
		 return ENOMEM;
	}


	/* Initialize the hash table for LVM requests           */
	for (i = 0; i < NHREQ; ++i) {
		lvmio[i].next = (request *) &lvmio[i];
		lvmio[i].prev = (request *) &lvmio[i];
	}

	return 0;
}

/* The request blocks are allocated individually since they must
 * be freed individually if the pool grows too big.
 */
struct request *
morereqs(int count)
{
	int     i, rc;
	struct request  *rp, *rpp = NULL;

	/* Allocate requests separately   */
	for (i = 0; i < count; ++i) {
		rp = (struct request *) xmalloc(sizeof(struct request),
							0, pinned_heap);
		if (rp ==  NULL) {
			/* free allocated reqs  */
			while (rpp) {
				rp = rpp->next;
				rc = xmfree((caddr_t) rpp, pinned_heap);
				ASSERT(rc == 0);
				rpp = rp;
			}
			return NULL;
		}
		else {  /* clear and chain with rp->next */
			rp->next = rpp;
			rpp = rp;
		}
	}
	return rp;
}

struct devtab *
moredevs()
{
	struct devtab *tabp;            /* pointer to devid # table */

	/* Allocate a device table chunk and initialize chain pointer */
	tabp = (struct devtab *) xmalloc(sizeof(struct devtab),
							 0, kernel_heap);
	if (tabp == (struct devtab *) NULL)
		return NULL;
	else
		bzero( (char *) tabp, sizeof(struct devtab));

	return tabp;
}
