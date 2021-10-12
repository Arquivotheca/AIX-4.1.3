static char sccsid[] = "@(#)27        1.63  src/bos/kernext/pse/str_scalls.c, sysxpse, bos41J, 9523C_all 6/9/95 12:12:26";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      osr_alloc
 *                 osr_free
 *                 str_open_init
 *                 str_open_term
 *                 sth_test_and_set_sth
 *                 pse_open
 *                 osr_open
 *                 osr_add_modules
 *                 pse_close
 *                 osr_close_subr
 *                 pse_read
 *                 pse_write
 *                 pse_ioctl
 *                 pse_select
 *                 pse_revoke
 *                 drv_priv
 *                 
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/** Copyright (c) 1989-1991  Mentat Inc. **/

#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/poll.h>

#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <pse/str_ioctl.h>
#include <sys/trchkid.h>

#define hkwd_pse_revoke_in      0x17
#define hkwd_pse_revoke_out     0x18

#include <sys/sleep.h>
#include <sys/stropts.h>
#include <sys/termio.h>
#include <stddef.h>		/* for offsetof() */

/*
 * The initial open of the console will need
 * at least a simple line discipline!
 */
const static struct strapush console_apush =
	{ SAP_ONE, 0, 0, 0, 1, { "ldterm" } };

extern	struct streamtab sthinfo;
extern void osrq_term();

OSRP
osr_alloc (sth, size, pri)
	STHP sth;
	int  size, pri;
{
	OSRP	osr = nilp(OSR);

again:
	if (sth && size == 0) {
		if (test_and_set((long*)&(sth->sth_ext_flags), F_STH_OSR_INUSE)) {
			osr = &sth->sth_osr;
		}
	}
	if ( !osr ) 
               osr = (OSRP) xmalloc(sizeof *osr + size, 2, pinned_heap);
	if (osr) {
		bzero((caddr_t) osr, sizeof(*osr) + size);
		osr->osr_sq.sq_sleeper = EVENT_NULL;
		osr->osr_sth = sth;
		osr->osr_sleeper = EVENT_NULL;
	} else {
		thread_tsleep(HZ, NULL, NULL);
		goto again;
	}
	return osr;
}

#ifdef	__GNUC__
__inline__
#endif
void
osr_free(osr)
	OSRP	osr;
{
#if	MACH_ASSERT
	if (!osr->osr_sth)
		panic("osr_free sth");
#endif
	if (osr->osr_creds) crfree(osr->osr_creds);
	if (osr == &osr->osr_sth->sth_osr) {
#if	MACH_ASSERT
		if (!(osr->osr_sth->sth_ext_flags & F_STH_OSR_INUSE))
			panic("osr_free sth_ext_flags");
#endif
		fetch_and_and((long *)&(osr->osr_sth->sth_ext_flags),
			      				~F_STH_OSR_INUSE);
	} else
               xmfree(osr, pinned_heap);
}

/*
 * We need to keep track of active streams for finding
 * abandoned streams under links. We now keep all of
 * them on these lists, but we could just keep ones
 * which we didn't close due to F_STH_LINKED.
 */

#define	STH_HASH(dev)		&sth_open_streams\
	[((unsigned)major(dev) ^ (unsigned)minor(dev)) % STH_HASH_TBL_SIZE]

STHP	sth_open_streams[STH_HASH_TBL_SIZE];
simple_lock_data_t streams_open_lock = {SIMPLE_LOCK_AVAIL};

void
str_open_init ()
{
	lock_alloc((&streams_open_lock), LOCK_ALLOC_PIN, PSE_OPEN_LOCK, -1);
	simple_lock_init(&streams_open_lock);
}

void
str_open_term()
{
        lock_free(&streams_open_lock);
}

int
sth_test_and_set_sth(dev, cloning, sthp, reopen)
reg	dev_t	dev;
	int	cloning;
	STHPP	sthp;
	int *	reopen;
{
	int	error = 0;
	int	indx;
	STHP	sth;
	STHPP	tmpsthp;
	queue_t * q = nilp(queue_t);
	struct streamtab * str;
	DISABLE_LOCK_DECL

	error = 0;
	*sthp = NULL;
	*reopen = FALSE;
	if (dev == NODEV)
		return 0;
	if (!cloning) {
		DISABLE_LOCK(&streams_open_lock);
		tmpsthp = STH_HASH(dev);
		for (sth = *tmpsthp; sth; sth = sth->sth_next)
			if (sth->sth_dev == dev)
				break;
		if (sth) {
			/* Check for races. Not much to do but fail. */
			if (sth->sth_flags & F_STH_CLOSING) {
				error = EINPROGRESS;
			} else {
				sth->sth_open_count++;
				*sthp = sth;
				*reopen = TRUE;
			}
			DISABLE_UNLOCK(&streams_open_lock);
			goto out;
		}
		DISABLE_UNLOCK(&streams_open_lock);
	}
	if ((indx = dcookie_to_dindex(major(dev))) < 0 ) {
		error = ENODEV;
		goto out;
	}
	if (!(str = dindex_to_str(indx))
	    ||  !str->st_rdinit
	    ||  !str->st_rdinit->qi_qopen
	    ||  !modsw_ref(str->st_rdinit, 1)) {
		error = ENXIO;
		goto out;
	}
	if (!(sth = sth_alloc())
	    ||  !(sth->sth_rq = q_alloc())
	    ||  !(q = q_alloc())) {
		if ( sth ) {
			if ( sth->sth_rq )
				q_free(sth->sth_rq);
			sth_free(sth);
		}
		(void) modsw_ref(str->st_rdinit, -1);
		error = EAGAIN;
		goto out;
	}
	sth->sth_wq = WR(sth->sth_rq);
	sth->sth_wq->q_sqh.sqh_parent = &sth->sth_rq->q_sqh;
	sth_set_queue(sth->sth_rq, sthinfo.st_rdinit, sthinfo.st_wrinit);
	noenable(sth->sth_wq);
	sth->sth_rq->q_ptr = (caddr_t)sth;
	sth->sth_wq->q_ptr = (caddr_t)sth;
	sth->sth_close_wait_timeout = 15000L;
	sth->sth_read_mode = RNORM | RPROTNORM;
	sth->sth_write_mode = SNDZERO;

	sth->sth_open_ev = EVENT_NULL;

	osrq_init(&sth->sth_read_osrq);
	osrq_init(&sth->sth_write_osrq);
	osrq_init(&sth->sth_ioctl_osrq);

	/*
	 * Initialize and set flow control pointers and linkage in
	 * the new queue and the Stream head.
	 */
	(void) sqh_set_parent(q, str);
	(void) sqh_set_parent(WR(q), str);
	sth_set_queue(q, str->st_rdinit, str->st_wrinit);

	q->q_ffcp = sth->sth_rq;
	q->q_bfcp = sth->sth_rq->q_bfcp;
	sth->sth_rq->q_bfcp = q;
	WR(q)->q_ffcp = nilp(queue_t);
	WR(q)->q_bfcp = sth->sth_wq;
	sth->sth_wq->q_ffcp = WR(q);
		q->q_next = sth->sth_rq;
	WR(q)->q_next = nilp(queue_t);
	sth->sth_wq->q_next = WR(q);

	DISABLE_LOCK(&streams_open_lock);
	if (!cloning) {
		STHP	sth2;

		for (sth2 = *tmpsthp; sth2; sth2 = sth2->sth_next)
			if (sth2->sth_dev == dev)
				break;
		if (sth2) {
			/* Check for races. Not much to do but fail. */
			if (sth2->sth_flags & F_STH_CLOSING) {
				error = EINPROGRESS;
			} else {
				sth2->sth_open_count++;
				*sthp = sth2;
				*reopen = TRUE;
			}
			DISABLE_UNLOCK(&streams_open_lock);
			(void) modsw_ref(RD(sth->sth_wq->q_next)->q_qinfo, -1);
			q_free(RD(sth->sth_wq->q_next));
			q_free(sth->sth_rq);
			sth_free(sth);
			goto out;
		}
		sth->sth_open_count++;
		sth->sth_next = *tmpsthp;
		sth->sth_dev = dev;
		*sthp = *tmpsthp = sth;
	} else {
		sth->sth_open_count++;
		*sthp = sth;
	}
	DISABLE_UNLOCK(&streams_open_lock);
out:
	return error;
}

