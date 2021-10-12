static char sccsid[] = "@(#)22  1.3  src/bos/kernext/dlpi/intr.c, sysxdlpi, bos41J, 9523A_all 6/1/95 10:33:19";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: dlpiether
 *		dlpiinput
 *		dlpillc
 *		dlpiraw
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
 * intr.c - interrupt handler for incoming frames
 *
 * public routines:
 *	dlpiraw - interrupt handler for raw streams
 *	dlpiether - interrupt handler for ethernet streams
 *	dlpillc - interrupt handler for LLC streams
 *	dlpiinput - LLC frame handler dispatcher
 * private routines:
 */

#include "include.h"
#include <netinet/if_802_5.h>	/* for RI_PRESENT; binary compat hacks */

static mblk_t *bincompat();
static void fmtprom();

/*
 * dlpiraw - interrupt handler for raw streams
 *
 * handles promiscuous and NS_INCLUDE_MAC requests
 */

int
dlpiraw(ndd, m, llhdr, isr)
	NDD *ndd;
	struct mbuf *m;
	caddr_t llhdr;
	struct isr_data_ext *isr;
{
	DLB *dlb = (DLB *)isr->isr_data;
	mblk_t *mp;

	if (!canput(dlb->dlb_rq)) {
discard:
		if (!dlb->dlb_promisc)
			m_freem(m);
		incstats(dlb, rx_discards);
		return;
	}

	/*
	 * _PHYS and _MULTI are similar: accept any (multicast) address,
	 * regardless of sap.  _SAP limits things to only this address,
	 * although it still doesn't care about saps.
	 *
	 * This routine also handles requests for NS_INCLUDE_MAC, which
	 * is already properly filtered, but is otherwise processed the
	 * same as promiscuous streams (that is, no processing).
	 */

	if (dlb->dlb_promisc) {
		if (dlb->dlb_promisc == DL_PROMISC_SAP) {
			if (!(m->m_hdr.mh_flags & M_BCAST) &&
			    bcmp(isr->dstp, ndd->ndd_physaddr, isr->dstlen))
				goto discard;
		}

		/* CDLI still responsible for mbuf, we must work with a copy */
		if (!(m = m_copym(m, 0, M_COPYALL, M_DONTWAIT))) {
			incstats(dlb, no_bufs);
			goto discard;
		}
	}

	/*
	 * NOTE: This header presents no useful information to the user.
	 *       This extra overhead should be removed from this driver
	 *       when binary compatibility with 411 is no longer required.
	 *       All this is necessary is an unprocessed M_DATA message.
	 *       The new code should be:
	 *		if (!(mp = mbuf_to_mblk(m, 0))) {
	 *			incstats(dlb, no_bufs);
	 *			goto discard;
	 *		}
	 */
	if (!(mp = bincompat(dlb, isr, m))) {
		incstats(dlb, no_bufs);
		goto discard;
	}

	/* allocate M_DATA body */
	if (!(mp->b_cont = mbuf_to_mblk(m, 0))) {
		incstats(dlb, no_bufs);
		freeb(mp);
		goto discard;
	}

	addstats(dlb, rx_bytes, m->m_pkthdr.len);
	incstats(dlb, rx_pkts);

	/* send raw, unprocessed frame upstream */
	putq(dlb->dlb_rq, mp);
}

/*
 * bincompat - create a DLPI header for raw and promisc modes
 *
 * NB: This function is for binary compatibility with AIX411 only.
 */

static mblk_t *
bincompat(dlb, isr, m)
	DLB *dlb;
	struct isr_data_ext *isr;
	struct mbuf *m;
{
	int fmt = dlb->dlb_pkt_format;
	dl_unitdata_ind_t *p;
	mblk_t *mp;

	/* allocate DLPI header */
	if (!(mp = dl_gethdr(dlb, 0)))
		return mp;

	/* format DLPI header */
	p = (dl_unitdata_ind_t *)mp->b_rptr;
	p->dl_primitive = DL_UNITDATA_IND;
	p->dl_group_address = m->m_hdr.mh_flags & (M_MCAST | M_BCAST);
	p->dl_dest_addr_offset = DL_DADDR_OFFSET;
	p->dl_src_addr_offset = DL_SADDR_OFFSET;

	if (dlb->dlb_promisc) {
		fmtprom(dlb, isr, m, mp);
	} else {
		/* NS_INCLUDE_MAC */
		int len = dlb->dlb_physlen;
		p->dl_dest_addr_length = len;
		bcopy(isr->dstp, mp->b_rptr+DL_DADDR_OFFSET, len);
		p->dl_src_addr_length = len;
		bcopy(isr->srcp, mp->b_rptr+DL_SADDR_OFFSET, len);
		p->dl_group_address =
			m->m_hdr.mh_flags & (M_MCAST | M_BCAST);
	}
	mp->b_wptr = mp->b_rptr+DL_SADDR_OFFSET+p->dl_src_addr_length;
	return mp;
}

