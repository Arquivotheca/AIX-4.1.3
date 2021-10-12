static char sccsid[] = "@(#)33        1.25  src/bos/kernext/pse/str_util.c, sysxpse, bos41J, 9521B_all 5/25/95 11:56:48";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      adjmsg
 *                 allocbi
 *                 alloc_qband
 *                 backq
 *                 bcanput
 *                 canput
 *                 copyb
 *                 copymsg
 *                 dupb
 *                 dupmsg
 *                 esballoc
 *                 flushband
 *                 flushq
 *                 freemsg
 *                 getadmin
 *                 getmid
 *                 getq
 *                 insq
 *                 insqband
 *                 linkb
 *                 msgdsize
 *                 pullupmsg
 *                 putbq
 *                 putbqband
 *                 putctl_comm
 *                 putctl
 *                 putctl1
 *                 putctl2
 *                 puthere
 *                 putnext
 *                 putq
 *                 putqband
 *                 putq_deferred
 *                 putq_owned
 *                 qreply
 *                 qsize
 *                 rmvb
 *                 rmvq
 *                 strqget
 *                 strqset
 *                 testb
 *                 unlinkb
 *                 canenable
 *                 datamsg
 *                 enableok
 *                 noenable
 *                 OTHERQ
 *                 RD
 *                 WR
 *                 wantio
 *                 wantmsg
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/


#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <sys/strstat.h>

staticf	int	alloc_qband(queue_t * q, unsigned char band);
staticf int	putctl_comm(queue_t *, int, int, int, int);
staticf int	putqband(queue_t *q, MBLKP mp);
staticf void	putq_deferred(queue_t *, MBLKP);
staticf int	putbqband(queue_t *q, MBLKP mp);
staticf int	insqband(queue_t *q, MBLKP emp, MBLKP nmp);

/** Trim bytes in a message */
int
adjmsg (mp, len_to_trim)
	MBLKP	mp;
	int	len_to_trim;
{
reg	int	len_we_have;
reg	MBLKP	mp1;
	int	type;

	mp1 = mp;
	len_we_have = mp1->b_wptr - mp1->b_rptr;
	if (len_to_trim >= 0) {
		if (len_we_have < len_to_trim) {
			type = mp1->b_datap->db_type;
			do {		
				mp1 = mp1->b_cont;
				if (!mp1  ||  mp1->b_datap->db_type != type)
					return 0;
				len_we_have += mp1->b_wptr - mp1->b_rptr;
			} while (len_we_have < len_to_trim);
			do {
				mp->b_rptr = mp->b_wptr;
				mp = mp->b_cont;
			} while (mp != mp1);
		}
		mp1->b_rptr = mp1->b_wptr - (len_we_have - len_to_trim);
		return 1;
	}
	len_to_trim = -len_to_trim;
	if (!mp1->b_cont) {
		if (len_we_have >= len_to_trim) {
			mp1->b_wptr -= len_to_trim;
			return 1;
		}
		return 0;
	}
	type = mp1->b_datap->db_type;
	do {
		mp1 = mp1->b_cont;
		if (mp1->b_datap->db_type != type) {
			mp = mp1;
			type = mp1->b_datap->db_type;
			len_we_have = 0;
		}
		len_we_have += mp1->b_wptr - mp1->b_rptr;
	} while (mp1->b_cont);
	if ((mp1->b_wptr - mp1->b_rptr) >= len_to_trim) {
		mp1->b_wptr -= len_to_trim;
		return 1;
	}
	if (len_we_have < len_to_trim)
		return 0;
	mp1 = mp;
	for (;;) {
		len_we_have -= (mp1->b_wptr - mp1->b_rptr);
		if (len_we_have <= len_to_trim)
			break;
		mp1 = mp1->b_cont;
	}
	mp1->b_wptr -= (len_to_trim - len_we_have);
	while (mp1 = mp1->b_cont)
		mp1->b_wptr = mp1->b_rptr;
	return 1;
}

/*
 * Allocate an MBLK which points to data which must be freed using some
 * OS-specific mechanism.  allocbi is typically used to avoid a data copy
 * on inbound data, while providing a callback to later free the system buffer.
 */
MBLKP
allocbi (size, pri, f, arg, ptr)
	int		size;
	int		pri;
	void		(*f)(char *, char *);
	char *		arg;
	uchar *		ptr;
{
reg	MHP	mh;

	mh = (MHP)allocb(0, pri);
	if (!mh)
		return nil(MBLKP);
	mh->mh_frtn.free_func = f;
	mh->mh_frtn.free_arg = arg;
	mh->mh_dblk.db_frtnp = &mh->mh_frtn;
	mh->mh_mblk.b_rptr = ptr;
	mh->mh_mblk.b_wptr = ptr;
	mh->mh_dblk.db_base = ptr;
	mh->mh_dblk.db_lim = ptr + size;
	mh->mh_dblk.db_size = size;
	mh->mh_mblk.b_flag |= MSGEXT;
	return &mh->mh_mblk;
}

/*
 * This routine allocates a qband structure for "band".
 * Note that bands are allocated as a contiguous array.  When a high
 * band number is created, all lower bands are also created (this is
 * according to spec).
 * This routine is called after the queue is locked.
 */
staticf int
alloc_qband (
	queue_t	* q,
	unsigned char	band)
{
reg	qband_t	* qb;
	int	i;

	NET_MALLOC(qb, qband_t *, sizeof(qband_t) * band, M_STRQBAND, M_NOWAIT);
	if (!qb)
		return 0;
	if (q->q_bandp) {
		/* Avoid bcopy on structure array */
		for (i = 0; i < q->q_nband; i++)
			qb[i] = (q->q_bandp)[i];
		NET_FREE(q->q_bandp, M_STRQBAND);
	}
	q->q_bandp = qb;
	qb += band - 1;
	qb->qb_next = nilp(qband_t);
	for (;;) {
		qb->qb_count = 0;
		qb->qb_first = nil(MBLKP);
		qb->qb_last = nil(MBLKP);
		qb->qb_hiwat = q->q_hiwat;
		qb->qb_lowat = q->q_lowat;
		qb->qb_flag = 0;
		qb--;
		if (qb < &q->q_bandp[q->q_nband])
			break;
		qb->qb_next = &qb[1];
	}
	while (qb >= q->q_bandp) {
		qb->qb_next = &qb[1];
		qb--;
	}
	q->q_nband = band;
	return 1;
}

/** Get pointer to the queue behind a given queue */
queue_t *
backq (q)
	queue_t	* q;
{
reg	queue_t	* q1;

	if (q1 = q->q_bfcp) {
		/* Walk forward from the backward flow control pointer.
		 * Note that this walk along q_next is safe since
		 * the path from our flow control pointer to ourselves
		 * cannot change while we control our queue.
		 */
		while ( q1->q_next != q )
			q1 = q1->q_next;
		return q1;
	}
	return nilp(queue_t);
}

