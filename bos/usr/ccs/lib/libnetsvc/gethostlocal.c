static char sccsid[] = "@(#)55	1.7  src/bos/usr/ccs/lib/libnetsvc/gethostlocal.c, libcnet, bos411, 9428A410j 6/3/94 14:10:57";
/* 
 * COMPONENT_NAME: LIBCNET gethostlocal.c
 * 
 * FUNCTIONS: _endhtent, _gethtbyaddr, _gethtbyname_s, _gethtent, 
 *            _sethtent, endhostent_s, gethostbyaddr_s, gethostbyname_s, 
 *            gethostent_s, sethostent_s
 *
 * ORIGINS: 24 26 27 71
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
 *	These routines resolve names and ip addresses
 *	via searching the local /etc/hosts file.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/time.h>			/*AIX - NOTE location of include file */
#include <nl_types.h>
#include "libc_msg.h"

#include "../libc/ts_supp.h"

#ifdef _THREAD_SAFE

#define _HTSEARCH(n, h)			(h = htent, _htsearch(n, h, ht_data))
#define _SETHTENT(flg)			_sethtent_r(flg, ht_data)
#define _ENDHTENT()			_endhtent_r(ht_data)
#define _GETHTENT(h)			_gethtent_r(h, ht_data)
#define _GETHTBYNAME(n)			_gethtbyname_r(n, htent, ht_data)
#define _GETHTBYADDR(a, l, t)		_gethtbyaddr_r(a, l, t, htent, ht_data)

#define HOST				(*htent)
#define HOST_ADDR			(ht_data->host_addr)
#define H_ADDR_PTRS			(ht_data->h_addr_ptrs)
#define HOSTADDR			(ht_data->hostaddr)
#define HOSTBUF				(ht_data->hostbuf)
#define HOST_ALIASES			(ht_data->host_aliases)
#define HOST_ADDRS			(ht_data->host_addrs)
#define HOSTF				(ht_data->hostf)
#define STAYOPEN			(ht_data->stayopen)
#define	HOST_ADDRESSES			(ht_data->host_addresses)

#else
#define	_HTSEARCH(n, h)			(h = _htsearch(n))
#define _SETHTENT(flg)			_sethtent(flg)
#define _ENDHTENT()			_endhtent()
#define _GETHTENT(h)			(h = _gethtent())
#define _GETHTBYNAME(n)			_gethtbyname_s(n)
#define _GETHTBYADDR(a, l, t)		_gethtbyaddr(a, l, t)

#define HOST				ht_host
#define HOST_ADDR			host_addr
#define	H_ADDR_PTRS			h_addr_ptrs
#define HOSTADDR			hostaddr
#define HOSTBUF				hostbuf
#define	HOST_ALIASES			host_aliases
#define HOST_ADDRS			host_addrs
#define	HOSTF				hostf
#define STAYOPEN			stayopen
#define HOST_ADDRESSES			host_addresses


static char	*h_addr_ptrs[_MAXADDRS + 1];
static ulong	host_addresses[_MAXADDRS]; /* Actual addresses */
static struct hostent ht_host;
static char	*host_aliases[_MAXALIASES];
static char	hostbuf[_HOSTBUFSIZE];
static struct in_addr host_addr;
static FILE	*hostf = NULL;
static char	line[BUFSIZ+1];
static char	hostaddr[_MAXADDRS];
static char	*host_addrs[2];
static int	stayopen = 0;

static struct hostent	*_htsearch();

#endif /* _THREAD_SAFE */

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
static void
_endhtent_r(struct hostent_data *ht_data)
#else
static void
_endhtent(void)
#endif /* _THREAD_SAFE */
{
	if (HOSTF && !STAYOPEN) {
		(void) fclose(HOSTF);
		HOSTF = NULL;
	}
}

#ifdef _THREAD_SAFE
static int
gethostbyname_s(char *name, struct hostent *htent, struct hostent_data *ht_data)
{
#else
static struct hostent *
gethostbyname_s(char *name)
{
	struct	hostent *htent;
	extern struct hostent *_gethtbyname_s();
#endif /* _THREAD_SAFE */
	register char 		*cp;

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
	return (_GETHTBYNAME(name));
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
	extern struct hostent *_gethtbyaddr();
#endif /* _THREAD_SAFE */

	/*
	 * See comment in gethostbyname - but here the old name server
	 * cannot be tried because it does not translate an address
	 * to a name.
	 */
#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	if (addr == NULL || type != AF_INET) {
		TS_ERROR(EINVAL);
		return(TS_FAILURE);
	}
	return(_GETHTBYADDR(addr, len, type));
}

