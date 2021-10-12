static char sccsid[] = "@(#)16  1.2  src/bos/kernext/dlpi/drd.c, sysxdlpi, bos41J, 9520A_a 5/16/95 16:20:11";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: DRDHASH
 *		DRD_LOCK
 *		DRD_UNLOCK
 *		EQ
 *		drd
 *		drd_bind
 *		drd_cancel
 *		drd_flush
 *		drd_init
 *		drd_intr
 *		drd_timeout
 *		drd_timer
 *		drd_unbind
 *		drd_update
 *		findroute
 *		prhex
 *		tx_drd
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * drd.c - Dynamic Route Discovery
 *
 * find the correct source route for this address
 * calls (*func)(dlb, m, daddr, segp, seglen) when address is complete
 *
 * public routines:
 *	drd - find a source route via the DRD
 *	drd_bind - register the DRD sap with the provider
 *	drd_unbind - deregister the DRD sap with the provider
 *	drd_update - update DRD routing table
 *	drd_cancel - cancel a DRD request
 *	drd_flush - flush DRD routing entries
 * private routines:
 *	drd_init - init the DRD subsystem
 *	prhex - make a hex string printable
 *	findroute - look for addr in DRD hash table
 *	drd_timer - DRD aging timer
 *	drd_timeout - timeout all pending requests
 *	drd_intr - DRD interrupt handler
 *	tx_drd - send DRD probe
 *
 * Design assumption:
 *	The same address cannot exist on more than one ndd (interface).
 *	Failing this, we must have one hash table for each interface.
 *
 * Source route table notes:
 *	- The source routes are stored in the table in a ready-to-send
 *	  format; that is, the direction bit is correct for transmission.
 *	- The RI_PRESENT bit is set by the CDLI if the seglen is positive.
 *	- Source routes maintained by this module are mostly opaque;
 *	  only the direction and broadcast bits are examined and modified.
 */

#include "include.h"
#include <netinet/if_802_5.h>

#define	DRD_HZ		2		/* DRD ticks/second */
#define	UNSET_AGE	-1		/* not valid */
#define	STATIC_AGE	0		/* forever */
#define	LOOKING_AGE	10*DRD_HZ	/* 10 seconds */
#define	SUPPLYING_AGE	5*60*DRD_HZ	/* 5 minutes */
#define	DRD_TICKS	HZ/DRD_HZ

typedef struct drdroute_s {
	struct drdroute_s *next;	/* list of addresses with same hash */
	uchar addr[MAXADDR_LEN];	/* this address */
	uchar seg[MAXROUTE_LEN];	/* route for this address */
	int seglen;			/* route len; if -1, none yet */
	int age;			/* age of this routing entry */
	drdreq_t *list;			/* list of pending requests */
} drdroute_t;

#define	HASHSZ	256
static struct {
	int	drd_tag;		/* hash table id */
	drdroute_t *drd_hash[HASHSZ];	/* DRD address hash table */
} drdinfo;
#define	drdhash		drdinfo.drd_hash
#define	DRDHASH(addr)	(addr[0] ^ addr[PHYSLEN-1] % HASHSZ)
#define	EQ(a1,a2)	!bcmp(a1,a2,PHYSLEN)

static uchar drdroute[] = { 0xc2, 0x40 };	/* discovery route */
static uchar bcroute[] =  { 0xe2, 0x40 };	/* broadcast route */

static int drd_count;		/* number of DRD ppa's bound */
static int drd_id;		/* DRD timeout id */

static struct ns_8022 drd_ns;	/* DRD bind info */
static struct ns_user drd_user;

static Simple_lock drd_lock;

#define	DRD_LOCK_DECL	int _snox
#define	DRD_LOCK()	_snox = disable_lock(PL_IMP, &drd_lock)
#define	DRD_UNLOCK()	unlock_enable(_snox, &drd_lock)

static char *prhex();
static drdroute_t *findroute();
static void drd_init();
static void drd_timer(), drd_timeout(), drd_intr();
static void tx_drd();

/*
 * drd - find a source route via the DRD
 */

