static char sccsid[] = "@(#)12  1.1  src/bos/kernext/dlpi/convert.c, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:18";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: free_m
 *		free_mp
 *		free_pad
 *		mblk_to_mbuf
 *		mbuf_to_mblk
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
 * convert.c - mblk to mbuf conversion routines
 *
 * public routines:
 *	mbuf_to_mblk - convert an mbuf chain to a streams message
 *	mblk_to_mbuf - convert a streams message to an mbuf chain, w/ pkthdr
 */

#include "include.h"

struct mbuf *m_clattach();

/* ARGSUSED */
static void
free_m(uchar *m, uchar *b)
{
	(void)m_free((struct mbuf *)m);
}

/* ARGSUSED */
static void
free_mp(uchar *a, ulong b, uchar *mp)
{
	freeb((mblk_t *)mp);
}

/* ARGSUSED */
static void
free_pad(uchar *a, ulong b, uchar *pad)
{
	/* null */
}

/*
 * mbuf_to_mblk - convert an mbuf chain to a streams message
 *
 * on error, the mbuf chain is returned unmolested
 */

mblk_t *
mbuf_to_mblk(m, off)
	struct mbuf *m;
	int off;
{
	mblk_t *mp, *top, **pp;

	if (off) {
		m->m_data += off;
		m->m_len -= off;
		m->m_pkthdr.len -= off;
	}

	top = 0;
	for (pp = &top; m; m = m->m_next) {
		/* XXX what if mbuf is zero length? */
		mp = allocbi(m->m_len, BPRI_HI, free_m, (uchar *)m, m->m_data);
		if (!mp) {
			while (top) {
				mp = top->b_cont;
				freeb(top);
				top = mp;
			}
			break;
		}
		mp->b_wptr += m->m_len;
		*pp = mp;
		pp = &mp->b_cont;
	}
	return top;
}

/*
 * mblk_to_mbuf - convert a streams message to an mbuf chain, w/ pkthdr
 *
 * on error, the streams message and hdr is returned unmolested
 */

struct mbuf *
mblk_to_mbuf(hdr, mp, netware)
	struct mbuf *hdr;
	mblk_t *mp;
	int netware;
{
	struct mbuf *m, *top, **pp;
	dblk_t *dp;
	mblk_t *lastmp;
	int len = 0;
	int needhdr;

	/* NB: hdr must be a pkthdr if provided */
	top = 0;
	needhdr = !hdr;
	for (pp = &top; mp; mp = mp->b_cont) {
		dp = mp->b_datap;

		/* get a pkthdr if needed */
		if (needhdr) {
			m = m_gethdr(M_DONTWAIT, MT_DATA);
			m->m_pkthdr.len = 0;
			needhdr = 0;
		} else
			m = m_get(M_DONTWAIT, MT_DATA);

		if (!m) {
			while (top) {
				top->m_flags &= ~M_EXT;
				top = m_free(top);
			}
			return 0;
		}

		m->m_data = mp->b_rptr;
		m->m_len = mp->b_wptr - mp->b_rptr;
		m->m_flags |= M_EXT;

		/*
		 * if we're not the sole owner, only use current data area
		 * otherwise, we can use everything
		 */
		if (dp->db_ref > 1) {
			m->m_ext.ext_buf = mp->b_rptr;
			m->m_ext.ext_size = m->m_len;
		} else {
			m->m_ext.ext_buf = dp->db_base;
			m->m_ext.ext_size = dp->db_lim - dp->db_base;
		}

		m->m_extfree = free_mp;
		m->m_extarg = (uchar *)mp;
		m->m_forw = m->m_back = &m->m_ext.ext_ref;
		m->m_hasxm = 0;

		len += m->m_len;
		*pp = m;
		pp = &m->m_next;
		lastmp = mp;
	}

	if (hdr) {
		hdr->m_next = top;
		top = hdr;
	}

	top->m_pkthdr.len += len;
	top->m_pkthdr.rcvif = 0;

	/*
	 * Due to a bug in MSDOS Netware Ethernet/802.3 drivers,
	 * packets with an odd number of bytes are discarded!
	 * Therefore, pad (only) outbound Netware packets to
	 * avoid this, if possible; else just send it unpadded.
	 */

	if (netware && (top->m_pkthdr.len & 1)) {
		if (lastmp->b_wptr == lastmp->b_datap->db_lim) {
			static char pad = 0;
			if (*pp = m_clattach(&pad, free_pad, 1, 0, M_DONTWAIT))
				++top->m_pkthdr.len;
		} else {
			++m->m_len;
			++top->m_pkthdr.len;
		}
	}

	return top;
}
