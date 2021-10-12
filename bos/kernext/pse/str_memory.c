static char sccsid[] = "@(#)23        1.18  src/bos/kernext/pse/str_memory.c, sysxpse, bos41J, 9516A_all 4/18/95 11:33:43";
/*
 * COMPONENT_NAME: SYXPSE - STREAMS framework
 * 
 * FUNCTIONS:      he_alloc 
 *                 he_free
 *                 he_realloc
 *                 bufcall_configure
 *                 bufcall_rsrv
 *                 bufcall
 *                 pse_bufcall
 *                 unbufcall
 *                 bufcall_init
 *                 bufcall_term
 *                 allocb
 *                 freeb
 *                 ext_freeb
 *                 bufcall_funnel_rsrv
 *                 bufcall_funnel_init
 *                 bufcall_funnel_term
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

#include <sys/errno.h>

#if SEC_BASE
#include <kern/assert.h>
#endif

#include <pse/str_stream.h>
#include <pse/str_config.h>
#include <pse/str_proto.h>
#include <sys/strstat.h>

#include <net/netisr.h>

/*
 * Storage allocation for everything but messages
 *
 * CONTEXT	These routines are called only in process context
 *		(data structures other than messages are allocated
 *		only by stream head code, and that is always forced
 *		into process context) - thus, we may use blocking
 *		memory allocation services.
 *
 * PRIORITES	are passed to us for historical reasons. This module
 *		used to allocate any dynamically needed memory.
 *
 * EXPORTED: 	he_alloc(size, pri)
 *		he_free(data)
 *		he_realloc(data, old_size, new_size);
 */

caddr_t
he_alloc (size, pri)
	long	size;
	int	pri;
{
	register caddr_t addr;

	ENTER_FUNC(he_alloc, size, pri, 0, 0, 0, 0);

	NET_MALLOC(addr, caddr_t, size, M_STREAMS, M_WAITOK);

	LEAVE_FUNC(he_alloc, addr);
	return addr;
}

void
he_free (addr)
	caddr_t	addr;
{
	ENTER_FUNC(he_free, addr, 0, 0, 0, 0, 0);

	NET_FREE(addr, M_STREAMS);

	LEAVE_FUNC(he_free, 0);
}

caddr_t
he_realloc (old_addr, old_size, new_size)
	caddr_t	old_addr;
	long	old_size;
	long	new_size;
{
	register caddr_t	new_addr;

	ENTER_FUNC(he_realloc, old_addr, old_size, new_size, 0, 0, 0);

	if (new_size <= old_size) {
		LEAVE_FUNC(he_realloc, old_addr);
		return(old_addr);
	}

	if ((new_addr = he_alloc(new_size, BPRI_LO)) != NULL) {
		bcopy(old_addr, new_addr, old_size);
		bzero(new_addr + old_size, new_size - old_size);
		he_free(old_addr);
	}
	LEAVE_FUNC(he_realloc, new_addr);
	return(new_addr);
}
 
/*
 *	Storage allocation for messages
 */

static simple_lock_data_t bufc_lock = {SIMPLE_LOCK_AVAIL};
simple_lock_data_t mblk_lock = {SIMPLE_LOCK_AVAIL};

int	NSTREVENT = 20;

typedef struct bufcall_s {
	struct bufcall_s	* bc_next;	/* Must be first */
	struct bufcall_s	* bc_prev;
	uint			bc_size;
	int			bc_pri;
	bufcall_fcn_t		bc_fcn;
	bufcall_arg_t		bc_arg;
	int			bc_id;
	queue_t *		bc_queue;
} BUFCALL, * BUFCALLP, ** BUFCALLPP;

static struct module_info bminfo =  {
	0, "bufcall", 0, INFPSZ, 2048, 128
};

static struct qinit brinit = {
	NULL, bufcall_rsrv, NULL, NULL, NULL, &bminfo
};

struct streamtab bufcallinfo = { &brinit, &brinit };


