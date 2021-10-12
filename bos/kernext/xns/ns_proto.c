static char sccsid[] = "@(#)35	1.8  src/bos/kernext/xns/ns_proto.c, sysxxns, bos41B, 412_41B_sync 1/3/95 14:33:16";
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: 
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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
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
 *      Base:   ns_proto.c      7.4 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <sys/lock_alloc.h>

#include <netns/ns.h>

/*
 * NS protocol family: IDP, ERR, PE, SPP, ROUTE.
 */

extern	struct domain nsdomain;

static struct {
	Complex_lock	l;	/* the lock */
	int		c;	/* the depth */
} ns_lock;

/*
 * XNS protocols are unmodified for parallelization, and are
 * thus unconditionally funneled with a recursive domain funnel lock 
 * aquired by the socket layer from above and the netisr's from below.
 */

static void
ns_sanity()
{
	panic("ns unfunnel");
}

static void
ns_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = ns_sanity;
	if (--ns_lock.c < 0)
		panic("ns_unfunnel");
	if (ns_lock.c == 0)
		lock_clear_recursive(&ns_lock.l);
	lock_done(&ns_lock.l);
}

static void
ns_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("ns funnel");
	dfp->unfunnel = ns_unfunnel;
	lock_write(&ns_lock.l);
	if (++ns_lock.c == 1)
		lock_set_recursive(&ns_lock.l);
}

static void
ns_funfrc(dfp)
	struct domain_funnel *dfp;
{
	/* Same function used for both unfunnel and refunnel */
	if (dfp->unfunnel) {
		/* Restore lock(s) to same depth */
		lock_write(&ns_lock.l);
		lock_set_recursive(&ns_lock.l);
		while (++ns_lock.c != dfp->object.spl)
			lock_write(&ns_lock.l);
	} else {
		/* Release lock(s), saving depth in spl */
		if (ns_lock.c <= 0)
			panic("ns_funfrc");
		dfp->unfunnel = ns_funfrc;
		dfp->object.spl = ns_lock.c;
		do {
			if (--ns_lock.c == 0)
				lock_clear_recursive(&ns_lock.l);
			lock_done(&ns_lock.l);
		} while (ns_lock.c);
	}
}

ns_lock_init() {
	lock_alloc(&ns_lock.l, LOCK_ALLOC_PIN, NDD_LOCK_CLASS, -1);
	lock_init(&ns_lock.l, TRUE);
	ns_lock.c = 0;
}
struct protosw nssw[] = {
{ 0,		&nsdomain,	0,		0,
  0,		idp_output,	0,		0,
  0,		0,		0,
  ns_init,	0,		0,		0,
},
{ SOCK_DGRAM,	&nsdomain,	0,		PR_ATOMIC|PR_ADDR,
  0,		0,		idp_ctlinput,	idp_ctloutput,
  idp_usrreq,	0,		0,
  0,		0,		0,		0,
},
{ SOCK_STREAM,	&nsdomain,	NSPROTO_SPP,	PR_CONNREQUIRED|PR_WANTRCVD,
  spp_input,	0,		spp_ctlinput,	spp_ctloutput,
  spp_usrreq,	0,		0,
  spp_init,	spp_fasttimo,	spp_slowtimo,	0,
},
{ SOCK_SEQPACKET,&nsdomain,	NSPROTO_SPP,	PR_CONNREQUIRED|PR_WANTRCVD|PR_ATOMIC,
  spp_input,	0,		spp_ctlinput,	spp_ctloutput,
  spp_usrreq_sp,0,		0,
  0,		0,		0,		0,
},
{ SOCK_RAW,	&nsdomain,	NSPROTO_RAW,	PR_ATOMIC|PR_ADDR,
  idp_input,	idp_output,	0,		idp_ctloutput,
  idp_raw_usrreq, 0,		0,
  0,		0,		0,		0,
},
{ SOCK_RAW,	&nsdomain,	NSPROTO_ERROR,	PR_ATOMIC|PR_ADDR,
  idp_ctlinput,	idp_output,	0,		idp_ctloutput,
  idp_raw_usrreq, 0,		0,
  0,		0,		0,		0,
},
};

struct domain nsdomain =
    { AF_NS, "network systems", 0, 0, 0, 
      nssw, &nssw[sizeof(nssw)/sizeof(nssw[0])],
      0, 0, ns_funnel, ns_funfrc };