/*
 * Test for room in a queue band.
 * See Synchronization note in canput.
 */
int
bcanput (
reg	queue_t		* q,
	unsigned char	pri)
{
reg	qband_t	* qb;
	int status;
	DISABLE_LOCK_DECL


	/* If pri is 0, bcanput is the same as canput. */
	if (pri == 0) {
		return canput(q);
	}

	if ( !q  ||  pri > MAX_QBAND ) {
		return 0;
	}
	if ( !q->q_qinfo->qi_srvp  &&  q->q_ffcp )
		q = q->q_ffcp;
	/* Guard against reallocation of the qband array in putq and friends.*/
	DISABLE_LOCK(&q->q_qlock);
	/* If the band does not exist, it is writable. */
	if (pri > q->q_nband)
		status = 1;
	else {
		qb = &q->q_bandp[pri-1];
		if (qb->qb_flag & QB_FULL) {
			status = 0;
			qb->qb_flag |= QB_WANTW;
		} else
			status = 1;
	}
	DISABLE_UNLOCK(&q->q_qlock);

	return status;
}

/*
 * Test for room in a queue.
 *
 * Synchronization note:
 * q->q_ffcp is the pointer to the next queue downstream with an associated
 * service routine.  This is the queue that we depend on for flow control. 
 * q->q_ffcp is maintained by the Streams plumbing routines (e.g., osr_push,
 * osr_pop).  It will never become invalid except while the plumbing routine
 * has exclusive control of q.  This scheme requires that any changes to
 * q->q_next must be done by "official" Streams routines, otherwise the
 * flow control pointers will not be accurate!
 * Note we still need to lock the ffcp if we need to set bits.
 */
int
canput (q)
reg	queue_t	* q;
{
	int status = 0;
	DISABLE_LOCK_DECL

	if ( q ) {
		if ( !q->q_qinfo->qi_srvp  &&  q->q_ffcp )
			q = q->q_ffcp;
		/*
		** Obtain queue sync to request later qenable if full.
		*/
		if (q->q_flag & QFULL)  {
			DISABLE_LOCK(&q->q_qlock);
			if (q->q_flag & QFULL) {
				q->q_flag |= QWANTW;
			} else
				status = 1;
			DISABLE_UNLOCK(&q->q_qlock);
		} else
			status = 1;
	}
	return status;
}

/** Copy a message block */
MBLKP
copyb (bp)
	MBLKP	 bp;
{
reg	dblk_t	* dp, * new_dp;
reg	MBLKP	 new_bp;

	dp = bp->b_datap;
	if (!(new_bp = allocb(dp->db_size, BPRI_MED)))
		return nil(MBLKP);
	new_bp->b_band = bp->b_band;
	new_dp = new_bp->b_datap;
	new_bp->b_rptr = new_dp->db_base + (bp->b_rptr - dp->db_base);
	new_bp->b_wptr = new_dp->db_base + (bp->b_wptr - dp->db_base);
	new_dp->db_type = dp->db_type;
	bcopy((char *)bp->b_rptr, (char *)new_bp->b_rptr,
		bp->b_wptr - bp->b_rptr);
	return new_bp;
}

/** Copy a message */
MBLKP
copymsg (mp)
reg	MBLKP	 mp;
{
reg	MBLKP	 mp1;
	MBLKP	 new_mp;

	if (!(mp1 = copyb(mp)))
		return nil(MBLKP);
	new_mp = mp1;
	while (mp = mp->b_cont) {
		mp1->b_cont = copyb(mp);
		if (!(mp1 = mp1->b_cont)) {
			freemsg(new_mp);
			return nil(MBLKP);
		}
	}
	return new_mp;
}

/** Duplicate a message block descriptor */
MBLKP
dupb (bp)
reg	MBLKP	 bp;
{
reg	MBLKP	 new_bp;
decl_simple_lock_data(extern, mblk_lock)
	DISABLE_LOCK_DECL


	if (new_bp = allocb(0, BPRI_MED)) {
		new_bp->b_rptr = bp->b_rptr;
		new_bp->b_wptr = bp->b_wptr;
		new_bp->b_band = bp->b_band;
		new_bp->b_flag = bp->b_flag;
		new_bp->b_datap->db_ref = 0;
		new_bp->b_datap = bp->b_datap;
		DISABLE_LOCK(&mblk_lock);
		bp->b_datap->db_ref++;
		DISABLE_UNLOCK(&mblk_lock);
	}
	return new_bp;
}

/** Duplicate a message */
MBLKP
dupmsg (mp)
reg	MBLKP	 mp;
{
reg	MBLKP	 mp1;
	MBLKP	 new_mp;

	if (!(mp1 = dupb(mp)))
		return nil(MBLKP);
	new_mp = mp1;
	while (mp = mp->b_cont) {
		mp1->b_cont = dupb(mp);
		if (!(mp1 = mp1->b_cont)) {
			freemsg(new_mp);
			return nil(MBLKP);
		}
	}
	return new_mp;
}

/**
 * Allocate message and data blocks.
 * This routine performs the same function as allocbi.
 */
mblk_t *
esballoc (base, size, pri, fr_rtn)
	unsigned char	* base;
	int	size;
	int	pri;
	frtn_t	* fr_rtn;
{
reg	MHP	mh;

	if (!base  ||  size == 0  ||  !fr_rtn  ||  !fr_rtn->free_func)
		return nil(MBLKP);
	mh = (MHP)allocb(0, pri);
	if (!mh)
		return nil(MBLKP);
	mh->mh_frtn = *fr_rtn;
	mh->mh_mblk.b_rptr = base;
	mh->mh_mblk.b_wptr = base;
	mh->mh_dblk.db_base = base;
	mh->mh_dblk.db_lim = base + size;
	mh->mh_dblk.db_size = size;
	mh->mh_dblk.db_frtnp = fr_rtn;
	mh->mh_mblk.b_flag |= MSGEXT;
	return &mh->mh_mblk;
}

/**
 * Flush a queue band.
 * See synchronization note accompanying putq.
 */
