static char sccsid[] = "@(#)27	1.4  src/bos/kernext/intf/intf_usrreq.c, sysxintf, bos411, 9428A410j 12/6/93 15:57:34";
/*
 * COMPONENT_NAME: (SYSXINTF) raw interface services 
 *
 * FUNCTIONS: intf_usrreq.c
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
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>
#include <net/raw_cb.h>

/*ARGSUSED*/
intf_usrreq(so, req, m, nam, control)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	register int error = 0;
	register struct rawcb *rp = sotorawcb(so);

	switch (req) {
	case	PRU_ATTACH:
		if (rp)
			panic("intf_attach");

                NET_MALLOC(rp, struct rawcb *, sizeof *rp, M_PCB, M_WAITOK);
                bzero((caddr_t)rp, sizeof *rp);
		so->so_pcb = (caddr_t)rp;
		break;

	case	PRU_BIND:
		nam = m_copym(nam, 0, M_COPYALL, M_WAIT);
		rp->rcb_laddr = mtod(nam, struct sockaddr *);
		return(0);

	case	PRU_CONTROL:
		return (intf_control(so, (int)m, (caddr_t)nam,
			(struct ifnet *)control));
	default:
		break;
	}
	error = raw_usrreq(so, req, m, nam, control);

	if (error && (req == PRU_ATTACH) && so->so_pcb)
		NET_FREE(so->so_pcb, M_PCB);
	return(error);
}
