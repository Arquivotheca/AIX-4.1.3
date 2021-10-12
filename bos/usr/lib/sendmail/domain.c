static char sccsid[] = "@(#)95	1.8  src/bos/usr/lib/sendmail/domain.c, cmdsend, bos411, 9428A410j 9/13/91 12:52:23";
/* 
 * COMPONENT_NAME: CMDSEND domain.c
 * 
 * FUNCTIONS: getcanonname, getmxrr, getrr
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
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
 *
 * @(#)domain.c	5.19 (Berkeley) 1/1/89
 *
 */


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "conf.h"
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include "sysexits.h"
#include "sendmail.h"

extern char *malloc();

typedef union {
	HEADER qb1;
	char qb2[PACKETSZ];
} querybuf;

static char hostbuf[MAXMXHOSTS*PACKETSZ];

/* this does the work of doing a nameserver query and interpreting the
   return codes */

static int search_rr(host, type, rcode, answer)
char *host;
int type;
int *rcode;
querybuf *answer;

{
	int n;

	errno = 0;

	/* check nameserver for specified record type for host */
	n = res_search(host, C_IN, type, (char *)answer, sizeof(*answer));
#ifdef DEBUG
	if (tTd(8, 3))
		printf(
		"search_rr: res_search returned %d on %d query for host='%s'\n",
		    n, type, host);
#endif DEBUG

	if (n < 0) {  /* no data or error */
#ifdef DEBUG
		if (tTd(8, 1))
			printf(
			"search_rr: res_search failed (errno=%d, h_errno=%d)\n",
			    errno, h_errno);
#endif DEBUG
		/* if we couldn't reach the nameserver, and we're to treat
		   this as a temporary failure, queue it up for later;
		   for other errors, just indicate no data was retrieved */

		if (h_errno == TRY_AGAIN && UseNameServer &&
		    (errno == ECONNREFUSED || errno == ETIMEDOUT)) {
			*rcode = EX_TEMPFAIL;
			return(-1);
		} else
			return(0);
	}
	return(n);  /* return length of answer */
}


