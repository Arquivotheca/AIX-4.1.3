static char sccsid[] = "@(#)10  1.6  src/bos/kernext/dlpi/abind.c, sysxdlpi, bos41J, 9523A_all 6/5/95 14:53:53";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: GETDSAP
 *		GETSSAP
 *		GETTYPE
 *		chk_8022
 *		chk_ether
 *		chk_sap
 *		detach
 *		dl_add_filter
 *		dl_attach
 *		dl_bind
 *		dl_bind_sap
 *		dl_del_filter
 *		dl_detach
 *		dl_subs_bind
 *		dl_subs_unbind
 *		dl_unbind
 *		dl_unbind_sap
 *		isatalk
 *		isnetware
 *		mknddname
 *		ndd2dlpi
 *		reserved
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
 * abind.c - attach and bind requests
 *
 * major bind routines:
 *	dl_bind - handles DLPI front end for DL_BIND_REQ
 *	dl_bind_sap - validates sap from DL_BIND_REQ
 *	dl_subs_bind - handles DLPI front end for DL_SUBS_BIND_REQ
 *	dl_subs_unbind - handles DLPI front end for DL_SUBS_UNBIND_REQ
 *	dl_unbind_sap - counterpart of dl_bind_sap
 *	dl_unbind - handles DLPI front end for DL_BIND_REQ
 *
 *	chk_ether - validate ethernet types
 *	chk_8022 - validate 802.2 saps
 *	dl_add_filter - registers type with system (if needed)
 *	dl_del_filter - counterpart of dl_add_filter
 *
 * locking notes:
 *	dl_add_filter() and dl_del_filter() must be called while
 *	protected by INTR_LOCK, because they modify dlb_ppa.saps[].
 */

#include "include.h"

/* public interfaces */
void dl_attach(), dl_detach();
void detach();
void dl_bind(), dl_unbind();
int dl_unbind_sap();
void dl_subs_bind(), dl_subs_unbind();

/* private interfaces */
static char *mknddname();
static int ndd2dlpi();
static int dl_bind_sap();
static int chk_ether(), chk_8022(), chk_sap();
static int dl_add_filter(), dl_del_filter();

/* checks for reserved saps */
#define	reserved(t)	((t) == SAP_NULL || (t) == SAP_DRD)

/*
 * hacks to support backwards compatibility:
 *	isatalk supports legacy appletalk snaps
 *	isnetware supports auto-detect netware traffic for padding
 *		(see convert.c)
 *	isnetware2 detects other netware binds to disable the DRD
 *	GET* support legacy saps
 */

#define	SAP_NETWARE	0xff	/* 802.3 only */
#define	TYPE_NETWARE	0x8137
#define	TYPE_APPLETALK	0x809b
#define	isatalk(t)	((t) == TYPE_APPLETALK)

#define isnetware(mac, t) (((mac) == DL_ETHER || (mac) == DL_CSMACD) && \
			   ((t) == SAP_NETWARE || (t) == TYPE_NETWARE))

#define	isnetware2(mac, ns) (((ns).ethertype == TYPE_NETWARE) || \
			     (((mac) == DL_TPR || (mac) == DL_FDDI) && \
			      ((ns).dsap == 0xe0)))

/* legacy sap extraction - obsolete, do not use in new code */
#define	GETTYPE(x)	(((union sapu *)&x)->su_dst.type)
#define	GETDSAP(x)	(((union sapu *)&x)->su_dst.dsap)
#define	GETSSAP(x)	(((union sapu *)&x)->su_dst.ssap)

/*
 **************************************************
 * DL_ATTACH_REQ, DL_DETACH_REQ
 **************************************************
 */

/*
 * mknddname - return a pointer to an ndd name suitable for ns_alloc()
 *
 * ppa is 0 <= ppa < 99
 * base is assumed to be short, such that strlen(base) < sizeof buf
 *
 * NB: must be called with NONI_LOCK to protect static buf.
 */

static char *
mknddname(base, ppa)
	char *base;
	int ppa;
{
	static char buf[100];
	char *cp = buf;

	while (*cp++ = *base++)
		;
	--cp;
	if (ppa >= 10)
		*cp++ = (ppa / 10) + '0';
	*cp++ = (ppa % 10) + '0';
	*cp = 0;
	return buf;
}

