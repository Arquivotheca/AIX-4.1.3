static char sccsid[] = "@(#)54	1.6  src/bos/usr/ccs/lib/libnetsvc/gethostbind.c, libcnet, bos411, 9428A410j 6/3/94 14:06:54";
/* 
 * COMPONENT_NAME: LIBCNET gethostbind.c
 * 
 * FUNCTIONS: endhostent_s, getanswer_s, gethostbyaddr_s, gethostbyname_s,
 *	      gethostent_s, gethostfunctions, sethostent_s
 *
 * ORIGINS: 26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
 
/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * gethostnamadr.c	6.45 (Berkeley) 02/24/91 
 * sethostent.c	6.9 (Berkeley) 03/19/91 
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */


/*
 * NOTE:
 *	These routines resolve names vnd ip addresses 
 *      via the res_* routines.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/stat.h>
#include <netdb.h>
#include <nl_types.h>
#include "libc_msg.h"

#include "../libc/ts_supp.h" 

#ifdef _THREAD_SAFE

#define GETANSWER(h, a, l, q)	getanswer(a, l, q, h, ht_data)

#define	HOST			(*htent)
#define	HOST_ADDR		(ht_data->host_addr)
#define H_ADDR_PTRS		(ht_data->h_addr_ptrs)
#define HOSTADDR		(ht_data->hostaddr)
#define	HOSTBUF			(ht_data->hostbuf)
#define	HOST_ALIASES		(ht_data->host_aliases)
#define HOST_ADDRS		(ht_data->host_addrs)
#define HOSTF			(ht_data->hostf)
#define STAYOPEN		(ht_data->stayopen)
#define HOST_ADDRESSES		(ht_data->host_addresses)
#define	HOST_ROUTINES		(ht_data->host_routines)

#else

#define	GETANSWER(h, a, l, q)	(h = getanswer(a, l, q))

#define HOST			ht_host
#define	HOST_ADDR		host_addr
#define	H_ADDR_PTRS		h_addr_ptrs
#define	HOSTADDR		hostaddr
#define	HOSTBUF			hostbuf
#define	HOST_ALIASES		host_aliases
#define	HOST_ADDRS		host_addrs
#define	HOSTF			hostf
#define	STAYOPEN		stayopen
#define	HOST_ADDRESSES		host_addresses
#define	HOST_ROUTINES		host_routines

static	struct	hostent		ht_host;
static	struct	in_addr		host_addr;
static	char			*h_addr_ptrs[_MAXADDRS + 1];
static	ulong			host_addresses[_MAXADDRS]; /*Actual Addresses */
static	char			hostaddr[_MAXADDRS];
static	char			hostbuf[_HOSTBUFSIZE];
static	char			*host_aliases[_MAXALIASES];
static	char			*host_addrs[2];
static	int			stayopen = 0;
static	FILE			*hostf = NULL;
static	struct resolver_routines	*host_routines;

#endif /* _THREAD_SAFE */

#include <sys/types.h>

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _resolv_rmutex;
#endif /* _THREAD_SAFE */

#if PACKETSZ > 1024
#define	MAXPACKET	PACKETSZ
#else
#define	MAXPACKET	1024
#endif

typedef union {
	HEADER	hdr;
	u_char	buf[MAXPACKET];
} querybuf;

static union {
	long	al;
	char	ac;
} align;


#ifdef DEBUG
#define	PRINT_DBG(info) \
	if (_res.options & RES_DEBUG) \
		printf info
#else
#define	PRINT_DBG(info)
#endif /* DEBUG */

#ifdef _THREAD_SAFE
static int
getanswer(querybuf *answer, int anslen, int iquery, struct hostent *htent,
	  struct hostent_data *ht_data)
