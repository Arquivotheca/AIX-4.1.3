static char sccsid[] = "@(#)99  1.1.1.11  src/bos/kernext/aio/aio_syscalls.c, sysxaio, bos41J, 9523B_all 6/7/95 15:35:50";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: kaio_rdwr, ardwr, arl_rdwr, iosuspend, acancel, listio,
 *	      kaio_dump, kaio_debug
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

static int ardwr(int reqtype, int fildes, struct aiocb *ucbp,
		 struct aiocb *kcbp, lio_knot *knotp);

/* kaio_rdwr -- code common to aread and awrite
 *
 * copy the user's aiocb into kernel memory
 * call ardwr to actually queue the request
 */
int
kaio_rdwr(int reqtype,
	  int fildes,
	  struct aiocb *ucbp,	/* pointer to aiocb in user space */
	  int aio_type)		/* POSIX_AIO or SUN_AIO */
{
	struct aiocb kaiocb;

	if (!ucbp || copyin(ucbp, &kaiocb, sizeof kaiocb))
		return EFAULT;
	return ardwr(reqtype, fildes, ucbp, &kaiocb, NULL);
}

/* ardwr -- code common to aread, awrite, and listio
 *
 * initialize the request record
 * stick it in the queue
 */
static
int
ardwr(int reqtype,
      int fildes,
      struct aiocb *ucbp,       /* pointer to aiocb in user space */
      struct aiocb *kcbp,       /* pointer to in kernel copy of aiocb */
      lio_knot *knotp)          /* if this request has a knot associated */
{
	int             err = 0;
	int             i;
	struct file     *fp;
	pid_t           pid = getpid();         /* the originator's pid */
	tid_t           tid = thread_self();    /* the originator's tid */
	request         *rp = NULL;
	request         *nextrp;
	uint		new_offset = 0; /* our offset after the i/o */
	dev_t           dev;            /* lvdd device number           */
	uint            runpath;        /* AIO_LVM or AIO_KPROC         */
	struct devinfo  info;           /* LVM device information       */
	struct devtab   *devtabp;       /* LVM dev table pointer        */
	int             search = 1;     /* LVM dev table search flag    */

	/*
	 * Check to see what state the AIO extension is in. We
	 * will only proceed if the state is AIO_CONFIGURED.
	 */
	if (aio_state != AIO_CONFIGURED)
		return EBUSY;

	/*
	 * Check request type is either read or write
	 * if not, return EINVAL
	 */

	if ((reqtype != QREADREQ) && (reqtype != QWRITEREQ))
		return EINVAL;

	if (fp_getf(fildes, &fp)) {
		err = EBADF;
		goto exit;
	}

	/*
	 * If the file isn't open for what they want to do, complain.
	 */
	if ((fp->f_flag & (reqtype==QREADREQ ? FREAD : FWRITE)) == 0) {
		err = EBADF;
		goto exit1;
	}

	/*
	 * check that we aren't exceeding the request limit
	 */
	if (requestcount >= maxreqs) {
		err = EAGAIN;
		goto exit1;
	}

	/*
	 * validate the whence field
	 */
	if (kcbp->aio_whence != SEEK_SET &&
	    kcbp->aio_whence != SEEK_CUR &&
	    kcbp->aio_whence != SEEK_END) {
		err = EINVAL;
		goto exit1;
	}

	/*
	 * set fields in the aiocb and copy it back to user space
	 */
	kcbp->aio_fp = fp;
	kcbp->aio_handle = ucbp;
	kcbp->aio_return = 0;
	kcbp->aio_errno = EINPROG;
	if (copyout(kcbp, ucbp, sizeof(*ucbp))) {
		err = EFAULT;
		goto exit1;
	}

	runpath = AIO_KPROC;    /* set the default      */
	/* Determine if we can put this on the LVM path. */
	if ( (fp->f_type == DTYPE_VNODE) &&
	(fp->f_vnode->v_gnode->gn_type == VCHR) ) {
		dev = fp->f_vnode->v_gnode->gn_rdev;
		/* check the dev number table     */
		DEVTAB_LOCK();
		devtabp = devnop;      /* LVM dev table pointer        */
		while (search) {
			for (i = 0; i < devtabp->empty; ++i) {
				if ((devtabp->deventry[i].devid) == dev) {
				     if ((devtabp->deventry[i].devtype) ==
							       AIO_LVMDEV) {
					   /* switch the path   */
					   runpath = AIO_LVM;
				     }
				     DEVTAB_UNLOCK();
				     goto END_SEARCH;
				}
			}
			/* look at the next chunk      */
			if (devtabp->next)
				devtabp = devtabp->next;
			else
				search = 0;
		} /* end of LVM dev table search        */

		/* insert device number in the dev number table       */
		if ((fp_ioctl(fp, IOCINFO, (caddr_t) &info, 0)) == 0) {
			if (devtabp->empty == NUMVGS) {
				if (!devtabp->next)
					devtabp->next = moredevs();
				devtabp = devtabp->next;
			}
			devtabp->deventry[devtabp->empty].devid = dev;
			if (info.devsubtype == DS_LV) {
				/* LVM device           */
				runpath = AIO_LVM;
				devtabp->deventry[devtabp->empty].devtype
							 = AIO_LVMDEV;
			}
			else { /* not an LVM device       */
				devtabp->deventry[devtabp->empty].devtype
							 = AIO_OTHDEV;
			}
			devtabp->empty += 1; /* this table grows */
		}
		DEVTAB_UNLOCK();
	}

	END_SEARCH:
	/*
	 * allocate and initialize the request struct
	 */
	rp = getreq();
	if (!rp) {
		err = ENOMEM;
		goto exit1;
	}
	DPRINTF(("request block at: %x\n", rp));

	/* Initialize the request structure     */
	bzero((char *) rp, sizeof(request));
	bcopy((char *) kcbp, (char *) &rp->kaiocb, sizeof(struct aiocb));
	rp->pid      = pid;
	rp->tid      = tid;
	rp->fp       = fp;
	rp->fd       = fildes;
	rp->reqtype  = reqtype;
	rp->aiocbp   = ucbp;
	rp->knotp    = knotp;
	rp->runpath = runpath;

	/*
	 * xmattach to the aiocb and buf so they don't go away and the
	 * server will be able to access them.
	 * These are both xmdetached in delete_request() once the server
	 * has completed the request, or the request was canceled prior to
	 * the server executing the request.
	 */

	/* make user buffer accessable for data transfer
	 */
	rp->bufd.aspace_id = XMEM_INVAL;
	if (xmattach((char *)rp->kaiocb.aio_buf, rp->kaiocb.aio_nbytes,
			     &rp->bufd, USER_ADSPACE) != XMEM_SUCC) {
		err = EINVAL;
		releasereq(rp);
		goto exit1;
	}

	rp->aiocbd.aspace_id = XMEM_INVAL;
	if (xmattach((char *)ucbp, sizeof(struct aiocb), &(rp->aiocbd),
		     USER_ADSPACE) != XMEM_SUCC) {
		err = EINVAL;
		xmdetach(&(rp->bufd));
		releasereq(rp);
		goto exit1;
	}

	/* if we are doing AIO on a socket or a pipe,
	 * offsets are not maintained in the file
	 * table entry.
	 */
	if (fp->f_type == DTYPE_SOCKET ||
	    (fp->f_type == DTYPE_VNODE &&
	     fp->f_vnode->v_vntype == VFIFO)) {
		err = 0;
		setuerror(0);
		rp->kaiocb.aio_offset = 0;
		goto no_offset;
	}

	/* set the offset in the file table entry as 
	 * though the request completed. For reads and
	 * writes past the 2GB file limit, uphsyio()
	 * partially transfers bytes up to 2G and
	 * therefore the offset should be placed
	 * exactly at 2GB.
	 */
	if (fp->f_type == DTYPE_VNODE) {
		switch(kcbp->aio_whence)
		{
			case SEEK_SET:
			    new_offset = kcbp->aio_offset + kcbp->aio_nbytes;
			    break;

			case SEEK_CUR:
			    new_offset = fp->f_offset + kcbp->aio_offset +
					 kcbp->aio_nbytes;
			    break;

			case SEEK_END:
			{
			    struct stat statp;
			    err = fp_fstat(fp, &statp, 0, 0);
			    if (err == 0)
				new_offset = statp.st_size + kcbp->aio_offset +
					     kcbp->aio_nbytes;
			    else
			    {
				xmdetach(&(rp->aiocbd));
				xmdetach(&(rp->bufd));
				releasereq(rp);
				goto exit1;
			    }
			}
			    break;
		}

		/* trim the number of bytes 
		 * to the 2G limit. partial 
		 * transfer of bytes less than 2G.
		 */
		if (new_offset > OFF_MAX) {
			kcbp->aio_nbytes = new_offset - kcbp->aio_offset;
			new_offset = OFF_MAX + 1;
		}

		/* set the offset in the request
		 */
		rp->kaiocb.aio_offset = new_offset - rp->kaiocb.aio_nbytes;

		/* set the offset in the file table
		 */
		fp->f_offset = new_offset;
		fp->f_dir_off = 0;
	}
	else {
		xmdetach(&(rp->aiocbd));
		xmdetach(&(rp->bufd));
		releasereq(rp);
		err = EINVAL;
		goto exit1;
	}

no_offset:

	/*
	 * note that we've done aio on this fp
	 */
	FILE_LOCK(rp->fp);
	rp->fp->f_flag |= FAIO;
	FILE_UNLOCK(rp->fp);

	/* check for requests targeted to LVM character special files */
	if (rp->runpath == AIO_LVM)  {
		rp->kaiocb.aio_errno = 0;  /* initialize for completion  */
		rp->inprog = STARTED;   /* request never waits in a queue*/
		if ( (err = arl_rdwr(rp, &dev)) == 0) {
			/* the request was successfully submitted       */
			goto exit1;
		}
		/* error other than ENOMEM  */
		else if (err != ENOMEM) {
			xmdetach(&(rp->aiocbd));
			xmdetach(&(rp->bufd));
			releasereq(rp);
			goto exit1;
		}
		else { /* ENOMEM error, maybe kproc can complete it     */
			rp->kaiocb.aio_errno = EINPROG;
			err = 0;
			rp->runpath = AIO_KPROC;
		}
	}

	/*
	 * we increment the global request count right before
	 * add_request. By this time, there are no errors with
	 * enqueuing the request so we don't have to backtrack.
	 */
	fetch_and_add(&requestcount, 1);

	rp->inprog = QUEUED;
	add_request(rp);

      exit1:
	ufdrele(fildes);
      exit:
	if (err) {
		if (err != EFAULT) {
			kcbp->aio_return = -1;
			kcbp->aio_errno = err;
			/*
			 * We may be here because of an error that occurred
			 * before the user's aiocb was updated (in
			 * particular, before the aio_handle was set).
			 */
			kcbp->aio_handle = ucbp;
			if (copyout(kcbp, ucbp, sizeof(*ucbp)))
				err = EFAULT;
		}
	}

	return err;
}