void
sth_remove_sth(dev, sth)
	dev_t	dev;
	STHP	sth;
{
	STHPP	sthp;
	DISABLE_LOCK_DECL

	if (((STHP)devtosth(dev) == sth) || (sth->sth_flags & F_STH_CLOSED)) {
		DISABLE_LOCK(&streams_open_lock);
		sth->sth_open_count--;
		DISABLE_UNLOCK(&streams_open_lock);
		return;
	}
	DISABLE_LOCK(&streams_open_lock);
	if ((--sth->sth_open_count)) {
		DISABLE_UNLOCK(&streams_open_lock);
		return;
	}
	sthp = STH_HASH(sth->sth_dev);
	if (*sthp == sth)
		*sthp = sth->sth_next;
	else for ( ; *sthp; sthp = &((*sthp)->sth_next)) {
		if ((*sthp)->sth_next == sth) {
			(*sthp)->sth_next = sth->sth_next;
			break;
		}
	}
	DISABLE_UNLOCK(&streams_open_lock);
	(void) modsw_ref(RD(sth->sth_wq->q_next)->q_qinfo, -1);

	/*
	 * Unset flow control pointers and link between
	 * the new queue and the Stream head.
	 */
	sth->sth_rq->q_bfcp = nilp(queue_t);
	sth->sth_rq->q_ffcp = nilp(queue_t);
	sth->sth_wq->q_bfcp = nilp(queue_t);
	sth->sth_wq->q_ffcp = nilp(queue_t);
	sth->sth_wq->q_next->q_bfcp = nilp(queue_t);
	RD(sth->sth_wq->q_next)->q_bfcp = nilp(queue_t);
	RD(sth->sth_wq->q_next)->q_ffcp = nilp(queue_t);

	q_free(RD(sth->sth_wq->q_next));
	q_free(sth->sth_rq);
	sth_free(sth);
}

/*
 * pse_open - normal open of a streams device.
 *
 * This level distinguishes the various open cases
 *
 *		first open of a new device
 *		reopen of an already open device
 *
 */

