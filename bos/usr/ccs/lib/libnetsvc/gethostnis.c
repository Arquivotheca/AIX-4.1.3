static char sccsid[] = "@(#)56	1.5  src/bos/usr/ccs/lib/libnetsvc/gethostnis.c, libcnet, bos411, 9428A410j 6/3/94 14:11:42";
/* 
 * COMPONENT_NAME: LIBCNET gethostnis.c
 * 
 * FUNCTIONS: endhostent, gethostbyaddr, gethostbyname, gethostent, 
 *            interpret, sethostent, sethostfile, yellowup 
 *
 * ORIGINS: 24 26  27 71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 by Sun Microsystems, Inc.
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
 * gethostnamadr.c	6.45 (Berkeley) 2/24/91
 * sethostent.c	6.9 (Berkeley) 3/19/91 
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/* 
 * NOTE:
 *	These routines resolve names and ip addresses via
 *	yellow pages/nis.
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/time.h>			/*AIX - NOTE location of include file */
#include <nl_types.h>
#include "libc_msg.h"

#include "../libc/ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ypresolv_rmutex;
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE

#define	INTERPRET(h, val, len)	interpret(h, val, len, ht_data)
#define	YELLOWUP(flg)		yellowup(flg, ht_data)
	
#define	HOST			(*htent)
#define HOST_ADDR		(ht_data->host_addr)
#define	H_ADDR_PTRS		(ht_data->h_addr_ptrs)
#define	HOSTADDR		(ht_data->hostaddr)
#define	HOST_ALIASES		(ht_data->host_aliases)
#define	HOST_ADDRS		(ht_data->host_addrs)
#define	STAYOPEN		(ht_data->stayopen)

#define	CURRENT			ht_data->current
#define	CURRENTLEN		ht_data->currentlen

#else

#define	INTERPRET(h, val, len)	(h = interpret(val, len))
#define	YELLOWUP(flg)		yellowup(flg)

#define	HOST			ht_host
#define	HOST_ADDR		host_addr
#define	H_ADDR_PTRS		h_addr_ptrs
#define	HOSTADDR		hostaddr
#define	HOST_ALIASES		host_aliases
#define	HOST_ADDRS		host_addrs
#define	STAYOPEN		stayopen
#define	CURRENT			current
#define	CURRENTLEN		currentlen

static	struct	hostent	ht_host;
static	struct	in_addr	host_addr;
static	char	*h_addr_ptrs[_MAXADDRS + 1];
static	char	hostaddr[_MAXADDRS];
static	char	*host_aliases[_MAXALIASES];
static	char	*host_addrs[2];
static	int	stayopen = 0;
static	char	*current = NULL;	/* current entry, analogous to hostf */
static	int	currentlen;

#endif /* _THREAD_SAFE */

#ifndef _THREAD_SAFE
static	struct hostent *interpret();
#endif /* _THREAD_SAFE */        

static char	domain[256];

#if PACKETSZ > 1024
#define	MAXPACKET	PACKETSZ
#else
#define	MAXPACKET	1024
#endif

#ifdef DEBUG
#define PRINT_DBG(info) \
	if (_res.options & RES_DEBUG) \
		printf info
#else
#define	PRINT_DBG(info)
#endif /* DEBUG */

#ifdef _THREAD_SAFE
static int
gethostbyname_s(char *name, struct hostent *htent,
			struct hostent_data *ht_data)
{
#else
static struct hostent *
gethostbyname_s(const char *name)
{
	struct	hostent	*htent;
#endif /* _THREAD_SAFE */

	register char 		*cp;
	int 			reason, vallen, is_yp_up;
	char 			lowname[MAXDNAME];
	register char		*lp = lowname;
	char			*val = NULL;

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
	while (*name)
		if (isupper((int)*name))
			*lp++ = tolower((int)*name++);
		else
			*lp++ = *name++;
	*lp = '\0';
	TS_LOCK(&_ypresolv_rmutex);
	is_yp_up = yellowup(1);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (is_yp_up) {
		TS_LOCK(&_ypresolv_rmutex);
		reason = yp_match(domain, "hosts.byname", lowname, 
				  strlen(lowname), &val, &vallen);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason)
			PRINT_DBG(("reason yp_match failed is %d\n", reason));
		else {
			if ((INTERPRET(htent, val, vallen + 1)) != TS_FAILURE) {
				free((void *)val);
				return (TS_FOUND(htent));
			}
		}	
		free((void *)val);
#ifdef _THREAD_SAFE
		_Set_h_errno(HOST_NOT_FOUND);
#else
		h_errno = HOST_NOT_FOUND;
#endif /* _THREAD_SAFE */
	}
	else 
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */

	return(TS_FAILURE);
}

#ifdef _THREAD_SAFE
static int
gethostbyaddr_s(char *addr, int len, int type, struct hostent *htent,
		struct hostent_data *ht_data)
{
#else
static struct hostent *
gethostbyaddr_s(const char *addr, int len, int type)
{
	register struct hostent *htent = (struct hostent *) NULL;
#endif /* _THREAD_SAFE */
	char			*adrstr, *val = NULL;
	int		 	is_yp_up, reason, vallen;
	
#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */

	if (addr == NULL || type != AF_INET) {
		TS_ERROR(EINVAL);
		return (TS_FAILURE);
	}
	TS_LOCK(&_ypresolv_rmutex);
	is_yp_up = yellowup(1);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (is_yp_up) {
		adrstr = inet_ntoa(*(struct in_addr *)addr); 
		TS_LOCK(&_ypresolv_rmutex);
		reason = yp_match(domain, "hosts.byaddr", adrstr,
				  strlen(adrstr), &val, &vallen);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason)
			PRINT_DBG(("reason yp_match failed is %d\n", reason));
		else {
			if ((INTERPRET(htent, val, vallen + 1)) != TS_FAILURE) {
				free((void *)val);
				return (TS_FOUND(htent));
			}
		}
		free((void *)val);
#ifdef _THREAD_SAFE
		_Set_h_errno(HOST_NOT_FOUND);
#else
		h_errno = HOST_NOT_FOUND;
#endif /* _THREAD_SAFE */
	}
	else 
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */

