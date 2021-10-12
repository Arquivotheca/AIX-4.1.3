static char sccsid[] = "@(#)42	1.3  src/bos/usr/ccs/lib/libc/inet_netof.c, libcinet, bos411, 9428A410j 6/16/90 01:15:08";
/* 
 * COMPONENT_NAME: CMDNET inet_netof.c
 * 
 * FUNCTIONS: inet_netof 
 *
 * ORIGINS: 26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/types.h>
#include <netinet/in.h>

/*
 * Return the network number from an internet
 * address; handles class a/b/c network #'s.
 */
inet_netof(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (((i)&IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
	else if (IN_CLASSB(i))
		return (((i)&IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	else
		return (((i)&IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
}