/*
 * ndd2dlpi - convert NDD type to DLPI type
 */

static int
ndd2dlpi(dlb, nt)
	DLB *dlb;
	int nt;
{
	if (dlb->dlb_isether)		/* unfortunate */
		return DL_ETHER;

	switch (nt) {
	case NDD_ETHER:		return DL_ETHER;
	case NDD_ISO88023:	return DL_CSMACD;
	case NDD_ISO88024:	return DL_TPB;
	case NDD_ISO88025:	return DL_TPR;
	case NDD_ISO88026:	return DL_METRO;
	case NDD_FDDI:		return DL_FDDI;
	default:		return DL_OTHER;
	}
}

/*
 * dl_attach - attach ppa to this stream
 */

void
dl_attach(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	ppa_t *p;
	NDD *ndd;
	char *name;
	int ppa;
	int err = 0;

	if (dlb->dlb_state != DL_UNATTACHED)
		err = DL_OUTSTATE;
	else {
		ppa = (int)((dl_attach_req_t *)(mp->b_rptr))->dl_ppa;
		if (ppa < 0 || ppa > 99)
			err = DL_BADPPA;
	}
	if (err) {
error:
		putq(dlb->dlb_rq, errack(mp, DL_ATTACH_REQ, err));
		return;
	}

	NONI_LOCK();
	/* try to find an already initialized interface (ndd) */
	for (p = dlb->dlb_prov->p_ppa; p; p = p->next) {
		if (p->ppa == ppa)
			break;
	}

	/* if not found, then create new ppa */
	if (!p) {
		if (!(p = getmem(ppa_t, 1))) {
			NONI_UNLOCK();
			err = -ENOSR;
			goto error;
		}

		name = mknddname(dlb->dlb_prov->p_nddname, ppa);
		if (ns_alloc(name, &ndd)) {
			ERR(dlb, "cannot ns_alloc %s", name);
			NONI_UNLOCK();
			putmem(p);
			err = DL_BADPPA;
			goto error;
		}

		if (ndd->ndd_addrlen > sizeof(dlb->dlb_addr)) {
			ERR(dlb, "ndd_addr too long (%d)", ndd->ndd_addrlen);
error2:
			ns_free(ndd);
			NONI_UNLOCK();
			putmem(p);
			err = DL_INITFAILED;
			goto error;
		}

		/* must have an address resolution routine */
		if (!ndd->ndd_demuxer->nd_address_resolve) {
			ERR(dlb, "no address resolution routine");
			goto error2;
		}

		p->tag = TAG_PPA;
		p->ndd = ndd;
		p->ppa = ppa;

		/* add new ppa to provider's ppa list */
		p->next = dlb->dlb_prov->p_ppa;
		dlb->dlb_prov->p_ppa = p;

		/* bind DRD if needed */
		if (dlb->dlb_prov->p_drd)
			drd_bind(ndd, ppa);
	}

	/* gain a reference to the interface */
	++p->ref;
	ndd = p->ndd;
	NONI_UNLOCK();

	dlb->dlb_drd = dlb->dlb_prov->p_drd;
	dlb->dlb_ppa = p;
	dlb->dlb_addrlen = ndd->ndd_addrlen;
	bcopy(ndd->ndd_physaddr, dlb->dlb_addr, dlb->dlb_addrlen);
	dlb->dlb_mactype = ndd2dlpi(dlb, ndd->ndd_type);

	/*
	 * set defaults, if user hasn't already set
	 */

	if (!dlb->dlb_output)
		dlb->dlb_output = ndd->ndd_demuxer->nd_address_resolve;
	if (!dlb->dlb_input)
		dlb->dlb_input = ndd->ndd_demuxer->nd_address_input;
	if (dlb->dlb_n1 > ndd->ndd_mtu)
		dlb->dlb_n1 = ndd->ndd_mtu;

	dlb->dlb_state = DL_UNBOUND;
	putq(dlb->dlb_rq, okack(mp, DL_ATTACH_REQ));
}

/*
 * dl_detach - remove reference to the ndd
 */

void
dl_detach(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	if (dlb->dlb_state != DL_UNBOUND) {
		putq(dlb->dlb_rq, errack(mp, DL_DETACH_REQ, DL_OUTSTATE));
		return;
	}
	detach(dlb);
	putq(dlb->dlb_rq, okack(mp, DL_DETACH_REQ));
}

