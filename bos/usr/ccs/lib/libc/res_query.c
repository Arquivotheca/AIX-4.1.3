static char sccsid[] = "@(#)63	1.15  src/bos/usr/ccs/lib/libc/res_query.c, libcnet, bos411, 9428A410j 6/3/94 14:02:53";
/* 
 * COMPONENT_NAME: LIBCNET res_query.c
 * 
 * FUNCTIONS: hostalias, res_query, res_querydomain, res_search 
 *
 * ORIGINS: 26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "ts_supp.h"
#include "push_pop.h"

#if PACKETSZ > 1024
#define MAXPACKET	PACKETSZ
#else
#define MAXPACKET	1024
#endif

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _resolv_rmutex;

#define	RETURN(val)		return(TS_UNLOCK(&_resolv_rmutex), val)
#define POP_N_LEAVE(val)	{ rc = (val); goto pop_n_leave; }
#else
#define RETURN(val)		return(val)
#define POP_N_LEAVE(val)	return(val)
#endif /* _THREAD_SAFE */

#ifdef DEBUG
#define PRINT_DBG(info) \
	if (_res.options & RES_DEBUG) \
		printf info
#else
#define	PRINT_DBG(info)
#endif DEBUG

/*
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_errno.
 * Caller must parse answer and determine whether it answers the question.
 */
int
res_query(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer buffer */
{
	char buf[MAXPACKET];
	HEADER *hp;
	int n, rc;

	TS_LOCK(&_resolv_rmutex);
	TS_PUSH_CLNUP(&_resolv_rmutex);

	if ((_res.options & RES_INIT) == 0 && res_init() == -1) {
		POP_N_LEAVE(-1);
		}

	PRINT_DBG(("res_query(%s, %d, %d)\n", name, class, type));

	n = res_mkquery(QUERY, name, class, type, (char *)NULL, 0, NULL,
	    buf, sizeof(buf));

	if (n <= 0) {

		PRINT_DBG(("res_query: mkquery failed\n"));
#ifdef _THREAD_SAFE
		_Set_h_errno(NO_RECOVERY);
#else
		h_errno = NO_RECOVERY;
#endif /* _THREAD_SAFE */
		POP_N_LEAVE(n);
	}
	n = res_send(buf, n, answer, anslen);
	if (n < 0) {
		PRINT_DBG(("res_query: send error\n"));
#ifdef _THREAD_SAFE
		_Set_h_errno(TRY_AGAIN);
#else
		h_errno = TRY_AGAIN;
#endif /* _THREAD_SAFE */
		POP_N_LEAVE(n);
	}

	hp = (HEADER *) answer;
	if (hp->rcode != NOERROR || ntohs(hp->ancount) == 0) {
		PRINT_DBG(("rcode = %d, ancount=%d\n", hp->rcode, 
				ntohs(hp->ancount)));
		switch (hp->rcode) {
			case NXDOMAIN:
#ifdef _THREAD_SAFE
				_Set_h_errno(HOST_NOT_FOUND);
#else
				h_errno = HOST_NOT_FOUND;
#endif /* _THREAD_SAFE */
				break;
			case SERVFAIL:
#ifdef _THREAD_SAFE
				_Set_h_errno(TRY_AGAIN);
#else
				h_errno = TRY_AGAIN;
#endif /* _THREAD_SAFE */
				break;
			case NOERROR:
#ifdef _THREAD_SAFE
				_Set_h_errno(NO_DATA);
#else
				h_errno = NO_DATA;
#endif /* _THREAD_SAFE */
				break;
			case FORMERR:
			case NOTIMP:
			case REFUSED:
			default:
#ifdef _THREAD_SAFE
				_Set_h_errno(NO_RECOVERY);
#else
				h_errno = NO_RECOVERY;
#endif /* _THREAD_SAFE */
				break;
		}
		POP_N_LEAVE(-1);
	}
	rc = n;
pop_n_leave:
	TS_POP_CLNUP(0);
	RETURN(rc);		/* rc is set by POP_N_LEAVE(x) to the value x */
}

/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error number is left in h_errno.
 * Only useful for queries in the same name hierarchy as the local host
 * (not, for example, for host address-to-name lookups in domain in-addr.arpa).
 */
int
res_search(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	register char *cp, **domain, *after_first_dot;
	int n, ret, h_errno_sav;
	int rc;
#ifdef _THREAD_SAFE
	char	buf[MAXDNAME];
#endif /* _THREAD_SAFE */

	TS_LOCK(&_resolv_rmutex);
	TS_PUSH_CLNUP(&_resolv_rmutex);

	if ((_res.options & RES_INIT) == 0 && res_init() == -1)
		POP_N_LEAVE(-1);

	errno = 0;
#ifdef _THREAD_SAFE
	_Set_h_errno(HOST_NOT_FOUND);
#else
	h_errno = HOST_NOT_FOUND;		/* default, if we never query */
#endif /* _THREAD_SAFE */
	for (cp = name, n = 0; *cp; cp++)
		if (*cp == '.') {
			n++;
			if (n == 1)
				after_first_dot = cp + 1;
		}
#ifdef _THREAD_SAFE
	if (n == 0 && (cp = (hostalias_r(name, buf, sizeof(buf)) ? 0 : buf)))
#else
	if (n == 0 && (cp = hostalias(name)))
#endif /* _THREAD_SAFE */
		POP_N_LEAVE(res_query(cp, class, type, answer, anslen));

	/*
	 * We do at least one level of search if
	 *	- there is no dot and RES_DEFNAME is set, or
	 *	- there is at least one dot, there is no trailing dot,
	 *	  RES_DNSRCH is set, and the domain part of name is not the
	 *  	  same as the default domain (defdname).
	 * 	  This last check is to avoid tacking on the default domain
	 *	  on a fully qualified hostname.  And avoid searching for
	 *	  hostname hostname.defdname.defdname which is redundant.
	 */
	if ((n == 0 && _res.options & RES_DEFNAMES) ||
	   (n != 0 && *--cp != '.' && _res.options & RES_DNSRCH &&
	    strcasecmp(after_first_dot, _res.defdname) != 0))
	     for (domain = _res.dnsrch; *domain; domain++) {
		ret = res_querydomain(name, *domain, class, type,
		    answer, anslen);
		if (ret > 0)
			POP_N_LEAVE(ret);
		/*
		 * If no server present, give up.
		 * If name isn't found in this domain,
		 * keep trying higher domains in the search list
		 * (if that's enabled).
		 * On a NO_DATA error, keep trying, otherwise
		 * a wildcard entry of another type could keep us
		 * from finding this entry higher in the domain.
		 * If we get some other error (negative answer or
		 * server failure), then stop searching up,
		 * but try the input name below in case it's fully-qualified.
		 */
		if (errno == ECONNREFUSED) {
#ifdef _THREAD_SAFE
			_Set_h_errno(TRY_AGAIN);
#else
			h_errno = TRY_AGAIN;
#endif /* _THREAD_SAFE */
			POP_N_LEAVE(-1);
		}
		if ((h_errno != HOST_NOT_FOUND && 
		     h_errno != NO_DATA) ||
		    (_res.options & RES_DNSRCH) == 0)
			break;
	}
	/*
	 * If the search/default failed, try the name as fully-qualified,
	 * but only if it contained at least one dot (even trailing).
	 * This is purely a heuristic; we assume that any reasonable query
	 * about a top-level domain (for servers, SOA, etc) will not use
	 * res_search.
	 */
	/* Need to save h_errno from previous query.  If new query fails,
	   the first h_errno should be returned unless the error is 
	   HOST_NOT_FOUND. 
	 */
	h_errno_sav = h_errno;

	if (n && (ret = res_querydomain(name, (char *)NULL, class, type,
	    answer, anslen)) > 0)
		POP_N_LEAVE(ret);

	if (h_errno_sav != HOST_NOT_FOUND)
#ifdef _THREAD_SAFE
		_Set_h_errno(h_errno_sav);
#else
		h_errno = h_errno_sav;
#endif /* _THREAD_SAFE */

	rc = -1;

pop_n_leave:
	TS_POP_CLNUP(0);
	RETURN(rc);		/* rc is set by POP_N_LEAVE(x) to the value x */
}

/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int
res_querydomain(name, domain, class, type, answer, anslen)
	char *name, *domain;
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	char nbuf[2*MAXDNAME+2];
	char *longname = nbuf;
	int n;

	PRINT_DBG(("res_querydomain(%s, %s, %d, %d)\n",
			name, domain, class, type));

	if (domain == NULL) {
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = strlen(name) - 1;
		if (name[n] == '.' && n < sizeof(nbuf) - 1) {
			bcopy(name, nbuf, n);
			nbuf[n] = '\0';
		} else
			longname = name;
	} else
		(void)sprintf(nbuf, "%.*s.%.*s",
		    MAXDNAME, name, MAXDNAME, domain);

	return (res_query(longname, class, type, answer, anslen));
}