#ifdef _THREAD_SAFE
static int
sethostent_s(int stayopenflag, struct hostent_data *ht_data)
#else
static int
sethostent_s(int stayopenflag)
#endif /* _THREAD_SAFE */
{
	return(_SETHTENT(stayopenflag));
}

#ifdef _THREAD_SAFE
static int
endhostent_s(struct hostent_data *ht_data)
#else
static int
endhostent_s(void)
#endif /* _THREAD_SAFE */
{
	_ENDHTENT();
}

#ifdef _THREAD_SAFE
static int 
gethostent_s(struct hostent *htent, struct hostent_data *ht_data)
{
#else
static struct hostent *
gethostent_s()
{
	register struct hostent *htent;
	extern struct hostent *_gethtent();
#endif /* _THREAD_SAFE */

	return(_GETHTENT(htent));
}

#ifdef _THREAD_SAFE
static int
_sethtent_r(int f, struct hostent_data *ht_data)
#else
static int
_sethtent(int f)
#endif /* _THREAD_SAFE */
{
	int	flags = 0;

	STAYOPEN |= f;

	if (HOSTF) {
		rewind(HOSTF);
		return(TS_SUCCESS);
	}
	if ((HOSTF = fopen(_PATH_HOSTS, "r")) != NULL) {
		flags = fcntl(fileno(HOSTF), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(HOSTF), F_SETFD, flags) == 0)
			return(TS_SUCCESS);
		(void)fclose(HOSTF);
	}
	return(TS_FAILURE);
}