void
drd(dlb, m, daddr, func)
	DLB *dlb;
	struct mbuf *m;
	uchar *daddr;
	pfv_t func;
{
	mblk_t *mp;
	drdroute_t *p;
	drdreq_t *rp, **rpp;
	struct mbuf *xidm;
	int dlen = PHYSLEN;
	DRD_LOCK_DECL;

	/* if timer not running, start it */
	if (!drd_id)
		drd_timer(1);

	/* if group address, use all-rings broadcast */
	if (daddr[0] & 0x80) {
		(*func)(dlb, m, daddr, bcroute, sizeof(bcroute));
		return;
	}

	/* look for address in table, install if not found */
	DRD_LOCK();
	if (!(p = findroute(daddr))) {
		if (!(p = getmem(drdroute_t, 1))) {
			DRD_UNLOCK();
			(*func)(dlb, m, daddr, 0, 0);
			return;
		}
		p->seglen = -1;
		p->age = UNSET_AGE;
		p->list = 0;
		bcopy(daddr, p->addr, dlen);
		p->next = drdhash[DRDHASH(daddr)];
		drdhash[DRDHASH(daddr)] = p;
	}

	/* if already have source route, continue immediately */
	if (p->seglen != -1) {
		DRD_UNLOCK();
		/* XXX timer race here; but not if reset age first */
		(*func)(dlb, m, daddr, p->seg, p->seglen);
		return;
	}

	/* create new request for this address */
	if (!(mp = allocb(sizeof(drdreq_t), BPRI_HI))) {
		DRD_UNLOCK();
		(*func)(dlb, m, daddr, 0, 0);
		return;
	}
	mp->b_datap->db_type = M_START;
	rp = (drdreq_t *)mp->b_rptr;
	mp->b_wptr += sizeof(drdreq_t);
	rp->tag = TAG_DRD;
	rp->mp = mp;
	rp->dlb = dlb;
	rp->func = func;
	rp->arg = m;
	rp->addr = p->addr;
	rp->seg = 0;
	rp->seglen = -1;

	/* add new request to end of list */
	for (rpp = &p->list; *rpp; rpp = &(*rpp)->next)
		;
	*rpp = rp;
	rp->next = 0;

	/* if no XID yet sent, do it now */
	if (p->age == UNSET_AGE) {
		p->age = LOOKING_AGE;
		DRD_UNLOCK();
		tx_drd(dlb, daddr, dlen);
	} else
		DRD_UNLOCK();
}

/*
 * drd_bind - register the DRD sap with the provider
 */

void
drd_bind(ndd, ppa)
	NDD *ndd;
	int ppa;
{
	int err;

	if (!drd_ns.dsap)
		drd_init();

	TRC(0, "drd_bind: ndd 0x%x ppa %d", ndd, ppa);
	if (err = ns_add_filter(ndd, &drd_ns, sizeof(drd_ns), &drd_user))
		ERR(0, "drd_bind: cannot bind; ppa %d, err %d", ppa, err);
	else { 
		++drd_count;
		drd_timer(1);
	}
}

/*
 * drd_unbind - deregister the DRD sap with the provider
 */

void
drd_unbind(ndd, ppa)
	NDD *ndd;
	int ppa;
{
	int err;

	TRC(0, "drd_unbind: ndd 0x%x ppa %d", ndd, ppa);

	if (drd_count == 0)
		return;

	if (err = ns_del_filter(ndd, &drd_ns, sizeof(drd_ns)))
		ERR(0, "drd_unbind: cannot unbind; ppa %d, err %d", ppa, err);
	else {
		if (--drd_count == 0 && drd_id)
			drd_timer(2);
	}
}

/*
 * drd_update - update DRD routing table
 */

