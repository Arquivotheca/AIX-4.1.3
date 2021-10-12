#ifndef lint
static char sccsid[] = "@(#)79  1.3  src/bos/usr/ccs/lib/libc/ns_ntoa.c, cmdnet, bos411, 9428A410j 11/22/93 09:30:30";
#endif
/* 
 * COMPONENT_NAME: LIBCINET ns_ntoa.c
 * 
 * FUNCTIONS: 
 *           
 *          
 *
 * ORIGINS: 26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * @(#)ns_ntoa.c	6.5 (Berkeley) 6/1/90
 *
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <netns/ns.h>
#include <netinet/in.h>

#include "ts_supp.h"

static char	*spectHex();
#ifdef _THREAD_SAFE
int
ns_ntoa_r(struct ns_addr addr, char *obuf, int obuf_len)
#else
char *
ns_ntoa(struct ns_addr addr)
#endif /* _THREAD_SAFE */
{
#ifndef _THREAD_SAFE
	static char obuf[40];
#endif /* _THREAD_SAFE */
	char *spectHex();
	union { union ns_net net_e; u_long long_e; } net;
	u_short port = htons(addr.x_port);
	register char *cp;
	char *cp2;
	register u_char *up = addr.x_host.c_host;
	u_char *uplim = up + 6;

	TS_EINVAL((obuf == 0) || (obuf_len < 40));

	net.net_e = addr.x_net;
	sprintf(obuf, "%lx", ntohl(net.long_e));
	cp = spectHex(obuf);
	cp2 = cp + 1;
	while (*up==0 && up < uplim) up++;
	if (up == uplim) {
		if (port) {
			sprintf(cp, ".0");
			cp += 2;
		}
	} else {
		sprintf(cp, ".%x", *up++);
		while (up < uplim) {
			while (*cp) cp++;
			sprintf(cp, "%02x", *up++);
		}
		cp = spectHex(cp2);
	}
	if (port) {
		sprintf(cp, ".%x", port);
		spectHex(cp + 1);
	}
	return (TS_FOUND(obuf));
}

static char *
spectHex(char *p0)
{
	int ok = 0;
	int nonzero = 0;
	register char *p = p0;
	for (; *p; p++) switch (*p) {

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		*p += ('A' - 'a');
		/* fall into . . . */
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		ok = 1;
	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
		nonzero = 1;
	}
	if (nonzero && !ok) { *p++ = 'H'; *p = 0; }
	return (p);
}
