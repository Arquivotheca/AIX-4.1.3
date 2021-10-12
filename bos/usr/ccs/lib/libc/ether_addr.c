#ifndef lint
static char sccsid[] = "@(#)49  1.8  src/bos/usr/ccs/lib/libc/ether_addr.c, libcnet, bos411, 9428A410j 11/22/93 09:18:22";
#endif
/* 
 * COMPONENT_NAME: LIBCNET ether_addr.c
 * 
 * FUNCTIONS: EI, ether_aton, ether_hostton, ether_line, ether_ntoa, 
 *            ether_ntohost, useyp 
 *
 * ORIGINS: 24 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 *
 * All routines necessary to deal with the file /etc/ethers.  The file
 * contains mappings from 48 bit ethernet addresses to their corresponding
 * hosts name.  The addresses have an ascii representation of the form
 * "x:x:x:x:x:x" where x is a hex number between 0x00 and 0xff;  the
 * bytes are always in network order.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ypresolv_rmutex;
#endif /* _THREAD_SAFE */

#ifndef _THREAD_SAFE
static struct ether_addr ea;
#endif /* _THREAD_SAFE */

static int useyp();  /* tells whether we use yp or a local file */
static char domain[256]; /* yp domain name */
static char *filename = "/etc/ethers";

/*
 * Parses a line from /etc/ethers into its components.  The line has the form
 * 8:0:20:1:17:c8	krypton
 * where the first part is a 48 bit ethernet address and the second is
 * the corresponding hosts name.
 * Returns zero if successful, non-zero otherwise.
 */
ether_line(s, e, hostname)
	char *s;		/* the string to be parsed */
	struct ether_addr *e;	/* ethernet address struct to be filled in */
	char *hostname;		/* hosts name to be set */
{
	register int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x %s",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], hostname);
	if (i != 7) {
		return (7 - i);
	}
	for (i = 0; i < 6; i++)
		e->ether_addr_octet[i] = t[i];
	return (0);
}

/*
 * Converts a 48 bit ethernet number to its string representation.
 */
#define EI(i)	(unsigned int)(e->ether_addr_octet[(i)])
#ifdef _THREAD_SAFE
int
ether_ntoa_r(struct ether_addr *e, char *s)
{
#else
char *
ether_ntoa(struct ether_addr *e)
{
	static char s[18];
#endif /* _THREAD_SAFE */
	
	TS_EINVAL((s == 0));
	s[0] = 0;
	sprintf(s, "%x:%x:%x:%x:%x:%x",
	    EI(0), EI(1), EI(2), EI(3), EI(4), EI(5));
	return (TS_FOUND(s));
}

/*
 * Converts a ethernet address representation back into its 48 bits.
 */
#ifdef _THREAD_SAFE
int
ether_aton_r(char *s, struct ether_addr *e)
{
#else
struct ether_addr *
ether_aton(char *s)
{
	static struct ether_addr *e = &ea;
#endif /* _THREAD_SAFE */
	register int i;
	unsigned int t[6];
	
	TS_EINVAL((e == 0));
	i = sscanf(s, " %x:%x:%x:%x:%x:%x",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
	if (i != 6)
	    return (TS_FAILURE);
	for (i = 0; i < 6; i++) {
		e->ether_addr_octet[i] = t[i];
	}
	return(TS_FOUND(e));
}

/*
 * Given a host's name, this routine returns its 48 bit ethernet address.
 * Returns zero if successful, non-zero otherwise.
 */
ether_hostton(host, e)
	char *host;		/* function input */
	struct ether_addr *e;	/* function output */
{
	char currenthost[256];
	char buf[512];
	char *val = buf;
	int vallen;
	register int reason;
	FILE *f;
	
	TS_LOCK(&_ypresolv_rmutex);
	if (useyp()) {
		if (reason = yp_match(domain, "ethers.byname", host,
		    strlen(host), &val, &vallen)) {
			TS_UNLOCK(&_ypresolv_rmutex);
			return (reason);
		} else {
			TS_UNLOCK(&_ypresolv_rmutex);
			return (ether_line(val, e, currenthost));
		}
	}  
	TS_UNLOCK(&_ypresolv_rmutex);
	if ((f = fopen(filename, "r")) == NULL) {
		return (-1);
	}
	reason = -1;
	while (fscanf(f, "%[^\n] ", val) == 1) {
		if ((ether_line(val, e, currenthost) == 0) &&
		    (strcmp(currenthost, host) == 0)) {
			reason = 0;
			break;
		}
	}
	fclose(f);
	return (reason);
}

/*
 * Given a 48 bit ethernet address, this routine return its host name.
 * Returns zero if successful, non-zero otherwise.
 */
ether_ntohost(host, e)
	char *host;		/* function output */
	struct ether_addr *e;	/* function input */
{
	struct ether_addr currente;
	char buf[512];
	char *val = buf;
	int vallen;
	register int reason;
	FILE *f;
	char *a;

#ifdef _THREAD_SAFE
	ether_ntoa_r(e, a);
#else
	a = ether_ntoa(e);
#endif /* _THREAD_SAFE */

	TS_LOCK(&_ypresolv_rmutex);
	if (useyp()) {
		if (reason = yp_match(domain, "ethers.byaddr", a,
		    strlen(a), &val, &vallen)) {
			TS_UNLOCK(&_ypresolv_rmutex);
			return (reason);
		} else {
			TS_UNLOCK(&_ypresolv_rmutex);
			return (ether_line(val, &currente, host));
		}
	} 
	TS_UNLOCK(&_ypresolv_rmutex);
	if ((f = fopen(filename, "r")) == NULL) {
		return (-1);
	}
	reason = -1;
	while (fscanf(f, "%[^\n] ", val) == 1) {
		if ((ether_line(val, &currente, host) == 0) &&
		    (bcmp(e, &currente, sizeof(currente)) == 0)) {
			reason = 0;
			break;
		}
	}
	fclose(f);
	return (reason);
}

/*
 * Determines whether or not to use the yellow pages service to do lookups.
 *
 * Note: Calls to this program should be locked in a 
 *       thread-safe environment since static data is utilized.
 *	 getdomainname() is also not thread-safe'd.
 */
static int initted;
static int usingyp;
static int
useyp()
{
	if (!initted) {
		getdomainname(domain, sizeof(domain));
		usingyp = !yp_bind(domain);
		initted = 1;
	}
	return (usingyp);
}
