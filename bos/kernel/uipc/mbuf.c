static char sccsid[] = "@(#)29	1.83  src/bos/kernel/uipc/mbuf.c, sysuipc, bos41J, 9520A_all 5/10/95 18:53:44";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: m_adj
 *		m_cat
 *		m_clalloc
 *		m_clattach
 *		m_clgetm
 *		m_collapse
 *		m_copydata
 *		m_copym
 *		m_dereg
 *		m_free
 *		m_freea
 *		m_freeb
 *		m_freem
 *		m_get
 *		m_getclr
 *		m_getclustm
 *		m_gethdr
 *		m_leadingspace
 *		m_prepend
 *		m_pullup
 *		m_reg
 *		m_retry
 *		m_retryhdr
 *		m_trailingspace
 *		mb_varchange
 *		mbinit
 *		mblk_to_mbuf
 *		mbuf_to_mblk
 *		mbufintr
 *		
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * 
 * (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	uipc_mbuf.c	7.12 (Berkeley) 9/26/89
 *	Merged: uipc_mbuf.c	7.16 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#ifdef	_AIX_FULLOSF
#include "sys/kernel.h"
#endif	/* _AIX_FULLOSF */

#include "sys/mbuf.h"
#include "sys/protosw.h"

#include "net/net_malloc.h"
#include "net/netisr.h"
#ifdef	_AIX_FULLOSF
#include <streams.h>
#endif	/* _AIX_FULLOSF */
#ifdef	_AIX
#include <sys/var.h>		/* thewall is here */
#include <sys/sysconfig.h>
#include <net/netopt.h>
#include <net/spl.h>
#define	min(x,y)	MIN(x,y)
#define	max(x,y)	MAX(x,y)
#endif	/* _AIX */

struct	mbuf *mfreelater;	/* mbuf deallocation list */
int	max_linkhdr;		/* largest link-level header */
int	max_protohdr;		/* largest protocol header */
int	max_hdr;		/* largest link+protocol header */
int	max_datalen;		/* MHLEN - max_hdr */
struct	mbstat mbstat;		/* statistics */
int	mclbytes = MCLBYTES;
#if	NETSYNC_LOCK
simple_lock_data_t	mbuf_slock;
LOCK_ASSERTL_DECL
#endif

#ifdef	_AIX
struct xmem 	mclxmemd;
int		thewall, thewall_dflt;
caddr_t		mbufs, mbclusters;
struct cfgncb 	mb_cfgncb;		/* Notification from sysconfig */
int 		mb_varchange();		/* Function called when changed. */
extern struct var v;
NETOPTINT(thewall);

#endif	/* _AIX */

void
mbinit()
{

#ifdef	_AIX
	max_protohdr = 48;
	max_linkhdr = 24;

	/* Get Notified if the "var" struct changes.
	 * thewall can be changed by chdev et al
	 */
	mb_cfgncb.cbnext = NULL;
	mb_cfgncb.cbprev = NULL;
	mb_cfgncb.func = mb_varchange;
	cfgnadd(&mb_cfgncb);
	ADD_NETOPT(thewall, "%d");
#endif	/* _AIX */
	MBUF_LOCKINIT();
#ifdef _AIX
	netisrinit();
#endif /* _AIX */
#ifdef	DELAYED_FREE
	if (netisr_add(NETISR_MB, mbufintr,
				(struct ifqueue *)0, (struct domain *)0))
		panic("mbinit");
#endif
}

caddr_t
m_clalloc(m, siz, canwait)
	register struct mbuf *m;
	register int siz;
	int canwait;
{
	register caddr_t mcl;

	if (siz != MCLBYTES) {
		if (siz >= mclbytes)
			siz = mclbytes;
		else if (siz & (siz - 1))
			siz = (1 << BUCKETINDX(siz));
	}
	MALLOC(mcl, caddr_t, siz, M_CLUSTER, M_NOWAIT);
	if (mcl == 0 && canwait != M_DONTWAIT) {
		pfreclaim();
		MBSTAT(mbstat.m_drain, 1);
		MALLOC(mcl, caddr_t, siz, M_CLUSTER, M_WAITOK);
	}
	if (m && mcl) {
		m->m_data = m->m_ext.ext_buf = mcl;
		m->m_flags |= M_EXT;
		m->m_ext.ext_size = siz;
		m->m_ext.ext_free = 0;
		m->m_ext.ext_arg  = 0;
#ifdef _AIX
		m->m_hasxm = 0;
#endif /* _AIX */
		m->m_ext.ext_ref.forw = m->m_ext.ext_ref.back =
			&m->m_ext.ext_ref;
	}
	if (mcl)
		MBSTAT2(mbstat.m_clusters, 1);
	else
		MBSTAT(mbstat.m_drops, 1);
	return mcl;
}

#ifdef	DELAYED_FREE
/*
 * The mbuf "isr". Check cluster allocation and free any deferred
 * m_ext mbufs. Both done here to avoid calls from interrupt level.
 */
void
mbufintr()
{
	register struct mbuf *m;
	MBUF_LOCK_DECL()

	MBUF_LOCK();
	m = mfreelater;
	mfreelater = NULL;
	MBUF_UNLOCK();
	while (m) {
		if (m->m_flags & M_EXT) {
			if (MCLREFERENCED(m) || !m->m_ext.ext_free)
				panic("mfreelater");
			(*(m->m_ext.ext_free))(m->m_ext.ext_buf,
			    m->m_ext.ext_size, m->m_ext.ext_arg);
			m->m_flags &= ~M_EXT;
		}
		m = m_free(m);
	}
#if	STREAMS
	{
	extern void ext_freeb(void);
	ext_freeb();
	}
#endif
}
#endif	/* DELAYED_FREE */

/*
 * When MGET fails, ask protocols to free space when short of memory,
 * then re-attempt to allocate an mbuf.
 */

struct mbuf *
m_retry(canwait, type)
	int canwait, type;
{
	struct mbuf *m = 0;

#define	m_retry(m, t)	0
	if (canwait != M_DONTWAIT) {
		pfreclaim();
		MBSTAT(mbstat.m_drain, 1);
		MGET(m, canwait, type);
	}
	if (m == 0)
		MBSTAT(mbstat.m_drops, 1);
	return m;
#undef	m_retry
}

/*
 * As above; retry an MGETHDR.
 */
struct mbuf *
m_retryhdr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	if (m = m_retry(canwait, type)) {
		m->m_flags |= M_PKTHDR;
		m->m_data = m->m_pktdat;
	}
	return (m);
}

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;
	MGET(m, canwait, type);
	return(m);
}

