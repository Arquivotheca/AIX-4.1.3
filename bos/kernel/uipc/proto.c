static char sccsid[] = "@(#)94	1.9  src/bos/kernel/uipc/proto.c, sysuipc, bos41J, 9511A_all 2/27/95 14:15:33";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: uipc_configure
 *		uipc_funfrc
 *		uipc_funnel
 *		uipc_sanity
 *		uipc_unfunnel
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *	Base:	uipc_proto.c	7.3 (Berkeley) 6/29/88
 *	Merged: uipc_proto.c	7.4 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"

/*
 * Definitions of protocols supported in the UNIX domain.
 */

extern	struct domain unixdomain;		/* or at least forward */


#define UIPC_LOCK_FUNNEL (NETSYNC_SPL && NETSYNC_LOCKTEST)

#if	UIPC_LOCK_FUNNEL
/* Something of a demonstrator. */
#ifdef _AIX_FULLOSF
#include "kern/lock.h"
#else
#include "net/net_unixlock.h"
#endif
static struct {
	lock_data_t	l;	/* the lock */
	int		c;	/* the depth */
} uipc_lock;
#endif

uipc_configure()
{
#if	UIPC_LOCK_FUNNEL
	lock_init(&uipc_lock.l, 1);
	uipc_lock.c = 0;
#endif
	return domain_add(&unixdomain);
}

#if	NETSYNC_SPL && NETNCPUS > 1

#include "kern/parallel.h"

static void
uipc_sanity()
{
	panic("uipc unfunnel");
}

static void
uipc_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = uipc_sanity;
#if	!UIPC_LOCK_FUNNEL
#ifdef _AIX
	NETSPLX(dfp->object.spl);
#endif /* _AIX */
	unix_release();
#else
	if (--uipc_lock.c < 0)
		panic("uipc_unfunnel");
	if (uipc_lock.c == 0)
		lock_clear_recursive(&uipc_lock.l);
	lock_done(&uipc_lock.l);
#endif
}

static void
uipc_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("uipc funnel");
	dfp->unfunnel = uipc_unfunnel;
#if	!UIPC_LOCK_FUNNEL
	unix_master();
#ifdef _AIX
	NETSPL(dfp->object.spl,net);
#endif /* _AIX */
#else
	lock_write(&uipc_lock.l);
	if (++uipc_lock.c == 1)
		lock_set_recursive(&uipc_lock.l);
#endif
}

#if	!UIPC_LOCK_FUNNEL
/* No "force unfunnel" op required with unix_master/spl */
#define	uipc_funfrc	0
#else
static void
uipc_funfrc(dfp)
	struct domain_funnel *dfp;
{
	/* Same function used for both unfunnel and refunnel */
	if (dfp->unfunnel) {
		/* Restore lock(s) to same depth */
		lock_write(&uipc_lock.l);
		lock_set_recursive(&uipc_lock.l);
		while (++uipc_lock.c != dfp->object.spl)
			lock_write(&uipc_lock.l);
	} else {
		/* Release lock(s), saving depth in spl */
		if (uipc_lock.c <= 0)
			panic("uipc_funfrc");
		dfp->unfunnel = uipc_funfrc;
		dfp->object.spl = uipc_lock.c;
		do {
			if (--uipc_lock.c == 0)
				lock_clear_recursive(&uipc_lock.l);
			lock_done(&uipc_lock.l);
		} while (uipc_lock.c);
	}
}
#endif

#else
#define uipc_funnel	0
#define uipc_funfrc	0
#endif

CONST struct protosw unixsw[] = {
{ SOCK_STREAM,	&unixdomain,	0,	PR_CONNREQUIRED|PR_WANTRCVD|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,	0, 		0,
  uipc_init,	0,		0,		0
},
{ SOCK_DGRAM,	&unixdomain,	0,		PR_ATOMIC|PR_ADDR|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,	0,		0,
  0,		0,		uipc_slowtimo,	0
}
};

struct domain unixdomain =
    { AF_UNIX, "unix", 0, unp_externalize, unp_dispose,
      unixsw, &unixsw[sizeof(unixsw)/sizeof(unixsw[0])],
      0, 0, uipc_funnel, uipc_funfrc };
