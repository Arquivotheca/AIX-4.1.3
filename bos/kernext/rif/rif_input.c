static char sccsid[] = "@(#)89	1.2  src/bos/kernext/rif/rif_input.c, sysxrif, bos411, 9428A410j 9/18/93 14:48:29";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: rif_init
 *		rifintr
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

extern  struct protosw rifsw[];
extern	struct domain rifdomain;

#ifndef	RIFPROTO_MAX
#	define	RIFPROTO_MAX	2
#endif	RIFPROTO_MAX

#ifndef	RIFPROTO_RAW
#	define	RIFPROTO_RAW	1
#endif	RIFPROTO_RAW

u_char	rif_protox[RIFPROTO_MAX];
struct	ifqueue		rifintrq;

/*
 * RIF initialization: fill in rif protocol switch table.
 */
rif_init()
{
	register struct protosw *pr;
	register int i;

	pr = pffindproto(PF_RIF, RIFPROTO_RAW, SOCK_RAW);
	if (pr == 0)
		panic("rif_init");
	for (i = 0; i < RIFPROTO_MAX; i++)
		rif_protox[i] = pr - rifsw;
	for (pr = rifdomain.dom_protosw;
	    pr < rifdomain.dom_protoswNPROTOSW; pr++)
		if (pr->pr_domain->dom_family == PF_RIF &&
		    pr->pr_protocol && pr->pr_protocol != RIFPROTO_RAW)
			rif_protox[pr->pr_protocol] = pr - rifsw;

	rifintrq.ifq_maxlen = 2 * IFQ_MAXLEN;
}


/*
 * rifintr -	RIF interrupt handler
 *
 */
rifintr()
{
	register struct mbuf	*m;
	int			s;
	IFQ_LOCK_DECL()

next:
	/*
	 * Get next datagram off input queue 
	 */
	s = i_disable(INTMAX);
	IF_DEQUEUE(&rifintrq, m);
	i_enable(s);
	if (m == 0)
		return;

	/*
	 * Switch out to protocol's input routine.
	 */
	(*rifsw[rif_protox[0]].pr_input)(m, m->m_pkthdr.rcvif);
	goto next;
}