void
flushband (
reg	queue_t		* q,
	unsigned char	pri,
	int		flag)
{
reg	MBLKP	mp;
reg	MBLKP	mp1;
	MBLKP	mp2;
	int	type;
	int	notifyonly = flag & FLUSH_NOTIFYONLY;
	queue_t	* enable_q = nil(queue_t *);
	qband_t * qb;
	DISABLE_LOCK_DECL

	if (pri == 0) {
		flushq(q, flag);
		return;
	}
	DISABLE_LOCK(&q->q_qlock);
	if (pri > q->q_nband) {
		DISABLE_UNLOCK(&q->q_qlock);
		return;
	}
	qb = &q->q_bandp[pri-1];
	mp = qb->qb_first;
	flag &= ~FLUSH_NOTIFYONLY;
	for (; mp && mp->b_band == pri; mp = mp1) {
		mp1 = mp->b_next;
		if (flag == FLUSHDATA) {
			type = mp->b_datap->db_type;
			if (!(datamsg(type) || type == M_HPDATA))
				continue;
		}
		if (notifyonly && !(mp->b_flag & MSGNOTIFY))
			continue;
		if (qb->qb_first == mp) {
			if (qb->qb_last == mp) {
				qb->qb_first = nil(MBLKP);
				qb->qb_last = nil(MBLKP);
			} else
				qb->qb_first = mp->b_next;
		} else if (qb->qb_last == mp)
			qb->qb_last = mp->b_prev;

		if (q->q_first == mp)
			q->q_first = mp->b_next;
		else if (mp->b_prev)
			mp->b_prev->b_next = mp->b_next;
		if (mp->b_next)
			mp->b_next->b_prev = mp->b_prev;
		else if (q->q_last == mp)
			q->q_last = mp->b_prev;

		mp->b_next = mp->b_prev = nil(MBLKP);

		mp2 = mp;
		do {
			qb->qb_count -= mp->b_wptr - mp->b_rptr;
		} while (mp = mp->b_cont);

		freemsg(mp2);
	}
	if ((qb->qb_flag & QB_FULL)  &&  qb->qb_count <= qb->qb_lowat) {
		qb->qb_flag &= ~QB_FULL;
		if (qb->qb_flag & QB_WANTW) {
			qb->qb_flag &= ~QB_WANTW;
			enable_q = q->q_bfcp;
		}
	}
	DISABLE_UNLOCK(&q->q_qlock);
	if (enable_q && enable_q->q_qinfo->qi_srvp)
		qenable(enable_q);
}

/**
 * Flush a queue.
 * See synchronization note accompanying putq.
 */
void
flushq (q, flag)
reg	queue_t	* q;
	int	flag;
{
reg	MBLKP		mp;
reg	MBLKP		mp1;
	MBLKP		mp2;
	int		can_close = flag & FLUSH_CAN_CLOSE;
	int		notifyonly = flag & FLUSH_NOTIFYONLY;
	int		type;
	queue_t		* enable_q = nil(queue_t *);
	DISABLE_LOCK_DECL

	flag &= ~(FLUSH_CAN_CLOSE | FLUSH_NOTIFYONLY);
	DISABLE_LOCK(&q->q_qlock);
	for (mp = q->q_first; mp; mp = mp1) {
	    mp1 = mp->b_next;
	    type = mp->b_datap->db_type;
	    if (flag == FLUSHDATA && !(datamsg(type) || type == M_HPDATA))
		    continue;
	    if (notifyonly && !(mp->b_flag & MSGNOTIFY))
		    continue;
	    /*
	     * Special case M_PASSFP:
	     * This message type carries a file reference with it.
	     * Discarding it might include a close to that file.
	     * We can only risk to do that if we are in process
	     * context (FLUSH_CAN_CLOSE). The last flush will be
	     * called from close, so we will get rid of the message
	     * at least then.
	     */
	    if ( type == M_PASSFP && !can_close )
		    continue;
	    if (q->q_first == mp)
		    q->q_first = mp->b_next;
	    else if (mp->b_prev)
		    mp->b_prev->b_next = mp->b_next;
	    if (mp->b_next)
		    mp->b_next->b_prev = mp->b_prev;
	    else if (q->q_last == mp)
		    q->q_last = mp->b_prev;
	    mp->b_flag &= ~MSGCOMPRESS;
	    if (mp->b_band == 0) {
		mp->b_next = mp->b_prev = nil(MBLKP);
		mp2 = mp;
		do {
			q->q_count -= mp->b_wptr - mp->b_rptr;
		} while (mp = mp->b_cont);
		if ((q->q_flag & QFULL)  &&  ((q->q_int_count +
					q->q_count) <= q->q_lowat)) {
			q->q_flag &= ~QFULL;
			if (q->q_flag & QWANTW) {
				q->q_flag &= ~QWANTW;
				enable_q = q->q_bfcp;
			}
		}
	    } else if (mp->b_band <= q->q_nband) {
		qband_t * qb;

		qb = &q->q_bandp[mp->b_band-1];
		if (qb->qb_first == mp) {
			if (qb->qb_last == mp) {
				qb->qb_first = nil(MBLKP);
				qb->qb_last = nil(MBLKP);
			} else
				qb->qb_first = mp->b_next;
		} else if (qb->qb_last == mp)
			qb->qb_last = mp->b_prev;
		mp->b_next = mp->b_prev = nil(MBLKP);
		mp2 = mp;
		do {
			qb->qb_count -= mp->b_wptr - mp->b_rptr;
		} while (mp = mp->b_cont);
		if ((qb->qb_flag & QB_FULL)  &&  qb->qb_count <= qb->qb_lowat) {
			qb->qb_flag &= ~QB_FULL;
			if (qb->qb_flag & QB_WANTW) {
				qb->qb_flag &= ~QB_WANTW;
				enable_q = q->q_bfcp;
			}
		}
	    }
	    if ( type == M_PASSFP ) {
		    DISABLE_UNLOCK(&q->q_qlock);
		    discard_passfp(mp2);
		    DISABLE_LOCK(&q->q_qlock);
	    }
	    freemsg(mp2);
	}
	DISABLE_UNLOCK(&q->q_qlock);
	if (enable_q && enable_q->q_qinfo->qi_srvp) {
		qenable(enable_q);
	}	
}

/** Free all message blocks in a message */
void
freemsg (mp)
	MBLKP	 mp;
{
reg	MBLKP	 mp1;

	while (mp1 = mp) {
		mp = mp1->b_cont;
		freeb(mp1);
	}
}

/** Get a module's qadmin pointer */
int (*
getadmin (ushort mid))(void)
{
	struct streamtab	* str;

	if (str = mid_to_str(mid))
		return str->st_rdinit->qi_qadmin;
	return 0;
}

/** Get a module's idnum */
ushort
getmid (name)
	char	* name;
{
	struct streamtab	* str;

	if (str = fname_to_str(name))
		return str->st_rdinit->qi_minfo->mi_idnum;
	if (str = dname_to_str(name))
		return str->st_rdinit->qi_minfo->mi_idnum;
	return 0;
}

/**
 * Get a message from a queue.
 * See synchronization note accompanying putq.
 */
