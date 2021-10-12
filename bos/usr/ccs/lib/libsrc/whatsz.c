static char sccsid[] = "@(#)04	1.6  src/bos/usr/ccs/lib/libsrc/whatsz.c, libsrc, bos411, 9428A410j 4/29/91 16:50:05";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	src_what_sockaddr_size
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

int src_what_sockaddr_size(sun)
struct sockaddr_un *sun;
{
	if(sun->sun_family==AF_UNIX)
		return(SUN_LEN(sun) + 1);
	else
		return(sizeof(struct sockaddr_in));
}