/*
 * detach - common routine to detach a ppa
 */

void
detach(dlb)
	DLB *dlb;
{
	/*
	 * Lose the reference to the ppa, but do not de-init the NDD,
	 * in the hopes that it will be reused in a later attach.
	 * The term config entry point will properly release things
	 * if the ppa is not in use.
	 */

	NONI_LOCK();
	--dlb->dlb_ppa->ref;
	NONI_UNLOCK();

	dlb->dlb_ppa = 0;
	dlb->dlb_state = DL_UNATTACHED;
}

/*
 **************************************************
 * DL_BIND_REQ, DL_UNBIND_REQ,
 * DL_SUBS_BIND_REQ, DL_SUBS_UNBIND_REQ
 **************************************************
 */

/*
 * dl_bind - register sap with ndd
 */

void
dl_bind(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_bind_req_t	*req;
	dl_bind_ack_t	*ack;
	ulong		sap;
	int		len, err;
	mblk_t		*mp1;
	int		mode, xidtest, conind;

	req = (dl_bind_req_t *)mp->b_rptr;

	sap = req->dl_sap;
	mode = req->dl_service_mode;
	xidtest = req->dl_xidtest_flg;
	conind = (mode & DL_CODLS) ? min(req->dl_max_conind, MAXCONIND) : 0;

	err = 0;
	if (dlb->dlb_state != DL_UNBOUND)
		err = DL_OUTSTATE;
	else if (req->dl_conn_mgmt)
		err = DL_UNSUPPORTED;
	else if (mode & ~(DL_CLDLS | DL_CODLS))
		err = DL_UNSUPPORTED;
	else if (xidtest & ~(DL_AUTO_XID | DL_AUTO_TEST))
		err = -EINVAL;

	if (err) {
		putq(dlb->dlb_rq, errack(mp, DL_BIND_REQ, err));
		return;
	}

	TRC(dlb, "dl_bind: sap 0x%x mode 0x%x conind %d", sap, mode, conind);

	/* obtain an mblk big enough for a response */
	len = sizeof(dl_bind_ack_t) + MAXADDR_LEN;
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < len) {
		if (!(mp1 = allocb(len, BPRI_HI))) {
			incstats(dlb, no_bufs);
			putq(dlb->dlb_rq, errack(mp, DL_BIND_REQ, -ENOSR));
			return;
		}
		freemsg(mp);
		mp = mp1;
	} else
		mp->b_rptr = mp->b_datap->db_base;

	/* save info needed for binding */
	dlb->dlb_mode = mode;
	dlb->dlb_conind = conind;
	dlb->dlb_xidtest = xidtest;

	/* register type with system */
	if (err = dl_bind_sap(dlb, sap)) {
		putq(dlb->dlb_rq, errack(mp, DL_BIND_REQ, err));
		return;
	}

	/* sap accepted, record request */
	dlb->dlb_sap = sap; 

	/* format response */
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + len;
	ack = (dl_bind_ack_t *)mp->b_rptr;
	ack->dl_primitive = DL_BIND_ACK;
	ack->dl_sap = dlb->dlb_sap;
	ack->dl_addr_length = dlb->dlb_addrlen;
	ack->dl_addr_offset = sizeof(dl_bind_ack_t);
	ack->dl_max_conind = conind;
	ack->dl_xidtest_flg = xidtest;
	bcopy(dlb->dlb_addr, (char *)&ack[1], dlb->dlb_addrlen);

	dlb->dlb_state = DL_IDLE;

	putq(dlb->dlb_rq, mp);
}

/*
 * dl_bind_sap - bind sap to interface
 *
 * returns 0 on success, < 0 system error, > 0 dlpi error
 */

