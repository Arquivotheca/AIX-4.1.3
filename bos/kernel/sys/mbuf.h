/* @(#)62	1.22.1.11  src/bos/kernel/sys/mbuf.h, sockinc, bos411, 9428A410j 6/21/94 11:14:56 */
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: CLTOM
 *		DTOM
 *		MBSTAT
 *		MBUF_LOCK
 *		MBUF_LOCKINIT
 *		MBUF_UNLOCK
 *		MCHTYPE
 *		MCLALLOC
 *		MCLFREE
 *		MCLGET
 *		MCLREFERENCE
 *		MCLREFERENCED
 *		MCLUNREFERENCE
 *		MFREE
 *		MGET
 *		MGETHDR
 *		MH_ALIGN
 *		MTOCL
 *		MTOD
 *		M_ALIGN
 *		M_COPY_PKTHDR
 *		M_HASCL
 *		M_LEADINGSPACE
 *		M_PREPEND
 *		M_TRAILINGSPACE
 *		M_XMEMD
 *		dtom
 *		m_broadcast
 *		m_clget
 *		m_copy
 *		m_getclust
 *		mtocl
 *		mtod
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
 *	Base:	mbuf.h	7.12 (Berkeley) 9/4/89
 *	Merged: mbuf.h	7.13 (Berkeley) 6/28/90
 */

#ifndef	_SYS_MBUF_H_
#define	_SYS_MBUF_H_

#ifdef	_AIX
#define	MSIZE		256
#define	MCLBYTES	CLBYTES			/* XXX */
#include "sys/xmem.h"
#include "sys/trchkid.h"
#endif	/* _AIX */
#if	defined(_KERNEL) && !defined(_NET_GLOBALS_H_)
#include "net/net_globals.h"
#endif
#ifndef	NET_MALLOC
#include "net/net_malloc.h"
#endif

/*
 * Mbufs are of a single size, MSIZE (machine/machparam.h), which
 * includes overhead.  An mbuf may add a single "mbuf cluster" of size
 * MCLBYTES (also in machine/machparam.h), which has no additional overhead
 * and is used instead of the internal data area; this is done when
 * at least MINCLSIZE of data must be stored.
 */

#define	MLEN		(MSIZE - sizeof(struct m_hdr))	/* normal data len */
#define	MHLEN		(MLEN - sizeof(struct pkthdr))	/* data len w/pkthdr */

#define MINCLSIZE	(MHLEN + 3*MLEN) /* smallest amount to put in cluster */
#define	M_MAXCOMPRESS	(MHLEN / 2)	/* max amount to copy for compression */

/*
 * Macros for type conversion
 * mtod(m,t) -	convert mbuf pointer to data pointer of correct type
 * dtom(x) -	convert data pointer within mbuf to mbuf pointer (XXX)
 */
#define mtod(m,t)	((t)((m)->m_data))
#define	dtom(x)		((struct mbuf *)((u_long)(x) & ~(MSIZE-1)))
#ifdef	_AIX
#define	mtocl(m)	((m)->m_ext.ext_buf)
#define MTOD(m,t)	mtod(m,t)
#define	DTOM(x)		dtom(x)
#define	MTOCL(x)	mtocl(x)
#define	CLTOM(x)	cltom(x)
#endif	/* _AIX */

/* header at beginning of each mbuf: */
struct m_hdr {
	struct	mbuf *mh_next;		/* next buffer in chain */
	struct	mbuf *mh_nextpkt;	/* next chain in queue/record */
	long	mh_len;			/* amount of data in this mbuf */
	caddr_t	mh_data;		/* location of data */
	short	mh_type;		/* type of data in this mbuf */
	short	mh_flags;		/* flags; see below */
};

/* record/packet header in first mbuf of chain; valid if M_PKTHDR set */
struct	pkthdr {
	long	len;		/* total packet length */
	struct	ifnet *rcvif;	/* rcv interface */
};

/* description of external storage mapped into mbuf, valid if M_EXT set */
struct m_ext {
	caddr_t	ext_buf;		/* start of buffer */
#ifdef	_KERNEL
	void	(*ext_free)(caddr_t, u_long, caddr_t);
#else
	void	(*ext_free)();		/* free routine if not the usual */
#endif
	u_long	ext_size;		/* size of buffer, for ext_free */
	caddr_t	ext_arg;		/* additional ext_free argument */
	struct	ext_refq {		/* reference list */
		struct ext_refq *forw, *back;
	} ext_ref;
#ifdef	_AIX
	int	ext_hasxm;		/* cross memory descriptor present? */
	struct	xmem	ext_xmemd;	/* cross memory descriptor */
#endif	/* _AIX */
};

