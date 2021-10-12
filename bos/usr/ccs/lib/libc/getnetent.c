static char sccsid[] = "@(#)51        1.15  src/bos/usr/ccs/lib/libc/getnetent.c, libcnet, bos411, 9428A410j 4/20/94 18:44:48";
/* 
 * COMPONENT_NAME: LIBCNET getnetent.c
 * 
 * FUNCTIONS: any, endnetent, getnetbyaddr, getnetbyname, getnetent, 
 *            interpret, nettoa, setnetent, yellowup 
 *
 * ORIGINS: 24  26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <rpcsvc/ypclnt.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <nl_types.h>
#include "libc_msg.h"

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ypresolv_rmutex;
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE

#define NET_FP		(net_data->net_fp)

#define SETNETENT(f)	setnetent_r(f, net_data)
#define INTERPRET(n,v)	interpret(n, v, net_data->line, net_data->net_aliases)
#define ENDNETENT()	endnetent_r(net_data)

#define STAYOPEN	(net_data->_net_stayopen)
#define CURRENT		(net_data->current)
#define CURRENTLEN	(net_data->currentlen)
#define	LINE		(net_data->line)

#else

#define NET_FP		net_fp

#define	SETNETENT(f)	setnetent(f)
#define INTERPRET(n,v)	interpret(n, v, line, net_aliases)
#define ENDNETENT()	endnetent()

#define STAYOPEN	_net_stayopen
#define CURRENT		current
#define CURRENTLEN	currentlen
#define	LINE		line

static struct netent	net_networks;
static FILE		*net_fp;
static char		line[BUFSIZ+1];
static char		*net_aliases[_MAXALIASES];
static int		_net_stayopen;
static int 		currentlen;
static char		*current = NULL;

#endif /* _THREAD_SAFE */

static int yellowup();
static int interpret();
static char *any();
static char domain[256];
static char *nettoa();

#ifdef _THREAD_SAFE
void
endnetent_r(struct netent_data *net_data)
#else
void
endnetent(void)
#endif /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (net_data == 0) return;
#endif /* _THREAD_SAFE */

	if (CURRENT) {
		free((void *)CURRENT);
		CURRENT = NULL;
	}
	if (NET_FP) {
		fclose(NET_FP);
		NET_FP = NULL;
	}
	STAYOPEN = 0;
}

#ifdef _THREAD_SAFE
int
getnetbyaddr_r(register long net, register int type, 
		struct netent *netent, struct netent_data *net_data)
{
#else
struct netent *
getnetbyaddr(register long net, register int type)
{
	register struct netent *netent = &net_networks;
#endif /* _THREAD_SAFE */

	int is_yp_up, reason;
	char *adrstr, *val;
	int vallen;

	TS_EINVAL((netent == 0 || net_data == 0));

	if (SETNETENT(0) != TS_SUCCESS) 
		return (TS_FAILURE);

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (!is_yp_up) {
		while ((fgets(LINE, BUFSIZ, NET_FP)) != NULL) {
			if (INTERPRET(netent, strlen(LINE))) {
				if (netent->n_addrtype == type 
			    	    && netent->n_net == net) {
					if (!STAYOPEN)
						ENDNETENT();
					return (TS_FOUND(netent));
				}
			}
		}
	}
	else {

		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		adrstr = nettoa(net);
		reason = yp_match(domain, "networks.byaddr", adrstr, 
				  strlen(adrstr), &val, &vallen);

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);

		if (reason) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif 
		}
		else {
			strncpy(LINE, val, vallen);
			INTERPRET(netent, vallen);
			free((void *)val);
			if (!STAYOPEN)
				ENDNETENT();
			return(TS_FOUND(netent));
		}
	}
	if (!STAYOPEN)
		ENDNETENT();
	return(TS_FAILURE);
}

#ifdef _THREAD_SAFE
int 
getnetbyname_r(register const char *name, struct netent *netent, 
		struct netent_data *net_data)
{
#else
struct netent *
getnetbyname(register const char *name)
{
	register struct netent *netent = &net_networks;
#endif /* _THREAD_SAFE */
	register char **cp;
	int is_yp_up, reason;
	char *val;
	int vallen;

	TS_EINVAL((netent == 0 || net_data == 0));

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (SETNETENT(STAYOPEN) != TS_SUCCESS) 
		return (TS_FAILURE);
	if (!is_yp_up) {
		while ((fgets(LINE, BUFSIZ, NET_FP)) != NULL) {
			if (INTERPRET(netent, strlen(LINE))) {
				if (strcmp(netent->n_name, name) == 0) {
					if (!STAYOPEN)
						ENDNETENT();
					return (TS_FOUND(netent));
				}
				for (cp = netent->n_aliases; *cp != 0; cp++)
					if (strcmp(*cp, name) == 0) {
						if (!STAYOPEN)
							ENDNETENT();	
						return(TS_FOUND(netent));
					}
			}
		}
	}
	else {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_match(domain, "networks.byname", name,
				  strlen(name), &val, &vallen);

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
		}
		else {
			strncpy(LINE, val, vallen);
			INTERPRET(netent, vallen);
			free((void *)val);
			if (!STAYOPEN)
				ENDNETENT();
			return (TS_FOUND(netent));
		}
	}
	if (!STAYOPEN)
		ENDNETENT();
	return (TS_FAILURE);
}