/*
 * fmtprom - format binary compatible addresses for promiscuous mode
 *
 * NB: This function is for binary compatibility with AIX411 only.
 *
 * NB: The magic numbers buried within are for binary compatibility.
 *     They should not be changed, and they aren't worth naming.
 *
 * Notes:
 *	dlpi411 creates a tap user with NS_HANDLE_HEADERS and
 *	whatever else the dlb_pkt_format was.  The demuxers
 *	ignore the packet format, and send a copy of the entire
 *	frame to the tap user.  dlpi411 only put a DLPI header
 *	at the front of the message, and formatted the DLPI
 *	addresses according to dlb_pkt_format.  This is all
 *	we attempt here.
 */

static void
fmtprom(dlb, isr, m, mp)
	DLB *dlb;
	struct isr_data_ext *isr;
	struct mbuf *m;
	mblk_t *mp;
{
	dl_unitdata_ind_t *p = (dl_unitdata_ind_t *)mp->b_rptr;
	int len = dlb->dlb_physlen;
	int fmt = dlb->dlb_pkt_format;
	uchar *sa = mp->b_rptr + DL_SADDR_OFFSET + len;

	bcopy(isr->dstp, mp->b_rptr + DL_DADDR_OFFSET, len);
	bcopy(isr->srcp, mp->b_rptr + DL_SADDR_OFFSET, len);
	p->dl_dest_addr_length = len;
	p->dl_src_addr_length = len;

	if (dlb->dlb_mactype == DL_ETHER) {
		if (fmt == NS_PROTO || fmt == NS_PROTO_SNAP) {
			*(ushort *)sa = *(ushort *)isr->llcp;
			p->dl_src_addr_length += sizeof(ushort);
		}
	} else {
		if (fmt == NS_PROTO || fmt == NS_PROTO_SNAP) {
			int saplen = 1;
			if (isr->llcp[0] == 0xaa)
				saplen += 5;
			bcopy(isr->llcp, sa, saplen);
			p->dl_src_addr_length += saplen;
			if (isr->segp) {
				bcopy(isr->segp,sa+saplen,isr->seglen);
				p->dl_src_addr_length += isr->seglen;
			}
		}
	}
}

/*
 * dlpiether - interrupt handler for ethernet streams
 */

int
dlpiether(ndd, m, llhdr, isr)
	NDD *ndd;
	struct mbuf *m;
	caddr_t llhdr;
	struct isr_data_ext *isr;
{
	DLB *dlb;
	dl_unitdata_ind_t *udi;
	mblk_t *mp;
	multi_t *p;
	uchar *da, *sa;
	int dlen, slen;

	dlb = (DLB *)isr->isr_data;

	if (!canput(dlb->dlb_rq)) {
discard:
		m_freem(m);
		incstats(dlb, rx_discards);
		return;
	}

	/*
	 * If multicast, did user ask for it?
	 */
	if (m->m_hdr.mh_flags & M_MCAST) {
		for (p = dlb->dlb_multi; p; p = p->next) {
			if (memcmp(isr->dstp, p->addr, ndd->ndd_addrlen) == 0)
				break;
		}
		if (!p) goto discard;
	}

	/* address resolution */
	if (dlb->dlb_input)
		(*dlb->dlb_input)(llhdr, 1);

	/*
	 * prepare the DLPI header
	 */

	if (!(mp = dl_gethdr(dlb, 0)))
		goto discard;

	udi = (dl_unitdata_ind_t *)mp->b_rptr;
	udi->dl_primitive = DL_UNITDATA_IND;

	/* format the header according to user preferences */
	if (dlb->dlb_pkt_format != NS_PROTO_DL_DONTCARE) {
		udi->dl_dest_addr_offset = DL_DADDR_OFFSET;
		udi->dl_src_addr_offset  = DL_SADDR_OFFSET;

		da = mp->b_rptr + udi->dl_dest_addr_offset;
		sa = mp->b_rptr + udi->dl_src_addr_offset;

		BCOPY(isr->dstp, da, PHYSLEN);
		BCOPY(isr->srcp, sa, PHYSLEN);
		dlen = PHYSLEN;
		slen = PHYSLEN;

		switch (dlb->dlb_pkt_format) {
		case NS_PROTO:
		case NS_PROTO_SNAP:
			*(ushort *)&sa[slen] = *(ushort *)isr->llcp;
			slen += sizeof(ushort);
			break;
		case NS_PROTO_DL_COMPAT:
			*(ushort *)&da[dlen] = *(ushort *)isr->llcp;
			*(ushort *)&sa[slen] = *(ushort *)isr->llcp;
			dlen += sizeof(ushort);
			slen += sizeof(ushort);
			break;
		}

		udi->dl_dest_addr_length = dlen;
		udi->dl_src_addr_length = slen;
		udi->dl_group_address = m->m_hdr.mh_flags & (M_MCAST | M_BCAST);
	}