MBLKP
getq (q)
reg	queue_t	* q;
{
reg	MBLKP	mp;
reg	MBLKP	mp1;
	queue_t	* enable_q = nil(queue_t *);
	qband_t	* qb;
	DISABLE_LOCK_DECL


	DISABLE_LOCK(&q->q_qlock);
	if (!(mp = q->q_first)) {
		q->q_flag |= QWANTR;
		DISABLE_UNLOCK(&q->q_qlock);
		return nil(MBLKP);
	}

	if (q->q_first = mp->b_next)
		mp->b_next->b_prev = nil(MBLKP);
	else if (q->q_last == mp)
		q->q_last = nil(MBLKP);
	mp->b_flag &= ~MSGCOMPRESS;
	mp1 = mp;
	if (mp->b_band == 0) {
		/* Normal messages (including high pri ones) */
		do {
			q->q_count -= mp->b_wptr - mp->b_rptr;
		} while (mp = mp->b_cont);
		if ((q->q_flag & QFULL)  &&  ((q->q_int_count +
					q->q_count) <= q->q_lowat)) {
			q->q_flag &= ~QFULL;
			if (q->q_flag & QWANTW) {
				q->q_flag &= ~QWANTW;
				enable_q = q->q_bfcp;
			}
		}
	} else if (mp->b_band <= q->q_nband) {
		/* Above can only fail if a module changes b_band
		 * while the message is in the queue (a BIG no-no). */
		qb = &q->q_bandp[mp->b_band-1];
		if (qb->qb_last == mp) {
			qb->qb_first = nil(MBLKP);
			qb->qb_last = nil(MBLKP);
		} else
			qb->qb_first = mp->b_next;
		do {
			qb->qb_count -= mp->b_wptr - mp->b_rptr;
		} while (mp = mp->b_cont);
		if ((qb->qb_flag & QB_FULL)  &&  qb->qb_count <= qb->qb_lowat) {
			qb->qb_flag &= ~QB_FULL;
			if (qb->qb_flag & QB_WANTW) {
				qb->qb_flag &= ~QB_WANTW;
				enable_q = q->q_bfcp;
			}
		}
	}
	DISABLE_UNLOCK(&q->q_qlock);
	mp1->b_next = mp1->b_prev = nil(MBLKP);
	if (enable_q && enable_q->q_qinfo->qi_srvp) {
		qenable(enable_q);
	}
	return mp1;
}

/**
 * Put a message at a specific place in a queue.
 * See synchronization note accompanying putq.
 */
int
insq (q, emp, nmp)
reg	queue_t	* q;
	MBLKP	 emp;
reg	MBLKP	 nmp;
{
	int	should_schedule;
	DISABLE_LOCK_DECL

	
	if (nmp->b_band) {
		if (nmp->b_datap->db_type >= QPCTL) {
			return 0;
		}
		return insqband(q, emp, nmp);
	}
	DISABLE_LOCK(&q->q_qlock);
	if (!emp) {		/* Place nmp at end of queue */
		if (q->q_last) {
			if (nmp->b_datap->db_type >= QPCTL
			&&  q->q_last->b_datap->db_type < QPCTL) {
				DISABLE_UNLOCK(&q->q_qlock);
				return 0;
			}
			nmp->b_prev = q->q_last;
			q->q_last->b_next = nmp;
		} else {
			nmp->b_prev = nil(MBLKP);
			q->q_first = nmp;
		}
		q->q_last = nmp;
		nmp->b_next = nil(MBLKP);
	} else {		/* Place nmp before emp in queue */
		if (nmp->b_datap->db_type >= QPCTL) {
			if (emp->b_prev
			&&  emp->b_prev->b_datap->db_type < QPCTL) {
				DISABLE_UNLOCK(&q->q_qlock);
				return 0;
			}
		} else if (emp->b_datap->db_type >= QPCTL
		       ||  emp->b_band > nmp->b_band) {
			DISABLE_UNLOCK(&q->q_qlock);
			return 0;
			}
		if (emp == q->q_first)
			q->q_first = nmp;
		else
			emp->b_prev->b_next = nmp;
		nmp->b_prev = emp->b_prev;
		nmp->b_next = emp;
		emp->b_prev = nmp;
	}
	do {
		q->q_count += nmp->b_wptr - nmp->b_rptr;
	} while (nmp = nmp->b_cont);
	if ((q->q_int_count + q->q_count) >= q->q_hiwat)
		q->q_flag |= QFULL;
	/* We don't pay any attention to the priority of the message here. */
	should_schedule = ((q->q_flag & (QNOENB | QWANTR)) == QWANTR);
	DISABLE_UNLOCK(&q->q_qlock);
	if (should_schedule && q->q_qinfo->qi_srvp)
		qenable(q);
	return 1;
}

/** Put a message at a specific place in a queue band */
staticf int
insqband (q, emp, nmp)
reg	queue_t	* q;
	MBLKP	 emp;
reg	MBLKP	 nmp;
{
	int		should_schedule;
	unsigned char	band;
	qband_t		* qb;
	DISABLE_LOCK_DECL

	band = nmp->b_band;
	DISABLE_LOCK(&q->q_qlock);
	if (band > q->q_nband  &&  !alloc_qband(q, band)) {
		DISABLE_UNLOCK(&q->q_qlock);
		return 0;
	}
	qb = &q->q_bandp[band-1];
	if (!emp) {		/* Place nmp at end of queue */
		MBLKP	qlast = q->q_last;
		if (qlast) {
			if (qlast->b_datap->db_type < QPCTL
			&&  qlast->b_band < band) {
				DISABLE_UNLOCK(&q->q_qlock);
				return 0;
			}
			nmp->b_prev = qlast;
			qlast->b_next = nmp;
			if (!qb->qb_first)
				qb->qb_first = nmp;
		} else {
			q->q_first = nmp;
			qb->qb_first = nmp;
		}
		q->q_last = nmp;
		qb->qb_last = nmp;
		nmp->b_next = nil(MBLKP);
	} else {		/* Place nmp before emp in queue */
		if (emp->b_datap->db_type >= QPCTL
		|| (emp->b_band  &&  band < emp->b_band)
		|| (emp->b_prev  &&  emp->b_prev->b_datap->db_type < QPCTL
		&&  band > emp->b_prev->b_band)) {
			DISABLE_UNLOCK(&q->q_qlock);
			return 0;
		}
		if (emp == q->q_first)
			q->q_first = nmp;
		else
			emp->b_prev->b_next = nmp;
		nmp->b_prev = emp->b_prev;
		nmp->b_next = emp;
		emp->b_prev = nmp;
		if (!qb->qb_first) {
			qb->qb_last = nmp;
			qb->qb_first = nmp;
		} else if (qb->qb_first == emp)
			qb->qb_first = nmp;
	}
	do {
		qb->qb_count += nmp->b_wptr - nmp->b_rptr;
	} while (nmp = nmp->b_cont);
	if (qb->qb_count >= qb->qb_hiwat)
		qb->qb_flag |= QB_FULL;
	should_schedule = ((q->q_flag & (QNOENB | QWANTR)) == QWANTR);
	DISABLE_UNLOCK(&q->q_qlock);
	if (should_schedule && q->q_qinfo->qi_srvp)
		qenable(q);
	return 1;
}

