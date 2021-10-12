static char sccsid[] = "@(#)31  1.2  src/bos/kernext/dlpi/udata.c, sysxdlpi, bos41J, 9518A_all 4/28/95 16:52:31";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: dl_udatareq
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
 * udata.c - transmit a datagram
 */

#include "include.h"

static struct output_bundle zerobundle;

/*
 * dl_udatareq - send a UI frame
 */

void
dl_udatareq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_unitdata_req_t *req = (dl_unitdata_req_t *)mp->b_rptr;
	struct output_bundle b;
	llc_t *llc;
	struct mbuf *m;
	uchar *daddr;
	int llclen, mlen, dlen;
	int err = 0;

	/*
	 * unitdata_req verification:
	 *	- correct state
	 *	- correct message length
	 *	- address completely within mblk
	 */

	if (dlb->dlb_state != DL_IDLE && dlb->dlb_state != DL_DATAXFER)
		err = DL_OUTSTATE;
	else if ((mlen = msgdsize(mp->b_cont)) < 1 || mlen > dlb->dlb_n1)
		err = DL_BADDATA;
	else {
		dlen = req->dl_dest_addr_length;
		daddr = mp->b_rptr + req->dl_dest_addr_offset;
		if (daddr + dlen > mp->b_wptr)
			err = DL_BADADDR;
	}

	if (err) {
		putq(dlb->dlb_rq, uderrack(mp, err));
		return;
	}

	/*
	 * prepare frame for transmission
	 */

	if (dlb->dlb_isether ||
	    dlb->dlb_pkt_format == NS_INCLUDE_LLC)
		llclen = 0;
	else
		llclen = LLC_ULEN;

	if (!(m = mkframe(dlb, llclen, mp->b_cont))) {
		putq(dlb->dlb_rq, uderrack(mp, -ENOMEM));
		incstats(dlb, no_bufs);
		return;
	}
	mp->b_cont = 0;	/* now owned by mbuf */

	/* complete the addressing */
	if (dlb->dlb_isether) {
		ushort type = dlb->dlb_type;
		int len = dlb->dlb_physlen;

		if (dlen > len && *(ushort *)(daddr + len))
			type = *(ushort *)(daddr + len);

		/* make sure ethertype is valid; CDLI will use 802.3 if not */
		if (type < 0x600) {
			putq(dlb->dlb_rq, uderrack(mp, DL_BADSAP));
			m_freem(m);
			return;
		}

		b = zerobundle;
		b.key_to_find = daddr;
		b.helpers.ethertype = type;

		if (err = tx_ether(dlb, m, &b)) {
			putq(dlb->dlb_rq, uderrack(mp, err));
			return;
		}
	} else {
		if (llclen) {
			llc = setdsap(dlb, m, daddr, dlen);
			llc->ctl1 = UI;
		}
		if (err = tx_frame(dlb, m, daddr)) {
			putq(dlb->dlb_rq, uderrack(mp, err));
			return;
		}
	}
	
	freeb(mp);
	addstats(dlb, tx_bytes, mlen);
}
