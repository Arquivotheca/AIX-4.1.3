/* @(#)56	1.5  src/bos/usr/include/netiso/tp_clnp.h, sockinc, bos411, 9428A410j 3/5/94 12:41:26 */

/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/

/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
/* 
 * ARGO TP
 *
 * $Header: tp_clnp.h,v 5.1 88/10/12 12:16:36 root Exp $
 * $Source: /usr/argo/sys/netiso/RCS/tp_clnp.h,v $
 *
 * AF_ISO net-dependent structures and include files
 *
 */


#ifndef __TP_CLNP__
#define __TP_CLNP__

#ifndef SOCK_STREAM
#include "socket.h"
#endif /* SOCK_STREAM */

#ifndef RTFREE
#include <net/route.h>
#endif
#include <netiso/iso.h>
#include <netiso/clnp.h>
#include <netiso/iso_pcb.h>
#ifndef IF_DEQUEUE
#include <net/if.h>
#endif
#include <netiso/iso_var.h>

struct isopcb tp_isopcb;	
	/* queue of active inpcbs for tp ; for tp with dod ip */

#endif /* __TP_CLNP__ */