/** Concatenate two messages into one */
void
linkb (mp1, mp2)
reg	MBLKP	 mp1;
	MBLKP	 mp2;
{
	while (mp1->b_cont)
		mp1 = mp1->b_cont;
	mp1->b_cont = mp2;
}

/* Get the number of data bytes in a message */
int
msgdsize (mp)
reg	MBLKP	 mp;
{
reg	int	len;

	for (len = 0; mp; mp = mp->b_cont) {
		if (mp->b_datap->db_type == M_DATA
		||  mp->b_datap->db_type == M_HPDATA)
			len += mp->b_wptr - mp->b_rptr;
	}
	return len;
}

/** Concatenate bytes in a message */
int
pullupmsg (mp, len)
reg	MBLKP	 mp;
	int	len;
{
reg	unsigned char	* dst;
reg	MBLKP	 mp1;
reg	int	i1;
reg	struct datab	* db;

	if (!mp)
		return 0;
	i1 = mp->b_wptr - mp->b_rptr;
	db = mp->b_datap;
	if (len >= 0) {
		if (i1 < len) {
			int type = db->db_type;
			mp1 = mp;
			do {
				if (!(mp1 = mp1->b_cont)
				|| mp1->b_datap->db_type != type)
					return 0;
				i1 += mp1->b_wptr - mp1->b_rptr;
			} while (i1 < len);
			i1 = mp->b_wptr - mp->b_rptr;
		}
	} else {
		len = i1;
		if (mp1 = mp->b_cont) {
			int type = db->db_type;
			do {
				if (mp1->b_datap->db_type != type)
					break;
				len += mp1->b_wptr - mp1->b_rptr;
			} while (mp1 = mp1->b_cont);
		}
	}
	if (db->db_ref == 1
	&&  (dst = ALIGN(db->db_base)) <= mp->b_rptr
	&&  db->db_lim - dst >= len ) {
		if (ALIGNMENT(mp->b_rptr) != 0) {
			/* We guarantee non-overlap above. */
			bcopy((caddr_t)mp->b_rptr, (caddr_t)dst, i1);
			mp->b_rptr = dst;
		} else
			dst = mp->b_rptr;
	} else if (mp1 = allocb(max(i1, len), BPRI_MED)) {
		mp1->b_datap->db_type = db->db_type;
		bcopy((char *)mp->b_rptr, (char *)mp1->b_rptr, i1);
		/* Swap data pointers */
		mp->b_datap = mp1->b_datap;
		mp1->b_datap = db;
		dst = mp->b_rptr = mp1->b_rptr;
		freeb(mp1);
	} else
		return 0;
	for (;;) {
		dst += i1;
		len -= i1;
		if (len <= 0) {
			mp->b_wptr = dst;
			return 1;
		}
		mp1 = mp->b_cont;
		i1 = mp1->b_wptr - mp1->b_rptr;
		if (i1 <= len) {
			mp->b_cont = mp1->b_cont;
			bcopy((char *)mp1->b_rptr, (char *)dst, i1);
			freeb(mp1);
		} else {
			i1 = len;
			bcopy((char *)mp1->b_rptr, (char *)dst, i1);
			mp1->b_rptr += i1;
		}
	}
}

/**
 * Return a message to the beginning of a queue.
 * See synchronization note accompanying putq.
 */
int
putbq (q, mp)
reg	queue_t	* q;
	MBLKP	 mp;
{
reg	MBLKP	mp1;
	int	should_schedule = 0;
	DISABLE_LOCK_DECL


	if (mp->b_datap->db_type >= QPCTL)
		mp->b_band = 0;
	else if (mp->b_band) {
		return putbqband(q, mp);
	}
	DISABLE_LOCK(&q->q_qlock);
	mp1 = q->q_first;
	if (mp->b_datap->db_type < QPCTL) {
		for ( ; mp1; mp1 = mp1->b_next) {
			if (mp1->b_datap->db_type < QPCTL
			&&  mp1->b_band == 0)
				break;
		}
	} else
		should_schedule = 1;
	if (!mp1) {		/* Place mp on the end of the queue */
		mp1 = mp;
		if (mp1->b_prev = q->q_last)
			q->q_last->b_next = mp1;
		else
			q->q_first = mp1;
		q->q_last = mp1;
		mp1->b_next = nil(MBLKP);
	} else {		/* Place mp before mp1 in queue */
		if (mp1 == q->q_first)
			q->q_first = mp;
		else
			mp1->b_prev->b_next = mp;
		mp->b_prev = mp1->b_prev;
		mp->b_next = mp1;
		mp1->b_prev = mp;
		mp1 = mp;
	}
	do {
		q->q_count += mp1->b_wptr - mp1->b_rptr;
	} while (mp1 = mp1->b_cont);
	if ((q->q_int_count + q->q_count) >= q->q_hiwat)
		q->q_flag |= QFULL;
	if (should_schedule == 0)
		should_schedule = ((q->q_flag & (QNOENB | QWANTR)) == QWANTR);
	DISABLE_UNLOCK(&q->q_qlock);
	if (should_schedule && q->q_qinfo->qi_srvp)
		qenable(q);
	return 1;
}

/**
 * Return a message to the beginning of a queue band
 * See synchronization note accompanying putq.
 */
staticf int
putbqband (q, mp)
reg	queue_t	* q;
	MBLKP	 mp;
{
reg	MBLKP	mp1;
	qband_t	* qb;
	int	should_schedule;
	DISABLE_LOCK_DECL

	
	DISABLE_LOCK(&q->q_qlock);
	if (mp->b_band > q->q_nband  &&  !alloc_qband(q, mp->b_band)) {
		DISABLE_UNLOCK(&q->q_qlock);
		return 0;
	}
	qb = &q->q_bandp[mp->b_band-1];
	mp1 = qb->qb_first;
	qb->qb_first = mp;
	if (!mp1) {
		qb->qb_last = mp;
		for (mp1 = q->q_first; mp1; mp1 = mp1->b_next) {
			if (mp1->b_datap->db_type < QPCTL
			&&  mp1->b_band <= mp->b_band)
				break;
		}
	}
	if (!mp1) {
		mp1 = mp;
		if (mp1->b_prev = q->q_last)
			q->q_last->b_next = mp1;
		else
			q->q_first = mp1;
		q->q_last = mp1;
		mp1->b_next = nil(MBLKP);
	} else {
		if (mp1->b_prev)
			mp1->b_prev->b_next = mp;
		else
			q->q_first = mp;
		mp->b_prev = mp1->b_prev;
		mp->b_next = mp1;
		mp1->b_prev = mp;
		mp1 = mp;
	}
	do {
		qb->qb_count += mp1->b_wptr - mp1->b_rptr;
	} while (mp1 = mp1->b_cont);
	if (qb->qb_count >= qb->qb_hiwat)
		qb->qb_flag |= QB_FULL;
	should_schedule = ((q->q_flag & (QNOENB | QWANTR)) == QWANTR);
	DISABLE_UNLOCK(&q->q_qlock);
	if (should_schedule && q->q_qinfo->qi_srvp)
		qenable(q);
	return 1;
}