struct mbuf {
	struct	m_hdr m_hdr;
	union {
		struct {
			struct	pkthdr MH_pkthdr;	/* M_PKTHDR set */
			union {
				struct	m_ext MH_ext;	/* M_EXT set */
				char	MH_databuf[MHLEN];
			} MH_dat;
		} MH;
		char	M_databuf[MLEN];		/* !M_PKTHDR, !M_EXT */
	} M_dat;
};
#define	m_next		m_hdr.mh_next
#define	m_len		m_hdr.mh_len
#define	m_data		m_hdr.mh_data
#define	m_type		m_hdr.mh_type
#define	m_flags		m_hdr.mh_flags
#define	m_nextpkt	m_hdr.mh_nextpkt
#define	m_act		m_nextpkt
#define	m_pkthdr	M_dat.MH.MH_pkthdr
#define	m_ext		M_dat.MH.MH_dat.MH_ext
#define	m_pktdat	M_dat.MH.MH_dat.MH_databuf
#define	m_dat		M_dat.M_databuf

/* mbuf flags */
#define	M_EXT		0x0001	/* has associated external storage */
#define	M_PKTHDR	0x0002	/* start of record */
#define	M_EOR		0x0004	/* end of record */

/* mbuf pkthdr flags, also in m_flags */
#define	M_BCAST		0x0100	/* send/received as link-level broadcast */
#define	M_MCAST		0x0200	/* send/received as link-level multicast */
#define	M_WCARD		0x0400	/* received as network-level broadcast */

/* does mbuf hold a broadcast packet? */
#define m_broadcast(m)	((m)->m_flags & (M_BCAST|M_MCAST|M_WCARD))

/* flags copied when copying m_pkthdr */
#define	M_COPYFLAGS	(M_PKTHDR|M_EOR|M_BCAST|M_MCAST|M_WCARD)

/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define	MT_IFADDR	13	/* interface address */
#define MT_CONTROL	14	/* extra-data protocol message */
#define MT_OOBDATA	15	/* expedited data  */
#define MT_MAX		32	/* e.g. */

#ifdef IP_MULTICAST
/* to support IP multicasting */
#define	MT_IPMOPTS	16	/* internet multicast options */
#define	MT_IPMADDR	17	/* internet multicast address */
#define	MT_IFMADDR	18	/* link-level multicast address */
#define	MT_MRTABLE	19	/* multicast routing tables */
#endif

/* flags to m_get/MGET */
#define	M_DONTWAIT	0
#define	M_WAIT		1

/*
 * mbuf allocation/deallocation macros:
 *
 *	MGET(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain internal data.
 *
 *	MGETHDR(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain a packet header
 * and internal data.
 */

#define	MGET(m, how, type) { \
	TRCHKL5T(HKWD_MBUF | hkwd_m_get_in, how, type, getcaller(), \
		getcaller2(), getpid()); \
	MALLOC((m), struct mbuf *, MSIZE, M_MBUF, (!how)); \
	if (m) { \
		MBSTAT2(mbstat.m_mtypes[type], 1); \
		MBSTAT2(mbstat.m_mbufs, 1); \
		(m)->m_next = (m)->m_nextpkt = 0; \
		(m)->m_type = (type); \
		(m)->m_data = (m)->m_dat; \
		(m)->m_flags = 0; \
	} \
	TRCHKL2T(HKWD_MBUF | hkwd_m_get_out, m, mtod(m, caddr_t)); \
}

#define	MGETHDR(m, how, type) { \
	TRCHKL5T(HKWD_MBUF | hkwd_m_get_in, how, type, getcaller(), \
		getcaller2(), getpid()); \
	MALLOC((m), struct mbuf *, MSIZE, M_MBUF, (!how)); \
	if (m) { \
		MBSTAT2(mbstat.m_mtypes[type], 1); \
		MBSTAT2(mbstat.m_mbufs, 1); \
		(m)->m_next = (m)->m_nextpkt = 0; \
		(m)->m_type = (type); \
		(m)->m_data = (m)->m_pktdat; \
		(m)->m_flags = M_PKTHDR; \
	} \
	TRCHKL2T(HKWD_MBUF | hkwd_m_get_out, m, mtod(m, caddr_t)); \
}