struct mbuf *
m_gethdr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;
	MGETHDR(m, canwait, type);
	return (m);
}

struct mbuf *
m_getclr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	if (m)
		bzero(mtod(m, caddr_t), MLEN);
	return (m);
}

/*
 * Free an allocated mbuf, freeing associated cluster if present.
 */
struct mbuf *
m_free(m)
	struct mbuf *m;
{
	struct mbuf *n;

	MFREE(m, n);
	return (n);
}

void
m_freem(m)
	register struct mbuf *m;
{
	struct mbuf *n;

	while (m) {
		MFREE(m, n);
		m = n;
	}
}

/*
 * Mbuffer utility routines.
 */

/*
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 */
m_leadingspace(m)
register struct mbuf *m;
{
	if (m->m_flags & M_EXT) {
		if (MCLREFERENCED(m))
			return 0;
		return (m->m_data - m->m_ext.ext_buf);
	}
	if (m->m_flags & M_PKTHDR)
		return (m->m_data - m->m_pktdat);
	return (m->m_data - m->m_dat);
}

/*
 * Compute the amount of space available
 * after the end of data in an mbuf.
 */
m_trailingspace(m)
register struct mbuf *m;
{
	if (m->m_flags & M_EXT) {
		if (MCLREFERENCED(m))
			return 0;
		return (m->m_ext.ext_buf + m->m_ext.ext_size -
			(m->m_data + m->m_len));
	}
	return (&m->m_dat[MLEN] - (m->m_data + m->m_len));
}

/*
 * Lesser-used path for M_PREPEND:
 * allocate new mbuf to prepend to chain,
 * copy junk along.
 */
