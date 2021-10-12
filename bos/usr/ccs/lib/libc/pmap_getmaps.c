static char sccsid[] = "@(#)14  1.1  src/bos/usr/ccs/lib/libc/pmap_getmaps.c, libcrpc, bos411, 9428A410j 10/25/93 20:53:19";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: pmap_getmaps
 *		
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
static char sccsid[] = 	"@(#)pmap_getmaps.c	1.5 90/07/17 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.11 88/02/08 
 */


/*
 * pmap_getmap.c
 * Client interface to pmap rpc service.
 * contains pmap_getmaps, which is only tcp service involved
 */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <libc_msg.h>
#define NAMELEN 255
#define MAX_BROADCAST_SIZE 1400

extern int errno;

/*
 * Get a copy of the current port maps.
 * Calls the pmap service remotely to do get the maps.
 */
struct pmaplist *
pmap_getmaps(address)
	 struct sockaddr_in *address;
{
	struct pmaplist *head = (struct pmaplist *)NULL;
	int socket = -1;
	struct timeval minutetimeout;
	register CLIENT *client;

	minutetimeout.tv_sec = 60;
	minutetimeout.tv_usec = 0;
	address->sin_port = htons(PMAPPORT);
	client = clnttcp_create(address, PMAPPROG,
	    PMAPVERS, &socket, 50, 500);
	if (client != (CLIENT *)NULL) {
		if (CLNT_CALL(client, PMAPPROC_DUMP, xdr_void, NULL, xdr_pmaplist,
		    &head, minutetimeout) != RPC_SUCCESS) {
			(void) syslog(LOG_ERR, clnt_sperror(client, 
							    (char *)oncmsg(LIBCRPC,RPC79,"pmap_getmaps rpc problem")));
		}
		CLNT_DESTROY(client);
	}
	(void)close(socket);
	address->sin_port = 0;
	return (head);
}