#ifdef _THREAD_SAFE
int
setnetent_r(int f, struct netent_data *net_data)
#else
int
setnetent(int f)
#endif /* _THREAD_SAFE */
{
	int is_yp_up, flags;
	
	TS_EINVAL((net_data == 0));
	
	STAYOPEN |= f;

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(1);   /* recompute whether yellow pages are up */

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (is_yp_up) {
		if (CURRENT)
			free((void *)CURRENT);	
		CURRENT = NULL;
		return (TS_SUCCESS);
	}

	if (NET_FP) {
		rewind(NET_FP);
		return(TS_SUCCESS);
	}
	
	if ((NET_FP = fopen(_PATH_NETWORKS, "r")) != NULL) {
		flags = fcntl(fileno(NET_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(NET_FP), F_SETFD, flags) == 0)
			return(TS_SUCCESS);
		(void)fclose(NET_FP);
	}
	return(TS_FAILURE);
}

#ifdef _THREAD_SAFE
int getnetent_r(struct netent *netent, struct netent_data *net_data)
{
#else
struct netent *
getnetent(void)
{
	register struct netent *netent = &net_networks;
#endif /* _THREAD_SAFE */
	int is_yp_up, reason;
	char *key, *val;
	int keylen, vallen;

	TS_EINVAL((netent == 0 || net_data == 0));

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (!is_yp_up) {
		if (!NET_FP && SETNETENT(STAYOPEN) != TS_SUCCESS) 
			return (TS_FAILURE);
		while ((fgets(LINE, BUFSIZ, NET_FP)) != NULL) {
			if (INTERPRET(netent, strlen(LINE)))
				return(TS_FOUND(netent));
		}
		return(TS_FAILURE);
	}
	if (CURRENT == NULL) {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_first(domain, "networks.byaddr", &key, &keylen,
					&val, &vallen); 
		
		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
			return (TS_FAILURE);
		}
	}
	else {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_next(domain, "networks.byaddr", CURRENT, CURRENTLEN,
				 &key, &keylen, &val, &vallen);
		
		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_next failed is %d\n",
			    reason);
#endif
			return (TS_FAILURE);
		}
	}
	if (CURRENT)
		free((void *)CURRENT);
	CURRENT = key;
	CURRENTLEN = keylen;
	strncpy(LINE, val, vallen);
	free((void *)val);
	if (INTERPRET(netent, vallen))
		return (TS_FOUND(netent));
	else 
		return(TS_FAILURE);
}

static int 
interpret(struct netent *net, int len, char *line, char **net_aliases)
{
	char *p;
	register char *cp, **q;

	p = line;
	line[len] = '\n';
	if (*p == '#') 
		return (0);
	cp = any(p, "#\n");
	if (cp == NULL) 
		return (0);
	*cp = '\0';
	net->n_name = p;
	cp = any(p, " \t");
	if (cp == NULL) 
		return (0);
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	net->n_net = inet_network(cp);
	net->n_addrtype = AF_INET;
	q = net->n_aliases = net_aliases;
	if (p != NULL) 
		cp = p;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &net_aliases[_MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (1);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

/* 
 * check to see if yellow pages are up, and store that fact in usingyellow.
 * The check is performed once at startup and thereafter if flag is set
 *
 * THREAD_SAFE NOTE:
 * 	This routine should be locked whenever it is used since it
 *	contains static data and calls to getdomainname() and yp_bind()
 *	which are not thread-safe.
 */
static
yellowup(flag)
{
	static int firsttime = 1;
	static int usingyellow;
	nl_catd	catd;

	if (firsttime || flag) {
		firsttime = 0;
		if (domain[0] == 0) {
			if (getdomainname(domain, sizeof(domain)) < 0) {
                		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				fprintf(stderr, catgets(catd,LIBCNET,NET4,
			    "yellowup: getdomainname system call missing\n"));
				exit(1);
			}
		}
		usingyellow = !yp_bind(domain);
	}	
	return(usingyellow);
}

/* 
 * THREAD_SAFE NOTE:
 *	This routine should be locked whenever used since it contains
 *	static data.
 */
static
char *
nettoa(anet)
	int anet;
{
	static char buf[20];
	char *p;
	struct in_addr in;
	int addr;

	in = inet_makeaddr(anet, INADDR_ANY);
	addr = in.s_addr;
	strncpy(buf, inet_ntoa(in), sizeof(buf) - 1);
	if ((IN_CLASSA_HOST & htonl(addr)) == 0) {
		p = index(buf, '.');
		if (p == NULL)
			return NULL;
		*p = 0;
	}
	else if ((IN_CLASSB_HOST & htonl(addr)) == 0) {
		p = index(buf, '.');
		if (p == NULL)
			return NULL;
		p = index(p+1, '.');
		if (p == NULL)
			return NULL;
		*p = 0;
	}
	else if ((IN_CLASSC_HOST & htonl(addr)) == 0) {
		p = rindex(buf, '.');
		if (p == NULL)
			return (NULL);
		*p = 0;
	}
	return (buf);
}