/* ARGSUSED */
int
pse_open (dev, flags, private, newdev)
        dev_t   dev;
        int     flags;
        void    **private;
        dev_t   *newdev;        /* ignored for non-clones */
{
	OSRP	osr;
	STHP	sth;
	int	reopen, error = 0;
	DISABLE_LOCK_DECL
	extern int osr_add_modules();

	TRCHKL4T(HKWD_PSE | hkwd_pse_open_in, dev, flags, *private, newdev);

        ENTER_FUNC(pse_open, dev, flags, *private, newdev, 0, 0);
	
	if (error = sth_test_and_set_sth(dev, FALSE, &sth, &reopen))
		goto out;

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_open_dev    = dev;
	osr->osr_open_fflag  = flags;
	osr->osr_creds = crref();
	if (flags & O_NDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (flags & O_NONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;
	
	if (reopen) {
		/*
		 * 125628 against bos410
		 * F_STH_HANGUP  and F_STH_WRITE_ERROR are only reset, if we
		 * receive a close.
		 * If a tty is disconnected, and at least one process
		 * continues to hold the tty open, then when the tty
		 * is reconnected the new getty is unable to write the
		 * is reconnected the new getty is unable to write the
		 * login prompt.
		 */
		sth->sth_flags &= ~(F_STH_HANGUP | F_STH_WRITE_ERROR);
	}
	
	/*
	 * open the driver
	 */
	if (error = osr_open(osr)) {
		osr_free(osr);
		sth_remove_sth(dev, sth);
		goto out;
	}

	/*
	 * if module must be pushed and are not already pushed
	 * push it.
	 */
	if (error = osr_add_modules(osr)) {
		osr_free(osr);
		(void)pse_close((dev_t)-1,sth);
		goto out;
	}
	/*
	 * if no error register the stream
	 */
	if (error == 0) {
		sth->sth_flags &= ~F_STH_CLOSED;
		/*
		 * While we were opening the stream, someone might have
		 * We then have to do some special business...
		 */
		if (sth->sth_flags & F_STH_ISATTY) {
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
			error = sth_ttyopen(sth, flags);
			csq_release(&sth->sth_rq->q_sqh);
			if (error) {
				osr_free(osr);
				(void)pse_close((dev_t)-1, sth);
				goto out;
			}
		}
		if (error = devsetsth(dev, sth)) {
			osr_free(osr);
			(void)pse_close((dev_t)-1,sth);
			goto out;
		}
		DB_isopen(sth);
		DB_check_streams("OPEN");
		osr_free(osr);
		DISABLE_LOCK(&streams_open_lock);
		sth->sth_open_count--;
		DISABLE_UNLOCK(&streams_open_lock);
	}
out:
	TRCHKL1T(HKWD_PSE | hkwd_pse_open_out, error);
	LEAVE_FUNC(pse_open, error);
	return error;
}

int
osr_open (osr)
reg	OSRP	osr;
{
	queue_t * q;
reg	STHP	sth;
	struct open_args open_args;
	int	error = 0;

	ENTER_FUNC(osr_open, osr, 0, 0, 0, 0, 0);
	
	sth = osr->osr_sth;
	q = sth->sth_wq;
	while (!STREAM_END(q)) {
		q = q->q_next;
	}

	/*
	 * Open the driver.
	 */
	open_args.a_func  = RD(q)->q_qinfo->qi_qopen;
	open_args.a_queue = RD(q);
	open_args.a_devp  = &osr->osr_open_dev;
	open_args.a_fflag = osr->osr_open_fflag;
	open_args.a_sflag = (osr->osr_open_fflag & O_DOCLONE) ? CLONEOPEN : 0;
	open_args.a_creds = osr->osr_creds;

	mult_sqh_acquire(osr);
	error = csq_protect(RD(q), q,
		(csq_protect_fcn_t)open_wrapper,
		(csq_protect_arg_t)&open_args,
		&osr->osr_sq, TRUE);
	mult_sqh_release(osr);
	return error;
}

int
osr_add_modules(osr)
reg     OSRP osr;
{
	struct strapush stra;
	int error = 0;
	int dev, i;
reg	STHP	sth = osr->osr_sth;

	dev = osr->osr_open_dev;
	osr->osr_flags |= F_OSR_NEED_MULT_SQH;
	if (sad_get_autopush((long)major(dev), (long)minor(dev), &stra)) {
	    if (stra.sap_npush) {
		e_assert_wait(&sth->sth_open_ev,INTERRUPTIBLE);
		if (!sth->sth_push_cnt
		    && test_and_set((long*)&(sth->sth_ext_flags), F_STH_PUSHING)
		    && ((sth->sth_ext_flags & (F_STH_PUSHING|F_STH_PUSHED)) == F_STH_PUSHING)) {
			e_clear_wait(thread_self(), THREAD_AWAKENED);
			osr->osr_handler = osr_push;
			osr->osr_osrq = &sth->sth_ioctl_osrq;
			osr->osr_ioctl_arg1 = osr->osr_open_fflag;
			osr->osr_ioctl_arg2 = dev;
			for (i = 0; i < stra.sap_npush; i++) {
				osr->osr_ioctl_arg0p = stra.sap_list[i];
				osr->osr_next = NULL;
				if (error = osr_run(osr)) {
					break;
				}
		       }
		       fetch_and_or((long*)&(sth->sth_ext_flags), F_STH_PUSHED);
		       fetch_and_and((long*)&(sth->sth_ext_flags),
				     			~F_STH_PUSHING);
		       if (sth->sth_open_ev != EVENT_NULL)
				e_wakeup(&sth->sth_open_ev);
		} else {
			queue_t *wq, *rq;
			struct open_args open_args;

			if ((sth->sth_ext_flags & (F_STH_PUSHING|F_STH_PUSHED))
== F_STH_PUSHING)
				e_block_thread();
			else {
				fetch_and_and((long*)&(sth->sth_ext_flags),
					      			~F_STH_PUSHING);
				e_clear_wait(thread_self(), THREAD_AWAKENED);
			}
			/*
			 * case of reopen
			 */
			wq = sth->sth_wq;
			open_args.a_fflag = osr->osr_open_fflag;
			open_args.a_sflag = MODOPEN;
			open_args.a_creds = osr->osr_creds;
			open_args.a_devp  = 0;
			while (!STREAM_END(wq)) {
				wq = wq->q_next;
			}
			rq = RD(wq)->q_next;
			while ( rq && (rq != sth->sth_rq)) {
				open_args.a_queue = rq;
				open_args.a_func  = rq->q_qinfo->qi_qopen;
				error = csq_protect(rq, WR(rq),
					(csq_protect_fcn_t)open_wrapper,
					(csq_protect_arg_t)&open_args,
					&osr->osr_sq, TRUE);
				if (error) break;
				rq = rq->q_next;
			}
		}
	    }
	}
	return error;
}

int
pse_close (dev, private)
        dev_t   dev;
        void    *private;
{
	OSRP	osr;
	STHP	sth;
	int	error = 0;
	OSRQ	close_osrq;
	DISABLE_LOCK_DECL

	TRCHKL2T(HKWD_PSE | hkwd_pse_close_in, dev, private);

        ENTER_FUNC(pse_close, dev, private, 0, 0, 0, 0);

	if (dev == (dev_t)-1) {
		sth = (STHP)private;
		if ((STHP)devtosth(sth->sth_dev) == sth) {
			DISABLE_LOCK(&streams_open_lock);
			sth->sth_open_count--;
			DISABLE_UNLOCK(&streams_open_lock);
			goto cl_out;
		}
		DISABLE_LOCK(&streams_open_lock);
		if ( --sth->sth_open_count) {
			DISABLE_UNLOCK(&streams_open_lock);
			goto cl_out;
		}
		DISABLE_UNLOCK(&streams_open_lock);
	} else
		sth = (STHP)devtosth(dev);
        if (!sth) {
		error = ENOSTR;
		goto cl_out;
        }
	/*
	 * To avoid panic situation for non-clone driver with sleep() in
	 * it's close routine.
	 * Open driver and Close it, will wait in close routine.
	 * Open same driver, it will return EBUSY and file system
	 * calls us back in this routine, we have valid "sth" (it's still open)
	 * and so it will call driver's close() and will wait there too !
	 * At the wakeup time only one will successed and other will panic !!
	 */
	if (sth->sth_flags & (F_STH_CLOSING | F_STH_CLOSED)) {
		error = EINPROGRESS;
		goto cl_out;
	}

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_osrq = &sth->sth_ioctl_osrq;
	osr->osr_creds = crref();
	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
	sth->sth_flags |= F_STH_CLOSED;
	csq_release(&sth->sth_rq->q_sqh);

	/*
	 * Chase other callers out...
	 */
	osrq_cancel(&sth->sth_ioctl_osrq);
	osrq_cancel(&sth->sth_read_osrq);
	osrq_cancel(&sth->sth_write_osrq);

	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);

	select_wakeup((POLLSP)&sth->sth_pollq);
	select_dequeue_all(&sth->sth_pollq);

	/* There may be signal structures left. */
	while ( sth_sigs_active(sth) ) {
		SIGSP ss = (SIGSP)sth->sth_sigsq.next;
		remque(&ss->ss_link);
		NET_FREE(ss, M_STRSIGS);
	}

	if (sth->shttyp) sth_ttyclose(sth);

	if ( sth->sth_flags & F_STH_LINKED ) {
		/*
		 * ... then don't proceed any further. This
		 * stream will be wiped out during unlink.
		 * See comment at sth_get_sth().
		 */
		csq_release(&sth->sth_rq->q_sqh);
		osr_free(osr);
		error = 0;
	} else {
		csq_release(&sth->sth_rq->q_sqh);
		osrq_init(&close_osrq);
		osrq_insert(&close_osrq, osr);
		error = osr_close_subr(&close_osrq);
		osrq_term(&close_osrq);
	}
	DB_check_streams("CLOSE");

	REPORT_FUNC();
cl_out:
	LEAVE_FUNC(pse_close, error);

	TRCHKL1T(HKWD_PSE | hkwd_pse_close_out, error);
	return error;
}

/*
 * osr_close_subr - dismantle one or more streams
 *
 * The standard call to this routine will be with a list
 * of one element, right from pse_close (see above).
 *
 * Another call is possible from osr_unlink which hands
 * over a list of streams which have become obsolete since
 * they were already closed, and have now been unlinked.
 *
 * If there are persistent links under this stream, it is not
 * closed.
 *
 * Furthermore, at some point during close processing, we have
 * to unlink our lower streams (if any), which may lead to more
 * streams to be closed. The osr_unlink_subr, which we use for
 * that, is prepared to report back such streams.
 *
 * On entry, no resources are held.
 * On exit, no resources are held, and all stream heads and OSR's
 * have been deallocated. (THIS IS DIFFERENT, compared to the other
 * cases, for obvious reasons.)
 */
int
osr_close_subr (osrq)
	OSRQP	osrq;
{
	STHP		sth;
	OSRP		osr;
	queue_t *	q;
	int		err, error = 0, closecount = 0;
	int		tmo;
	STHPP		sthp;	
	MBLKP 		mp;
	DISABLE_LOCK_DECL

	ENTER_FUNC(osr_close_subr, osrq, 0, 0, 0, 0, 0);

	while ( (osr = osrq_remove(osrq)) != nil(OSRP) ) {
		sth = osr->osr_sth;
		osrq_insert(&sth->sth_ioctl_osrq, osr);	/* for synch&wakeups */
		osr->osr_flags |= F_OSR_NEED_MULT_SQH;
		mult_sqh_acquire(osr);
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
		sth->sth_flags |= F_STH_CLOSING;

		/*
		 * Unlink lower streams, if there are any. This might
		 * add work to our list of streams to be closed...
		 */
		if ( sth->sth_mux_top ) {
			err = osr_unlink_subr(osr, MUXID_ALL, I_UNLINK, osrq);
STR_DEBUG(if (err) printf("streams: error %d during close/unlink\n", err));
		}

		/*
		 * If persistent links still exist, become a orphan.
		 */
		if ( sth->sth_pmux_top ) {
			mult_sqh_release(osr);
			if (osr != osrq_remove(&sth->sth_ioctl_osrq))
				panic("osr_close_subr osr");
			sth->sth_flags &= ~F_STH_CLOSING;
			csq_release(&sth->sth_rq->q_sqh);
			osr_free(osr);
			continue;
		}

		q = sth->sth_wq;
		/*
		 * If this stream is one end of a pipe, discard its stat
		 * and fattach buffer, and if the other end has not been
		 * closed, send an M_HANGUP message down the stream.
		 */
		if ( (sth->sth_flags & (F_STH_PIPE|F_STH_HANGUP)) == F_STH_PIPE
		&&   q->q_next )
			putctl(q->q_next, M_HANGUP);

		/*
		 * Time to pop the modules. The spec requires that we wait 15
		 * seconds per non-empty module. We take the stream head into
		 * this consideration in the first round. We now allow
		 * interrupts during the wait, and take that as indication
		 * not to wait any more during the following pop operations.
		 * Each queue is polled every second until empty.
		 * Should we bother to wait on linked streamheads?
		 */
		err = 0;
		if (sth->sth_dev != NODEV) {
			devsetsth(sth->sth_dev, (caddr_t)0);
		}
                /*
                 * Why are we keeping the data on the read side?
                 */
                flushq(RD(sth->sth_wq), FLUSHALL);
		while ( q = sth->sth_wq->q_next ) {

			if ( (q->q_first || sth->sth_wq->q_first)
			&&   sth->sth_close_wait_timeout > 0
			&&   !(osr->osr_flags & F_OSR_NBIO) ) {
				tmo = MS_TO_TICKS(sth->sth_close_wait_timeout);
				if (tmo == 0) tmo = 1;
				while (mp = getq(sth->sth_wq))
					putnext(sth->sth_wq, mp);
				for (;;) {
					err = osr_sleep(osr, TRUE, MIN(hz,tmo));
					if (err == ETIME) {
						err = 0;
						tmo -= hz;
					}
					if (err) {
						sth->sth_close_wait_timeout = 0;
						break;
					}
					if (tmo <= 0 || !(q->q_first
					     || sth->sth_wq->q_first))
						break;
				}
			}

			/*
			 * Check for any welds before popping the next queue!
			 */
			if ( sth->sth_wq->q_flag & QWELDED )	/* backq(q) */
				unweldq_exec(sth->sth_wq, q, &osr->osr_sq);
			q = OTHERQ(q);
			if ( q->q_next && (q->q_flag & QWELDED) )
				unweldq_exec(q, q->q_next, &osr->osr_sq);
			if ((q = sth->sth_wq->q_next) == nilp(queue_t)) {
				err = 0;
				break;
			}

			/* 
			 * The last error will be the driver's.
			 */
			err = osr_pop_subr(osr, q);

			/*
			 * After the sth_wq wait, flush it to avoid data
			 * out of sequence and further clogs.
			 */
			flushq(sth->sth_wq, FLUSHALL);
		}
		/*
		 * Call osr_pop_subr for the stream head queue pair.
		 * Note osr_pop_subr drops the mult_sqh and sth locks
		 * when it sees a stream head queue. Then remember
		 * the main stream's error for return.
		 */
		(void) osr_pop_subr(osr, sth->sth_wq);
		if (closecount++ == 0)
			error = err;
		osr_free(osr);

		/*
		 * Now remove the stream from the open streams hash table.
		 */
		if (sth->sth_dev != NODEV) {
			DISABLE_LOCK(&streams_open_lock);
			sthp = STH_HASH(sth->sth_dev);
			if (*sthp == sth)
				*sthp = sth->sth_next;
			else for ( ; *sthp; sthp = &((*sthp)->sth_next)) {
				if ((*sthp)->sth_next == sth) {
					(*sthp)->sth_next = sth->sth_next;
					break;
				}
			}
			DISABLE_UNLOCK(&streams_open_lock);
		}

		/*
		 * Time to chuck pipe info and fdetach other end.
		 */
		if (sth->sth_flags & F_STH_PIPE)
			sth_update_times(sth, FSYNC, (struct stat *)0);

		DB_isclosed(sth);
		osrq_term(&sth->sth_read_osrq);
		osrq_term(&sth->sth_write_osrq);
		osrq_term(&sth->sth_ioctl_osrq);
		sth_free(sth);
	}
	if (error == ERESTART)
		error = EINTR;
	LEAVE_FUNC(osr_close_subr, error);
	return error;
}

int
pse_read (dev, uiop, private, fflag)
        dev_t           dev;
        struct uio      *uiop;
        void            *private;
        int             fflag;
{
	OSRP	osr;
	STHP	sth;
	int	error;
	queue_t *q;


	TRCHKL4T(HKWD_PSE | hkwd_pse_read_in, dev, uiop, private, fflag);

	ENTER_FUNC(pse_read, dev, uiop, fflag, 0, 0, 0);

	if (!(sth = (STHP)devtosth(dev))) {
		error = ENOSTR;
		goto re_out;
	}
        for (q = sth->sth_wq; q->q_next; q = q->q_next);
        if (q) {
                q = OTHERQ(q);          /* at the bottom of the read side */
                while (q) {
                        struct wantio *w;

                        if ((w = q->q_wantio) &&
                            w->wantiosw->w_read &&
                            (error = (*w->wantiosw->w_read)(q, uiop, 0)) != -1)
                                goto re_out;
                        q = q->q_next;
                }
        }

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_creds = crref();
	if (uiop->uio_fmode & FNDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (uiop->uio_fmode & FNONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;
	osr->osr_flags   |= F_OSR_RTTY_CHECK;
	osr->osr_rw_uio   = uiop;
	osr->osr_rw_count = uiop->uio_resid;
	osr->osr_handler  = osr_read;
	osr->osr_osrq     = &sth->sth_read_osrq;
	osr->osr_closeout = RL_ERROR_FLAGS;
	osr->osr_rw_rw = UIO_READ;

	error = osr_run(osr);

/*
 *	Streams are allowed to return EAGAIN with system V FNDELAY,
 *	so STREAMS framework returns EAGAIN with the high bit set
 *	lfs turns it off in rdwr().
 */
	if (error == EAGAIN) error |= (1 << 31);

	if (osr->osr_rw_total)
		sth_uiodone(osr);

	osr_free(osr);

	DB_check_streams("READ");
	LEAVE_FUNC(pse_read, error);
re_out:
	LEAVE_FUNC(pse_read, error);
	TRCHKL1T(HKWD_PSE | hkwd_pse_read_out, error);
	return error;
}

int
pse_write (dev, uiop, private, fflag)
        dev_t           dev;
        struct uio      *uiop;
        void            *private;
        int             fflag;
{
	OSRP	osr;
	STHP	sth;
	int	error;

	TRCHKL4T(HKWD_PSE | hkwd_pse_write_in, dev, uiop, private, fflag);

	ENTER_FUNC(pse_write, dev, uiop, fflag, 0, 0, 0);

	if (!(sth = (STHP)devtosth(dev))) {
		error = ENOSTR;
		goto wr_out;
	}
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_creds = crref();

	if (uiop->uio_fmode & FNDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (uiop->uio_fmode & FNONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;

	osr->osr_flags   |= F_OSR_WTTY_CHECK;
	osr->osr_rw_uio   = uiop;
	osr->osr_rw_count = uiop->uio_resid;
	osr->osr_handler  = osr_write;
	osr->osr_osrq     = &sth->sth_write_osrq;
	osr->osr_closeout = WHL_ERROR_FLAGS;
	osr->osr_rw_rw = UIO_WRITE;

	error = osr_run(osr);

/*
 *	Streams are allowed to return EAGAIN with system V FNDELAY,
 *	so STREAMS framework returns EAGAIN with the high bit set
 *	lfs turns it off in rdwr().
 */
	if (error == EAGAIN) error |= (1 << 31);

	if (osr->osr_rw_total)
		sth_uiodone(osr);

	osr_free(osr);

	DB_check_streams("WRITE");
wr_out:
	LEAVE_FUNC(pse_write, error);

	TRCHKL1T(HKWD_PSE | hkwd_pse_write_out, error);
	return error;
}

/* Ioctl permissions */
#define MUSTHAVE(flag, errno) do {		\
	if ((fflag & (flag)) != (flag)		\
	&&  drv_priv(osr->osr_creds) != 0) {	\
		error = (errno);		\
		goto done;			\
	}					\
} while (0)

int
pse_ioctl (dev, cmd, data, fflag, private, ext, retval)
        dev_t           dev;    /* major + minor device number */
        int             cmd;    /* cmd argument to ioctl system call */
        caddr_t         data;   /* *pointer* to 3rd user argument */
        int             fflag;  /* f_flag from file structure */
        void            *private;
        int             ext;
        int             *retval;
{
	STHP		sth;
	OSRP		osr;
	int		error = 0;
	int		len = 0;
	char *		buf;
	int		tmp;
	int             closeout = RWHL_ERROR_FLAGS;
	pid_t		pgid;

	TRCHKL5T(HKWD_PSE | hkwd_pse_ioctl_in,dev,cmd,data,fflag,retval);

	ENTER_FUNC(pse_ioctl, dev, cmd, data, fflag, 0, 0);

	if (!(sth = (STHP)devtosth(dev))) {
		error = ENOSTR;
		goto out;
	}

	/*
	 * This switch is for ioctls that DO NOT need to be 
	 * handled via osr_run()...
	 */
	switch (cmd) {
	case TIOCSPGRP:
	case TXSPGRP:
		if ((fflag & (UIO_READ)) != (UIO_READ)) {
			struct ucred *cred;

			error = drv_priv(cred = crref()) ? EACCES : 0;
			crfree(cred);
		}

		if (!error && !(error = copyin(data, &pgid, sizeof(pid_t))))
			error = sth_tiocspgrp(sth, pgid, cmd);
		goto out;

	case TXISATTY:
		error = (sth->sth_flags & F_STH_ISATTY) ? 0 : ENOTTY;
		goto out;
	}

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_creds = crref();


	if (fflag & FNDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (fflag & FNONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;
	if (fflag & FKERNEL)
		osr->osr_flags |= F_OSR_KCOPY;
	if (sth->sth_flags & F_STH_ISATTY)
		osr->osr_flags |= F_OSR_TTY_IOCTL;

	/* We need to make osr->osr_ioctl_arg0p and buf point at the
	 * place in the osr where the union streams_ioctl_buffer of
	 * the IOCTL_OSR begins.
	 */
	osr->osr_ioctl_arg0p = buf =
		(char *)osr + offsetof(OSR, osr_osru) +
			offsetof(IOCTL_OSR, ioc_buf);
		
	/*
	 * Streams ioctl's - trickery speeds switch.
	 */
	if (((cmd & ~0xff) ^ _IO('S', 0)) == 0)
	switch ((unsigned char)(cmd & 0xff)) {

	case I_ATMARK & 0xff:
		osr->osr_handler    = osr_atmark;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case I_CANPUT & 0xff:
		osr->osr_handler    = osr_canput;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case I_CKBAND & 0xff:
		osr->osr_handler    = osr_ckband;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case I_FDINSERT & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_fdinsert;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags      |= 
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strfdinsert));
#if SEC_BASE
		((struct strfdinsert_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_FDINSERT_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_fdinsert;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags      |=
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strfdinsert_attr));
#else
		error = EINVAL;
#endif
		break;

	case I_FIFO & 0xff:
		osr->osr_handler    = osr_fifo;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		break;

	case I_FIND & 0xff:
		osr->osr_handler     = osr_find;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_closeout    = RWL_ERROR_FLAGS;
		len = FMNAMESZ + 1;
		while ((error = copyin(data, buf, len)) && --len)
			;
		buf[len] = '\0';
		len = 0;
		break;

	case I_FLUSH & 0xff:
		osr->osr_handler    = osr_flush;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
#ifdef SEC_BASE
		if (osr->osr_ioctl_arg1 & FLUSHR)
			MUSTHAVE(UIO_READ, EACCES);
		if (osr->osr_ioctl_arg1 & FLUSHW)
			MUSTHAVE(UIO_WRITE, EACCES);
#endif
		break;

	case I_FLUSHBAND & 0xff:
		osr->osr_handler    = osr_flushband;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct bandinfo));
		if (error == 0) {
			if (((struct bandinfo *)buf)->bi_flag & FLUSHR)
				MUSTHAVE(UIO_READ, EACCES);
			if (((struct bandinfo *)buf)->bi_flag & FLUSHW)
                                MUSTHAVE(UIO_WRITE, EACCES);
		}
		break;

	case I_GETBAND & 0xff:
		osr->osr_handler    = osr_getband;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case I_GETCLTIME & 0xff:
		osr->osr_handler    = osr_getcltime;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case I_GETMSG & 0xff:
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_getmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpeek);
		error = copyin(data, buf, len);
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.maxlen = -1;
#endif
		break;

	case I_GETMSG_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_getmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpeek_attr);
		error = copyin(data, buf, len);
#else
		error = EINVAL;
#endif
		break;

	case I_GETPMSG & 0xff:
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_getpmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpmsg);
		error = copyin(data, buf, len);
#if SEC_BASE
		((struct strpmsg_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_GETPMSG_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_getpmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpmsg_attr);
		error = copyin(data, buf, len);
#else
		error = EINVAL;
#endif
		break;

	case I_GETSIG & 0xff:
		osr->osr_handler    = osr_getsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case I_GRDOPT & 0xff:
		osr->osr_handler    = osr_grdopt;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case I_GWROPT & 0xff:
		osr->osr_handler    = osr_gwropt;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case I_ISASTREAM & 0xff:
		osr->osr_handler    = osr_isastream;
		osr->osr_osrq       = &sth->sth_read_osrq;
		break;

	case I_LINK & 0xff:
	case I_PLINK & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler    = osr_link;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		osr->osr_ioctl_arg2 = cmd;
		break;

	case I_LIST & 0xff:
		osr->osr_handler     = osr_list;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_flags      |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout    = RWL_ERROR_FLAGS;
		if (data)
			error = copyin(data, buf, sizeof(struct str_list));
		else
			osr->osr_ioctl_arg0p = nilp(char);
		break;

	case I_LOOK & 0xff:
		osr->osr_handler    = osr_look;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		break;

	case I_NREAD & 0xff:
		osr->osr_handler    = osr_nread;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case I_POP & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler    = osr_pop;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = fflag;
		break;

	case I_PEEK & 0xff:
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_peek;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpeek));
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.maxlen = -1;
#endif
		len = sizeof(struct strpeek);
		break;

	case I_PEEK_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_peek;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len = sizeof(struct strpeek_attr);
		error = copyin(data, buf, len);