/* Put a control message with optional two parameter bytes */
staticf int
putctl_comm (q, type, len, p1, p2)
	queue_t	* q;
	int	type;
	int	len;
	int	p1;
	int	p2;
{
	MBLKP	 mp;

	if ( !q )
		return 0;
	switch (type) {
	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:
		return 0;
	default:
		break;
	}
	if (!(mp = allocb(len, BPRI_HI)))
		return 0;
	mp->b_datap->db_type = type;
	if (len >= 1)
		*mp->b_wptr++ = (char)p1;
	if (len == 2)
		*mp->b_wptr++ = (char)p2;
	puthere(q, mp);
	return 1;
}

/** Put a control message */
int
putctl (q, type)
	queue_t	* q;
	int	type;
{
	if (type == M_DELAY)
		return 0;
	return putctl_comm(q, type, 0, 0, 0);
}

/** Put a control message with a single data byte */
int
putctl1 (q, type, c)
	queue_t	* q;
	int	type;
	int	c;
{
	return putctl_comm(q, type, 1, c, 0);
}

/** Put a control message with a two data bytes */
int
putctl2 (q, type, c1, c2)
	queue_t	* q;
	int	type;
	int	c1;
	int	c2;
{
	return putctl_comm(q, type, 2, c1, c2);
}

/*
 * Put a message on the queue. This routine calls csq_lateral() to check,
 * whether the resources are busy or not. 
 */

void
puthere (q, mp)
reg	queue_t	* q;
	MBLKP	 mp;
{
	int	count = 0;
	MBLKP	 mp1;
	DISABLE_LOCK_DECL

	if (q == 0 || q->q_qinfo->qi_putp == 0)
		freemsg(mp);
	else {
		mp->b_sq.sq_entry = (sq_entry_t)q->q_qinfo->qi_putp;
		mp->b_sq.sq_queue = q;
		mp->b_sq.sq_arg0  = q;
		mp->b_sq.sq_arg1  = mp;
		csq_lateral(&q->q_sqh, &mp->b_sq, 1);
	}
}

void
putnext (q, mp)
	queue_t	* q;
	MBLKP	 mp;
{
	q = q->q_next;
	while ( q && q->q_is_interesting
		&& !q->q_first && ! ((*q->q_is_interesting) (mp))) {
		q = q->q_next;
	}
	puthere(q, mp);
}

/**
 * Put a message on a queue.
 * 
 * Synchronization note: 
 *
 * The routines that perform insertion and removal of elements from Streams
 * queues must be called while holding synchronized control of the target
 * queue. 
 * These routines include flushq, flushband, getq, insq, putbq, qsize, and rmvq.
 *
 * putq is special in that it can be called from drivers as the first entry
 * of an execution thread into the Streams synchronization context.  Since
 * putq may be called at high priority, something must be done to ensure
 * synchronized access to the target queue, preferrably without imposing
 * major overhead on all the other invocations of the above routines. 
 * To do this, we detect here an "unsafe" target queue which we define as
 * one that has no backward flow control pointer.  This indicates a module
 * on the bottom end of the stream.  In this case, we arrange for a "safe"
 * callback using the csq_lateral mechanism.  In all other cases, we plow
 * ahead assuming exclusive control of the target. Note that if a module,
 * (or more likely a multiplexor), wants to be able to target an arbitrary
 * queue that it has some knowledge of, it must assure synchronized access
 * to that queue using (for example) MODULE, or module family, synchronization.
 */
int
putq (q, mp)
reg	queue_t	* q;
	MBLKP	 mp;
{
	ASSERT(q != 0 && mp != 0);
	if (!q->q_bfcp) {
		mp->b_sq.sq_entry = (sq_entry_t)putq_deferred;
		mp->b_sq.sq_queue = q;
		mp->b_sq.sq_arg0  = q;
		mp->b_sq.sq_arg1  = mp;
		csq_lateral(&q->q_sqh, &mp->b_sq, 0);
		/*
		 * Putq_deferred() notes any failure and frees message.
		 * We'd like to return this to the driver, but to do so
		 * requires more work in this, the interrupt path.
		 * Therefore it is recommended to use strqset() on the
		 * read queue in the open procedure of a driver which
		 * generates banded messages. This will guarantee that
		 * this putq will not fail (it may in fact succeed, but
		 * no status can be returned).
		 */ 
		return 1;
	}
	return putq_owned(q, mp);
}

staticf int
putqband (q, mp)
reg	queue_t	* q;
	MBLKP	 mp;
{
	unsigned char	band;
reg	MBLKP	mp1;
	qband_t	* qb;
	int	should_schedule;
	int	count = 0;
	DISABLE_LOCK_DECL

 
	band = mp->b_band;
	DISABLE_LOCK(&q->q_qlock);
	/*
	 * We believe the possibility of putqband() failing makes for
	 * a violation of basic Streams assumptions, i.e. that putq()
	 * may fail. However, the SVR4 spec calls for it.
	 */
	if (band > q->q_nband  &&  !alloc_qband(q, band)) {
		DISABLE_UNLOCK(&q->q_qlock);
		return 0;
	}
	qb = &q->q_bandp[band-1];
	mp1 = qb->qb_last;
	qb->qb_last = mp;
	if (mp1) {
		/*
		 * There's already a message in the band, so just drop this
		 * new message in at the end. Compress like mblks.
		 */
		if ((mp->b_flag & mp1->b_flag & MSGCOMPRESS)
		&&  mp->b_datap->db_type == mp1->b_datap->db_type
		&&  !mp1->b_cont && (mp1->b_wptr - mp1->b_rptr) > 0
		&&  !mp->b_cont && (count = mp->b_wptr - mp->b_rptr) > 0
		&&  count <= mp1->b_datap->db_lim - mp1->b_wptr) {
			bcopy(mp->b_rptr, mp1->b_wptr, count);
			mp1->b_wptr += count;
			freeb(mp);
		} else {
			count = 0;
			if (mp1->b_next)
				mp1->b_next->b_prev = mp;
			else
				q->q_last = mp;
			mp->b_next = mp1->b_next;
			mp1->b_next = mp;
			mp->b_prev = mp1;
		}
	} else {
		/* This band is empty */
		qb->qb_first = mp;
		if (mp1 = q->q_first) {
			/*
			 * There is at least one message on the queue, so
			 * walk through until we find the proper location
			 * for this new message.
			 */
			while (mp1  &&  mp1->b_datap->db_type >= QPCTL)
				mp1 = mp1->b_next;
			if (mp1) {
				while (mp1  &&  mp1->b_band >= band)
					mp1 = mp1->b_next;
			}
			if (mp1) {
				if (mp1->b_prev)
					mp1->b_prev->b_next = mp;
				else
					q->q_first = mp;
				mp->b_prev = mp1->b_prev;
				mp1->b_prev = mp;
				mp->b_next = mp1;
			} else {
				q->q_last->b_next = mp;
				mp->b_prev = q->q_last;
				q->q_last = mp;
				mp->b_next = nil(MBLKP);
			}
		} else {
			/*
			 * The queue was empty, so this is the only message
			 * on it.
			 */
			q->q_first = mp;
			q->q_last = mp;
		}
	}
	if (count)
		qb->qb_count += count;	/* compressed */
	else {
		mp1 = mp;
		do {
			qb->qb_count += mp1->b_wptr - mp1->b_rptr;
		} while (mp1 = mp1->b_cont);
	}
	if (qb->qb_count >= qb->qb_hiwat)
		qb->qb_flag |= QB_FULL;
	should_schedule = ((q->q_flag & (QNOENB | QWANTR)) == QWANTR);
	DISABLE_UNLOCK(&q->q_qlock);
	if (should_schedule && q->q_qinfo->qi_srvp)
		qenable(q);
#ifdef MACH_ASSERT
	ASSERT(q->q_first != NULL);
#endif
	return 1;
}