/*
 * Mbuf cluster macros.
 * MCLALLOC(caddr_t p, int how) allocates an mbuf cluster.
 * MCLGET adds such clusters to a normal mbuf;
 * the flag M_EXT is set upon success.
 * MCLFREE unconditionally frees a cluster allocated by MCLALLOC,
 */

#define	MCLALLOC(p, how) \
	(p = m_clalloc((struct mbuf *)0, MCLBYTES, (how)))

#define	MCLFREE(p) { \
	MBSTAT2(mbstat.m_clusters, -1); \
	FREE((p), M_CLUSTER); \
}

#define	MCLGET(m, how) \
	(void) m_clalloc((m), MCLBYTES, (how))

#define MCLREFERENCED(m) \
	((m)->m_ext.ext_ref.forw != &((m)->m_ext.ext_ref))

/*
 * MFREE(struct mbuf *m, struct mbuf *n)
 * Free a single mbuf and associated external storage.
 * Place the successor, if any, in n.
 */
#define	MFREE(m, n) {							\
	MBUF_LOCK_DECL()						\
	TRCHKL5T(HKWD_MBUF | hkwd_m_free_in, m, mtod(m, caddr_t), getcaller(), getcaller2(), getpid());\
	if (m->m_flags & M_EXT) {					\
		MBUF_LOCK();						\
		if (MCLREFERENCED(m)) {	/* Unlink with lock held */	\
			remque(&m->m_ext.ext_ref);			\
			MBUF_UNLOCK();					\
		} else if (m->m_ext.ext_free == NULL) {			\
			MBUF_UNLOCK();					\
			MBSTAT2(mbstat.m_clusters, -1);			\
			FREE(m->m_ext.ext_buf, M_CLUSTER);		\
		} else {						\
			MBUF_UNLOCK();					\
			(*(m->m_ext.ext_free))(m->m_ext.ext_buf,	\
			    m->m_ext.ext_size, m->m_ext.ext_arg);	\
			m->m_flags &= ~M_EXT;				\
		}							\
	}								\
									\
	MBSTAT2(mbstat.m_mbufs, -1);					\
	MBSTAT2(mbstat.m_mtypes[m->m_type], -1);			\
	(n) = (m)->m_next; 						\
	FREE(m, M_MBUF);						\
	TRCHKL1T(HKWD_MBUF | hkwd_m_free_out, m);		\
}

#define	m_extfree	m_ext.ext_free
#define	m_extarg	m_ext.ext_arg
#define	m_forw		m_ext.ext_ref.forw
#define	m_back		m_ext.ext_ref.back
#define	m_hasxm		m_ext.ext_hasxm
#define	m_xmemd		m_ext.ext_xmemd

#define  m_getclust(h, t)	m_getclustm(h, t, MCLBYTES)

#define M_HASCL(m)	((m)->m_flags & M_EXT)

#define M_XMEMD(m)	(M_HASCL(m) ? \
				(m->m_hasxm ? &(m->m_xmemd) : &mclxmemd) \
				: &mclxmemd)

extern struct xmem mclxmemd;

struct mbreq {
	int low_mbuf;		/* mbuf low water mark                  */
	int low_clust;		/* mbuf page size low water mark        */
	int initial_mbuf;	/* allocation of mbufs needed           */
	int initial_clust;	/* allocation of page size mbufs needed */
};

#define	MCLREFERENCE(m, n) 	m_clreference((m), (n))

#define	MCLUNREFERENCE(m) 	m_clunreference((m))

/*
 * Copy mbuf pkthdr from from to to.
 * from must have M_PKTHDR set, and to must be empty.
 */
#define	M_COPY_PKTHDR(to, from) { \
	(to)->m_pkthdr = (from)->m_pkthdr; \
	(to)->m_flags = (from)->m_flags & M_COPYFLAGS; \
	(to)->m_data = (to)->m_pktdat; \
}

/*
 * Set the m_data pointer of a newly-allocated mbuf (m_get/MGET) to place
 * an object of the specified size at the end of the mbuf, longword aligned.
 */
#define	M_ALIGN(m, len) \
	{ (m)->m_data += (MLEN - (len)) &~ (sizeof(long) - 1); }