#else
		error = EINVAL;
#endif
		break;

	case I_PIPE & 0xff:
		osr->osr_handler    = osr_pipe;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case I_PUSH & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler     = osr_push;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_flags      |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout    = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1  = fflag;
		osr->osr_ioctl_arg2  = dev;
		len = FMNAMESZ + 1;
		while ((error = copyin(data, buf, len)) && --len)
			;
		buf[len] = '\0';
		len = 0;
		break;

	case I_PUTMSG & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_putmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpeek));
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_PUTMSG_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_putmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpeek_attr));
#else
		error = EINVAL;
#endif
		break;

	case I_PUTPMSG & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_putpmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpmsg));
#if SEC_BASE
		((struct strpmsg_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_PUTPMSG_ATTR & 0xff:
#if SEC_BASE
                MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_putpmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpmsg_attr));
#else
		error = EINVAL;
#endif
		break;

	case I_RECVFD & 0xff:
                MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler    = osr_recvfd;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_flags	   |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1p= data;
		len = sizeof(struct strrecvfd);
#if SEC_BASE
		/* In/out struct in SEC_BASE - zap both lengths */
		((struct strrecvfd_attr *)buf)->attrbuf.maxlen = -1;
		((struct strrecvfd_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_RECVFD_ATTR & 0xff:
#if SEC_BASE
                MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler    = osr_recvfd;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_flags	   |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1p= (int)data;
		len = sizeof(struct strrecvfd_attr);
		error = copyin(data, buf, len);
		if (error)
			len = 0; /* don't try to copyout() below */
#else
		error = EINVAL;
#endif
		break;

	case I_SENDFD & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler    = osr_sendfd;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_flags     |=
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
#if SEC_BASE
		((struct strsendfd_attr *)buf)->fd = osr->osr_ioctl_arg1;
		((struct strsendfd_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_SENDFD_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler    = osr_sendfd;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_flags     |=
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strsendfd_attr));
		osr->osr_ioctl_arg1 = ((struct strsendfd_attr *)buf)->fd;
#else
		error = EINVAL;
#endif
		break;

	case I_SETCLTIME & 0xff:
		osr->osr_handler    = osr_setcltime;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(int));
		break;

	case I_SETSIG & 0xff:
		osr->osr_handler    = osr_setsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case I_SRDOPT & 0xff:
		osr->osr_handler    = osr_srdopt;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case I_STR & 0xff:
		len = sizeof(struct strioctl);
		if (osr->osr_flags & F_OSR_KCOPY)
			bcopy(data, buf, len);
		else if ( error = copyin(data, buf, len) )
			break;
		goto other;

	case I_STR_ATTR & 0xff:
#if SEC_BASE
		len = sizeof(struct strioctl_attr);
		if ( error = copyin(data, buf, len) )
			break;
		goto other;
#else
		error = EINVAL;
#endif
		break;

	case I_SWROPT & 0xff:
		osr->osr_handler    = osr_swropt;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case I_UNLINK & 0xff:
	case I_PUNLINK & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler    = osr_unlink;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		osr->osr_ioctl_arg2 = cmd;
		break;
	default:
		/*
                 * Group 'S' is reserved.
                 */
                error = EINVAL;
                break;
        }
	/*
	 * TTY ioctl's
         */
	else if (((cmd & ~0xff) ^ TXISATTY) == 0)
	switch (cmd & 0xff) {
	case TXGPGRP & 0xff:
		osr->osr_handler    = osr_tiocgpgrp;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg2 = cmd;
		break;


	case TXSETLD & 0xff:
		MUSTHAVE(UIO_READ, EACCES);
		osr->osr_flags |= F_OSR_ITTY_CHECK;
		closeout = RWL_ERROR_FLAGS;
		goto other;

	default:
		closeout = RWL_ERROR_FLAGS;
		goto other;
	}
	else if (((cmd & ~0xff) ^ TIOC) == 0)
	switch (cmd & 0xff) {
        case TCTRUST & 0xff:
                osr->osr_handler    = osr_tctrust;
                osr->osr_osrq       = &sth->sth_ioctl_osrq;
                osr->osr_flags     |= F_OSR_ITTY_CHECK;
                osr->osr_closeout   = RWHL_ERROR_FLAGS;
                error = copyin(data, &osr->osr_ioctl_arg1, sizeof(int));
                break;

        case TCQTRUST & 0xff:
                osr->osr_handler    = osr_tcqtrust;
                osr->osr_osrq       = &sth->sth_ioctl_osrq;
                osr->osr_closeout   = RWHL_ERROR_FLAGS;
                break;

	case TCSAK & 0xff:
		/*
		 * First checks the user argument values and sets some values
		 * in the streamhead as needed.
		 */
		if (osr->osr_flags & F_OSR_KCOPY)
			osr->osr_ioctl_arg1 = (int)*data;
		else
		   if (error = copyin(data, &osr->osr_ioctl_arg1, sizeof(int)))
			break;
		if (error = osr_tcsak(osr))
			goto done;
		else {
			closeout = RWL_ERROR_FLAGS;
			goto other; /* now treats that ioctl as a transparent */
		}

	case TCQSAK & 0xff:
		osr->osr_handler   = osr_tcqsak;
		osr->osr_osrq      = &sth->sth_ioctl_osrq;
		osr->osr_closeout  = RWHL_ERROR_FLAGS;
		break;

	case TCKEP & 0xff:
		osr->osr_handler   = osr_tcskep;
		osr->osr_osrq      = &sth->sth_ioctl_osrq;
		osr->osr_closeout  = RWHL_ERROR_FLAGS;
		error = copyin(data, &osr->osr_ioctl_arg1, sizeof(int));
		break;
	case TCXONC & 0xff:
		MUSTHAVE(UIO_READ, EACCES);
		osr->osr_handler    = osr_tcxonc;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_flags     |= F_OSR_ITTY_CHECK;
		osr->osr_ioctl_arg1 = (int) data;
		break;
	case TCFLSH & 0xff:
		MUSTHAVE(UIO_READ, EACCES);
		switch ((int) data) {
		case TCIFLUSH:
			osr->osr_ioctl_arg1 = FLUSHR;
			break;
		case TCOFLUSH:
			osr->osr_ioctl_arg1 = FLUSHW;
			break;
		case TCIOFLUSH:
			osr->osr_ioctl_arg1 = FLUSHRW;
			break;
		default:
			error = EINVAL;
			break;
		}
		osr->osr_handler    = osr_flushdata;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_flags     |= F_OSR_ITTY_CHECK;
		break;

/* POSIX ioctls */
	case TCSETS & 0xff:
	case TCSETSW & 0xff:
	case TCSETSF & 0xff:
	case TCSBRK & 0xff:

/* SVID ioctls */
	case TCSETAW & 0xff:
	case TCSETAF & 0xff:
	case TCSETA & 0xff:
	case TCSBREAK & 0xff:
		/*
		 * If the tty ioctl involves modification,
		 * request to hang if in the background.
		 */
		MUSTHAVE(UIO_READ, EACCES);
		osr->osr_flags |= F_OSR_ITTY_CHECK;
		closeout = RWL_ERROR_FLAGS;
		goto other;
	default:
		closeout = RWL_ERROR_FLAGS;
		goto other;
	}
        else if ((((cmd & ~0xff) ^ _IO('t', 0)) == 0)
			|| (((cmd & ~0xff) ^ _IOWR('t',0,0)) == 0)
			|| (((cmd & ~0xff) ^ _IOR('t',0,0)) == 0)
			|| (((cmd & ~0xff) ^ _IOW('t',0,0)) == 0))
	switch (cmd & 0xff) {
	case TIOCGPGRP & 0xff:
		osr->osr_handler    = osr_tiocgpgrp;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg2 = cmd;
		break;

	case TIOCGSID & 0xff:
		osr->osr_handler    = osr_tiocgsid;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		break;

	case TIOCCONS & 0xff:
		osr->osr_handler    = osr_tioccons;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		if (osr->osr_flags & F_OSR_KCOPY)
			osr->osr_ioctl_arg1 = (int)*data;
		else
			error = copyin(data, &osr->osr_ioctl_arg1, sizeof(int));
		break;

	case TIOCSTI & 0xff:
		MUSTHAVE(UIO_READ, EPERM);
		/* No need to lock sth here... */
		if (sth_isctty(kgetsid(0), sth) == 0
		    &&  drv_priv(osr->osr_creds) != 0) {
			error = EACCES;
			goto done;
		}
		osr->osr_flags |= F_OSR_ITTY_CHECK;
		closeout = RWL_ERROR_FLAGS;
		goto other;

/* SVID.3 starts */
	case TIOCMBIC & 0xff:
	case TIOCMBIS & 0xff:
	case TIOCMSET & 0xff:
	case TIOCMGET & 0xff:
/* SVID.3 ends */
	case TIOCSETD & 0xff:
	case TIOCFLUSH & 0xff:
	case TIOCSWINSZ & 0xff:
/* COMPAT_BSD_4.3 */
	case TIOCSETP & 0xff:
	case TIOCSETN & 0xff:
	case TIOCSETC & 0xff:
	case TIOCSLTC & 0xff:
	case TIOCLBIS & 0xff:
	case TIOCLBIC & 0xff:
	case TIOCLSET & 0xff:
/* COMPAT_BSD_4.3 ends */
		/*
		 * If the tty ioctl involves modification,
		 * request to hang if in the background.
		 */
		MUSTHAVE(UIO_READ, EACCES);
		osr->osr_flags |= F_OSR_ITTY_CHECK;
		closeout = RWL_ERROR_FLAGS;
		goto other;
	default:
		closeout = RWL_ERROR_FLAGS;
		goto other;
	}
	/*
	 * Old IBM streams ioctl's
	 */
	else if (((cmd & ~0xff) ^ STR_IO(t, 0)) == 0)
	switch ((unsigned char)(cmd & 0xff)) {
	case OLD_IATMARK & 0xff:
		osr->osr_handler    = osr_atmark;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case OLD_ICANPUT & 0xff:
		osr->osr_handler    = osr_canput;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case OLD_ICKBAND & 0xff:
		osr->osr_handler    = osr_ckband;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case OLD_IFDINSERT & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_fdinsert;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags      |= 
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strfdinsert));
#if SEC_BASE
		((struct strfdinsert_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case OLD_IFIND & 0xff:
		osr->osr_handler     = osr_find;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_closeout    = RWL_ERROR_FLAGS;
		len = FMNAMESZ + 1;
		while ((error = copyin(data, buf, len)) && --len)
			;
		buf[len] = '\0';
		len = 0;
		break;

	case OLD_IFLUSH & 0xff:
		osr->osr_handler    = osr_flush;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		if (osr->osr_ioctl_arg1 & FLUSHR)
			MUSTHAVE(UIO_READ, EACCES);
		if (osr->osr_ioctl_arg1 & FLUSHW)
			MUSTHAVE(UIO_WRITE, EACCES);
		break;

	case OLD_IFLUSHBAND & 0xff:
		osr->osr_handler    = osr_flushband;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct bandinfo));
		if (error == 0) {
			if (((struct bandinfo *)buf)->bi_flag & FLUSHR)
				MUSTHAVE(UIO_READ, EACCES);
			if (((struct bandinfo *)buf)->bi_flag & FLUSHW)
                                MUSTHAVE(UIO_WRITE, EACCES);
		}
		break;

	case OLD_IGETBAND & 0xff:
		osr->osr_handler    = osr_getband;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case OLD_IGETCLTIME & 0xff:
		osr->osr_handler    = osr_getcltime;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case OLD_IGETMSG & 0xff:
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_getmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpeek);
		error = copyin(data, buf, len);
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.maxlen = -1;
#endif
		break;

	case OLD_IGETPMSG & 0xff:
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_getpmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpmsg);
		error = copyin(data, buf, len);