/*
 * Called from putq when lateral required and putq might fail.
 */
staticf void
putq_deferred (q, mp)
	queue_t	* q;
	MBLKP	 mp;
{
#ifdef MACH_ASSERT
	ASSERT(q != NULL && mp != NULL);
#endif
	if (putq_owned(q, mp) == 0)
		freemsg(mp);
}

/*
 * putq_owned is the functional part of putq. It is used as the callback
 * where putq is forced to lateral, via putq_deferred which checks for
 * any failures due to banding.
 * putq_owned is also called by the Stream head code, since we know we
 * have exclusive control in those contexts.
 * It is not intended that this routine be visible externally
 * (i.e., to module writers).
 */
int
putq_owned (q, mp)
reg	queue_t	* q;
	MBLKP	 mp;
{
reg	MBLKP	mp1;
	int	should_schedule = 0;
	int	count = 0;
	DISABLE_LOCK_DECL

#ifdef MACH_ASSERT
	ASSERT(q != 0 && mp != 0);
#endif
	if (mp->b_datap->db_type >= QPCTL)
		mp->b_band = 0;
	else if (mp->b_band) {
		return putqband(q, mp);
	}
	mp1 = nil(MBLKP);
	DISABLE_LOCK(&q->q_qlock);
	if (mp->b_datap->db_type >= QPCTL) {
		for (mp1 = q->q_first; mp1; mp1 = mp1->b_next) {
			if (mp1->b_datap->db_type < QPCTL) {
				if (mp1 == q->q_first)
					q->q_first = mp;
				else
					mp1->b_prev->b_next = mp;
				mp->b_prev = mp1->b_prev;
				mp->b_next = mp1;
				mp1->b_prev = mp;
				mp1 = mp;
				break;
			}
		}
		should_schedule = 1;
	}
	if (!mp1) {		/* Place mp at end of queue */
		if ((mp1 = q->q_last)
		&&  (mp->b_flag & mp1->b_flag & MSGCOMPRESS)
		&&  mp->b_datap->db_type == mp1->b_datap->db_type
		&&  !mp1->b_cont && (mp1->b_wptr - mp1->b_rptr) > 0
		&&  !mp->b_cont && (count = mp->b_wptr - mp->b_rptr) > 0
		&&  count <= mp1->b_datap->db_lim - mp1->b_wptr) {
			bcopy(mp->b_rptr, mp1->b_wptr, count);
			mp1->b_wptr += count;
			freeb(mp);
		} else {
			count = 0;
			mp1 = mp;
			if (mp1->b_prev = q->q_last)
				q->q_last->b_next = mp1;
			else
				q->q_first = mp1;
			q->q_last = mp1;
			mp1->b_next = nil(MBLKP);
		}
	}
#ifdef MACH_ASSERT
	ASSERT(q->q_first != NULL);
#endif
	if (count)
		q->q_count += count;	/* compressed */
	else {
		do {
			q->q_count += mp1->b_wptr - mp1->b_rptr;
		} while (mp1 = mp1->b_cont);
	}
	if ((q->q_int_count + q->q_count) >= q->q_hiwat)
		q->q_flag |= QFULL;
	if (should_schedule == 0)
		should_schedule = ((q->q_flag & (QNOENB | QWANTR)) == QWANTR);
	DISABLE_UNLOCK(&q->q_qlock);
	if (should_schedule && q->q_qinfo->qi_srvp)
		qenable(q);
	return 1;
}

void
qreply (q, mp)
	queue_t	* q;
	MBLKP	 mp;
{
	q = OTHERQ(q)->q_next;
	while ( q && q->q_is_interesting
		&& !q->q_first && ! ((*q->q_is_interesting) (mp))) {
		q = q->q_next;
	}
	puthere(q, mp);
}

/**
 * Find the number of messages on a queue.
 * See synchronization note accompanying putq.
 */
int
qsize (q)
	queue_t	* q;
{
reg	MBLKP	mp;
reg	int	cnt = 0;

	for (mp = q->q_first; mp; mp = mp->b_next)
		cnt++;
	return cnt;
}

/** Remove a message block from a message */
MBLKP
rmvb (mp, bp)
	MBLKP	 mp;
reg	MBLKP	 bp;
{
reg	MBLKP	 m = mp;

	if (!m  ||  !bp)
		return (MBLKP)-1;
	if (m == bp)
		mp = m->b_cont;
	else {
		while (m->b_cont != bp) {
			if (!(m = m->b_cont))
				return (MBLKP)-1;
		}
		m->b_cont = bp->b_cont;
	}
	bp->b_cont = nil(MBLKP);
	return mp;
}

/**
 * Remove a message from a queue.
 * See synchronization note accompanying putq.
 */
void
rmvq (q, mp)
	queue_t	* q;