/***************************************************************************
 * NAME: arl_rdwr
 *
 * FUNCTION: initialization routine for the LVM path
 *
 *
 * EXECUTION ENVIRONMENT: process level - this routine does not require
 *                        pinning and may page fault.
 *
 * RETURNS:
 *
 *      EINVAL - invalid request parameter
 *      ENOMEM - could not obtain bufs, or request too big
 *      EFAULT - invalid address range
 *
 ***************************************************************************
 */
int arl_rdwr(request *rp, dev_t *devp) {

	int     i,              /* index for bufs                       */
		blkwt,          /* block offset within an LTG           */
		startblk,       /* first block in the request           */
		blktotal,       /* total number of blocks for this request*/
		rc, err;        /* return code                          */
	uint    bcount;         /* transfer count remaining             */
	caddr_t baddr;          /* b_addr value for buf                 */
	caddr_t pinfail_addr;   /* Set if unable to pin user buffer     */
	off_t   offset;         /* device block offset                  */
	struct  buf   *bp, *abp;  /* pointer to bufs   */
	struct  buf   *nbp;      /* for updating buf chain              */

	/* check that request is a multiple of blocks on the device     */
	if (rp->kaiocb.aio_nbytes & DBSIZE-1)   /*  from LVM mincnt */
		return EINVAL;

	/* check that the request starts on a 512-byte boundary. uphysio()
	 * tosses the rightmost bits on the synchronous path.
	 */
	if (rp->kaiocb.aio_offset & (UBSIZE-1) ) {
		return EINVAL;
	}

	/* Calculate the number of bufs needed for this request. At least
	 * one buf, and one more than the number of LTG boundary crossings.
	 */
	blktotal = BYTE2BLK(rp->kaiocb.aio_nbytes);  /* # of .5K blocks */
	startblk = BYTE2BLK(rp->kaiocb.aio_offset);  /* 1st block       */
	blkwt =  startblk & (BLKPTRK-1);          /* offset in first LTG*/
	rp->buf_cnt = 1 + ((blkwt+blktotal-1)/(BLKPTRK));
	DPRINTF(("bufs needed (rp->buf_cnt = %d\n", rp->buf_cnt));

	/* RS hardware needs IOCC cache consistency protection for i/o
	   reads from different adapters to adjacent memory which is not
	   aligned on a cache line boundary (64-bytes) */
	if ( ((unsigned int) rp->kaiocb.aio_buf & 0x3f)  && __power_rs() &&
	(rp->reqtype == QREADREQ) ) {
		/* If can't serialize, do this on the kproc path   */
		DPRINTF(("serialized request\n"));
		if (rp->buf_cnt > 1)
			return ENOMEM;
	}

	/* pin the user-space aiocb for update from the iodone routine  */
	err = xmempin((caddr_t) rp->aiocbp, sizeof(struct aiocb), &rp->aiocbd);
	if (err)
		return err;

	/* Get the bufs and add request to the list of active LVM requests*/
	if ((abp = fetchbufs(rp)) == NULL) {
		rc = xmemunpin((caddr_t) rp->aiocbp, sizeof(struct aiocb),
							&rp->aiocbd);
		ASSERT(rc == 0);

		return ENOMEM;
	}
	DPRINTF(("chain of bufs at: abp = %x\n", abp));

	/* initialize buf setup variables       */
	bp = abp;                               /* first buf    */
	baddr = (caddr_t) rp->kaiocb.aio_buf;
	bcount = rp->kaiocb.aio_nbytes;
	offset = rp->kaiocb.aio_offset;         /* absolute offset      */

	/* loop until user buffer data is transferred to bufs */
	while(bp) {
		bp->b_flags = B_BUSY | B_MPSAFE;
		bp->b_flags |= (rp->reqtype == QREADREQ ?
							B_READ: B_WRITE);
		bp->b_iodone = arl_iodone;
		bp->b_vp = (struct vnode *) NULL;
		bp->b_dev = *devp;
		bp->b_error = 0;
		bp->b_resid = 0;
		bp->b_work = 0;
		bp->b_options = 0;
		bp->b_event = NULL;
		bp->b_start.tv_sec = 0;
		bp->b_start.tv_nsec = 0;
		bp->b_xmemd = rp->bufd;

		/* Duplicate what is done by the lvm mincnt routine. The
		 * request cannot cross Logical Track Group boundaries.
		 */
		bp->b_baddr = baddr;
		bp->b_blkno = (daddr_t) offset >> UBSHIFT; /* block number*/
		bp->b_forw = (struct buf *)rp; /* iodone needs this*/
		if (!bp->av_forw) { /* last buf         */
			bp->b_bcount = bcount;
			/* Initialize for transfer count calculation. If
			 * no error, then following values will give the
			 * correct transfer count. If error or all data
			 * not transferred, these values are reset in the
			 * iodone routine.
			 */
			rp->b_bcount = bcount;
			rp->b_blkno = bp->b_blkno;
		}
		else { /* all bufs except the last        */
			bp->b_bcount = BLK2BYTE(BLKPTRK-blkwt);
			/* update varbs for the next buf  */
			baddr += bp->b_bcount;
			offset += bp->b_bcount;
			bcount -= bp->b_bcount;
			blkwt = 0;      /* always true when i>0 */
		}

		/* pin the user-space buffer */
		err = xmempin(bp->b_baddr, bp->b_bcount, &(rp->bufd));
		/* check for error on the pin   */
		if (err) {
			DPRINTF(("error on buffer pin=%d\n",err));
			/* unpin user-space aiocb       */
			rc = unpinu((caddr_t) rp->aiocbp,
				    sizeof(struct aiocb), UIO_USERSPACE);
			ASSERT(rc == 0);
			pinfail_addr = bp->b_baddr;
			/* free up pinned stuff                 */
			bp = abp;       /* head of chain        */
			while (bp) {
				/* unpin user-space buffer      */
				if (bp->b_baddr < pinfail_addr) {
					rc = xmemunpin(bp->b_baddr,
						   bp->b_bcount,&(rp->bufd));
					ASSERT(rc == 0);
				}
				nbp = bp->av_forw;
				if (err == ENOMEM) {
					/* free the buf       */
					rc = xmfree(bp, pinned_heap);
					ASSERT(rc == 0);
				}
				else {
					/*return buf to the pool */
					releasebuf(bp);
				}
				bp = nbp; /* next in the chain  */
			}
			rc = xmdetach(&(rp->bufd));
			ASSERT(rc == XMEM_SUCC);

			/* remove the request from the LVM active queue */
			remque(rp);

			return err;
		}
		bp = bp->av_forw; /* next unused buf     */

	} /* another buf completed      */

	/* update counter for physical read or write    */
	if (rp->reqtype == QREADREQ) {
		fetch_and_add(&sysinfo.phread, 1);
	}
	else {  /* rp->reqtype == QWRITEREQ  */
		fetch_and_add(&sysinfo.phwrite, 1);
	}

	err = devstrat(abp);	/* send request to LVM  */
	ASSERT(err == 0);	/* only error is ENODEV */
	return(0);

} /* end arl_rdwr       */

