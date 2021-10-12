static char sccsid[] = "@(#)91	1.3  src/bos/kernext/rif/rif_usrreq.c, sysxrif, bos411, 9428A410j 12/6/93 15:58:27";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: rif_usrreq
 *
 *   ORIGINS: 26,27
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
rif_usrreq(so, req, m, nam, rights, control)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *rights, *control;
{
	register int error = 0;
	register struct rawcb *rp = sotorawcb(so);
	struct mbuf *rpm;

	switch (req) {
	case	PRU_ATTACH:
		if (rp)
			panic("rif_attach");

                NET_MALLOC(rp, struct rawcb *, sizeof *rp, M_PCB, M_WAITOK);
                bzero((caddr_t)rp, sizeof *rp);

		rp = mtod(rpm, struct rawcb *);
		so->so_pcb = (caddr_t)rp;
		break;

	case	PRU_BIND:
		rp = sotorawcb(so);
		nam = m_copym(nam, 0, M_COPYALL, M_WAIT);
		rp->rcb_laddr = mtod(nam, struct sockaddr *);
		return(0);

	case	PRU_CONTROL:
		return (rif_control(so, (int)m, (caddr_t)nam,
			(struct ifnet *)rights));
	default:
		break;
	}
	error =  raw_usrreq(so, req, m, nam, control);

	if (error && (req == PRU_ATTACH) && so->so_pcb)
		NET_FREE(so->so_pcb, M_PCB);
	return (error);
}