	mp->b_wptr += (DL_SADDR_OFFSET + slen);

	/*
	 * construct data portion from mbuf chain
	 */

	if (!(mp->b_cont = mbuf_to_mblk(m, 0))) {
		freeb(mp);
		goto discard;
	}

	addstats(dlb, rx_bytes, m->m_pkthdr.len);
	incstats(dlb, rx_pkts);

	/* send udata upstream */
	putq(dlb->dlb_rq, mp);
}

/*
 * dl_handler[] - table of LLC frame handlers and their characteristics
 */

/* frame handler flags */
#define	FH_CODLS	0x0001		/* connection-oriented only */
#define	FH_CONNECT	0x0002		/* connection setup/teardown */
#define	FH_HDR		0x0004		/* needs a DLPI header */
#define	FH_ADDR		0x0008		/* DLPI header needs address(es) */
#define FH_LISTEN	(FH_CODLS | FH_CONNECT)

/* frame handler routines */
extern void rx_i(), rx_super();
extern void rx_ui(), rx_xid(), rx_test(), rx_sabme(), rx_disc();
extern void rx_ua(), rx_dm(), rx_frmr();

/* extern because used by LLCMSG in tx.c */
handler_t dl_handler[] = {
/* 0*/	{ 0,        "%s bogus llc cmd",    0				},
/* 1*/	{ rx_i,     "%s I %s nr %d ns %d", FH_CODLS			},
/* 2*/	{ rx_super, "%s RR %s nr %d",	   FH_CODLS			},
/* 3*/	{ rx_super, "%s RNR %s nr %d",	   FH_CODLS			},
/* 4*/	{ rx_super, "%s REJ %s nr %d",	   FH_CODLS			},
/* 5*/	{ rx_ui,    "%s UI %s",			       FH_HDR | FH_ADDR },
/* 6*/	{ rx_xid,   "%s XID %s",		       FH_HDR | FH_ADDR },
/* 7*/	{ rx_test,  "%s TEST %s",		       FH_HDR | FH_ADDR },
/* 8*/	{ rx_sabme, "%s SABME %s",	   FH_LISTEN | FH_HDR | FH_ADDR },
/* 9*/	{ rx_disc,  "%s DISC %s",	   FH_CODLS  | FH_HDR		},
/*10*/	{ rx_ua,    "%s UA %s",		   FH_CODLS  | FH_HDR		},
/*11*/	{ rx_dm,    "%s DM %s",		   FH_LISTEN | FH_HDR | FH_ADDR },
/*12*/	{ rx_frmr,  "%s FRMR %s",	   FH_CODLS  | FH_HDR		},
};

/*
 * f2index[] - convert LLC frame type to function handler index
 */

/* extern because used by LLCMSG in tx.c */
char f2index[256] = {
	1, 2, 1, 5, 1, 3, 1, 0, 1, 4, 1, 0, 1, 0, 1,11,
	1, 0, 1, 5, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,11,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,

	1, 0, 1, 9, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 9, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1,10, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 8,
	1, 0, 1,10, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 8,

	1, 0, 1, 0, 1, 0, 1,12, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1,12, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 6,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 6,

	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 7, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 7, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
};

/*
 * dlpillc - interrupt handler for LLC streams
 */

dlpillc(ndd, m, llhdr, isr)
	NDD *ndd;
	struct mbuf *m;
	caddr_t llhdr;
	struct isr_data_ext *isr;
{
	DLB *dlb;
	llc_t *llc;
	llcsnap_t *snap;
	mblk_t *mp;
	pfv_t *f;
	int i, fmt;
	uchar ctl;
	multi_t *p;
	conn_t *c;
	uchar *addr;
	uchar xaddr[MAXADDR_LEN];
	INTR_LOCK_DECL;

	/*
	 * find the desired stream
	 * (see search note in misc.c)
	 */

	dlb = 0;
	llc = mtod(m, llc_t *);

	snap = (llcsnap_t *)isr->llcp;
	if (snap->dsap == SAP_SNAP)
		addr = snap->org_id;
	else {
		/* need to remove RI_PRESENT for address checks */
		addr = xaddr;
		bcopy(isr->srcp, xaddr, ndd->ndd_addrlen);
		xaddr[0] &= ~RI_PRESENT;
	}
	if (!(c = findconn(isr->isr_data, addr, llc->dsap, PRIM(llc)))) {
		TRC(0, "dlpillc: sap 0x%x: no stream drop", llc->dsap);
discard: 
		m_freem(m);
		incstats(dlb, rx_discards);
		return;
	}