/*
 * iosuspend -- block waiting on completion of one of a group of aios
 *
 * Block until one or more of the aios referred to by the aiocbpa
 * is finished (successfully or not).  Return its index (if more than
 * one is finished, return any of them.)  As a special case, if one is
 * finished at the time of the call, return immediately without blocking.
 */

int
iosuspend(int cnt, struct aiocb *aiocbpa[])
{
	struct aiocb  **cbpa;
	int		err;
	int		rc;
	ulong		event;
	pid_t		pid = getpid();
	tid_t           tid = thread_self();    /* the suspendor's tid */
	suspender      *susp;
	int		cba_index;
	ulong		gen;
	
	if (aio_state != AIO_CONFIGURED) {
		setuerror(EBUSY);
		return -1;
	}
	/*
	 * Make sure the number of control blocks does not
	 * exceed the maximum number of requests we can have
	 * outstanding at any time in the system(64,000).
	 */
	if (cnt > MAXSYSREQ) {
		setuerror(EINVAL);
		return -1;
	}

	GEN(gen);	/* get a generation number for this iosuspend */

	if (!(susp = create_suspender(pid, tid, gen))) {
		setuerror(ENOMEM);
		return -1;
	}

	/*
	 * Get our own copy of the array of pointers.
	 */
	if (!(cbpa = (struct aiocb **)malloc(cnt * sizeof(struct aiocb *)))) {
		remove_suspender(susp);
		rc = xmfree(susp, pinned_heap);
		ASSERT(rc == 0);
		setuerror(ENOMEM);
		return -1;
	}
	else if (rc = copyin(aiocbpa, cbpa, cnt * sizeof(struct aiocb *))) {
		remove_suspender(susp);
		err = xmfree(susp, pinned_heap);
		ASSERT(err == 0);
		err = free(cbpa);
		ASSERT(err == 0);
		setuerror(rc);  /* rc = EIO, EFAULT, ENOMEM, or ENOSPC  */
		return -1;
	}

	if ((rc = suspend_set(cnt, cbpa, pid, susp)) != SS_OK)
	{
		/*
		 * suspend_set either:
		 *	didn't find anything,
		 *	found one that was finished, or
		 *	failed
		 */
		switch (rc) {
		      case SS_ALL_NULL:
			/*
			 * in this case no bits were set
			 * we still need to delete the suspender,
			 * but susp->aiocbp s/b NULL
			 */
			remove_suspender(susp);
			ASSERT(susp->aiocbp == NULL);
			setuerror(EINVAL);
			rc = -1;
			break;

		      default:
			/* At least one was already finished or
			 * the request was not found on any of the queues.
			 * We don't need to do anything in this case.
			 * The return code indicates which one completed
			 * or which one didn't have a matching request.
			 * Note that this is the only path for which the
			 * suspender has already been deleted
			 */
			if (susp->aiocbp) {
				DPRINTF(("iosuspend: discarding event\n"));
				event=et_wait(EVENT_NDELAY,EVENT_ASUSPEND,0);
				DPRINTF(("iosuspend: event discarded\n"));
			}
			break;
		}
		err = xmfree(susp, pinned_heap);
		ASSERT(err == 0);
		err = free(cbpa);
		ASSERT(err == 0);
		return rc;
	}

	/*
	 * rc == SS_OK -- now we wait for some i/o to complete
	 */
	if ((event = et_wait(EVENT_ASUSPEND, EVENT_ASUSPEND, EVENT_SIGRET))
	    == EVENT_SIG) {
		/*
		 * our wait was interrupted by a signal
		 */
		remove_suspender(susp);
		if (susp->aiocbp) {
			DPRINTF(("iosuspend: discarding event\n"));
			event = et_wait(EVENT_NDELAY, EVENT_ASUSPEND, 0);
			DPRINTF(("iosuspend: event discarded\n"));
		}
		err = xmfree(susp, pinned_heap);
		ASSERT(err == 0);
		err = free(cbpa);
		ASSERT(err == 0);
		setuerror(EINTR);
		return -1;
	}

	/*
	 * we awakened because some i/o completed
	 */
	remove_suspender(susp);
	for (cba_index = 0; cba_index < cnt; ++cba_index)
		if (cbpa[cba_index] == susp->aiocbp) {
			rc = cba_index;
			goto exit;
		}

	PANIC("iosuspend: completed aiocb not found");
	/*
	 * if the panic is only with DEBUG on, we can't
	 * just stumble through if it's off
	 * perhaps we should disable aio if we find our way here
	 */
	rc = -1;
	setuerror(EINVAL);

      exit:
	err = xmfree(susp, pinned_heap);
	ASSERT(err == 0);
	err = free(cbpa);
	ASSERT(err == 0);
	return rc;
} /* end iosuspend() */