struct mbuf *
m_prepend(m, len, how)
	register struct mbuf *m;
	int len, how;
{
	struct mbuf *mn;

	MGET(mn, how, m->m_type);
	if (mn == (struct mbuf *)NULL) {
		m_freem(m);
		return ((struct mbuf *)NULL);
	}
	if (m->m_flags & M_PKTHDR) {
		M_COPY_PKTHDR(mn, m);
		m->m_flags &= ~M_PKTHDR;
	}
	mn->m_next = m;
	m = mn;
	if (len < MHLEN)
		MH_ALIGN(m, len);
	m->m_len = len;
	return (m);
}

/*
 * Make a copy of an mbuf chain starting "off0" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * The wait parameter is a choice of M_WAIT/M_DONTWAIT from caller.
 */
struct mbuf *
m_copym(m, off0, len, wait)
	register struct mbuf *m;
	int off0, wait;
	register int len;
{
	register struct mbuf *n, **np;
	register int off = off0;
	struct mbuf *top;
	int copyhdr = 0;
	MBUF_LOCK_DECL()

	TRCHKL3T(HKWD_MBUF | hkwd_m_copy_in, m, off, len);
	if (off < 0 || len < 0) {
		panic("m_copym sanity");
		return 0;
	}
	if (off == 0 && m->m_flags & M_PKTHDR)
		copyhdr = 1;
	while (off > 0) {
		if (m == 0) {
			panic("m_copym offset");
			return 0;
		}
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL) {
				panic("m_copym length");
				goto nospace;
			}
			break;
		}
		MGET(n, wait, m->m_type);
		*np = n;
		if (n == 0)
			goto nospace;
		if (copyhdr) {
			M_COPY_PKTHDR(n, m);
			if (len == M_COPYALL)
				n->m_pkthdr.len -= off0;
			else
				n->m_pkthdr.len = len;
			copyhdr = 0;
		}
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_flags & M_EXT) {
			MBUF_LOCK();
			n->m_ext = m->m_ext;
			insque(&n->m_ext.ext_ref, &m->m_ext.ext_ref);
			MBUF_UNLOCK();
			n->m_data = m->m_data + off;
			n->m_flags |= M_EXT;
		} else {
			/*
			 * Try to compress sequential, identical, small
			 * mbufs into a cluster. Do this carefully, but
			 * since we didn't copy flags before, we'll play
			 * loose for now. Note that MT_DATA is effectively
			 * identical to MT_HEADER, which are 1 and 2.
			 */
			register int extra = 0;
			if (len > n->m_len) {
				register struct mbuf *peek = m->m_next;
				while (peek && !(peek->m_flags & M_EXT) &&
				    (m->m_type == peek->m_type ||
				     m->m_type + peek->m_type == 
						MT_DATA + MT_HEADER) 
				     && ((extra + peek->m_len) <= 
						(MCLBYTES - n->m_len))) {

					extra += peek->m_len;
					peek = peek->m_next;
				}
				if (n->m_len + extra > len)
					extra = len - n->m_len;
				if (extra > M_TRAILINGSPACE(n) + MLEN)
					MCLGET(n, wait);
			}
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
			if (extra > 0 && extra <= M_TRAILINGSPACE(n)) {
				off = n->m_len;
				n->m_len += extra;
				do {
					m = m->m_next;
					bcopy(mtod(m, caddr_t),
						mtod(n, caddr_t)+off,
						(unsigned)MIN(extra, m->m_len));
					off += m->m_len;
				} while ((extra -= m->m_len) > 0);
			}
		}
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	TRCHKL1T(HKWD_MBUF | hkwd_m_copy_out, top);
	return (top);
nospace:
	m_freem(top);
	return (0);
}

/*
 * Copy data from an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes, into the indicated buffer.
 */
