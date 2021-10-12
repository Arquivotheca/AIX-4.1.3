static char sccsid[] = "@(#)24	1.5  src/bos/kernext/intf/intf_input.c, sysxintf, bos411, 9428A410j 6/14/94 16:57:03";
/*
 * COMPONENT_NAME: (SYSXINTF) raw interface services 
 *
 * FUNCTIONS: intf_init, intfintr
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <net/if.h>
#include <net/route.h>
#include <net/spl.h>

#include <aixif/net_if.h>

extern  struct protosw intfsw[];
extern	struct domain intfdomain;

#ifndef	INTFPROTO_MAX
#	define	INTFPROTO_MAX	2
#endif	INTFPROTO_MAX

#ifndef	INTFPROTO_RAW
#	define	INTFPROTO_RAW	1
#endif	INTFPROTO_RAW

u_char	intf_protox[INTFPROTO_MAX];
struct	ifqueue		intfintrq;

int intf_drops = 0;

struct	sockaddr rawdst = { sizeof(rawdst), AF_INTF };
struct	sockaddr rawsrc = { sizeof(rawsrc), AF_INTF };
struct	sockproto intfproto = { AF_INTF, PF_INTF };


/*
 * INTF initialization: fill in intf protocol switch table.
 */
intf_init()
{
	register struct protosw *pr;
	register int i;

	pr = pffindproto(PF_INTF, INTFPROTO_RAW, SOCK_RAW);
	if (pr == 0)
		panic("intf_init");
	for (i = 0; i < INTFPROTO_MAX; i++)
		intf_protox[i] = pr - intfsw;
	for (pr = intfdomain.dom_protosw;
	    pr < intfdomain.dom_protoswNPROTOSW; pr++)
		if (pr->pr_domain->dom_family == PF_INTF &&
		    pr->pr_protocol && pr->pr_protocol != INTFPROTO_RAW)
			intf_protox[pr->pr_protocol] = pr - intfsw;

	intfintrq.ifq_maxlen = 4 * IFQ_MAXLEN;
}


/*
 * intfintr -	INTF interrupt handler
 *
 */
intfintr()
{
	register struct mbuf		*m;
	struct packet_trace_header	*pth;
	IFQ_LOCK_DECL()

next:
	/*
	 * Get next datagram off input queue 
	 */
	IF_DEQUEUE(&intfintrq, m);
	if (m == 0)
		return;

	pth = mtod(m, struct packet_trace_header *);
	pth->rdrops = intf_drops;
	curtime(&(pth->ts));

	if (raw_input(m, &intfproto, &rawsrc, &rawdst))
		intf_drops = 0;
	else
		intf_drops++;
	goto next;
}