static int
dl_bind_sap(dlb, sap)
	DLB *dlb;
	ulong sap;
{
	struct	ns_8022	ns;
	uchar	sapbuf[6];
	int	saplen;
	int	err;
	int	dobind = 1;
	int	netware = 0;
	INTR_LOCK_DECL;

	/*
	 * parse and validate saps according to interface
	 */

	if (dlb->dlb_isether) {
		if (dlb->dlb_mode != DL_CLDLS)
			return DL_UNSUPPORTED;
		if (err = chk_ether(&ns, &sap, sizeof(sap)))
			return err;
		saplen = sizeof(ushort);
		*(ushort *)sapbuf = ns.ethertype;
		netware = isnetware(dlb->dlb_mactype, ns.ethertype);
	} else {
		if (err = chk_8022(&ns, &sap, sizeof(sap)))
			return err;
		if (ns.filtertype == NS_8022_LLC_DSAP) {
			saplen = sizeof(uchar);
			sapbuf[0] = ns.dsap;
			if (ns.dsap == SAP_SNAP)
				dobind = 0;
			/* only allow sap 0xff on 802.3 connection-less */
			if (ns.dsap == SAP_NETWARE) {
				if (dlb->dlb_mactype != DL_CSMACD)
					return DL_BADADDR;
				if (dlb->dlb_mode != DL_CLDLS)
					return DL_BADADDR;
			}
			netware = isnetware(dlb->dlb_mactype, ns.dsap);
		} else {
			sapbuf[0] = ns.dsap;
			saplen = SNAP_LEN + 1;
			BCOPY(ns.orgcode, sapbuf + 1, SNAP_LEN);
			netware = isnetware(dlb->dlb_mactype, ns.ethertype);
		}

		/* optimization: llc for default unitdata case */
		dlb->dlb_llc.ssap = ns.dsap;
		dlb->dlb_llc.dsap = ns.dsap;
		dlb->dlb_llc.ctl1 = UI;
		BCOPY(ns.orgcode, dlb->dlb_llc.org_id, SNAP_LEN);
	}

	/* bind only if have complete information */
	if (dobind) {
		INTR_LOCK();
		err = dl_add_filter(dlb, &ns, 0);
		INTR_UNLOCK();
		if (err)
			return err;
		dlb->dlb_netware = netware;
	}

	/* disable DRD for netware */
	if (dlb->dlb_drd &&
	    (dlb->dlb_netware || isnetware2(dlb->dlb_mactype, ns)))
		dlb->dlb_drd = 0;

	/* update reported address */
	bcopy(sapbuf, dlb->dlb_addr + dlb->dlb_addrlen, saplen);
	dlb->dlb_addrlen += saplen;
	dlb->dlb_saplen = saplen;
	dlb->dlb_sap = sap;

	return 0;
}

/*
 * dl_unbind - unbind a SAP
 *
 * must unbind ALL bound information; snaps included (DLPI 4.1.8)
 */

void
dl_unbind(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	int err = 0;

	if (dlb->dlb_state != DL_IDLE)
		err = DL_OUTSTATE;
	else if (dlb->dlb_promisc || dlb->dlb_multi)
		err = -EBUSY;
	else
		err = dl_unbind_sap(dlb);

	if (err) mp = errack(mp, DL_UNBIND_REQ, err);
	else	 mp = okack(mp, DL_UNBIND_REQ);

	putq(dlb->dlb_rq, mp);
}

/*
 * dl_unbind_sap - unbind all SAPs from interface
 */

int
dl_unbind_sap(dlb)
	DLB *dlb;
{
	bind_t *bind;
	int err;
	INTR_LOCK_DECL;

	while (bind = dlb->dlb_bound) {
		INTR_LOCK();
		err = dl_del_filter(dlb, &bind->ns, bind->class);
		INTR_UNLOCK();
		if (err) {
			ERR(dlb, "dl_unbind: dl_del_filter err %d", err);
			return err;
		}
	}

	/* return things to the DL_UNBOUND state */
	dlb->dlb_addrlen = dlb->dlb_physlen;
	dlb->dlb_saplen = 0;
	dlb->dlb_sap = 0;

	putctl1(dlb->dlb_rq->q_next, M_FLUSH, FLUSHRW);	/* DLPI apdx B */
	dlb->dlb_state = DL_UNBOUND;

	/* decstats(dlb, binds);	* XXX really? */
	return 0;
}

/*
 * dl_subs_bind - hierarchical and peer binds
 */

