static char sccsid[] = "@(#)86	1.2  src/bos/kernext/rif/raw_rif.c, sysxrif, bos411, 9428A410j 5/27/92 17:14:43";
/*
 * COMPONENT_NAME: (SYSXRIF) raw interface services 
 *
 * FUNCTIONS: rif_input, rif_output
 *
 * ORIGINS: 26, 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/socketvar.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>
#include <net/raw_cb.h>

/*
 * Raw interface to RIF protocol.
 */

struct	sockaddr rawdst = { sizeof(rawdst), AF_RIF };
struct	sockaddr rawsrc = { sizeof(rawdst), AF_RIF };
struct	sockproto rifproto = { PF_RIF };

/*
 * Setup generic address and protocol structures
 * for raw_input routine, then pass them along with
 * mbuf chain.
 */
rif_input(m, ifp)
	struct mbuf *m;
	struct ifnet *ifp;
{
	struct	ifaddr		*ifa;
	int			namelen;

	rifproto.sp_protocol = PF_RIF;
	bzero(&rawsrc.sa_data[0], sizeof(rawsrc.sa_data));
	bzero(&rawdst.sa_data[0], sizeof(rawdst.sa_data));

	strncpy(rawdst.sa_data, ifp->if_name,
		sizeof(rawdst.sa_data));
	namelen = strlen(rawdst.sa_data);
	rawdst.sa_data[namelen] = '0' + ifp->if_unit;
	rawdst.sa_data[namelen + 1] = '\0';

	raw_input(m, &rifproto, (struct sockaddr *)&rawsrc,
	  (struct sockaddr *)&rawdst);
}

/*
 * 	RAW RIF output - use interface name to pick interface
 */
rif_output(m0, so)
	struct mbuf *m0;
	struct socket *so;
{
	register struct ifnet *ifp;
	int error = 0;
	struct sockaddr dst;
	struct mbuf *m;
	struct rawcb	*rp;
	struct ifaddr	*ia;

	rp = sotorawcb(so);

	dst.sa_family = AF_RIF;
	bcopy(((struct sockaddr *)rp->rcb_faddr)->sa_data, dst.sa_data,
	       sizeof(dst.sa_data));

	ifp = ifunit(dst.sa_data);
	if (ifp == 0) {
		error = ENETUNREACH;
		goto bad;
	}

	for (m = m0; m; m = m0) {
		m0 = m->m_nextpkt;
		m->m_nextpkt = 0;
		if (error == 0)
			error = (*ifp->if_output)(ifp, m, &dst, (struct rtentry *)NULL);
		else
			m_freem(m);
	}
	return (error);
bad:
	m_freem(m0);
	return (error);
}