void
m_copydata(m, off, len, cp)
	register struct mbuf *m;
	register int off;
	register int len;
	caddr_t cp;
{
	register unsigned count;

	TRCHKL4T(HKWD_MBUF | hkwd_m_copydata_in, m, off, len, cp);
	if (off < 0 || len < 0)
		panic("m_copydata sanity");
	while (off > 0) {
		if (m == 0)
			panic("m_copydata offset");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	while (len > 0) {
		if (m == 0)
			panic("m_copydata length");
		count = MIN(m->m_len - off, len);
		bcopy(mtod(m, caddr_t) + off, cp, count);
		len -= count;
		cp += count;
		off = 0;
		m = m->m_next;
	}
	TRCHKL0T(HKWD_MBUF | hkwd_m_copydata_out);
}

/*
 * Concatenate mbuf chain n to m.
 * Both chains must be of the same type (e.g. MT_DATA).
 * Any m_pkthdr is not updated.
 */
void
m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (n->m_len > MINCLSIZE || n->m_len > m_trailingspace(m)) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		    (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

void
m_adj(mp, req_len)
	struct mbuf *mp;
{
	register int len = req_len;
	register struct mbuf *m;
	register count;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		/*
		 * Trim from head.
		 */
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_data += len;
				len = 0;
			}
		}
		if ((m = mp)->m_flags & M_PKTHDR)
			m->m_pkthdr.len -= (req_len - len);
	} else {
		/*
		 * Trim from tail.  Scan the mbuf chain,
		 * calculating its length and finding the last mbuf.
		 * If the adjustment only affects this mbuf, then just
		 * adjust and return.  Otherwise, rescan and truncate
		 * after the remaining size.
		 */
		len = -len;
		count = 0;
		for (;;) {
			count += m->m_len;
			if (m->m_next == (struct mbuf *)0)
				break;
			m = m->m_next;
		}
		if (m->m_len >= len) {
			m->m_len -= len;
			if ((m = mp)->m_flags & M_PKTHDR)
				m->m_pkthdr.len -= len;
			return;
		}
		count -= len;
		if (count < 0)
			count = 0;
		/*
		 * Correct length for chain is "count".
		 * Find the mbuf with last data, adjust its length,
		 * and toss data from remaining mbufs on chain.
		 */
		if ((m = mp)->m_flags & M_PKTHDR)
			m->m_pkthdr.len = count;
		for (; m; m = m->m_next) {
			if (m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while (m = m->m_next)
			m->m_len = 0;
	}
}

/*
 * Rearange an mbuf chain so that len bytes are contiguous
 * and in the data area of an mbuf (so that mtod and dtom
 * will work for a structure of size len).  Returns the resulting
 * mbuf chain on success, frees it and returns null on failure.
 * If there is room, it will add up to max_protohdr-len extra bytes to the
 * contiguous region in an attempt to avoid being called next time.
 */
struct mbuf *
m_pullup(n, len)
	register struct mbuf *n;
	int len;
{
	register struct mbuf *m;
	register int count;
	int space;

	/*
	 * If first mbuf has no cluster, and has room for len bytes
	 * without shifting current data, pullup into it,
	 * otherwise allocate a new mbuf to prepend to the chain.
	 */
	if ((n->m_flags & M_EXT) == 0 &&
	    n->m_data + len < &n->m_dat[MLEN] && n->m_next) {
		if (n->m_len >= len)
			return (n);
		m = n;
		n = n->m_next;
		len -= m->m_len;
	} else {
		if (len > MHLEN)
			goto bad;
		MGET(m, M_DONTWAIT, n->m_type);
		if (m == 0)
			goto bad;
		m->m_len = 0;
		if (n->m_flags & M_PKTHDR) {
			M_COPY_PKTHDR(m, n);
			n->m_flags &= ~M_PKTHDR;
		}
	}
	space = &m->m_dat[MLEN] - (m->m_data + m->m_len);
	do {
		count = min(min(max(len, max_protohdr), space), n->m_len);
		TRCHKL0T(HKWD_MBUF | hkwd_m_pullup_1);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		  (unsigned)count);
		TRCHKL0T(HKWD_MBUF | hkwd_m_pullup_2);

		len -= count;
		m->m_len += count;
		n->m_len -= count;
		space -= count;
		if (n->m_len)
			n->m_data += count;
		else
			n = m_free(n);
	} while (len > 0 && n);
	if (len > 0) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}

#if	STREAMS