void
dl_subs_bind(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	struct ns_8022 ns;
	dl_subs_bind_req_t *dlsbr = (dl_subs_bind_req_t *)mp->b_rptr;
	uchar *sap = mp->b_rptr + dlsbr->dl_subs_sap_offset;
	int len = dlsbr->dl_subs_sap_length;
	int class = dlsbr->dl_subs_bind_class;
	bind_t *wasbound;
	int err = 0;
	INTR_LOCK_DECL;

	wasbound = dlb->dlb_bound;

	TRC(dlb, "dl_subs_bind: class %x", class);

	if (!BOUND(dlb))
		err = DL_OUTSTATE;
	else if (class != DL_HIERARCHICAL_BIND && class != DL_PEER_BIND)
		err = DL_UNSUPPORTED;
	else if (sap + len > mp->b_wptr)
		err = DL_BADADDR;
	else if (dlb->dlb_isether) {
		if (class != DL_PEER_BIND)
			err = DL_UNSUPPORTED;
		else
			err = chk_ether(&ns, sap, len);
	} else {
		if (dlb->dlb_mode & DL_CODLS) {
			if (dlb->dlb_conind == 0 ||
			    class != DL_PEER_BIND || len != sizeof(uchar))
				err = DL_UNSUPPORTED;
		}
		if (!err) {
			/* check sap; do not allow bind to just SNAP SAP */
			/* also disallow subs bind to netware SAP */
			err = chk_8022(&ns, sap, len);
			if (!err &&
			    ns.filtertype == NS_8022_LLC_DSAP &&
			    (ns.dsap == SAP_NETWARE || ns.dsap == SAP_SNAP))
				err = DL_BADADDR;
		}
	}

	if (!err) {
		INTR_LOCK();
		err = dl_add_filter(dlb, &ns, 1);
		INTR_UNLOCK();
	}
	
	if (err) {
		putq(dlb->dlb_rq, errack(mp, DL_SUBS_BIND_REQ, err));
		return;
	}

	/*
	 * If no bind records existed before this bind, then the current
	 * address is incomplete, because no binds where completed;
	 * this will only occur when the DL_BIND_REQ was for 0xaa.
	 * In this case, this is the first subs bind, and it will
	 * contain the rest of the DLSAP.
	 *
	 * As a special case, not really specified by the DLPI spec,
	 * if the DL_BIND_REQ was for 0xaa, but this _first_ subs bind
	 * contains only a sap (not a snap), then the DL_BIND_REQ is
	 * ignored, and this subs bind will replace it.  Other interpretations
	 * can cause confusion or inconsistent treatment.  In particular,
	 * consider the case:
	 *	dl_bind_req(0xaa); dl_subs_bind(peer, 0xfe);
	 * The first bind is, by definition, incomplete, thus unusable,
	 * but enough to transit the state machine to DL_IDLE.  The
	 * subs bind does contain complete info, although it is not
	 * a continuation of the snap sap.  This implementation will
	 * prefer the 0xfe to the 0xaa when reporting the current address
	 * and when sending datagrams using the default address.
	 *
	 * NB:
	 *	For a stream to be marked as a Netware stream, the first
	 *	bind must be to either a Netware sap, or to the snap sap
	 *	followed by a hier bind to a Netware ethertype.
	 */

	if (!wasbound) {
		dlb->dlb_llc.ssap = ns.dsap;
		dlb->dlb_llc.dsap = ns.dsap;
		dlb->dlb_llc.ctl1 = UI;
		BCOPY(ns.orgcode, dlb->dlb_llc.org_id, SNAP_LEN);

		if (ns.filtertype == NS_8022_LLC_DSAP) {
			/* replace 0xaa with new sap */
			dlb->dlb_addr[dlb->dlb_addrlen] = ns.dsap;
		} else {
			/* append snap to the current address */
			BCOPY(ns.orgcode, dlb->dlb_addr+dlb->dlb_addrlen,
				SNAP_LEN);
			dlb->dlb_addrlen += SNAP_LEN;
			dlb->dlb_saplen += SNAP_LEN;
			if (dlb->dlb_drd && isnetware2(dlb->dlb_mactype, ns))
				dlb->dlb_drd = 0;
		}
	}

	mp->b_datap->db_type = M_PCPROTO;
	dlsbr->dl_primitive = DL_SUBS_BIND_ACK;
	putq(dlb->dlb_rq, mp);
}

/*
 * dl_subs_unbind - undo subs binds 
 */

