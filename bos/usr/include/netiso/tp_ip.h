/* @(#)58	1.5  src/bos/usr/include/netiso/tp_ip.h, sockinc, bos411, 9428A410j 3/5/94 12:41:30 */

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
 * $Header: tp_ip.h,v 5.1 88/10/12 12:19:47 root Exp $
 * $Source: /usr/argo/sys/netiso/RCS/tp_ip.h,v $
 *
 * internet IP-dependent structures and include files
 *
 */


#ifndef __TP_IP__
#define __TP_IP__

#ifndef SOCK_STREAM
#include "socket.h"
#endif /* SOCK_STREAM */

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>


struct inpcb tp_inpcb;	
	/* queue of active inpcbs for tp ; for tp with dod ip */

#endif /* __TP_IP__ */