/* Mbuf <-> Mblk conversion routines */
/*
 * Copy mbufs to and from mblks. Note these structures are very
 * similar but do not _quite_ line up.
 * A semi-complete list:
 *	MBUF		MBLK
 *	m_data		b_rptr
 *	m_len		b_wptr-b_rptr
 *	m_data+m_len	b_wptr
 *	m_next		b_cont
 *	m_nextpkt	b_next
 *	m_ext		mh_dblk
 *
 *	m_get()		allocb()/allocbi()
 *	m_freem()	freeb()
 *
 * When copying, for now we simply wrap each buffer in the chain
 * inside a buffer of the destination. This is to 1) avoid bcopies,
 * 2) simplify, 3) support only the necessary stuff, and could well
 * be changed.
 */

#include <sys/stream.h>

/*ARGSUSED*/
static void
m_freeb(p, size, bp)
	caddr_t p, bp;
	int size;
{
	(void) freeb((mblk_t *)bp);
}

/*ARGSUSED*/
static void
m_freea(p, q)
	char *p, *q;
{
	(void) m_free((struct mbuf *)p);
}


mblk_t *
mbuf_to_mblk(m, pri)
	struct mbuf *m;
{
	mblk_t *bp, *top = 0, **bpp = &top;
	struct mbuf *nm;

	while (m) {
		int front = m_leadingspace(m);
		int back  = m_trailingspace(m);
		bp = allocbi(front + m->m_len + back, pri,
			m_freea, (char *)m, (uchar *)(m->m_data - front));
		if (bp == 0) {
			while (top) {
				++top->b_datap->db_ref;
				bp = top->b_cont;
				(void) freeb(top);
				top = bp;
			}
			break;
		}
		bp->b_rptr = (unsigned char *)m->m_data;
		bp->b_wptr = (unsigned char *)(m->m_data + m->m_len);
		*bpp = bp;
		bpp = &bp->b_cont;
		nm = m->m_next;
		/*m->m_next = 0;*/
		m = nm;
	}
	return top;
}

struct mbuf *
mblk_to_mbuf(bp, canwait)
	mblk_t *bp;
{
	struct mbuf *m, *top = 0;
	struct mbuf **mp = &top;
	mblk_t *nbp;
	int len = 0;

	while (bp) {
		if (top == 0)
			m = m_gethdr(canwait, MT_DATA);
		else
			m = m_get(canwait, MT_DATA);
		if (m == 0) {
			while (top) {
				top->m_flags &= ~M_EXT;
				top = m_free(top);
			}
			break;
		}
		m->m_data = (caddr_t)bp->b_rptr;
		m->m_len = bp->b_wptr - bp->b_rptr;
		m->m_flags |= M_EXT;
		if (bp->b_datap->db_ref > 1) {		/* referenced */
			m->m_ext.ext_buf = m->m_data;
			m->m_ext.ext_size = m->m_len;
		} else {				/* available */
			m->m_ext.ext_buf = (caddr_t)bp->b_datap->db_base;
			m->m_ext.ext_size =
				bp->b_datap->db_lim - bp->b_datap->db_base;
		}
		m->m_ext.ext_free = m_freeb;
		m->m_ext.ext_arg = (caddr_t)bp;
		m->m_ext.ext_ref.forw = m->m_ext.ext_ref.back = 
			&m->m_ext.ext_ref;
		*mp = m;
		mp = &m->m_next;
		len += m->m_len;
		nbp = bp->b_cont;
		/*bp->b_cont = 0;*/
		bp = nbp;
	}
	if (top) {
		top->m_pkthdr.len = len;
		top->m_pkthdr.rcvif = 0;
	}
	return top;
}

#endif	/* STREAMS */

#ifdef	_AIX
/***************************************************************************** 
 * m_clattach() - allocate mbuf whose data is owned by someone else 
 ****************************************************************************/ 