reg	MBLKP	 mp;
{
	queue_t	* enable_q = nil(queue_t *);
	qband_t	* qb;
	DISABLE_LOCK_DECL


	DISABLE_LOCK(&q->q_qlock);
	if (q->q_first == mp)
		q->q_first = mp->b_next;
	else if (mp->b_prev)
		mp->b_prev->b_next = mp->b_next;
	if (mp->b_next)
		mp->b_next->b_prev = mp->b_prev;
	else if (q->q_last == mp)
		q->q_last = mp->b_prev;
	mp->b_flag &= ~MSGCOMPRESS;
	if (mp->b_band == 0) {
		mp->b_next = mp->b_prev = nil(MBLKP);
		/* Normal messages (including high pri ones) */
		do {
			q->q_count -= mp->b_wptr - mp->b_rptr;
		} while (mp = mp->b_cont);
		if ((q->q_flag & QFULL)  &&  ((q->q_int_count +
					q->q_count) <= q->q_lowat)) {
			q->q_flag &= ~QFULL;
			if (q->q_flag & QWANTW) {
				q->q_flag &= ~QWANTW;
				enable_q = q->q_bfcp;
			}
		}
	} else if (mp->b_band <= q->q_nband) {
		/* Above can only fail if a module changes b_band
		 * while the message is in the queue (a BIG no-no). */
		qb = &q->q_bandp[mp->b_band-1];
		if (qb->qb_first == mp) {
			if (qb->qb_last == mp) {
				qb->qb_first = nil(MBLKP);
				qb->qb_last = nil(MBLKP);
			} else
				qb->qb_first = mp->b_next;
		} else if (qb->qb_last == mp)
			qb->qb_last = mp->b_prev;
		mp->b_next = mp->b_prev = nil(MBLKP);
		do {
			qb->qb_count -= mp->b_wptr - mp->b_rptr;
		} while (mp = mp->b_cont);
		if ((qb->qb_flag & QB_FULL)  &&  qb->qb_count <= qb->qb_lowat) {
			qb->qb_flag &= ~QB_FULL;
			if (qb->qb_flag & QB_WANTW) {
				qb->qb_flag &= ~QB_WANTW;
				enable_q = q->q_bfcp;
			}
		}
	}
	DISABLE_UNLOCK(&q->q_qlock);
	if (enable_q && enable_q->q_qinfo->qi_srvp)
		qenable(enable_q);
}

int
strqget (
reg	queue_t		* q,
	qfields_t	what,
	unsigned char	pri,
	long		* valp)
{
	MBLKP		mp;
reg	qband_t		* qb;
	int		error = 0;
	DISABLE_LOCK_DECL

	if (pri != 0) {
		DISABLE_LOCK(&q->q_qlock);
		if (pri > q->q_nband) {
			DISABLE_UNLOCK(&q->q_qlock);
			return EINVAL;
		}
		qb = &q->q_bandp[pri-1];
	}
	switch (what) {
	case QHIWAT:
		*valp = (pri == 0) ? q->q_hiwat : qb->qb_hiwat;
		break;
	case QLOWAT:
		*valp = (pri == 0) ? q->q_lowat : qb->qb_lowat;
		break;
	case QMAXPSZ:
		*valp = q->q_maxpsz;
		break;
	case QMINPSZ:
		*valp = q->q_minpsz;
		break;
	case QCOUNT:
		*valp = (pri == 0) ? q->q_count : qb->qb_count;
		break;
	case QFIRST:
		mp = (pri == 0) ? q->q_first : qb->qb_first;
		*valp = (long)mp;
		break;
	case QLAST:
		mp = (pri == 0) ? q->q_last : qb->qb_last;
		*valp = (long)mp;
		break;
	case QFLAG:
		*valp = (pri == 0) ? q->q_flag : qb->qb_flag;
		break;
	case QBAD:
	default:
		error = ENOENT;
		break;
	}
	if (pri != 0) {
		DISABLE_UNLOCK(&q->q_qlock);
	}
	return error;
}

int
strqset (
reg	queue_t		* q,
	qfields_t	what,
	unsigned char	pri,
	long		val)
{
reg	qband_t		* qb;
	int		error = 0;
	DISABLE_LOCK_DECL

	if (pri != 0) {
		DISABLE_LOCK(&q->q_qlock);
		if (pri > q->q_nband  &&  !alloc_qband(q, pri)) {
			DISABLE_UNLOCK(&q->q_qlock);
			return EINVAL;
		}
		qb = &q->q_bandp[pri-1];
	}
	switch (what) {
	case QHIWAT:
		if (pri == 0)
			q->q_hiwat = val;
		else
			qb->qb_hiwat = val;
		break;
	case QLOWAT:
		if (pri == 0)
			q->q_lowat = val;
		else
			qb->qb_lowat = val;
		break;
	case QMAXPSZ:
		q->q_maxpsz = val;
		break;
	case QMINPSZ:
		q->q_minpsz = val;
		break;
	case QCOUNT:
	case QFIRST:
	case QLAST:
	case QFLAG:
		error = EPERM;
		break;
	case QBAD:
	default:
		error = ENOENT;
		break;
	}
	if (pri != 0) {
		DISABLE_UNLOCK(&q->q_qlock);
	}
	return error;
}

/** Check for an available buffer */
int
testb (size, pri)
	int	size;
	uint	pri;
{
	MBLKP	 mp;

	if (mp = allocb(size, pri)) {
		freeb(mp);
		return 1;
	}
	return 0;
}

/** Remove a message block from the head of a message */
MBLKP
unlinkb (mp)
	MBLKP	 mp;
{
	MBLKP	 bp;

	if (bp = mp->b_cont)
		mp->b_cont = nil(MBLKP);
	return bp;
}

/*
 * DDI functions here.  We don't put these alphabetically so
 * we can use the macro versions internally.
 */
#undef canenable
int
canenable (q)
	queue_t	* q;
{
	int ret;
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&q->q_qlock);
	ret = ((q->q_flag & QNOENB) == 0);
	DISABLE_UNLOCK(&q->q_qlock);

	return ret;
}

#undef datamsg
int
datamsg (type)
	int	type;
{
	switch (type) {
	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:
	case M_DELAY:
		return 1;
	default:
		return 0;
	}
}

#undef enableok
void
enableok (q)
	queue_t	* q;
{
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&q->q_qlock);
	q->q_flag &= ~QNOENB;
	DISABLE_UNLOCK(&q->q_qlock);

}

#undef noenable
void
noenable (q)
	queue_t	* q;
{
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&q->q_qlock);
	q->q_flag |= QNOENB;
	DISABLE_UNLOCK(&q->q_qlock);

}

#undef OTHERQ
queue_t *
OTHERQ (q)
	queue_t	* q;
{
	return q->q_other;
}

#undef RD
queue_t	*
RD (q)
	queue_t	* q;
{
	return q->q_other;
}

#undef WR
queue_t	*
WR (q)
	queue_t	* q;
{
	return q->q_other;
}

int
wantmsg(q, f)
	queue_t * q;
	int (*f)();
{
	if (q->q_qinfo->qi_srvp)
		return 0;
	q->q_is_interesting = f;
	return 1;
}

int
wantio(queue_t *q, struct wantio *w)
{
/* put the pointer on both sides now, (and still somehow ...) */
        OTHERQ(q)->q_wantio = q->q_wantio = w;
        return 0;
}