void
dl_subs_unbind(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	struct ns_8022 ns;
	dl_subs_unbind_req_t *dlsur = (dl_subs_unbind_req_t *)mp->b_rptr;
	uchar *sap = mp->b_rptr + dlsur->dl_subs_sap_offset;
	int len = dlsur->dl_subs_sap_length;
	ushort curtype;
	ushort type;
	int err = 0;
	INTR_LOCK_DECL;

	if (!BOUND(dlb))
		err = DL_OUTSTATE;
	else if (sap + len > mp->b_wptr)
		err = DL_BADADDR;
	else if (dlb->dlb_isether)
		err = chk_ether(&ns, sap, len);
	else
		err = chk_8022(&ns, sap, len);

	if (!err) {
		INTR_LOCK();
		err = dl_del_filter(dlb, &ns, 1);
		INTR_UNLOCK();
	}

	if (err) {
		putq(dlb->dlb_rq, errack(mp, DL_SUBS_UNBIND_REQ, err));
		return;
	}

	/*
	 * See the note in dlb_subs_bind.
	 *
	 * If there are no further bind records, then the first bound
	 * address MUST have been SAP_SNAP, since that is the only way
	 * to have the last bound address removed by a subs unbind.
	 * In this case, the address is recreated as in dlb_bind_sap.
	 */

	if (!dlb->dlb_bound) {
		bzero(&dlb->dlb_llc, sizeof(dlb->dlb_llc));
		dlb->dlb_llc.ssap = SAP_SNAP;
		dlb->dlb_llc.dsap = SAP_SNAP;
		dlb->dlb_llc.ctl1 = UI;
		dlb->dlb_saplen = 1;
		dlb->dlb_addrlen = dlb->dlb_physlen + dlb->dlb_saplen;
	}

	putq(dlb->dlb_rq, okack(mp, DL_SUBS_UNBIND_REQ));
}

/*
 **************************************************
 * bind support routines
 **************************************************
 */

/*
 * chk_ether - validate ethernet types and init ns_8022
 */

static int
chk_ether(ns, sap, len)
	struct ns_8022 *ns;
	uchar *sap;
	int len;
{
	ushort type;

	bzero(ns, sizeof(*ns));

	switch (len) {
	case sizeof(ushort):	type = *(ushort *)sap; break;
	case sizeof(ulong):	type = (ushort)*(ulong *)sap; break;
	default:		return DL_BADADDR;
	}
	if (type < 0x600)
		return DL_BADADDR;

	ns->filtertype = NS_ETHERTYPE;
	ns->ethertype = type;
	return 0;
}

/*
 * chk_8022 - validate 802.2 saps and init ns_8022
 */

static int
chk_8022(ns, sap, len)
	struct ns_8022 *ns;
	uchar *sap;
	int len;
{
	ulong lsap;
	uchar csap;
	int err;

	bzero(ns, sizeof(*ns));

	switch (len) {
	case sizeof(uchar):
		/* hack: allow netware 802.3 raw sap */
		if ((*sap != SAP_NETWARE) && ((*sap & 1) || reserved(*sap)))
			return DL_BADADDR;
		ns->filtertype = NS_8022_LLC_DSAP;
		ns->dsap = *sap;
		break;
	case SNAP_LEN:
		ns->filtertype = NS_8022_LLC_DSAP_SNAP;
		ns->dsap = SAP_SNAP;
		BCOPY(sap, ns->orgcode, SNAP_LEN);
		break;
	case sizeof(ulong):
		lsap = *(ulong *)sap;
		csap = (uchar)lsap;
		if (lsap < 0x100)
			return chk_8022(ns, &csap, sizeof(csap));

		/* handle legacy (AIX 320) saps - deprecated */
		csap = GETSSAP(lsap);
		if (err = chk_8022(ns, &csap, sizeof(csap)))
			return err;
		if (ns->dsap == SAP_SNAP) {
			ns->filtertype = NS_8022_LLC_DSAP_SNAP;
			ns->ethertype = GETTYPE(sap);
			if (isatalk(ns->ethertype)) {
				ns->orgcode[0] = 0x08;
				ns->orgcode[1] = 0x00;
				ns->orgcode[2] = 0x07;
			}
		}
		break;
	default:
		return DL_BADADDR;
	}

	return 0;
}