getmxrr(host, mxhosts, localhost, rcode)
	char *host, **mxhosts, *localhost;
	int *rcode;
{
	extern int h_errno;
	register u_char *eom, *cp;
	register int i, j, n, nmx;
	register char *bp;
	HEADER *hp;
	querybuf answer;
	int ancount, qdcount, buflen, seenlocal;
	u_short pref, localpref, type, prefer[MAXMXHOSTS];


	/* query the nameserver for MX records for this host */
	if ((n = search_rr(host, T_MX, rcode, &answer)) < 0)
		return(-1);
	if (! n)  /* no data available: try hostname as given */
		goto punt;

	/* find first satisfactory answer */
	hp = (HEADER *)&answer;
	cp = (u_char *)&answer + sizeof(HEADER);
	eom = (u_char *)&answer + n;
	for (qdcount = ntohs(hp->qdcount); qdcount--; cp += n + QFIXEDSZ)
		if ((n = dn_skipname(cp, eom)) < 0)
			goto punt;
	nmx = 0;
	seenlocal = 0;
	buflen = sizeof(hostbuf);
	bp = hostbuf;
	ancount = ntohs(hp->ancount);
#ifdef DEBUG
	if (tTd(8, 3))
		printf("getmxrr: ancount=%d\n", ancount);
#endif DEBUG
	while (--ancount >= 0 && cp < eom && nmx < MAXMXHOSTS) {
		if ((n = dn_expand((char *)&answer, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
		GETSHORT(type, cp);
 		cp += sizeof(u_short) + sizeof(u_long);
		GETSHORT(n, cp);
		if (type != T_MX)  {
#ifdef DEBUG
			if (tTd(8, 1) || _res.options & RES_DEBUG)
				printf("unexpected answer type %d, size %d\n",
				    type, n);
#endif DEBUG
			cp += n;
			continue;
		}
		GETSHORT(pref, cp);
		if ((n = dn_expand((char *)&answer, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
#ifdef DEBUG
		if (tTd(8, 3))
			printf("getmxrr: mxhost='%s', pref=%d\n", bp, pref);
#endif DEBUG
		if (!strcasecmp(bp, localhost)) {
			if (seenlocal == 0 || pref < localpref)
				localpref = pref;
			seenlocal = 1;
			continue;
		}
		prefer[nmx] = pref;
		mxhosts[nmx++] = bp;
		n = strlen(bp) + 1;
		bp += n;
		buflen -= n;
	}
	if (nmx == 0) {
punt:		mxhosts[0] =(char *) strcpy(hostbuf,host);
#ifdef DEBUG
		if (tTd(8, 3))
			printf("getmxrr: punt: returning host='%s'\n", hostbuf);
#endif DEBUG
		return(1);
	}

	/* sort the records */
	for (i = 0; i < nmx; i++) {
		for (j = i + 1; j < nmx; j++) {
			if (prefer[i] > prefer[j] ||
			    (prefer[i] == prefer[j] && ((rand()/3&1) == 0))) {
				register int temp;
				register char *temp1;

				temp = prefer[i];
				prefer[i] = prefer[j];
				prefer[j] = temp;
				temp1 = mxhosts[i];
				mxhosts[i] = mxhosts[j];
				mxhosts[j] = temp1;
			}
		}
		if (seenlocal && prefer[i] >= localpref) {
			/*
			 * truncate higher pref part of list; if we're
			 * the best choice left, we should have realized
			 * awhile ago that this was a local delivery.
			 */
			if (i == 0)
				goto punt;
			nmx = i;
			break;
		}
	}
	return(nmx);
}

getcanonname(host, hbsize)
	char *host;
	int hbsize;
{
	register u_char *eom, *cp;
	register int n; 
	HEADER *hp;
	querybuf answer;
	u_short type;
	int first, ancount, qdcount, loopcnt;
	char nbuf[PACKETSZ];

	/*
	** Do standard gethostbyname() call if we aren't configured
	** to use the nameserver
	*/

	for (n = BITMAPINTS - 1; n >= 0; --n)  /* check all ints in bitmap */
		if (NameServOpt[n])  /* to see if any bits are set */
			break;
	if (n < 0) {  /* no bits set */
		struct hostent *hp;

		hp = gethostbyname(host);
		if (hp != NULL && (strlen(hp->h_name) < hbsize))
			(void) strcpy(host, hp->h_name);
		return;
	}

	/*
	** Do nameserver query
	*/

	loopcnt = 0;
loop:
	/*
	 * Use query type of ANY if possible (NS_ANY bit is set), which will
	 * find types CNAME, A, and MX, and will cause all existing records
	 * to be cached by our local server.  If there is (might be) a
	 * wildcard MX record in the local domain or its parents that are
	 * searched, we can't use ANY; it would cause fully-qualified names
	 * to match as names in a local domain.
	 */
	n = res_search(host, C_IN,
		(bitnset(NS_ANY, NameServOpt) ? T_ANY : T_CNAME),
		(char *)&answer, sizeof(answer));
#ifdef DEBUG
	if (tTd(8, 3))
		printf("getcanonname:  res_search returned %d on host='%s'\n",
			n, host);
#endif DEBUG
	if (n < 0) {
#ifdef DEBUG
		if (tTd(8, 1))
			printf("getcanonname:  res_search failed (errno=%d, h_errno=%d)\n",
			    errno, h_errno);
#endif DEBUG
		return;
	}

	/* find first satisfactory answer */
	hp = (HEADER *)&answer;
	ancount = ntohs(hp->ancount);
#ifdef DEBUG
	if (tTd(8, 3))
		printf("getcanonname:  ancount=%d\n", ancount);
#endif DEBUG

	/* we don't care about errors here, only if we got an answer */
	if (ancount == 0) {
#ifdef DEBUG
		if (tTd(8, 1))
			printf("rcode = %d, ancount=%d\n", hp->rcode, ancount);
#endif DEBUG
		return;
	}
	cp = (u_char *)&answer + sizeof(HEADER);
	eom = (u_char *)&answer + n;
	for (qdcount = ntohs(hp->qdcount); qdcount--; cp += n + QFIXEDSZ)
		if ((n = dn_skipname(cp, eom)) < 0)
			return;

	/*
	 * just in case someone puts a CNAME record after another record,
	 * check all records for CNAME; otherwise, just take the first
	 * name found.
	 */
	for (first = 1; --ancount >= 0 && cp < eom; cp += n) {
		if ((n = dn_expand((char *)&answer, eom, cp, nbuf,
		    sizeof(nbuf))) < 0)
			break;
		if (first) {			/* XXX */
			(void)strncpy(host, nbuf, hbsize);
			host[hbsize - 1] = '\0';
#ifdef DEBUG
			if (tTd(8, 3))
				printf("getcanonname:  first record='%s'\n",
					host);
#endif DEBUG
			first = 0;
		}
		cp += n;
		GETSHORT(type, cp);
#ifdef DEBUG
		if (tTd(8, 3))
			printf("getcanonname:  type=%d\n", type);
#endif DEBUG
 		cp += sizeof(u_short) + sizeof(u_long);
		GETSHORT(n, cp);
		if (type == T_CNAME)  {
			/*
			 * assume that only one cname will be found.  More
			 * than one is undefined.  Copy so that if dn_expand
			 * fails, `host' is still okay.
			 */
			if ((n = dn_expand((char *)&answer, eom, cp, nbuf,
			    sizeof(nbuf))) < 0)
				break;
			(void)strncpy(host, nbuf, hbsize); /* XXX */
			host[hbsize - 1] = '\0';
#ifdef DEBUG
			if (tTd(8, 3))
				printf("getcanonname:  found CNAME '%s'\n",
					host);
#endif DEBUG
			if (++loopcnt > 8)	/* never be more than 1 */
				return;
			goto loop;
		}
	}
}

/*
 *  getrr()
 *  get resource record(s) of the given type for the host.  getrr allocates
 *  space for an array of ptrs to strings that hold the response(s), and
 *  returns a ptr to this array in rlist; the array is terminated by a NULL
 *  ptr.  rcode is returned non-zero iff we return -1, and is a standard
 *  sysexits.h code.  we return the number of responses read.
 *  NOTE that we only handle records that have one domain name member in the
 *  rdata field, e.g. MB, MG, etc. as opposed to MX, MINFO, A, etc.!!!
 */
int getrr(type, host, rlist, rcode)
int type;
char *host;
char ***rlist;
int *rcode;
{
	extern int h_errno;
	register u_char *eom, *cp, **cpp;
	register int i, j, n;
	register char *bp, *cp2;
	HEADER *hp;
	querybuf answer;
	int ancount, qdcount, count;
	char buf[PACKETSZ];


	/* query the nameserver for records of type for this host */
	if ((n = search_rr(host, type, rcode, &answer)) < 0)
		return(-1);
	if (! n)  /* no data available: try hostname as given */
		goto punt;

	/* find first satisfactory answer */
	hp = (HEADER *)&answer;
	cp = (u_char *)&answer + sizeof(HEADER);
	eom = (u_char *)&answer + n;
	/* skip over query portion */
	for (qdcount = ntohs(hp->qdcount); qdcount--; cp += n + QFIXEDSZ)
		if ((n = dn_skipname(cp, eom)) < 0)
			goto punt;
	bp = buf;
	ancount = count = ntohs(hp->ancount);
	/* malloc space for response ptrs and terminating NULL */
	if (! (*rlist = cpp = (char **) malloc((ancount + 1) * sizeof(char *))))
	    return(-1);
#ifdef DEBUG
	if (tTd(8, 3))
		printf("getrr: ancount=%d, malloc()ed %d\n",
		    ancount, (ancount +1) * sizeof(char *));
#endif DEBUG
	while (--ancount >= 0 && cp < eom) {
		if ((n = dn_expand((char *)&answer, eom, cp, bp, sizeof(buf)))
		    < 0)
			break;
		cp += n;  /* skip the domain name */
		GETSHORT(i, cp);  /* get response type */
 		cp += sizeof(u_short) + sizeof(u_long);  /* skip class & ttl */
		GETSHORT(n, cp);  /* get rdata length */
		if (i != type)  {
#ifdef DEBUG
			if (tTd(8, 1) || _res.options & RES_DEBUG)
				printf(
				"getrr: unexpected answer type %d, size %d\n",
				i, n);
#endif DEBUG
			--count;
			cp += n;
			continue;
		}
		/* expand the rdata field */
		if ((n = dn_expand((char *)&answer, eom, cp, bp, sizeof(buf)))
		    < 0)
			break;
		cp += n;
#ifdef DEBUG
		if (tTd(8, 3))
			printf("getrr: reply='%s'\n", bp);
#endif DEBUG
		/* malloc and copy the rdata, and put in ptr list */
		if (! (*cpp = malloc(strlen(bp) + 1)))
		    return(-1);
		strcpy(*cpp++, bp);
	}
	*cpp = NULL;  /* terminate list */
	return(count);
punt:
	/* just return a list consisting of the host name */
	if (! ((*rlist = cpp = (char **) malloc(2 * sizeof(char *))) &&
	    (*cpp = malloc(strlen(host) + 1))))
		return(-1);
	strcpy(*cpp++, host);
	*cpp = NULL;
	return(0);
}