	dlb = c->dlb;
	fmt = dlb->dlb_pkt_format;

	/* save incoming source route info */
	if (dlb->dlb_drd)
		drd_update(isr->srcp, isr->srclen, isr->segp, isr->seglen);

	/*
	 * If multicast, did user ask for it?
	 */
	if (m->m_hdr.mh_flags & M_MCAST) {
		for (p = dlb->dlb_multi; p; p = p->next) {
			if (memcmp(isr->dstp, p->addr, ndd->ndd_addrlen) == 0)
				break;
		}
		if (!p) {
			TRC(dlb, "dlpillc: multi drop");
			goto discard;
		}
	}

	/*
	 * this frame is for us, now decide what to do with it
	 *
	 * Note the following anomoly:
	 * If A and B make a connection, listening station is B,
	 * B disconnects but A doesn't, further transmissions from A
	 * will be directed to the listener(!).  Send a DM to any such frames.
	 */

	/* LLC control-field filtering */
	if (fmt == NS_INCLUDE_LLC) {
		i = f2index[UI];	/* always treat as UI */
	} else {
		llc = mtod(m, llc_t *);
		ctl = llc->ctl1;
		if (!(i = f2index[ctl])) {
			TRC(dlb, "dlpillc: ctl 0x%x: bad LLC drop", ctl);
			goto discard;
		}
		if ((dl_handler[i].flags & FH_CODLS) &&
		    !(dlb->dlb_mode & DL_CODLS)) {
			TRC(dlb, "dlpillc: ctl 0x%x: codls mode drop", ctl);
			goto discard;
		}
		/* filter packets not applicable to listeners */
		if (dlb->dlb_conind && !(dl_handler[i].flags & FH_CONNECT)) {
			if (dl_handler[i].flags & FH_CODLS) {
				TRC(dlb, "dlpillc: ctl 0x%x: listen DM", ctl);
				(void)tx_adm_dm(dlb->dlb_ndd, m, isr);
			} else {
				TRC(dlb, "dlpillc: ctl 0x%x: listen drop",ctl);
				goto discard;
			}
			return;
		}
		TRC(dlb, LLCMSG(llc), "->", CRPF(llc), GETNR(llc), GETNS(llc));
	}

	/* get DLPI header; format address(es) */
	mp = 0;
	if ((dl_handler[i].flags & FH_HDR)) {
		if (!(mp = dl_gethdr(dlb, isr->seglen)))
			goto discard;
		if (dl_handler[i].flags & FH_ADDR) {
			mp->b_rptr[0] =
				dl_daddr(mp->b_rptr+DL_DADDR_OFFSET, fmt, isr);
			mp->b_rptr[1] =
				dl_saddr(mp->b_rptr+DL_SADDR_OFFSET, fmt, isr);
		}
	}

	/* call the selected address resolution routine */
	if (dlb->dlb_input)
		(*dlb->dlb_input)(llhdr, !dlb->dlb_drd);

	/* save precious information in mbuf; overwriting MAC header space */
	M_PREPEND(m, 2*sizeof(void **), M_DONTWAIT);
	if (!m) {
		DB(assert(m););
		if (mp) freeb(mp);
		goto discard;
	}
	((void **)m->m_data)[0] = (void *)dl_handler[i].func;	/* handler */
	((void **)m->m_data)[1] = (void *)mp;			/* header */

	/* enque the newly arrived message and process in service context */
	INTR_LOCK();
	if (dlb->dlb_mtail)
		dlb->dlb_mtail->m_nextpkt = m;
	else
		dlb->dlb_mhead = m;
	dlb->dlb_mtail = m;
	INTR_UNLOCK();

	incstats(dlb, rx_pkts);

	qenable(dlb->dlb_rq);
}

/*
 * dlpiinput - LLC frame handler dispatcher
 */

void
dlpiinput(dlb)
	DLB *dlb;
{
	struct mbuf *m;
	pfv_t f;
	mblk_t *mp;
	INTR_LOCK_DECL;

	INTR_LOCK();
	while (m = dlb->dlb_mhead) {
		if (!(dlb->dlb_mhead = m->m_nextpkt))
			dlb->dlb_mtail = 0;
		INTR_UNLOCK();
		f = (pfv_t)(mtod(m, void **)[0]);	/* handler */
		mp = (mblk_t *)(mtod(m, void **)[1]);	/* header */
		m->m_data += 2*sizeof(void **);
		m->m_len -= 2*sizeof(void **);
		m->m_pkthdr.len -= 2*sizeof(void **);
		(*f)(dlb, m, mp);
		INTR_LOCK();
	}
	INTR_UNLOCK();
}