#ifdef _THREAD_SAFE
#define LEN	len
int
hostalias_r(register char *name, char *abuf, int len)
#else
#define LEN	(sizeof(abuf))
char *
hostalias(register char *name)
#endif /* _THREAD_SAFE */
{
	register char *C1, *C2;
	FILE *fp;
	char *file;
	char buf[BUFSIZ];
#ifndef _THREAD_SAFE
	static char abuf[MAXDNAME];
#endif /* _THREAD_SAFE */

	TS_EINVAL((abuf == 0 || len < MAXDNAME));
	file = getenv("HOSTALIASES");
	if (file == NULL || (fp = fopen(file, "r")) == NULL)
		return (TS_FAILURE);
	buf[sizeof(buf) - 1] = '\0';
	while (fgets(buf, (int)sizeof(buf), fp)) {
		for (C1 = buf; *C1 && !isspace((int)*C1); ++C1);
		if (!*C1)
			break;
		*C1 = '\0';
		if (!strcasecmp(buf, name)) {
			while (isspace((int)*++C1));
			if (!*C1)
				break;
			for (C2 = C1 + 1; *C2 && !isspace((int)*C2); ++C2);
			abuf[LEN - 1] = *C2 = '\0';
			(void)strncpy(abuf, C1, LEN - 1);
			fclose(fp);
			return (TS_FOUND(abuf));
		}
	}
	fclose(fp);
	return (TS_FAILURE);
}
