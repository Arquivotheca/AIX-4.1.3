static char sccsid[] = "@(#)12  1.1  src/bos/usr/ccs/lib/libc/get_myaddress.c, libcrpc, bos411, 9428A410j 10/25/93 20:52:52";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: get_myaddress
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
static char sccsid[] = 	"@(#)get_myaddress.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.5 88/02/08 
 */


/*
 * get_myaddress.c
 *
 * Get client's IP address via ioctl.  This avoids using the NIS.
 * Copyright (C) 1990, Sun Microsystems, Inc.
 */

#include <rpc/types.h>
#include <rpc/pmap_prot.h>
#include <sys/socket.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/syslog.h>
#include <libc_msg.h>

/*
 * don't use gethostbyname, which would invoke NIS
 */
get_myaddress(addr)
	struct sockaddr_in *addr;
{
	int s;
	char buf[BUFSIZ*8], *cp, *cplim;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
 	struct sockaddr_in save;
 	int saved_it = 0;
	int len;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    (void) syslog(LOG_ERR, (char *)oncmsg(LIBCRPC,RPC76,
			"get_myaddress: socket: %m"));
	    exit(1);
	}
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
		(void) syslog(LOG_ERR, 
			      (char *)oncmsg(LIBCRPC,RPC77,
					     "get_myaddress: ioctl (get interface configuration): %m"));
		exit(1);
	}
	ifr = ifc.ifc_req;
#ifdef AF_LINK
#define max(a, b) (a > b ? a : b)
#define size(p)	max((p).sa_len, sizeof(p))
#else
#define size(p) (sizeof (p))
#endif
	cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim;
	     cp += max(sizeof(struct ifreq), 
		       (sizeof (ifr->ifr_name) + size(ifr->ifr_addr))) ) {

		ifr = (struct ifreq *)cp;
		ifreq = *ifr;
		if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			(void) syslog(LOG_ERR, (char *)oncmsg(LIBCRPC,RPC78,
							      "get_myaddress: ioctl: %m"));
			exit(1);
		}
		if ((ifreq.ifr_flags & IFF_UP) &&
	 	    !(ifreq.ifr_flags & IFF_POINTOPOINT) &&
		    ifr->ifr_addr.sa_family == AF_INET) {
 			/*
 			 * if this isn't the loopback interface, get it now!
 			 */
 			if ((ifreq.ifr_flags & IFF_LOOPBACK) == 0) {
				*addr = *((struct sockaddr_in *)&ifr->ifr_addr);
				addr->sin_port = htons(PMAPPORT);
 				saved_it = 0;	/* don't use the saved one */
				break;
			}else if (!saved_it) {
 				saved_it++;
 				/*
 				 * pray here that nothing we're saving could
 				 * be overwritten in a later iteration of
 				 * this loop...
 				 */
 				save = *((struct sockaddr_in *)&ifr->ifr_addr);
			}
		}
	}
 	/*
 	 * we got this far, and the only one we found was the loopback.
 	 * guess we'll have to use it...
 	 */
 	if (saved_it) {
 		*addr = save;
 		addr->sin_port = htons(PMAPPORT);
	}
 
	(void) close(s);
}