/*
 * chk_sap - reports what need to do with sap
 *
 * returns:
 * 0 - nothing bound
 * 1 - bound, no listener
 * 2 - bound, listener present
 *
 * NB: must be called with INTR_LOCK held
 */

static int
chk_sap(dlb, ssap)
	DLB *dlb;
	uchar ssap;
{
	conn_t *c;
	int rc = 0;

	/* SNAPs are checked for uniqueness by ns_add_filter */
	if (ssap == SAP_SNAP)
		return 0;

	/* INTR_LOCK must already be held */
	rc = dlb->dlb_ppa->saps[SAPINDEX(ssap)][1] ? 1 : 0;
	for (c = dlb->dlb_ppa->saps[SAPINDEX(ssap)][0]; c; c = c->next) {
		if (c->listen) {
			rc = 2;
			break;
		}
		rc = 1;
	}
	return rc;
}

/*
 * dl_add_filter - add a CDLI filter (if necessary)
 *
 * NB: must be called with INTR_LOCK held
 */

static int
dl_add_filter(dlb, ns, class)
	DLB *dlb;
	struct ns_8022 *ns;
	int class;
{
	struct ns_user ns_user;
	ppa_t *ppa;
	bind_t *bind, **pp;
	conn_t *c, *pend;
	int docdli;
	int err, rc;
	int index, issnap;

	extern void dlpiraw(), dlpiether(), dlpillc();

	TRC(dlb,"dl_add_filter: ns 0x%x %s", ((int *)ns)[1], class?"subs":"");

	ppa = dlb->dlb_ppa;
	issnap = 0;

	/*
	 * terms:
	 *	CL	DL_CLDLS
	 *	CO	DL_CODLS
	 *	CLCO	DL_CLDLS | DL_CODLS
	 *
	 * binding rules:
	 *	1. NS_INCLUDE_* only allowed for CL
	 *	2. NS_PROTO_DL_DONTCARE only allowed for CL
	 *	3. only one CL allowed per sap
	 *	4. if CL is bound, CLCO disallowed, and vice-versa
	 *	5. N CO or COCL streams allowed per sap
	 */

	switch (dlb->dlb_mode) {
	case DL_CLDLS:
		if (dlb->dlb_isether)
			break;
		if (ns->filtertype == NS_8022_LLC_DSAP_SNAP)
			issnap = 1;
		for (c = ppa->saps[SAPINDEX(ns->dsap)][0]; c; c = c->next) {
			if (c->dlb->dlb_mode & DL_CLDLS)
				return DL_BOUND;
		}
		index = 1;
		break;
	case DL_CODLS | DL_CLDLS:
		 if (ppa->saps[SAPINDEX(ns->dsap)][1])
			return DL_BOUND;
		/* fallthrough */
	case DL_CODLS:
		if (dlb->dlb_pkt_format == NS_INCLUDE_MAC ||
		    dlb->dlb_pkt_format == NS_INCLUDE_LLC ||
		    dlb->dlb_pkt_format == NS_PROTO_DL_DONTCARE)
			return -EINVAL;
		index = 0;
		break;
	default:
		DB(assert(0););
		return -EINVAL;
	}

	/* allocate a bind record */
	if (!(bind = getmem(bind_t, 1)))
		return -ENOSR;

	/*
	 * determine if CDLI bind is required (non-ethernet)
	 *
	 * only non-ethernet streams get put in the ppa->sap[] table
	 * this prevents the table from having to accomodate ethertypes
	 */

	c = pend = 0;
	docdli = 1;

	if (!dlb->dlb_isether) {
		if (rc = chk_sap(dlb, ns->dsap)) {
			/* only one listener per sap */
			if (rc == 2 && dlb->dlb_conind)
				return DL_BOUND;
			/* only one connectionless stream per sap */
			if (dlb->dlb_ppa->saps[SAPINDEX(ns->dsap)][1] &&
			    dlb->dlb_mode == DL_CLDLS)
				return DL_BOUND;
			docdli = 0;
		}

		/* create a connection record */
		if (!(c = getmem(conn_t, 1)))
			return -ENOSR;

		c->dlb = dlb;
		c->listen = !!dlb->dlb_conind;	/* conind as a boolean */
		c->bind = bind;
		if (issnap)
			bcopy(ns->orgcode, c->remaddr, SNAP_LEN);

		if (dlb->dlb_conind && !dlb->dlb_pend) {
			if (!(pend = getmem(conn_t, dlb->dlb_conind))) {
				putmem(c);
				return -ENOSR;
			}
		}
	}

	/*
	 * attempt to register sap with CDLI
	 */

	if (docdli) {
		ns_user.isr_data = (caddr_t)dlb;
		ns_user.pkt_format = NS_INCLUDE_LLC | NS_HANDLE_HEADERS;
		if (dlb->dlb_pkt_format == NS_INCLUDE_MAC) {
			ns_user.pkt_format = NS_INCLUDE_MAC | NS_HANDLE_HEADERS;
			ns_user.isr = dlpiraw;
		} else if (dlb->dlb_isether)
			ns_user.isr = dlpiether;
		else {
			ns_user.isr_data = (caddr_t)ppa;
			ns_user.isr = dlpillc;
		}
		ns_user.protoq = 0;
		ns_user.netisr = 0;
		ns_user.ifp = 0;

		/* ns_add_filter() will fail with EEXIST if duplicate type */
		err = ns_add_filter(dlb->dlb_ndd, ns, sizeof(*ns), &ns_user);
		if (err) {
			ERR(dlb, "ns_add_filter: ns_add err %d", err);
			if (c) putmem(c);
			if (pend) putmem(pend);
			putmem(bind);
			if (err == EEXIST || err == EBUSY)
				err = -DL_BOUND;
			return -err;
		}
	}

	/*
	 * New binds are linked on the tail of the list,
	 * so unbinds can intelligently update info_ack addresses.
	 */

	bind->ns = *ns;
	bind->class = class;

	for (pp = &dlb->dlb_bound; *pp; pp = &(*pp)->next)
		;
	*pp = bind;

	/*
	 * No order is associated with the connection record list,
	 * so just add new records at the front.
	 */

	if (c) {
		c->next = ppa->saps[SAPINDEX(ns->dsap)][index];
		ppa->saps[SAPINDEX(ns->dsap)][index] = c;

		if (pend)
			dlb->dlb_pend = pend;
	}

	incstats(dlb, binds);
	return 0;
}