int
acancel(int fildes, struct aiocb *aiocbp)
{
	pid_t		pid = getpid();
	struct file    *fp;
	int 		rc;
	
	if (aio_state != AIO_CONFIGURED) {
		setuerror(EBUSY);
		return -1;
	}

	if (fp_getf(fildes, &fp)) {
		setuerror(EBADF);
		return -1;
	}

	if (aiocbp)
		rc = cancel_request(pid, fp, fildes, aiocbp, NULL, NULL);
	else
		rc = cancel_fd(pid, fp, fildes, 0);

	ufdrele(fildes);
	return rc;
}

int
listio(int cmd, struct liocb *list[], int nent, struct sigevent *eventp)
{
	struct liocb  **klist;
	struct liocb	lcb;
	lio_knot       *lkp = NULL;
	int		count = nent;	/* count of active entries */
	int             i;
	int		rc = 0;
	ulong		event;		/* what awakened us? */
	int		this_rc;
	int		eagain_count = 0;
	
	if (aio_state != AIO_CONFIGURED)
		return EBUSY;

	if (cmd == LIO_ASYNC) {
		setuerror(ENOSYS);
		return -1;
	}

	if (cmd != LIO_WAIT && cmd != LIO_NOWAIT && cmd != LIO_ASIG) {
		setuerror(EINVAL);
		return -1;
	}

	if (nent > AIO_LISTIO_MAX_SIZE) {
		setuerror(EINVAL);
		return -1;
	}
	
	/*
	 * Get our own copy of the list.
	 */
	if (!(klist = (struct liocb **)malloc(nent * sizeof(struct liocb *))))
	{
		setuerror(EAGAIN);
		return -1;
	}

	if (copyin(list, klist, nent * sizeof(struct liocb *))) {
		rc = EAGAIN;
		goto exit;
	}

	/*
	 * Allocate and initialize a knot if necessary.
	 */
	if (cmd != LIO_NOWAIT) {
		lkp = create_knot(cmd, nent);
		if (!lkp) {
			rc = EAGAIN;
			goto exit;
		}
	}

	for (i = 0; i < nent; ++i) {
		/*
		 * Get our own copy of the control block.
		 * If the copyin fails we return EFAULT immediately.
		 */
		if (copyin(klist[i], &lcb, sizeof(struct liocb))) {
		    /* It is guaranteed that knot still exists if it
		     * was created at all (if lkp != NULL).
		     */
		    if (lkp) {
			/* not going to wait */
			(void) slip_knot(lkp);
			/* This request and all following in the list will
			 * not be submitted.
			 */
			if (untie_knot(lkp, nent-i)) {
				lkp = NULL;
			}
			/* clear the event word in case this thread was
			 * posted before slip_knot
			 */
			(void) et_wait(EVENT_NDELAY,EVENT_ASUSPEND,0);
		    }
		    rc = EFAULT;
		    goto exit;
		}

		/*
		 * If it's a nop
		 *	extract it from the (possible) knot
		 *	and skip to the next cb.
		 * Note that the knot cannot go away prematurely
		 * (e.g., from the kproc's completing some i/o)
		 * because we initialized it's count field to the
		 * length of the list, so it can't go to zero
		 * before we've queued everything.
		 */
		if (lcb.lio_opcode == LIO_NOP) {
			if (lkp) {
				if (untie_knot(lkp, 1))
					lkp = NULL;
			}
			continue; /* all done with this lio entry       */
		}
		/*
		 * Untie and decrement as above if the queuing fails.
		 */
		if ( this_rc = ardwr(lcb.lio_opcode, lcb.lio_fildes,
			     (struct aiocb *)( (char *)klist[i] +
			     offsetof(struct liocb, lio_aiocb) ),
			     (struct aiocb *)(&(lcb.lio_aiocb)), lkp) ) {
			/* synchronous error, request not submitted.
			 * If the knot existed before the ardwr() call,
			 * then it must still exist. It could not have
			 * been deleted since the count could not go to 0*/
			if (lkp) {
				if (untie_knot(lkp, 1))
					lkp = NULL;
			}
			if (this_rc == EAGAIN)
				++eagain_count;
		}
	} /* end of loop to submit requests  */

	/*
	 * If it's LIO_WAIT and the knot still exists, then wait. The
	 * knot may have been deleted because a synchronous error was
	 * returned by ardwr() or an LIO_NOP on the request that
	 * decremented the knot counter to zero.
	 */
	if (cmd == LIO_WAIT)  {
	    if (lkp) {
		if ((event = et_wait(EVENT_ASUSPEND, EVENT_ASUSPEND,
				    EVENT_SIGRET)) == EVENT_SIG) {
			/*
			 * our wait was interrupted by a signal. Flag
			 * so that the last request completing will not
			 * post this thread on completion.
			 */
			slip_knot(lkp);
			/* There will not be a post now, but the post
			 * event may have occurred between the signal
			 * and the flagging of the knot. Need to clear
			 * the event word again since we don't know.
			 */
			(void) et_wait(EVENT_NDELAY,EVENT_ASUSPEND,0);
			rc = EINTR;
			goto exit;
		}
	    }
	    /*
	     * If here, either the last request to complete was a LIO_NOP
	     * or it was completed by a kproc or iodone routine.
	     * Look for errors set by kproc or iodone
	     */
	    for (i = 0; i < nent; ++i) {
		/*
		 * Get our own copy of the control block.
		 * If the copyin fails, we return EFAULT.
		 */
		if (copyin(klist[i], &lcb, sizeof(struct liocb))) {
			rc = EFAULT;
			goto exit;
		}
		/*
		 * skip nops
		 */
		if (lcb.lio_opcode == LIO_NOP)
			continue;

		/*      ASSERT(lcb.lio_aiocb.aio_errno != EINPROG); */
		/*
		 * if this aio failed, return EIO w/o looking further
		 */
		if (lcb.lio_aiocb.aio_return == -1) {
			rc = EIO;
			break;
		}
	    } /* end of error search    */
	} /* end of LIO_WAIT completion */

      exit:
	free(klist);

	/* if any of the requests weren't queued because of resource
	 * limitations, let the caller know
	 */
	if (eagain_count) {
		setuerror(EAGAIN);
		return -1;
	}
	if (rc) {
		setuerror(rc);
		return -1;
	}
	return 0;

}       /* end of listio()      */