#else
static struct hostent *
getanswer(querybuf *answer, int anslen, int iquery)
#endif /* _THREAD_SAFE */
{
	register HEADER *hp;
	register u_char *cp;
	register int n;
	u_char *eom;
	char *bp, **ap;
	int type, class, buflen, ancount, qdcount;
	int haveanswer, getclass = C_ANY;
	char **hap;

	bzero((char *)&HOST, sizeof(HOST));
	eom = answer->buf + anslen;
	/*
	 * find first satisfactory answer
	 */
	hp = &answer->hdr;
	ancount = ntohs(hp->ancount);
	qdcount = ntohs(hp->qdcount);

	buflen = sizeof(HOSTBUF);
	bp = HOSTBUF;
	cp = answer->buf + sizeof(HEADER);
	if (qdcount) {
		if (iquery) {
			if ((n = dn_expand((char *)answer->buf, eom,
			     cp, bp, buflen)) < 0) {
#ifdef _THREAD_SAFE
				_Set_h_errno(NO_RECOVERY);
#else
				h_errno = NO_RECOVERY;
#endif /* _THREAD_SAFE */
				return (TS_FAILURE);
			}
			cp += n + QFIXEDSZ;
			HOST.h_name = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
		} else
			cp += dn_skipname(cp, eom) + QFIXEDSZ;
		while (--qdcount > 0)
			cp += dn_skipname(cp, eom) + QFIXEDSZ;
	} else if (iquery) {
		if (hp->aa)
#ifdef _THREAD_SAFE
			_Set_h_errno(HOST_NOT_FOUND);
#else
			h_errno = HOST_NOT_FOUND;
#endif /* _THREAD_SAFE */
		else
#ifdef _THREAD_SAFE
			_Set_h_errno(TRY_AGAIN);
#else
			h_errno = TRY_AGAIN;
#endif /* _THREAD_SAFE */
		return (TS_FAILURE);
	}
	ap = HOST_ALIASES;
	*ap = NULL;
	HOST.h_aliases = HOST_ALIASES;
	hap = H_ADDR_PTRS;
	*hap = NULL;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
	HOST.h_addr_list = H_ADDR_PTRS;
#endif
	haveanswer = 0;
	while (--ancount >= 0 && cp < eom) {
		if ((n = dn_expand((char *)answer->buf, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
		type = _getshort(cp);
 		cp += sizeof(u_short);
		class = _getshort(cp);
 		cp += sizeof(u_short) + sizeof(u_long);
		n = _getshort(cp);
		cp += sizeof(u_short);
		if (type == T_CNAME) {
			cp += n;
			if (ap >= &HOST_ALIASES[_MAXALIASES-1])
				continue;
			*ap++ = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
			continue;
		}
		if (iquery && type == T_PTR) {
			if ((n = dn_expand((char *)answer->buf, eom,
			    cp, bp, buflen)) < 0) {
				cp += n;
				continue;
			}
			cp += n;
			HOST.h_name = bp;
			return(TS_FOUND(&HOST));
		}
		if (iquery || type != T_A)  {
			PRINT_DBG(("unexpected answer type %d, size %d\n",
					type, n));
			cp += n;
			continue;
		}
		if (haveanswer) {
			if (n != HOST.h_length) {
				cp += n;
				continue;
			}
			if (class != getclass) {
				cp += n;
				continue;
			}
		} else {
			HOST.h_length = n;
			getclass = class;
			HOST.h_addrtype = (class == C_IN) ? AF_INET : AF_UNSPEC;
			if (!iquery) {
				HOST.h_name = bp;
				bp += strlen(bp) + 1;
			}
		}

		bp += sizeof(align) - ((u_long)bp % sizeof(align));

		if (bp + n >= &HOSTBUF[sizeof(HOSTBUF)]) {
			PRINT_DBG(("size (%d) too big\n", n));
			break;
		}
		bcopy(cp, *hap++ = bp, n);
		bp +=n;
		cp += n;
		haveanswer++;
	}
	if (haveanswer) {
		*ap = NULL;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
		*hap = NULL;
#else
		HOST.h_addr = h_addr_ptrs[0];
#endif
		return (TS_FOUND(&HOST));
	} else {
#ifdef _THREAD_SAFE
		_Set_h_errno(TRY_AGAIN);
#else
		h_errno = TRY_AGAIN;
#endif /* _THREAD_SAFE */
		return (TS_FAILURE);
	}
}

#ifdef _THREAD_SAFE
static int 
gethostbyname_s(char *name, struct hostent *htent, 
		struct hostent_data *ht_data)
{
#else
static struct hostent *
gethostbyname_s(const char *name)
{
	struct hostent *htent;
#endif /* _THREAD_SAFE */

	querybuf 		buf;
	const char 		*cp;
	int 			n;
	struct 	stat		sbuf;

#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	if (name == NULL) {
		TS_ERROR(EINVAL);
		return(TS_FAILURE);
	}
	/*
	 * disallow names consisting only of digits/dots, unless
	 * they end in a dot.
	 */
	if (isdigit(name[0]))
		for (cp = name;; ++cp) {
			if (!*cp) {
				if (*--cp == '.')
					break;
				/*
				 * All-numeric, no dot at the end.
				 * Fake up a hostent as if we'd actually
				 * done a lookup.  What if someone types
				 * 255.255.255.255?  The test below will
				 * succeed spuriously... ???
				 */
				if ((HOST_ADDR.s_addr = inet_addr(name)) == -1){
#ifdef _THREAD_SAFE
					_Set_h_errno(HOST_NOT_FOUND);
#else
					h_errno = HOST_NOT_FOUND;
#endif /* _THREAD_SAFE */
					return (TS_FAILURE);
				}
				HOST.h_name = (char *)name;
				HOST.h_aliases = HOST_ALIASES;
				HOST_ALIASES[0] = NULL;
				HOST.h_addrtype = AF_INET;
				HOST.h_length = sizeof(u_long);
				H_ADDR_PTRS[0] = (char *)&HOST_ADDR;
				H_ADDR_PTRS[1] = (char *)0;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
				HOST.h_addr_list = H_ADDR_PTRS;
#else
				HOST.h_addr = H_ADDR_PTRS[0];
#endif
				return (TS_FOUND(&HOST));
			}
			if (!isdigit(*cp) && *cp != '.') 
				break;
		}

		if (stat(_PATH_RESCONF, &sbuf) != 0) {
#ifdef _THREAD_SAFE
			_Set_h_errno(SERVICE_UNAVAILABLE);
#else
			h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
			return (TS_FAILURE);
		}
		
		if ((n = res_search((char *)name, C_IN, T_A, (uchar *)buf.buf, sizeof(buf))) < 0) {
			PRINT_DBG("res_search failed\n");
			return (TS_FAILURE);
		}
		return (GETANSWER(htent, &buf, n, 0));
}

#ifdef _THREAD_SAFE
static int
gethostbyaddr_s(char *addr, int len, int type, struct hostent *htent,
		struct hostent_data *ht_data)
{
#else
static struct hostent *
gethostbyaddr_s(char *addr, int len, int type)
{
	register struct hostent *htent = (struct hostent *)NULL;
#endif /* _THREAD_SAFE */

	int 			n;
	querybuf 		buf;
	char 			qbuf[MAXDNAME];
	struct	stat		sbuf;
	
	if (stat(_PATH_RESCONF, &sbuf) != 0) {
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
		return (TS_FAILURE);
	}

#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	if (addr == NULL || type != AF_INET) {
		TS_ERROR(EINVAL);
		return(TS_FAILURE);
	}
		(void)sprintf(qbuf, "%u.%u.%u.%u.in-addr.arpa",
			((unsigned)addr[3] & 0xff),
			((unsigned)addr[2] & 0xff),
			((unsigned)addr[1] & 0xff),
			((unsigned)addr[0] & 0xff));
		n = res_query(qbuf, C_IN, T_PTR, (char *)&buf, sizeof(buf));
		if (n < 0) {
			PRINT_DBG(("res_query failed\n"));
			return (TS_FAILURE);
		}
		if (GETANSWER(htent, &buf, n, 1) == TS_FAILURE)
			return(TS_FAILURE);
		htent->h_addrtype = type;
		htent->h_length = len;
		H_ADDR_PTRS[0] = (char *)&HOST_ADDR;
		H_ADDR_PTRS[1] = (char *)0;
		HOST_ADDR = *(struct in_addr *)addr;
#if BSD < 43 && !defined(h_addr)	/* new-style hostent structure */
		htent->h_addr = H_ADDR_PTRS[0];
#endif 
		return(TS_FOUND(htent));
}

#ifdef _THREAD_SAFE
static int
sethostent_s(int stayopen, struct hostent_data *ht_data)
#else
static int
sethostent_s(int stayopen)
#endif /* _THREAD_SAFE */
{
	struct stat	sbuf;

#ifdef _THREAD_SAFE
	_Set_h_errno(NO_RECOVERY);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	/* if you aren't running bind, then you don't want to really
	 * do the stuff below, which only makes sense to have if
         * you are running bind. Besides, BSD only does this for BIND.
	 */

	if (stat(_PATH_RESCONF, &sbuf) != 0) 
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
	else {
		if (stayopen) {
			TS_LOCK(&_resolv_rmutex);
			_res.options |= RES_STAYOPEN | RES_USEVC;
			TS_UNLOCK(&_resolv_rmutex);
		}
	}
}

#ifdef _THREAD_SAFE
static int
endhostent_s(struct hostent_data *ht_data)
#else
static int
endhostent_s(void)
#endif /* _THREAD_SAFE */
{
	struct	stat	sbuf;
	extern void	_res_close();

#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	/* if you aren't running bind, then you don't want to really
	 * do the stuff below, which only makes sense to have if
         * you are running bind. Besides, BSD only does this for BIND.
	 */

	if (stat(_PATH_RESCONF, &sbuf) != 0) 
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
	

	TS_LOCK(&_resolv_rmutex);
	_res.options &= ~(RES_STAYOPEN | RES_USEVC);
	TS_UNLOCK(&_resolv_rmutex);
	_res_close();
}

#ifdef _THREAD_SAFE
static int
gethostent_s(struct hostent *htent, struct hostent_data *ht_data)
#else
static struct hostent *
gethostent_s()
#endif /* _THREAD_SAFE */
{
	/* this routine is only used when reading /etc/hosts */
#ifdef _THREAD_SAFE
	_Set_h_errno(SERVICE_UNAVAILABLE);
#else
	h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
	return(TS_FAILURE);
}

void
gethostfunctions(struct resolver_routines *routines)
{
	routines->gethostbyname = gethostbyname_s;
	routines->gethostbyaddr = gethostbyaddr_s;
	routines->gethostent = gethostent_s;
	routines->sethostent = sethostent_s;
	routines->endhostent = endhostent_s;
}