void
drd_update(addr, addrlen, seg, seglen)
	uchar *addr;
	int addrlen;
	uchar *seg;
	int seglen;
{
	drdroute_t *p;
	drdreq_t *rp, *rpnext;
	ushort *rcf;
	DRD_LOCK_DECL;

	/* only if address has source routing present */
	if (!(addr[0] & RI_PRESENT))
		return;

	/*
	 * reset RII, complement the direction, and strip the broadcast bits.
	 */

	addr[0] &= ~RI_PRESENT;
	rcf = (ushort *)seg;
	*rcf ^= RCF_DIRECTION;
	*rcf &= ~(RCF_ALL_BROADCAST | RCF_LOCAL_BROADCAST);

	/* if address is on the local ring, discard nop route */
	if (seglen == 2)
		seglen = 0;

	/*
	 * update source route in hash table, create if new address
	 */

	TRC(0, "drd_update: address %s route %s",
		prhex(addr, addrlen), prhex(seg, seglen));

	DRD_LOCK();
	if (!(p = findroute(addr))) {
		/* if no route exists, create one */
		if (!(p = getmem(drdroute_t, 1))) {
			DRD_UNLOCK();
			return;
		}
		p->list = 0;
		bcopy(addr, p->addr, addrlen);
		p->age = SUPPLYING_AGE;
		p->seglen = seglen;
		bcopy(seg, p->seg, seglen);
		p->next = drdhash[DRDHASH(addr)];
		drdhash[DRDHASH(addr)] = p;
	} else {
		/* else update route info */
		if (p->age != STATIC_AGE) {
			/* can reset age since these routes MUST be okay */
			p->age = SUPPLYING_AGE;
			p->seglen = seglen;
			bcopy(seg, p->seg, seglen);
		}

		/* and complete any pending requests */
		for (rp = p->list; rp; rp = rpnext) {
			rpnext = rp->next;
			rp->seg = p->seg;
			rp->seglen = p->seglen;
			TRC(rp->dlb, "drd_update: using route %s",
				prhex(p->seg, p->seglen));
			putq(rp->dlb->dlb_wq, rp->mp);
		}
		p->list = 0;
	}
	DRD_UNLOCK();
}

/*
 * drd_cancel - cancel a DRD request
 */

void
drd_cancel(dlb)
	DLB *dlb;
{
	drdroute_t *p, **pp;
	drdreq_t *rp, **rpp;
	int i;
	DRD_LOCK_DECL;

	DRD_LOCK();
	for (i = 0; i < HASHSZ; ++i) {
		for (pp = &drdhash[i]; p = *pp; ) {
			/* cancel all requests from this stream */
			for (rpp = &p->list; *rpp; ) {
				if ((*rpp)->dlb == dlb) {
					rp = *rpp;
					*rpp = (*rpp)->next;
					freemsg(rp->mp);
				} else
					rpp = &(*rpp)->next;
			}
			/* if just cancelled the last request on this address */
			if (!p->list && p->seglen == -1) {
				*pp = p->next;
				putmem(p);
			} else
				pp = &p->next;
		}
	}
	DRD_UNLOCK();
}

/*
 * drd_flush - flush DRD routing entries
 */

void
drd_flush(dlb, addr)
	DLB *dlb;
	uchar *addr;
{
	drdroute_t *p, **pp;
	DRD_LOCK_DECL;

	DRD_LOCK();
	for (pp = &drdhash[DRDHASH(addr)]; p = *pp; ) {
		if (EQ(p->addr, addr)) {
			drd_timeout(p);	/* XXX toss any pending requests */
			*pp = p->next;
			putmem(p);
			break;
		} else
			pp = &p->next;
	}
	DRD_UNLOCK();
}

/*
 * drd_init - init the DRD subsystem
 */

static void
drd_init()
{
	drdinfo.drd_tag = TAG_DRD;
	drd_ns.dsap = SAP_DRD;
	drd_ns.filtertype = NS_8022_LLC_DSAP;
	drd_user.isr_data = (char *)TAG_DRD;
	drd_user.pkt_format = NS_INCLUDE_LLC | NS_HANDLE_HEADERS;
	drd_user.isr = drd_intr;
}

/*
 * prhex - make a hex string printable (not really DRD-specific)
 */

static char *
prhex(p, len)
	uchar *p;
	int len;
{
	static char buf1[60], buf2[60], *bp = buf2;
	static char tohex[] = "0123456789abcdef";
	char *cp;

	if (!len)
		return "<empty>";

	cp = bp = (bp == buf1) ? buf2 : buf1;
	while (len--) {
		*cp++ = tohex[*p >> 4];
		*cp++ = tohex[*p & 0x0f];
		*cp++ = ':';
		++p;
	}
	*--cp = 0;	/* nuke trailing colon */
	return bp;
}