/*
 * As above, for mbufs allocated with m_gethdr/MGETHDR
 * or initialized by M_COPY_PKTHDR.
 */
#define	MH_ALIGN(m, len) \
	{ (m)->m_data += (MHLEN - (len)) &~ (sizeof(long) - 1); }

/*
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 */
#define M_LEADINGSPACE(m) \
        ((m)->m_flags & M_EXT ? ( MCLREFERENCED((m)) ? 0 :  \
            (m)->m_data - (m)->m_ext.ext_buf ) : \
            (m)->m_flags & M_PKTHDR ? (m)->m_data - (m)->m_pktdat : \
            (m)->m_data - (m)->m_dat)

/*
 * Compute the amount of space available
 * after the end of data in an mbuf.
 */
#define M_TRAILINGSPACE(m) \
        ((m)->m_flags & M_EXT ? ( MCLREFERENCED((m)) ? 0 :  \
	    (m)->m_ext.ext_buf + (m)->m_ext.ext_size - \
            ((m)->m_data + (m)->m_len) ) : \
            &(m)->m_dat[MLEN] - ((m)->m_data + (m)->m_len))

/*
 * Arrange to prepend space of size plen to mbuf m.
 * If a new mbuf must be allocated, how specifies whether to wait.
 * If how is M_DONTWAIT and allocation fails, the original mbuf chain
 * is freed and m is set to NULL.
 */
#define	M_PREPEND(m, plen, how) { \
	if (M_LEADINGSPACE(m) >= (plen)) { \
		(m)->m_data -= (plen); \
		(m)->m_len += (plen); \
	} else \
		(m) = m_prepend((m), (plen), (how)); \
	if ((m) && (m)->m_flags & M_PKTHDR) \
		(m)->m_pkthdr.len += (plen); \
}

/* change mbuf to new type */
#define MCHTYPE(m, t) { \
	MBSTAT2(mbstat.m_mtypes[(m)->m_type], -1); \
	MBSTAT2(mbstat.m_mtypes[t], 1); \
	(m)->m_type = t;\
}

/* length to m_copy to copy all */
#define	M_COPYALL	1000000000

/* compatiblity with 4.3 */
#define  m_copy(m, o, l)	m_copym((m), (o), (l), M_DONTWAIT)
#ifdef	_AIX
#define  m_clget(m)		m_clgetm((m), M_DONTWAIT, MCLBYTES)

#define  m_getclust(h, t)	m_getclustm(h, t, MCLBYTES)
#endif	/* _AIX */


/*
 * Mbuf statistics.
 */
struct mbstat {
	u_long	m_mbufs;	/* mbufs obtained from page pool - UNUSED */
	u_long	m_clusters;	/* clusters obtained from page pool - UNUSED */
	u_long	m_mfree;	/* free mbufs - UNUSED */
	u_long	m_clfree;	/* free clusters - UNUSED */
	u_long	m_drops;	/* times failed to find space */
	u_long	m_wait;		/* times waited for space - UNUSED */
	u_long	m_drain;	/* times drained protocols for space */
	u_long	m_mtypes[MT_MAX]; /* type specific mbuf allocations - UNUSED */
};

#ifdef	_KERNEL
extern	simple_lock_data_t mbuf_slock;
#define MBUF_LOCK_DECL() int    _mbufs;
#define MBUF_LOCKINIT() {                                               \
        lock_alloc(&mbuf_slock, LOCK_ALLOC_PIN, MBUF_LOCK_FAMILY, -1);  \
        simple_lock_init(&mbuf_slock);                                  \
}
#ifdef	_POWER_MP
#define MBUF_LOCK()	_mbufs = disable_lock(PL_IMP, &mbuf_slock)
#define MBUF_UNLOCK()	unlock_enable(_mbufs, &mbuf_slock)
#else
#define MBUF_LOCK()	_mbufs = disable_ints()
#define MBUF_UNLOCK()	enable_ints(_mbufs)
#endif
#define MBSTAT(x, i)	(x) += (i)
#define MBSTAT2(x, i)

extern struct	mbstat mbstat;		/* statistics */
extern int	max_linkhdr;		/* largest link-level header */
extern int	max_protohdr;		/* largest protocol header */
extern int	max_hdr;		/* largest link+protocol header */
extern int	max_datalen;		/* MHLEN - max_hdr */
extern int	mclbytes;		/* variable version of MCLBYTES */
#endif
#endif