int
bufcall_configure (op, indata, indatalen, outdata, outdatalen)
        uint         op;
        str_config_t *  indata;
        size_t          indatalen;
        str_config_t *  outdata;
        size_t          outdatalen;
{
	struct streamadm	sa;
	dev_t			devno;

        int                     error;
	extern int bufcall_init();
	extern void bufcall_term();
	extern void ext_freeb();

	ENTER_FUNC(bufcall_configure, op, outdata, outdatalen, 0, 0, 0);
	error = 0;

	sa.sa_flags		= STR_IS_MODULE | STR_IS_MPSAFE | STR_NOTTOSPEC;
	sa.sa_ttys		= nil(caddr_t);
	sa.sa_sync_level	= SQLVL_MODULE;
	sa.sa_sync_info		= nil(caddr_t);
	strcpy(sa.sa_name, 	"bufcall");

	switch(op) {
	case CFG_INIT:
	    if ((devno = strmod_add(NODEV, &bufcallinfo, &sa)) == NODEV) {
		LEAVE_FUNC(bufcall_configure, ENODEV);
		return ENODEV;
	     }

	     (void)bufcall_init();
	     netisr_add(NETISR_MBLK, ext_freeb,
			(struct ifqueue *)0, (struct domain *)0);
            break;
	case CFG_TERM:
            error = strmod_del(devno, &bufcallinfo, &sa);
            if (error) {
                LEAVE_FUNC(bufcall_configure, error);
                return(error);
            }
	    bufcall_term ();
	    break;
	default:
            LEAVE_FUNC(bufcall_configure, EINVAL);
            return EINVAL;
	}

        if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
                outdata->sc_devnum = NODEV;
                outdata->sc_sa_flags = sa.sa_flags;
                strcpy(outdata->sc_sa_name, sa.sa_name);
        }

	LEAVE_FUNC(bufcall_configure, 0);
	return 0;
}

staticf	struct { BUFCALLP next, prev; } bc_q;
staticf	BUFCALLP	bc_ref, bc_free;
staticf	queue_t	*	bufcallq;
staticf	int		bufcall_needed  = 0;
staticf	int		bufcall_happened = 0;

int
bufcall_rsrv (q)
	queue_t	* q;
{
reg	BUFCALLP	bc, nextbc;
	int		threshold;
	DISABLE_LOCK_DECL

	ENTER_FUNC(bufcall_rsrv, q, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&bufc_lock);
again:
	bufcall_happened = 0;
	threshold = (STRMSGSZ > 0) ? STRMSGSZ+1 : INT_MAX;
	for ( nextbc = bc_q.next; nextbc != (BUFCALLP)&bc_q; ) {
		bc = nextbc;
		nextbc = nextbc->bc_next;
		if (bc->bc_size >= threshold)
			continue;
		if (testb(bc->bc_size, bc->bc_pri) == 0) {
			threshold = bc->bc_size;
			continue;
		}
		remque(bc);
		DISABLE_UNLOCK(&bufc_lock);

		(void) csq_protect(	bc->bc_queue,
					nil(queue_t *),
					(csq_protect_fcn_t)bc->bc_fcn,
					(csq_protect_arg_t)bc->bc_arg,
					q->q_runq_sq, TRUE);

		DISABLE_LOCK(&bufc_lock);
		bc->bc_next = bc_free;
		bc_free = bc;
		/*
		 * If bufcall() was called while we didn't
		 * hold the bufc_lock, we have to restart
		 * the loop.
		 */
		if ( bufcall_happened )
			goto again;
		nextbc = bc_q.next;
	}
	if (bc_q.next == (BUFCALLP)&bc_q)
		bufcall_needed = 0;

	DISABLE_UNLOCK(&bufc_lock);

	LEAVE_FUNC(bufcall_rsrv, 0);
	return 0;
}

int
(bufcall)(s, p, f, a)
	uint s; int p; int (*f)(); long a;
{

	ENTER_FUNC(bufcall, s, p, f, a, 0, 0);
	LEAVE_FUNC(bufcall, 0);

	return bufcall(s, p, (bufcall_fcn_t)f, (bufcall_arg_t)a);
}