struct mbuf *
m_clattach(ext_buf, ext_free, ext_size, ext_arg, wait)
caddr_t		ext_buf;
void		(*ext_free)();
int		ext_size;
int		ext_arg;
int		wait;
{
	register struct mbuf *m;

	TRCHKL4T(HKWD_MBUF | hkwd_m_clattach_in, wait, getcaller(),
		getcaller2(), getpid());
	MGET(m, wait, MT_DATA);
	if (m == 0) {
		TRCHKL2T(HKWD_MBUF | hkwd_m_clattach_out, 0, 0);
		return (0);
	}

	m->m_ext.ext_size = ext_size;
	m->m_ext.ext_free = ext_free;
	m->m_ext.ext_buf = ext_buf;
	m->m_ext.ext_arg = ext_arg;
	m->m_data = ext_buf;
	m->m_len = ext_size;
	m->m_flags |= M_EXT;
	m->m_hasxm = 0;
	m->m_forw = m->m_back = &(m)->m_ext.ext_ref;

	TRCHKL2T(HKWD_MBUF | hkwd_m_clattach_out, m, mtod(m, caddr_t));
	return (m);
}

/***************************************************************************** 
 * m_getclustm() - get mbuf and attached cluster
 ****************************************************************************/ 
struct mbuf *
m_getclustm(canwait, type, size)
	int canwait, type;
	int size;
{
	struct mbuf *m;
	caddr_t p;

	MALLOC(m, struct mbuf *, MSIZE, M_MBUF, !canwait);
	if (!m)
		return(NULL);

	MALLOC(p, caddr_t, size, M_CLUSTER, !canwait);
	if (p) {
                m->m_next = m->m_nextpkt = 0;
		m->m_flags = M_EXT;
		m->m_ext.ext_free = 0;
		m->m_hasxm = 0;
                m->m_type = type;
		m->m_ext.ext_size = (size < MAXALLOCSAVE) ? 
					(1<<BUCKETINDX(size)) : 
					round_page(size);
		m->m_data = m->m_ext.ext_buf = p;
		m->m_forw = m->m_back = &(m)->m_ext.ext_ref;
		MBSTAT2(mbstat.m_clusters, 1);
                MBSTAT2(mbstat.m_mtypes[type], 1);
                MBSTAT2(mbstat.m_mbufs, 1);
	} else {
		MBSTAT(mbstat.m_drops, 1);
		FREE(m, M_MBUF);
		m = 0;
	}

	return(m);
}


/* following functions for AIX 3.2 compat */

m_reg() {
	return;
}

m_dereg() {
	return;
}

/***************************************************************************** 
 * m_clgetm() - Given an mbuf, allocate and attach a cluster mbuf to it.
 ****************************************************************************/ 
m_clgetm(m, how, size)
	register struct mbuf *m;
	int	how;
{
	MCLGET(m, how);
	if (M_HASCL(m)) {
		m->m_len = MCLBYTES;
		return(1);
	} else
		return(0);
}

/***************************************************************************** 
 * m_collapse() - collapse mbuf chain to at most "max_mbufs" 
 ****************************************************************************/ 
struct mbuf *
m_collapse(m, max_mbufs)
	struct mbuf *m;
	u_int max_mbufs;
{
	struct mbuf *n, *m0;
	u_int num_mbufs = 1;
	caddr_t p;
	struct mbuf *anchor;

	n = m;
	while (n = n->m_next)
		num_mbufs++;

	if (num_mbufs <= max_mbufs)	/* alright as is	*/
		return(m);

	num_mbufs -= max_mbufs;	/* num_mbufs is now the number to trim */

	/* get a cluster for collapsing	*/
	n = m_getclustm(M_DONTWAIT, MT_DATA, MAXALLOCSAVE);
	if (!n) {
		m_freem(m);
		return(NULL);
	}
	if (m->m_flags & M_PKTHDR) {
		n->m_pkthdr = m->m_pkthdr;
		n->m_flags |= m->m_flags & M_COPYFLAGS;
	}
	
	num_mbufs++;	/* since we're using one to collect
			   have to trim an extra		*/
	anchor = n;	/* anchor will be the top of the chain	*/
	m0 = anchor;	/* m0 will be the mbuf for gather	*/

	/* Follow the mbuf chain and copy data into cluster	*/	
	p = mtod(n, caddr_t);
	n->m_len = 0;
	while (num_mbufs && m) {
		/* get new "gather" mbuf if data won't fit	*/
		if ( ((n->m_len + m->m_len) > MAXALLOCSAVE) ) {
			n = m_getclustm(M_DONTWAIT, MT_DATA, MAXALLOCSAVE);
			if (!n) {
				m_freem(m);
				m_freem(anchor);
				return(NULL);
			}
			m0->m_next = n;
			m0 = n;
			p = mtod(n, caddr_t);
			n->m_len = 0;
			num_mbufs++;	/* account for new n	*/
		}
		bcopy(mtod(m, caddr_t), p, m->m_len);
		n->m_len += m->m_len;
		p += m->m_len;
		m = m_free(m);
		num_mbufs--;
	}
	n->m_next = m;
	if (num_mbufs) {	/* couldn't collapse into max_mbufs	*/
		m_freem(anchor);
		return(NULL);
	} else
		return(anchor);
}

