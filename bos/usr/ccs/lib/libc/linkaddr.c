#ifndef lint
static char sccsid[] = "@(#)81  1.3  src/bos/usr/ccs/lib/libc/linkaddr.c, cmdnet, bos411, 9428A410j 11/22/93 09:30:08";
#endif
/*
 * COMPONENT_NAME: LIBCINET linkaddr.c 
 *
 * FUNCTIONS: link_addr, link_ntoa
 *
 * ORIGINS: 26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*-
 * Copyright (c) 1990 The Regents of the University of California.
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

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)linkaddr.c	5.1 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>

#include "ts_supp.h"

/* States*/
#define NAMING	0
#define GOTONE	1
#define GOTTWO	2
#define RESET	3
/* Inputs */
#define	DIGIT	(4*0)
#define	END	(4*1)
#define DELIM	(4*2)
#define LETTER	(4*3)

link_addr(addr, sdl)
register char *addr;
register struct sockaddr_dl *sdl;
{
	register char *cp = sdl->sdl_data;
	char *cplim = sdl->sdl_len + (char *)sdl;
	register int byte = 0, state = NAMING, new;

	bzero((char *)&sdl->sdl_family, sdl->sdl_len - 1);
	sdl->sdl_family = AF_LINK;
	do {
		state &= ~LETTER;
		if ((*addr >= '0') && (*addr <= '9')) {
			new = *addr - '0';
		} else if ((*addr >= 'a') && (*addr <= 'f')) {
			new = *addr - 'a' + 10;
		} else if ((*addr >= 'A') && (*addr <= 'F')) {
			new = *addr - 'A' + 10;
		} else if (*addr == 0) {
			state |= END;
		} else if (state == NAMING &&
			   (((*addr >= 'A') && (*addr <= 'Z')) ||
			   ((*addr >= 'a') && (*addr <= 'z'))))
			state |= LETTER;
		else
			state |= DELIM;
		addr++;
		switch (state /* | INPUT */) {
		case NAMING | DIGIT:
		case NAMING | LETTER:
			*cp++ = addr[-1]; continue;
		case NAMING | DELIM:
			state = RESET; sdl->sdl_nlen = cp - sdl->sdl_data; continue;
		case GOTTWO | DIGIT:
			*cp++ = byte; /*FALLTHROUGH*/
		case RESET | DIGIT:
			state = GOTONE; byte = new; continue;
		case GOTONE | DIGIT:
			state = GOTTWO; byte = new + (byte << 4); continue;
		default: /* | DELIM */
			state = RESET; *cp++ = byte; byte = 0; continue;
		case GOTONE | END:
		case GOTTWO | END:
			*cp++ = byte; /* FALLTHROUGH */
		case RESET | END:
			break;
		}
		break;
	} while (cp < cplim); 
	sdl->sdl_alen = cp - LLADDR(sdl);
	new = cp - (char *)sdl;
	if (new > sizeof(*sdl))
		sdl->sdl_len = new;
	return;
}

static char hexlist[] = "0123456789abcdef";

#define OBUFSIZ		64

#ifdef _THREAD_SAFE
int
link_ntoa_r(register const struct sockaddr_dl *sdl, char *obuf, int len)
#else
char *
link_ntoa(register const struct sockaddr_dl *sdl)
#endif /* _THREAD_SAFE */
{
#ifndef _THREAD_SAFE
	static char obuf[OBUFSIZ];
#endif /* _THREAD_SAFE */

	register char *out = obuf; 
	register int i;
	register u_char *in = (u_char *)LLADDR(sdl);
	u_char *inlim = in + sdl->sdl_nlen;
	int firsttime = 1;

	TS_EINVAL((obuf == 0 || len < OBUFSIZ));

	if (sdl->sdl_nlen) {
		bcopy(sdl->sdl_data, obuf, sdl->sdl_nlen);
		out += sdl->sdl_nlen;
		*out++ = ':';
	}
	while (in < inlim) {
		if (firsttime) firsttime = 0; else *out++ = '.';
		i = *in++;
		if (i > 0xf) {
			out[1] = hexlist[i & 0xf];
			i >>= 4;
			out[0] = hexlist[i];
			out += 2;
		} else
			*out++ = hexlist[i];
	}
	*out = 0;
	return(TS_FOUND(obuf));
}