/** Recover from failure of allocb */
int
pse_bufcall (size, pri, func, arg)
	uint	size;
	int	pri;
	bufcall_fcn_t	func;
	bufcall_arg_t	arg;
{
static	int		bufcall_id;
reg	BUFCALLP	bc;
        int             timeid = 0;
	DISABLE_LOCK_DECL

	ENTER_FUNC(bufcall, size, pri, func, arg, 0, 0);

	DISABLE_LOCK(&bufc_lock);
	
	if (!(bc = bc_free)) {
		DISABLE_UNLOCK(&bufc_lock);
		STR_DEBUG(printf("WARNING: bufcall: could not allocate stream event\n"));
		LEAVE_FUNC(bufcall, 0);
		return 0;
	}
	bc_free = bc->bc_next;
	bc->bc_size = size;
	bc->bc_pri = pri;
	bc->bc_fcn = func;
	bc->bc_arg = arg;
	if ((bc->bc_id = ++bufcall_id) == 0)
		bc->bc_id = ++bufcall_id;
	if (func != (bufcall_fcn_t)qenable &&
	    func != (bufcall_fcn_t)osr_bufcall_wakeup)
		bc->bc_queue = csq_which_q();
	else
		bc->bc_queue = nil(queue_t *);

	insque(bc, bc_q.prev);
        if (bufcall_needed++ == 0)
		timeid = str_timeout_trb(qenable, bufcallq, hz);
       
        bufcall_happened = 1;
	DISABLE_UNLOCK(&bufc_lock);

	LEAVE_FUNC(bufcall, 1);
	return bc->bc_id;
}

void
unbufcall (id)
	int	id;
{
reg	BUFCALLP	bc;
	DISABLE_LOCK_DECL

	ENTER_FUNC(unbufcall, id, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&bufc_lock);
	for (bc = bc_q.next; bc != (BUFCALLP)&bc_q; bc = bc->bc_next) {
		if (bc->bc_id == id) {
			remque(bc);
			bc->bc_next = bc_free;
			bc_free = bc;
			break;
		}
	}
	DISABLE_UNLOCK(&bufc_lock);

        LEAVE_FUNC(unbufcall, 0);
}

/* Used to initialize mblks in allocb. */
static	MH	mh_template;
/* "Attached" mblks held here for safe freeing */
MHP	mh_freelater;

MHP	mh_cache_128 = NULL;
int	mh_cache_num_128 = 0;
MHP	mh_cache_256 = NULL;
int	mh_cache_num_256 = 0;
#define	MAX_CACHE_SIZE	128

int
bufcall_init ()
{
reg	BUFCALL	* bc;

        ENTER_FUNC(bufcall_init, 0, 0, 0, 0, 0, 0);
	bc_q.next = bc_q.prev = (BUFCALLP)&bc_q;
        lock_alloc((&bufc_lock), LOCK_ALLOC_PIN, PSE_BUFC_LOCK, -1);
	simple_lock_init(&bufc_lock);
        lock_alloc((&mblk_lock), LOCK_ALLOC_PIN, PSE_MBLK_LOCK, -1);
	simple_lock_init(&mblk_lock);
	mh_template.mh_dblk.db_ref = 1;
	mh_template.mh_dblk.db_type = M_DATA;
	/* Others are 0 or initialized on demand */

	NET_MALLOC(bc, BUFCALL *, NSTREVENT * sizeof *bc, M_STREAMS, M_WAITOK);
	if (bufcallq = q_alloc()) {
		sth_set_queue(	bufcallq,
				bufcallinfo.st_rdinit,
				bufcallinfo.st_wrinit);
		bc_ref = bc;
		for (bc_free = bc; bc < &bc_free[NSTREVENT-1]; bc++)
			bc->bc_next = &bc[1];
		bc->bc_next = nilp(BUFCALL);
		LEAVE_FUNC(bufcall_init, 0);
		return ;
	}
	NET_FREE(bc, M_STREAMS);
	panic("bufcall_init");
        LEAVE_FUNC(bufcall_init, 0);
}

void
bufcall_term ()
{
	lock_free(&bufc_lock);
	lock_free(&mblk_lock);
	
	NET_FREE(bc_ref, M_STREAMS);
}

/*
 * allocb - allocate a message block.
 */