/*
 * findroute - look for addr in DRD hash table
 */

static drdroute_t *
findroute(addr)
	uchar *addr;
{
	drdroute_t *p;

	for (p = drdhash[DRDHASH(addr)]; p; p = p->next) {
		if (EQ(addr, p->addr))
			break;
	}
	return p;
}

/*
 * drd_timer - DRD aging timer
 *
 * argument: 0 = timer fired, 1 = start timer, 2 = stop timer and cleanup
 */

static void
drd_timer(arg)
	int arg;
{
	drdroute_t *p, **pp;
	int i;
	DRD_LOCK_DECL;

	switch (arg) {
	case 0:	/* timer fired */
		break;
	case 1:	/* start timer if stopped */
		TRC(0, "drd_timer: started");
		if (!drd_id)
			drd_id = timeout(drd_timer, 0, DRD_TICKS);
		return;
	case 2:	/* stop timer, cleanup */
		TRC(0, "drd_timer: stopped");
		if (drd_id)
			untimeout(drd_id);
		drd_id = 0;
		return;
	default:	/* bug */
		TRC(0, "drd_timer: invalid argument!");
		DB(assert(0););
		return;
	}

	/* for each element in the hash table ... */
	DRD_LOCK();
	for (i = 0; i < HASHSZ; ++i) {
		for (pp = &drdhash[i]; p = *pp; ) {
			/* ... check to see what action is required */
			if (p->age != STATIC_AGE && --p->age == 0) {
				/* timed out */
				drd_timeout(p);
				*pp = p->next;
				putmem(p);
			} else {
				/* resend probe if still looking */
				if (p->seglen == -1) {
					DB(assert(p->list););
					tx_drd(p->list->dlb, p->addr, PHYSLEN);
				}
				pp = &p->next;
			}
		}
	}
	DRD_UNLOCK();

	drd_id = timeout(drd_timer, 0, DRD_TICKS);
}

/*
 * drd_timeout - timeout all pending requests
 */

static void
drd_timeout(p)
	drdroute_t *p;
{
	drdreq_t *rp, *rpnext;

	TRC(0, "drd_timeout: address %s", prhex(p->addr, PHYSLEN));

	for (rp = p->list; rp; rp = rpnext) {
		TRC(rp->dlb, "drd_timeout: no route");
		rpnext = rp->next;
		putq(rp->dlb->dlb_wq, rp->mp);
	}
	p->list = 0;
}

/*
 * drd_intr - DRD interrupt handler
 */

static void
drd_intr(ndd, m, llhdr, isr)
	NDD *ndd;
	struct mbuf *m;
	caddr_t llhdr;
	struct isr_data_ext *isr;
{
	llc_t *llc;
	DRD_LOCK_DECL;

	if (isr->isr_data != (char *)TAG_DRD) {
		TRC(0, "drd_intr: bogus interrupt");
discard:
		m_freem(m);
		return;
	}

	llc = mtod(m, llc_t *);
	TRC(0, LLCMSG(llc), "<-", CRPF(llc), GETNR(llc), GETNS(llc));

	if (PRIM(llc) != XID) {
		TRC(0, "drd_intr: unexpected LLC PDU 0x%x", PRIM(llc));
		goto discard;
	}
	if (ISCMD(llc)) {
		TRC(0, "drd_intr: unexpected XID CMD");
		goto discard;
	}

	drd_update(isr->srcp, isr->srclen, isr->segp, isr->seglen);

	m_freem(m);
}

/*
 * tx_drd - send DRD probe
 */

static void
tx_drd(dlb, daddr, dlen)
	DLB *dlb;
	uchar *daddr;
	int dlen;
{
	struct mbuf *m;
	llc_t *llc;

	if (!(m = mkframe(dlb, LLC_ULEN, 0))) {
		incstats(dlb, no_bufs);
		/* DRD timer will retry */
		return;
	}
	llc = mtod(m, llc_t *);
	llc->ssap = SAP_DRD;
	llc->dsap = SAP_NULL;
	llc->ctl1 = XID;
	SETPF1(llc);
	SETCMD(llc);
	(void)tx_frame2(dlb, m, daddr, drdroute, sizeof(drdroute));
}