#if SEC_BASE
		((struct strpmsg_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case OLD_IGETSIG & 0xff:
		osr->osr_handler    = osr_getsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)getpid();
		len		    = sizeof(int);
		break;

	case OLD_IGRDOPT & 0xff:
		osr->osr_handler    = osr_grdopt;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case OLD_IGWROPT & 0xff:
		osr->osr_handler    = osr_gwropt;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case OLD_ILINK & 0xff:
	case OLD_IPLINK & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler    = osr_link;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		osr->osr_ioctl_arg2 = cmd;
		break;

	case OLD_ILIST & 0xff:
		osr->osr_handler     = osr_list;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_flags      |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout    = RWL_ERROR_FLAGS;
		if (data)
			error = copyin(data, buf, sizeof(struct str_list));
		else
			osr->osr_ioctl_arg0p = nilp(char);
		break;

	case OLD_ILOOK & 0xff:
		osr->osr_handler    = osr_look;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		break;

	case OLD_INREAD & 0xff:
		osr->osr_handler    = osr_nread;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(int);
		break;

	case OLD_IPOP & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler    = osr_pop;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = fflag;
		break;

	case OLD_IPEEK & 0xff:
		MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler     = osr_peek;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpeek));
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.maxlen = -1;
#endif
		len = sizeof(struct strpeek);
		break;

	case OLD_IPIPE & 0xff:
		osr->osr_handler    = osr_pipe;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case OLD_IPUSH & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler     = osr_push;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_flags      |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout    = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1  = fflag;
		osr->osr_ioctl_arg2  = dev;
		len = FMNAMESZ + 1;
		while ((error = copyin(data, buf, len)) && --len)
			;
		buf[len] = '\0';
		len = 0;
		break;

	case OLD_IPUTMSG & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_putmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpeek));
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case OLD_IPUTPMSG & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler     = osr_putpmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(struct strpmsg));
#if SEC_BASE
		((struct strpmsg_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case OLD_IRECVFD & 0xff:
                MUSTHAVE(UIO_READ, EBADF);
		osr->osr_handler    = osr_recvfd;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_flags	   |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1p= data;
		len = sizeof(struct strrecvfd);
#if SEC_BASE
		/* In/out struct in SEC_BASE - zap both lengths */
		((struct strrecvfd_attr *)buf)->attrbuf.maxlen = -1;
		((struct strrecvfd_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case OLD_ISENDFD & 0xff:
		MUSTHAVE(UIO_WRITE, EBADF);
		osr->osr_handler    = osr_sendfd;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_flags     |=
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
#if SEC_BASE
		((struct strsendfd_attr *)buf)->fd = osr->osr_ioctl_arg1;
		((struct strsendfd_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case OLD_ISETCLTIME & 0xff:
		osr->osr_handler    = osr_setcltime;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		error = copyin(data, buf, sizeof(int));
		break;

	case OLD_ISETSIG & 0xff:
		osr->osr_handler    = osr_setsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)getpid();
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case OLD_ISRDOPT & 0xff:
		osr->osr_handler    = osr_srdopt;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case OLD_ISTR & 0xff:
		len = sizeof(struct strioctl);
		if ( error = copyin(data, buf, len) )
			break;
		goto other;

	case OLD_ISWROPT & 0xff:
		osr->osr_handler    = osr_swropt;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		break;

	case OLD_IUNLINK & 0xff:
	case OLD_IPUNLINK & 0xff:
		MUSTHAVE(UIO_READ|UIO_WRITE, EACCES);
		osr->osr_handler    = osr_unlink;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = (int)data;
		osr->osr_ioctl_arg2 = cmd;
		break;
	default:
		/*
                 * Group 't' with an empty high short is reserved.
		 * Old IBM Streams ioctl's
                 */
                error = EINVAL;
                break;
        }
	/*
	 * File ioctl's
	 */
	else if (IOCGROUP(cmd) == 'f') switch (cmd) {

	case FIOASYNC:
		/* Hack this into a funny I_SETSIG */
		DB0(DB_CTTY, "FIOASYNC ioctl\n");
		osr->osr_handler    = osr_setsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		*(int *)osr->osr_ioctl_arg0p = 1;
		if (osr->osr_flags & F_OSR_KCOPY)
		    osr->osr_ioctl_arg1 = (int)*data;
		else
		    if (error = copyin(data, &osr->osr_ioctl_arg1, sizeof(int)))
			break;
		if (osr->osr_ioctl_arg1)
			osr->osr_ioctl_arg1 = S_INPUT|S_OUTPUT|S_ERROR|S_HANGUP;
		osr->osr_ioctl_arg2p= (char *)getpid();
		break;

	case FIOFATTACH:
	case FIOFDETACH:
		MUSTHAVE(FKERNEL, EINVAL);
		osr->osr_handler    = osr_fattach;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_ioctl_arg1 = (cmd == FIOFATTACH);
		osr->osr_ioctl_arg2p= (char *)((void **)data);
		break;

	case FIONREAD:
		osr->osr_handler    = osr_fionread;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		break;

	case FIOPIPESTAT:
		osr->osr_handler    = osr_pipestat;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		break;

	default:
		goto other;
	}

	/*
	 * Others
	 */
	else
other:	{

		struct strioctl *stri = (struct strioctl *)buf;

		/*
		 * If len set, then is I_STR; else an unknown
		 * ioctl gets mapped to a TRANSPARENT ioctl.
		 */
		if ( len ) {
			osr->osr_ioctl_arg1p = stri->ic_dp;
			osr->osr_ioctl_arg1_len = stri->ic_len;
			if ( stri->ic_len == TRANSPARENT )
				osr->osr_ioctl_arg1_len = 0;
			else if ( stri->ic_len < 0 ) {
				error = EINVAL;
				goto done;
			}
			/* SVID says I_STR always blocks */
			osr->osr_flags &= ~F_OSR_NBIO;
		} else {
			stri->ic_cmd    = cmd;
			stri->ic_timout = -1;
			stri->ic_dp             = data;
			osr->osr_ioctl_arg1p    = data;
			stri->ic_len            = TRANSPARENT;
			osr->osr_ioctl_arg1_len = sizeof(caddr_t);
			/* TTY ioctls always block */
			if (sth->sth_flags & F_STH_ISATTY)
				osr->osr_flags &= ~F_OSR_NBIO;
		}
		osr->osr_handler  = osr_str;
		osr->osr_osrq     = &sth->sth_ioctl_osrq;
		osr->osr_closeout = closeout;
#if SEC_BASE
		if ( len != sizeof (struct strioctl_attr) ) {
			/* In/out struct in SEC_BASE - zap both lengths */
			((struct strioctl_attr *)buf)->attrbuf.maxlen = -1;
			((struct strioctl_attr *)buf)->attrbuf.len = -1;
		}
#endif
	}
	if ( error || (error = osr_run(osr)) ||
	     (osr->osr_flags & F_OSR_BLOCK_TTY))
		goto done;

	/*
	 * Tail-end processing of non-copyout ioctl's here. There are
	 * four ways for an ioctl to return data to the user:
	 * 1)  via M_COPYOUT messages from driver/module.
	 * 2a) by setting arg0p and arg0_len (usually arg0p => (osr + 1)).
	 * 2b) by setting arg0p and len above (overrides arg0_len).
	 * 3)  by setting arg1p and placing an mblk chain in ioctl_data.
	 *
	 * (2a and 2b are mutually exclusive, others are not)
	 */
	if ( len == 0 && osr->osr_ioctl_arg0p )
		len = osr->osr_ioctl_arg0_len;
	if ( len > 0 ) {
		if (osr->osr_flags & F_OSR_KCOPY) {
			bcopy((caddr_t)osr->osr_ioctl_arg0p, data, len);
		} else
			error = copyout((caddr_t)osr->osr_ioctl_arg0p,
					data, len);
	}
	if ( osr->osr_ioctl_data ) {
		if (error == 0 && (len = osr->osr_ioctl_arg1_len) > 0) {
			MBLKP mp = osr->osr_ioctl_data;
			if ((cmd & (IOC_IN|IOC_OUT))
			    &&  (!(cmd & IOC_OUT) || len > IOCPARM_LEN(cmd)))
				error = EFAULT;
			else do {
				if (!mp || mp->b_datap->db_type != M_DATA ||
				    (unsigned)(mp->b_wptr - mp->b_rptr) <
						osr->osr_ioctl_arg1_len) {
					error = EFAULT;
					break;
				}
				if (osr->osr_flags & F_OSR_KCOPY)
					bcopy((caddr_t)mp->b_rptr,
					    (caddr_t)osr->osr_ioctl_arg1p, len);
				else if (error = copyout((caddr_t)mp->b_rptr,
					    (caddr_t)osr->osr_ioctl_arg1p, len))
					break;
				mp = mp->b_cont;
				osr->osr_flags |= F_OSR_AUDIT_READ;
				osr->osr_ioctl_arg1p += len;
			} while ((osr->osr_ioctl_arg1_len -= len) > 0);
			if (error == 0 && osr->osr_ioctl_arg1_len)
				error = EFAULT;
		}
		freemsg(osr->osr_ioctl_data);
	}

#if SEC_BASE
	/*
	 * Make sure that the ioctl commands that perform data transfers
	 * get audited as reads and writes.
	 */
	if (error == 0 &&
	    (osr->osr_flags & (F_OSR_AUDIT_READ|F_OSR_AUDIT_WRITE))) {
		int fd;
		STHP nsth;
		/*
		 * We need the fd this ioctl is operating on for the
		 * audit record. We no longer have u.u_arg so we use
		 * sth_fd_to_sth() until we match our sth. While we
		 * might get it wrong if we've dup'd the fd, we're at
		 * least close.
		 */
		if (u.utask->uu_fd) {
			for (fd = 0; fd <= u.utask->uu_fd->fd_lastfile; fd++)
				if (sth_fd_to_sth(fd, &nsth) == 0
					&& sth == nsth)
					break;
			if (fd <= u.utask->uu_fd->fd_lastfile) {
				if (osr->osr_flags & F_OSR_AUDIT_READ)
					audstub_rdwr1(UIO_READ, fd);
				if (osr->osr_flags & F_OSR_AUDIT_WRITE)
					audstub_rdwr1(UIO_WRITE, fd);
			}
		}
	}
#endif
	if (error == 0)
		*retval = osr->osr_ret_val;

done:	DB_check_streams("IOCTL");

	osr_free(osr);

out:
	LEAVE_FUNC(pse_ioctl, error);
	TRCHKL1T(HKWD_PSE | hkwd_pse_ioctl_out, error);
	return error;
}

int
sth_poll_check(sth, events, revents)
	STHP    sth;
	ushort events;
	ushort revents;
{
	MBLKP		mp;

	/*
	 * The manual says:
	 *
	 * POLLIN  - non-priority message available (or M_PASSFP)
	 * POLLPRI - priority message available
	 * POLLRDNORM - band 0 message available
	 * POLLRDBAND - non-0 band message available
	 * POLLOUT - non-priority messages can be sent
	 * POLLWRNORM - same as POLLOUT
	 * POLLWRBAND - non-0 band message may be sent
	 * POLLMSG - SIGPOLL has reached front of q (note: only
	 *		this signal is queued by sth_rput)
	 * POLLERR - error message has arrived
	 * POLLHUP - hangup condition
	 * POLLNVAL - bogus file descriptor
	 *
	 * POLLIN and POLLPRI are mutually exclusive on return.
	 * POLLERR and POLLHUP are not requested, only reported.
	 * if POLLHUP is true, then POLLOUT can't be true.
	 *
	 * For consistency with the handling of requests, we add:
	 * POLLERR is mutually exclusive with any other return value!
	 */ 

	if ( sth->sth_flags & (F_STH_READ_ERROR | F_STH_WRITE_ERROR) ){
		revents = POLLERR;
		goto done;
	}

	if ( sth->sth_flags & F_STH_LINKED ) {
		revents = POLLNVAL;
		goto done;
	}

	revents = 0;
	if ( (events & (POLLPRI|POLLIN|POLLRDNORM|POLLRDBAND|POLLMSG) )
	&&   (mp = sth->sth_rq->q_first) ) {
		if ( mp->b_datap->db_type >= QPCTL ) {
			if (events & POLLPRI)
				revents |= POLLPRI;
			if ((events & POLLMSG)
			&&  mp->b_datap->db_type == M_PCSIG
			&&  (int)*mp->b_rptr == SIGPOLL)
				revents |= POLLMSG;
		} else {
			if (events & POLLIN)
				revents |= POLLIN;
			if ((events & POLLMSG)
			&&  mp->b_datap->db_type == M_SIG
			&&  (int)*mp->b_rptr == SIGPOLL)
				revents |= POLLMSG;
			if ( mp->b_band == 0 ) {
				if (events & POLLRDNORM)
					revents |= POLLRDNORM;
			} else if (events & POLLRDBAND)
				revents |= POLLRDBAND;
		}
	}

	if ( sth->sth_flags & F_STH_HANGUP ) {
		revents |= POLLHUP;
		goto done;
	}

	if (events & (POLLOUT | POLLWRNORM | POLLWRBAND)) {
		if (sth_canput(sth, 0)) {
			if (events & POLLOUT)
				revents |= POLLOUT;
			if (events & POLLWRNORM)
				revents |= POLLWRNORM;
		}
		if (events & POLLWRBAND) {
			int i1 = sth->sth_wq->q_nband;
			if (sth->sth_wq->q_next
			&&  sth->sth_wq->q_next->q_nband > i1)
				i1 = sth->sth_wq->q_next->q_nband;
			for ( ; i1 >= 1; i1--){
				if (sth_canput(sth, i1)) {
					revents |= POLLWRBAND;
					break;
				}
			}
		}
	}
done:
	return revents;
}

/*
 *	pse_select - select (poll) system call
 *
 *	Parameter	In/Out		Semantics
 *
 *	dev		in		device number, used to locate stream
 *	events		in		events that are requested
 *	revents		out		reported events (immediate hits)
 */

int
pse_select (dev_t dev, ushort events, ushort *revents, void *private)
{
	OSRP		osr;
	STHP		sth;
	int		error = 0;
	SQP		sq = nil(SQP);
	uint		ndelay = 0;
	queue_t *q;

	TRCHKL4T(HKWD_PSE | hkwd_pse_poll_in, dev, events, revents, private);

        ENTER_FUNC(pse_select, dev, events, revents, private, 0, 0);

	if (!(sth = (STHP)devtosth(dev))) {
		error = ENOSTR;
		goto out;
	}
	
        /*
         * The select code is slightly different than read or write.
         * We call all of the select entry points.  Any errors we get
         * are reported back up.  The only error we expect is EIO when
         * we hit a bus fault.  Usually the select code just returns
         * 0.  The -1 convention of read and write is not used.  All
         * the modules are called since the conditions may change and
         * the various modules all need to know that there is a select
         * outstanding.
         */
        for (q = sth->sth_wq; q; q = q->q_next) {
                struct wantio *w;

                if ((w = q->q_wantio) &&
                    w->wantiosw->w_select &&
                    (error = (*w->wantiosw->w_select)(q, events, revents, private)))
                        goto out;
        }

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_osrq  = &sth->sth_read_osrq;
	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);

	if (events & POLLSYNC) {
		events &= ~POLLSYNC;
		/*
		 * Set ONDELAY as a signal to the Stream Head to just
		 * check and return. If ONDELAY is not set, the Stream
                 * head will return to us, but will call back in 
		 * streams_poll_notify when an event hits.
		 */
		ndelay = F_OSR_NDELAY;
	}

	*revents |= (ushort) sth_poll_check(sth, events, *revents);

	if (events & POLLIN && !(*revents & POLLIN)) {

	    if (sth->sth_flags & F_STH_MREADON) {
		    MBLKP mp;

		    while (!(mp = allocb(sizeof(long), BPRI_HI))) {
			if (error = osr_bufcall(osr, TRUE, 0, sizeof(long), 
				BPRI_HI))
			    goto done;
			continue;
		    }
		    *(long *)mp->b_rptr = 0;
		    mp->b_wptr += sizeof (long);
		    mp->b_datap->db_type = M_READ;
		    putnext(sth->sth_wq, mp);
		    *revents |= (ushort) sth_poll_check(sth, events, *revents);
	    } 
	}

	/*
	 * defect 67269 against bos325
	 * defect 124252 against bos410
	 * Keyboard Emulation Program flag is set.  Send a ACK out to
	 * tell it that there is no data to read.
	 */
	if (sth->shttyp && (sth->shttyp->shtty_flags & F_SHTTY_KEP)) {
		if (events & POLLIN && !(*revents & POLLIN))
			error = osr_kepcheck(osr);
	}

	if ( *revents == 0  && !(ndelay & F_OSR_NDELAY))
		select_enqueue(sth, events, (chan_t)private);

done:
	csq_release(&sth->sth_rq->q_sqh);
	osr_free(osr);
out:
	TRCHKL1T(HKWD_PSE | hkwd_pse_poll_out, error);
	LEAVE_FUNC(pse_select, error);
	return error;
}

pse_revoke(dev, private, fflag)
        dev_t           dev;
        void            *private;
        int             fflag;
{
	STHP sth;
	OSRP osr;
	int error;
extern	int osr_revoke();

	TRCHKL3T(HKWD_PSE | hkwd_pse_revoke_in, dev, private, fflag);

        ENTER_FUNC(pse_revoke, dev, private, fflag, 0, 0, 0);

	if (!(sth = (STHP)devtosth(dev))) {
		error = ENOSTR;
		goto re_out;
	}
	
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_handler    = osr_revoke;
	osr->osr_osrq       = &sth->sth_ioctl_osrq;
	osr->osr_closeout   = RWHL_ERROR_FLAGS;
	osr->osr_ioctl_arg1 = (int)fflag;

	error = osr_run(osr);

re_out:
	LEAVE_FUNC(pse_revoke, error);

	TRCHKL1T(HKWD_PSE | hkwd_pse_revoke_out, error);
	return error;
}

/*
 * Driver interface for cred passed to open procedures.
 */
int 
drv_priv(cred_t *cr)
{
	return (cr->cr_uid == 0 ||
		(cr->cr_epriv.pv_priv[0] || cr->cr_epriv.pv_priv[1]))
		? 0 : EPERM;
}