MBLKP
allocb (size, pri)
reg	int	size;
	uint	pri;
{
reg	MHP	mh;
	uchar *	mem;
	extern int thewall, allocated, throttle;
	DISABLE_LOCK_DECL

	ENTER_FUNC(allocb, size, pri, 0, 0, 0, 0);

	if (size < 0 || (STRMSGSZ > 0 && size > STRMSGSZ))
		goto bad;
	pri = (pri == BPRI_WAITOK) ? M_WAITOK : M_NOWAIT;
	if (size <= 128 - sizeof (MH)) {

		DISABLE_LOCK(&mblk_lock);
		if (mh_cache_128) {
			mh = mh_cache_128;
			mh_cache_128 = (MHP)mh_cache_128->mh_mblk.b_next;
			mh_cache_num_128--;
			DISABLE_UNLOCK(&mblk_lock);
		} else {
			DISABLE_UNLOCK(&mblk_lock);
			if (allocated > throttle) goto bad;
			NET_MALLOC(mh, MHP, 128, M_MBLK, pri);
		}
		mem = (uchar *)(mh + 1);
		if (size) 
			size = 128 - sizeof (MH);

	} else if (size <= 256 - sizeof (MH)) {

		DISABLE_LOCK(&mblk_lock);
		if (mh_cache_256) {
			mh = mh_cache_256;
			mh_cache_256 = (MHP)mh_cache_256->mh_mblk.b_next;
			mh_cache_num_256--;
			DISABLE_UNLOCK(&mblk_lock);
		} else {
			DISABLE_UNLOCK(&mblk_lock);
			if (allocated > throttle) goto bad;
			NET_MALLOC(mh, MHP, 256, M_MBLK, pri);
		}
		mem = (uchar *)(mh + 1);
		if (size) 
			size = 256 - sizeof (MH);
	} else {
		if (allocated > throttle) goto bad;
		NET_MALLOC(mh, MHP, sizeof (MH), M_MBLK, pri);
		if (mh) {

			/*
			 * Round up size to net_malloc()'s quantum.
			 * XXX we know too much.
			 */
			if (size > MAXALLOCSAVE)
				size = round_page(size);
			else if (size & (size - 1))
				size = (1 << BUCKETINDX(size));
			NET_MALLOC(mem, uchar *, size, M_MBDATA, pri);
		}
	}
	if (mh == 0 || mem == 0) {
		if (mh) NET_FREE(mh, M_MBLK);
bad:
		throttle = 922 * thewall; /* Update throttle in case user has changed thewall. */
		LEAVE_FUNC(allocb, 0);
		return nil(MBLKP);
	}
	*mh = mh_template;
	sq_init(&mh->mh_mblk.b_sq);
	mh->mh_mblk.b_datap = &mh->mh_dblk;
	mh->mh_mblk.b_rptr = mh->mh_mblk.b_wptr = mh->mh_dblk.db_base = mem;
	mh->mh_dblk.db_lim = &mem[size];
	mh->mh_dblk.db_size = size;
	mh->mh_frtn.free_func = 0;
	LEAVE_FUNC(allocb, &mh->mh_mblk);
	return &mh->mh_mblk;
}

/*
 * freeb - free a message block
 */