#ifdef _THREAD_SAFE
static int
_gethtent_r(struct hostent *htent, struct hostent_data *ht_data)
#else
static struct hostent *
_gethtent(void)
#endif /* _THREAD_SAFE */
{
	char *p;
	register char *cp, **q;

	if (HOSTF == NULL && _SETHTENT(0) != TS_SUCCESS) {
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
		return (TS_FAILURE);
	}
again:
	if ((p = fgets(HOSTBUF, BUFSIZ, HOSTF)) == NULL) {
		return (TS_FAILURE);
	}

	/* skip any beginning white space */
	while (*p <= ' ' && *p != '\n')
		p++;
	
	if (*p == '#')
		goto again;
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	cp = strpbrk(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
	HOST.h_addr_list = HOST_ADDRS;
#endif
#ifdef aiws
	/* the p18cc compacts the arrays and the ibm032 can't write a word */
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
	q = HOST.h_aliases = HOST_ALIASES;
	cp = strpbrk(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &HOST_ALIASES[_MAXALIASES - 1])
			*q++ = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (TS_FOUND(&HOST));
}

#ifdef _THREAD_SAFE
static int
_gethtbyname_r(const char *name, struct hostent *htent,
		struct hostent_data *ht_data)
#else
static struct hostent *
_gethtbyname_s(const char *name)
#endif /* _THREAD_SAFE */   
{
	register struct hostent *ht;

	if (name == NULL || *name == '\0') {
		TS_ERROR(EINVAL);
		return(TS_FAILURE);
	}

	_SETHTENT(0);

	/* Look in /etc/hosts file */
	if ((_HTSEARCH(name, ht) == TS_FAILURE) && (h_errno == 0))
{
#ifdef _THREAD_SAFE
		_Set_h_errno(HOST_NOT_FOUND);
#else
		h_errno = HOST_NOT_FOUND;
#endif /* _THREAD_SAFE */
		_ENDHTENT();
		return(TS_FAILURE);
	}
	_ENDHTENT();
	return(TS_FOUND(ht));
}

#ifdef _THREAD_SAFE
static int
_gethtbyaddr_r(const char *addr, int len, int type, struct hostent *htent,
		struct hostent_data *ht_data)
{
#else
static struct hostent *
_gethtbyaddr(char *addr, int len, int type)
{
	register struct hostent *htent;
#endif /* _THREAD_SAFE */

	if (addr == NULL) {
		TS_ERROR(EINVAL);
		return(TS_FAILURE);
	}

	if (_SETHTENT(0) == TS_SUCCESS) {
		while (_GETHTENT(htent) != TS_FAILURE)
			if (htent->h_addrtype == type &&
			    !memcmp(htent->h_addr, addr, len)) {
				_ENDHTENT();
				return(TS_FOUND(htent));
			}
		_ENDHTENT();
	}
#ifdef _THREAD_SAFE
	_Set_h_errno(HOST_NOT_FOUND);
#else
	h_errno = HOST_NOT_FOUND;
#endif /* _THREAD_SAFE */
	return(TS_FAILURE);
}

/*
 * Search the /etc/hosts table for all entries matching "name".
 * "name" may either be a hostname or an alias.
 *
 * Return a pointer to the hostent structure on success,
 *	NULL on failure.
 */
#ifdef _THREAD_SAFE
static int
_htsearch(char *name, struct hostent *htent, struct hostent_data *ht_data)
#else
static struct hostent *
_htsearch(char *name)
#endif /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	char line[BUFSIZ+1];
#endif /* _THREAD_SAFE */
	struct hostent *ht = NULL;
	struct {
		char *ipp; /* IP addr pointer. */
		char *namep; /* hostname ptr. */
		char *aliases[_MAXALIASES+1];
	} ptrs;
	char **ptrsp, **np, **ap, *cp;
	char **limit = &ptrs.aliases[_MAXALIASES];
	char *bufp = HOSTBUF, *bp;
	int state, nextstate, i, aid, hid;
#define WHITECHECK(c) (((c)==' ') || ((c)=='\t'))
/* States for line parsing */
#define LOOKING 0
#define INIPADDR 0x01
#define INNAMES  0x02
#define INFIELD  (state & (INIPADDR | INNAMES))

#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	/* Open hosts file if not already open. */
	if (HOSTF == NULL && _SETHTENT(0) != TS_SUCCESS) {
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
		return(TS_FAILURE);
	}

	/* for each entry in hosts file: */
	while (fgets(line, BUFSIZ, HOSTF) != NULL) {
		ptrsp = &ptrs;
		line[BUFSIZ-1] = '\0'; /* Guarantee we'll quit. */

		/* For each character. */
		for (cp=line,bp=bufp,state=LOOKING,nextstate=INIPADDR; ; cp++) {
			/* Quit on '#', '\n', or '\0'.
			 * Terminate the line though.
			 */
			if (!*cp || (*cp=='#') || (*cp=='\n')) {
				*bp++ = '\0';
				break;
			}

			if (WHITECHECK(*cp)) {
			    /* when looking, bypass white space. */
			    if (state==LOOKING) continue;
			    else {
				/* white space in field, end it. */
				if (state==INIPADDR) *cp = '\0';
				else *bp++ = '\0';
				if (ptrsp==limit) break;
				state=LOOKING;
			    }
			}
			else { /* Not white space. */
			    if (state==LOOKING) {
				/* Start field if looking. */
				state=nextstate;
				nextstate=INNAMES;
				*ptrsp++ = (state==INNAMES)? bp: cp;
			    }

			    /* copy to hostbuf */
			    if (state==INNAMES) {
				*bp++ = *cp;
				/* Quit if out of room. */
				if (bp == &HOSTBUF[BUFSIZ]) {
					ptrsp--; /* Ignore last field. */
					break;
				}
			    }
			}
		}

		/* Skip record if we didn't get an address and name. */
		/* ptrsp will point just past the last pointer that was set. */
		if ((ulong)ptrsp < (ulong)&ptrs.aliases[0]) continue;

		*ptrsp = NULL;

		/* if name is matched by hostname or an alias, */
		for (np=&ptrs.namep; *np; np++) {
		    if (strcasecmp(name,*np)==0) {
			/* Setup struct on 1st match. */
			if (!ht) {
				ht = &HOST;
				HOST.h_addr_list = H_ADDR_PTRS;
				hid = 0;
				/* Use hostbuf to hold text. */
				HOST.h_name = HOSTBUF;

				/* Setup aliases. */
				HOST.h_aliases = NULL;
				aid = 0;

				/* Setup type and address length. */
				HOST.h_length = sizeof (u_long);
				HOST.h_addrtype = AF_INET;
			}

			/* Add address. */
			/* Ignore duplicate IP there are address entries. */
			HOST_ADDRESSES[hid] = inet_addr(ptrs.ipp);
			for (i=0; i<hid; i++) {
				if (HOST_ADDRESSES[i] == HOST_ADDRESSES[hid])
					goto next_record;
			}
			H_ADDR_PTRS[hid] = &HOST_ADDRESSES[hid];
			H_ADDR_PTRS[++hid] = NULL;

			/* Add any aliases. */
			for (ap=&ptrs.aliases[0]; ap!=ptrsp && aid<_MAXALIASES; ap++) {
				if (HOST.h_aliases==NULL) {
					HOST.h_aliases = &HOST_ALIASES[0];
				}

				/* Skip if it's a dup. */
				for (i=0; i<aid; i++) {
					if (strcmp(HOST.h_aliases[i],*ap)==0)
						goto next_alias;
				}

				HOST_ALIASES[aid++] = *ap;
				HOST_ALIASES[aid] = NULL;
next_alias:;
			}
			/* If we can't handle more addresses, quit now. */
			if (hid == _MAXADDRS) goto quit;

			/* move bufp to start next batch of data. */
			bufp = bp;

			break;
		    }
		}
next_record:;
	}
quit:;

	return((ht == NULL) ? TS_FAILURE : TS_FOUND(ht));
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
