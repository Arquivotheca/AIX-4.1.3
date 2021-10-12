#ifndef lint
static char sccsid[] = "@(#)80  1.3  src/bos/usr/ccs/lib/libc/iso_addr.c, cmdnet, bos411, 9428A410j 11/22/93 09:29:49";
#endif
/*
 * COMPONENT_NAME: LIBCINET iso_addr.c
 *
 * FUNCTIONS: iso_addr, iso_ntoa
 *
 * ORIGINS: 26  27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)iso_addr.c	5.3 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <netiso/iso.h>
#include "ts_supp.h"
/* States*/
#define VIRGIN	0
#define GOTONE	1
#define GOTTWO	2
/* Inputs */
#define	DIGIT	(4*0)
#define	END	(4*1)
#define DELIM	(4*2)

#ifdef _THREAD_SAFE
int
iso_addr_r(register char *addr, struct iso_addr out_addr)
{
#else
struct iso_addr *
iso_addr(register char *addr)
{
	static struct iso_addr out_addr;
#endif /* _THREAD_SAFE */
	register char *cp = out_addr.isoa_genaddr;
	char *cplim = cp + sizeof(out_addr.isoa_genaddr);
	register int byte = 0, state = VIRGIN, new;

	bzero((char *)&out_addr, sizeof(out_addr));
	do {
		if ((*addr >= '0') && (*addr <= '9')) {
			new = *addr - '0';
		} else if ((*addr >= 'a') && (*addr <= 'f')) {
			new = *addr - 'a' + 10;
		} else if ((*addr >= 'A') && (*addr <= 'F')) {
			new = *addr - 'A' + 10;
		} else if (*addr == 0) 
			state |= END;
		else
			state |= DELIM;
		addr++;
		switch (state /* | INPUT */) {
		case GOTTWO | DIGIT:
			*cp++ = byte; /*FALLTHROUGH*/
		case VIRGIN | DIGIT:
			state = GOTONE; byte = new; continue;
		case GOTONE | DIGIT:
			state = GOTTWO; byte = new + (byte << 4); continue;
		default: /* | DELIM */
			state = VIRGIN; *cp++ = byte; byte = 0; continue;
		case GOTONE | END:
		case GOTTWO | END:
			*cp++ = byte; /* FALLTHROUGH */
		case VIRGIN | END:
			break;
		}
		break;
	} while (cp < cplim); 
	out_addr.isoa_len = cp - out_addr.isoa_genaddr;
	return (TS_FOUND(&out_addr));
}
static char hexlist[] = "0123456789abcdef";

#ifdef _THREAD_SAFE
int
iso_ntoa_r(struct iso_addr *isoa, char *obuf, int len)
{
#else
char *
iso_ntoa(struct iso_addr *isoa)
{
	static char obuf[64];
#endif /* _THREAD_SAFE */
	register char *out = obuf; 
	register int i;
	register u_char *in = (u_char *)isoa->isoa_genaddr;
	u_char *inlim = in + isoa->isoa_len;

	TS_EINVAL((obuf == 0) || (len < 64));
	out[1] = 0;
	while (in < inlim) {
		i = *in++;
		*out++ = '.';
		if (i > 0xf) {
			out[1] = hexlist[i & 0xf];
			i >>= 4;
			out[0] = hexlist[i];
			out += 2;
		} else
			*out++ = hexlist[i];
	}
	*out = 0;
	return(obuf + 1);
}