void
freeb (mp)
	MBLKP	 mp;
{
reg	MHP		mh;
	dblk_t *	db;
	uchar		*md1, *md2;
	MH		*mh1, *mh2;
	DISABLE_LOCK_DECL

	if (mp->b_datap == 0 || mp->b_datap->db_ref == 0)
		panic("freeing free mblk");
	ENTER_FUNC(freeb, mp, 0, 0, 0, 0, 0);
#if SEC_BASE
	/*
	 * If there are security attributes associated with this buffer,
	 * release them first.
	 */
	if (mp->b_attr) {
		ASSERT(	!mp->b_attr->b_attr && \
			!mp->b_attr->b_cont && \
			!mp->b_attr->b_next);
		freeb(mp->b_attr);
		mp->b_attr = nil(MBLKP);
	}
#endif
	md1 = md2 = NULL;
	mh1 = mh2 = NULL;
	DISABLE_LOCK(&mblk_lock);
	db = mp->b_datap;
	mp->b_datap = nilp(dblk_t);
	db->db_ref--;
	mh = (MHP)(&((MBLKP)db)[-1]);
	/*
	 * Following loop executed at most twice - once for the
	 * mp->b_datap and its associated mblk, and a second
	 * time if that mblk isn't mp. Set mh* and md* as we go
	 * in order to take mblk_lock and splstr only once.
	 */
	for (;;) {
		if ( db->db_ref == 0 ) {
			uchar *p = db->db_base;
			db->db_base = NULL;
			/*
			 * If free_func, defer free to memory isr thread when
			 * _both_ the mblk and dblk are unreferenced. (We need
			 * the mblk and the mh_frtn to queue it)
			 */
			if ( mh->mh_frtn.free_func ) {
				assert(mh->mh_mblk.b_flag & MSGEXT);
				if ( mh->mh_mblk.b_datap == NULL
				&&   mh->mh_dblk.db_base == NULL ) {
					mh->mh_mblk.b_prev =(MBLKP)p;
					mh->mh_mblk.b_next =(MBLKP)mh_freelater;
					mh_freelater = mh;
				} else	/* later... */
					db->db_base = p;
			/*
			 * Else free any mbdata and unused mblk.
			 */
			} else {
				if ( p != (uchar *)(mh + 1) )
					if (md1) md2 = p; else md1 = p;
				if ( mh->mh_mblk.b_datap == NULL
				&&   mh->mh_dblk.db_base == NULL ) {
				    if (mh_cache_num_128 < MAX_CACHE_SIZE
					&& mh == mp
					&& (mh->mh_dblk.db_size <=
						128 - sizeof (MH))) {
					mh->mh_mblk.b_next =(MBLKP)mh_cache_128;
					mh_cache_128 = mh;
					mh_cache_num_128++;
				    } else if (mh_cache_num_256 < MAX_CACHE_SIZE
						&& mh == mp
						&& ((mh->mh_dblk.db_size >
							128 - sizeof (MH)) && 
						(mh->mh_dblk.db_size <= 
							256 - sizeof (MH)))) {
					mh->mh_mblk.b_next =(MBLKP)mh_cache_256;
					mh_cache_256 = mh;
					mh_cache_num_256++;
				    } else {
					if (mh1) mh2 = mh; else mh1 = mh;
				    }
				} /* if */
			}
		}
		if ( mh == (MHP)mp )
			break;
		mh = (MHP)mp;
		db = &mh->mh_dblk;
	}
	mh = mh_freelater;
	DISABLE_UNLOCK(&mblk_lock);
	if (mh1) NET_FREE(mh1, M_MBLK);
	if (mh2) NET_FREE(mh2, M_MBLK);
	if (md1) NET_FREE(md1, M_MBDATA);
	if (md2) NET_FREE(md2, M_MBDATA);
	if (mh)  schednetisr(NETISR_MBLK);
	if (bufcall_needed)
		qenable(bufcallq);

	LEAVE_FUNC(freeb, 0);
}

/*
 * Called from NETISR_MBLK for interrupt-safe freeing.
 */
void
ext_freeb()
{
reg	MHP	mh, mh_next;
	char *	p;
	DISABLE_LOCK_DECL

	ENTER_FUNC(ext_freeb, 0, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&mblk_lock);

	mh = mh_freelater;
	mh_freelater = 0;

	DISABLE_UNLOCK(&mblk_lock);

	while (mh) {
		if (p = (char *)mh->mh_mblk.b_prev)
			(*mh->mh_frtn.free_func)(mh->mh_frtn.free_arg, p);
		mh->mh_mblk.b_flag &= ~MSGEXT;
		mh_next = (MHP)mh->mh_mblk.b_next;
		NET_FREE(mh, M_MBLK);
		mh = mh_next;
	}

        LEAVE_FUNC(ext_freeb, 0);
}

#include <pse/str_funnel.h>

void
bufcall_funnel_init()
{
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&bufcallq->q_qlock);
	bufcallq->q_runq_sq->sq_entry = funnel_sq_wrapper;
	DISABLE_UNLOCK(&bufcallq->q_qlock);
	
	DISABLE_LOCK(&WR(bufcallq)->q_qlock);
	WR(bufcallq)->q_runq_sq->sq_entry = funnel_sq_wrapper;
	DISABLE_UNLOCK(&WR(bufcallq)->q_qlock);
}

void
bufcall_funnel_term()
{
	extern void sq_wrapper();
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&bufcallq->q_qlock);
	bufcallq->q_runq_sq->sq_entry = sq_wrapper;
	DISABLE_UNLOCK(&bufcallq->q_qlock);

	DISABLE_LOCK(&WR(bufcallq)->q_qlock);
	WR(bufcallq)->q_runq_sq->sq_entry = sq_wrapper;
	DISABLE_UNLOCK(&WR(bufcallq)->q_qlock);
}
