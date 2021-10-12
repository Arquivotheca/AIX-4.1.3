static char sccsid[] = "@(#)04  1.8.1.3  src/bos/usr/ccs/lib/libc/subr_kudp.c, libcrpc, bos41J, 9511A_all 3/3/95 09:51:59";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: ku_recvfrom
 *		rpc_debug
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)subr_kudp.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.26 88/02/08 
 */

#ifdef	_KERNEL

/*
 * subr_kudp.c
 * Subroutines to do UDP/IP sendto and recvfrom in the kernel
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_systm.h>
#include <netinet/ip_var.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <nfs/nfs_trc.h>

/*
 * General kernel udp stuff.
 * The routines below are used by both the client and the server side
 * rpc code.
 */

/*
 * Kernel recvfrom.
 * Pull address mbuf and data mbuf chain off socket receive queue.
 *
 * This routine is to be called with the socket and socket buffer locked.
 */
struct mbuf *
ku_recvfrom(so, from)
	struct socket *so;
	struct sockaddr_in *from;
{
	register struct mbuf	*m;
	register struct mbuf	*m0;
	register struct sockbuf	*sb;
	struct mbuf		*nextrecord;
	register int		len = 0;
	struct	ip	*ip;
	struct	udphdr	*uh;

#ifdef RPCDEBUG
	rpc_debug(4, "urrecvfrom so=%X\n", so);
#endif
	SOCKET_LOCK(so);
	if (sosblock(&(so->so_rcv), so)) {
		SOCKET_UNLOCK(so);
		return((struct mbuf *)NULL);
	}

	sb = &(so->so_rcv);
	m = so->so_rcv.sb_mb;

	if (m == NULL) {
		sbunlock(&(so->so_rcv));
		SOCKET_UNLOCK(so);
		return(m);
	}


	/* remove this packet from the socket buffer */
	nextrecord = m->m_nextpkt;

	/* save the address for the caller */
	if (from)   
		*from = *mtod(m, struct sockaddr_in *);

	/* free the socket name mbuf */
	sbfree(sb, m);
	m = m_free(m);

	/*
	 * Transfer ownership of the remainder of the packet
	 * record away from the socket and advance the socket
	 * to the next record.  Calculate the record's length
	 * while we're at it.
	 */
	for (len = 0, m0 = m; m0; m0 = m0->m_next) {
		sbfree(sb, m0);
		len += m0->m_len;
	}
	so->so_rcv.sb_mb = nextrecord;

	if (len > UDPMSGSIZE) {
		TRCHKL1(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_KURECVFROM_2, len);
	}

	sbunlock(&(so->so_rcv));
	SOCKET_UNLOCK(so);

	ip = (struct ip *)(mtod(m, caddr_t) - sizeof(struct udpiphdr));
	uh = (struct udphdr *)((caddr_t)ip + sizeof(struct ip));
	if (so->so_options & SO_CKSUMRECV) {
		if (uh->uh_sum) {
			register u_int	sum;

			((struct ipovly *)ip)->ih_next = 0;
			((struct ipovly *)ip)->ih_prev = 0;
			((struct ipovly *)ip)->ih_x1 = 0;
			((struct ipovly *)ip)->ih_len = uh->uh_ulen;

			sum =  *(u_short *)((caddr_t)ip);
			sum +=  *(u_short *)((caddr_t)ip + 2);
			sum +=  *(u_short *)((caddr_t)ip + 4);
			sum +=  *(u_short *)((caddr_t)ip + 6);
			sum +=  *(u_short *)((caddr_t)ip + 8);
			sum +=  *(u_short *)((caddr_t)ip + 10);
			sum +=  *(u_short *)((caddr_t)ip + 12);
			sum +=  *(u_short *)((caddr_t)ip + 14);
			sum +=  *(u_short *)((caddr_t)ip + 16);
			sum +=  *(u_short *)((caddr_t)ip + 18);
			sum +=  *(u_short *)((caddr_t)ip + 20);
			sum +=  *(u_short *)((caddr_t)ip + 22);
			sum +=  *(u_short *)((caddr_t)ip + 24);
			sum +=  *(u_short *)((caddr_t)ip + 26);

			sum = (sum & 0xffff) + (sum >> 16);
			sum += sum >> 16;
			sum = (~sum & 0xffff);

			sum += (u_int)in_cksum(m, ntohs((u_short)uh->uh_ulen)
				- sizeof(struct udphdr));
			sum += sum >> 16;
			sum = (~sum & 0xffff);

			if (sum) {
				NETSTAT_LOCK(&udpstat.udps_lock);
				udpstat.udps_badsum++;
				NETSTAT_UNLOCK(&udpstat.udps_lock);

				/* release the bad mbufs */
				m_freem(m);
				return((struct mbuf *)0);
			}
		}
	}

	/*
	 * Advance to the data part of the packet,
	 * freeing the address part (and rights if present).
	 */
	for (m0 = m; 
	     (m0 &&
	      m0->m_type != MT_DATA &&
	      m0->m_type != MT_HEADER) ||
	     m0->m_len == 0; ) {
		m0 = m_free(m0);
	}
	if (m0 == NULL) {
		TRCHKL0(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_KURECVFROM_1);
	}

	return(m0);
}

#ifdef RPCDEBUG
int rpcdebug = 2;

/*VARARGS2*/
rpc_debug(level, str, a1, a2, a3, a4, a5, a6, a7, a8, a9)
        int level;
        char *str;
        int a1, a2, a3, a4, a5, a6, a7, a8, a9;
{

        if (level <= rpcdebug)
                printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif
#endif	/* _KERNEL */