	return(TS_FAILURE);
}

#ifdef _THREAD_SAFE
static int
sethostent_s(int stayopenflag, struct hostent_data *ht_data)
#else
static int
sethostent_s(int stayopenflag)
#endif /* _THREAD_SAFE */
{
	int	is_yp_up, got_domain;
	nl_catd catd;

#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	TS_LOCK(&_ypresolv_rmutex);
	is_yp_up = yellowup(1);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (is_yp_up) {
		if (CURRENT)
			free((void *)CURRENT);
		CURRENT = NULL;
		STAYOPEN |= stayopenflag ? 1 : 0;
	} 
	else
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
}

#ifdef _THREAD_SAFE
static int
endhostent_s(struct hostent_data *ht_data)
#else
static int
endhostent_s()
#endif /* _THREAD_SAFE */
{
	if (CURRENT && !STAYOPEN) {
		(void) free((void *)CURRENT);
		CURRENT = NULL;
	}
}

#ifdef _THREAD_SAFE
static int
gethostent_s(struct hostent *htent, struct hostent_data *ht_data)
{
#else
static struct hostent *
gethostent_s()
{
	struct hostent	*htent;
#endif /* _THREAD_SAFE */
	int		is_yp_up, reason;
	char		*key, *val;
	int		keylen, vallen;

#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	TS_LOCK(&_ypresolv_rmutex);
	is_yp_up = yellowup(0);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (!is_yp_up) {
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
		return(TS_FAILURE);
	}
	if (CURRENT == NULL) {
		TS_LOCK(&_ypresolv_rmutex);
		reason = yp_first(domain, "hosts.byaddr", &key,
				  &keylen, &val, &vallen);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
			PRINT_DBG(("reason yp_first failed is %d\n", reason));
			return (TS_FAILURE);
		}
	}
	else {
		free((void *)val);
		TS_LOCK(&_ypresolv_rmutex);
		reason = yp_next(domain, "hosts.byaddr", CURRENT,
			 CURRENTLEN, &key, &keylen, &val, &vallen);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
			PRINT_DBG(("reason yp_next failed is %d\n", reason));
			return (TS_FAILURE);
		}
	}
	if (CURRENT)
		free((void *)CURRENT);
	CURRENT = key;
	CURRENTLEN = keylen;
	return (INTERPRET(htent, val, vallen+1));
}

#ifdef _THREAD_SAFE
static int
interpret(struct hostent *htent, char *val, int len,
	  struct hostent_data *ht_data)
#else
static struct hostent *
interpret(char *val, int len)
#endif /* _THREAD_SAFE */
{
	char line[BUFSIZ+1];
	char		*p;
	register char	*cp, **q;
	register char	*ucp, *lcp;

	/* skip any beginning white space */
	while ((*val <= ' ') && (len > 0)) {
		val++;
		len--;
	}
again:
	strncpy(line, val, len);
	p = line;
	if (*p == '#')
		return(TS_FAILURE);
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		return(TS_FAILURE);
	*cp = '\0';
	cp = strpbrk(p, " \t");
	if (cp == NULL)
		return(TS_FAILURE);
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	HOST.h_addr_list = HOST_ADDRS;
#ifdef aiws
	/* the pl8cc compacts the arrays and the ibm032 can't write a word */
	/* except on a word boundary, so the assignment is done this way */
	HOST_ADDRS[0] = (char *)&HOST_ADDR;	/* point to word aligned item */
	HOST_ADDR.s_addr = (ulong)inet_addr( p );	/* then fill it in */
	HOST_ADDRS[1] = (char *)0;
#else
	HOST.h_addr = HOSTADDR;
	*((u_long *)HOST.h_addr) = inet_addr(p);
#endif
	HOST.h_length = sizeof (u_long);
	HOST.h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	HOST.h_name = cp;
	lcp = HOST.h_name;
	ucp = HOST.h_name;
	q = HOST.h_aliases = HOST_ALIASES;
	cp = strpbrk(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	/* convert hostname from hosts to lower case for yellow page only */
	while (*ucp) {
            *lcp++ = (isupper((int)*ucp) ? tolower((int)*ucp) : *ucp);
	    ucp++;
	} 
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &HOST_ALIASES[_MAXALIASES - 1])
			*q++ = cp;
		ucp = lcp = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
		/* convert aliases from hosts to lower case for yellow page */
		while (*ucp) {
		   *lcp++ = (isupper((int)*ucp) ? tolower((int)*ucp) : *ucp);
		   ucp++;
		} 
	}
	*q = NULL;
	return (TS_FOUND(&HOST));
}

/*
 * Check to see if yellow pages are up, and store that fact in usingyellow.
 * The check is performed once at startup and thereafter if flag is set.
 */
static int
yellowup(flag)
{
	static int firsttime = 1;
	static int usingyellow;
	nl_catd catd;

	if (firsttime || flag) {
		firsttime = 0;
		if (domain[0] == 0) {
		    if(getdomainname(domain, sizeof(domain)) < 0) {
                	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			fprintf(stderr,catgets(catd,LIBCNET,NET2,
			   "getdomainname system call missing\n"));
#ifdef _THREAD_SAFE
			_Set_h_errno(SERVICE_UNAVAILABLE);
#else
			h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
			exit(1);
		     }
		}
		usingyellow = !yp_bind(domain);
	}	
	return(usingyellow);
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