/*
 * dl_del_filter - delete a CDLI filter
 *
 * NB: must be called with INTR_LOCK held
 */

static int
dl_del_filter(dlb, ns, class)
	DLB *dlb;
	struct ns_8022 *ns;
	int class;
{
	bind_t *bind, **pp;
	conn_t *c, **cpp;
	int index, docdli;
	int err;

	/*
	 * remove bind record from list of types bound
	 */

	for (pp = &dlb->dlb_bound; *pp; pp = &(*pp)->next) {
		if (memcmp(ns, &(*pp)->ns, sizeof(*ns)) == 0) {
			bind = *pp;
			/* catch attempts to unbind via incorrect primitive */
			if (class != bind->class)
				return DL_BADADDR;
			*pp = (*pp)->next;
			break;
		}
	}

	/*
	 * remove connection record if CODLS;
	 * determine if need to ns_del_filter
	 */

	docdli = 1;
	if (!dlb->dlb_isether) {
		index = (dlb->dlb_mode == DL_CLDLS ? 1 : 0);
		cpp = &dlb->dlb_ppa->saps[SAPINDEX(ns->dsap)][index];
		for (; *cpp; cpp = &(*cpp)->next) {
			if ((*cpp)->bind == bind) {
				c = *cpp;
				*cpp = (*cpp)->next;
				putmem(c);
				break;
			}
		}

		/* do not del_filter if still have a sap registered */
		if (ns->filtertype != NS_8022_LLC_DSAP_SNAP &&
		    dlb->dlb_ppa->saps[SAPINDEX(ns->dsap)][index])
			docdli = 0;
	}

	/*
	 * remove CDLI filter if requested
	 */

	if (docdli) {
		err = ns_del_filter(dlb->dlb_ndd, &bind->ns, sizeof(bind->ns));
		if (err)	/* nothing else we can do */
			ERR(dlb, "dl_del_filter: ns_del err %d", err);
	}

	putmem(bind);
	return 0;
}