/*
 * Function:  Change thewall if v.v_mbufhw is changed such that it
 *	exceeds thewall.
 *
 * Returns 0 on a CFGV_PREPARE call, n/a otherwise.
 */
int
mb_varchange(
register int	cmd,		/* command: CFGV_PREPARE/CFGV_COMMIT	*/
struct var	*cur,		/* current values of var structure	*/
struct var	*new)		/* proposed values of var structure	*/
{
	switch (cmd)
	{
	    case CFGV_PREPARE:
		/* thewall is about to change, potentially.
		 * We have no problem with it, so return 0.
		 */
		return(0);

	    case CFGV_COMMIT:
		/* Only increase thewall.  Since the whole var
		 * structure is passed down when a change is made,
		 * we really don't know if the user wanted to change
		 * thewall, so only allow increases, and that should
		 * work.  Besides, you need to reboot after decreasing it.
		 * This will work since it'll be changed in the database.
		 */
		if (new->v_mbufhw>thewall) thewall = new->v_mbufhw;
		return(0);
	}
}

void
m_clreference(struct mbuf *m, struct mbuf *n)
{
	MBUF_LOCK_DECL()

	MBUF_LOCK();
	insque(&(m->m_ext.ext_ref), &(n->m_ext.ext_ref));
	MBUF_UNLOCK();
}

void
m_clunreference(struct mbuf *m)
{
	MBUF_LOCK_DECL()

	MBUF_LOCK();
	remque(&(m->m_ext.ext_ref));
	MBUF_UNLOCK();
}

/*
 * Make a copy of an mbuf chain starting "off0" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * The wait parameter is a choice of M_WAIT/M_DONTWAIT from caller.
 */
struct mbuf *
m_copymext(m, off0, len, wait)
	register struct mbuf *m;
	int off0, wait;
	register int len;
{
	register struct mbuf *n, **np;
	register int off = off0;
	struct mbuf *top;
	int copyhdr = 0;
	MBUF_LOCK_DECL()

	TRCHKL3T(HKWD_MBUF | hkwd_m_copy_in, m, off, len);
	if (off < 0 || len < 0) {
		panic("m_copymext sanity");
		return 0;
	}
	if (off == 0 && m->m_flags & M_PKTHDR)
		copyhdr = 1;
	while (off > 0) {
		if (m == 0) {
			panic("m_copymext offset");
			return 0;
		}
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL) {
				panic("m_copymext length");
				goto nospace;
			}
			break;
		}
		MGET(n, wait, m->m_type);
		*np = n;
		if (n == 0)
			goto nospace;
		if (copyhdr) {
                        n->m_pkthdr = m->m_pkthdr;
                        n->m_flags = m->m_flags & M_COPYFLAGS;
			if (len == M_COPYALL)
				n->m_pkthdr.len -= off0;
			else
				n->m_pkthdr.len = len;
			copyhdr = 0;
		}
		if (m->m_flags & M_EXT) {
                        MCLGET(n, wait);
			n->m_flags |= M_EXT;
		} 
		n->m_len = MIN(len, m->m_len - off);
		bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			(unsigned)n->m_len);
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	TRCHKL1T(HKWD_MBUF | hkwd_m_copy_out, top);
	return (top);
nospace:
	m_freem(top);
	return (0);
}

#endif	/* _AIX */
