#ifndef lint
static char sccsid[] = "@(#)52        1.13  src/bos/usr/ccs/lib/libc/getrpcent.c, libcnet, bos411, 9428A410j 4/20/94 18:45:00";
#endif
/* 
 * COMPONENT_NAME: LIBCNET getrpcent.c
 * 
 * FUNCTIONS: any, endrpcent, getrpcbyname, getrpcbynumber, getrpcent, 
 *            interpret, setrpcent, yellowup 
 *
 * ORIGINS: 24  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */

 /* Copyright (c) 1985 by Sun Microsystems, Inc. */

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rpcsvc/ypclnt.h>
#include <fcntl.h>
#include <nl_types.h>
#include "libc_msg.h"

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ypresolv_rmutex;
#endif _THREAD_SAFE

#ifdef _THREAD_SAFE

#define	SETRPCENT(f)	setrpcent_r(f, rpc_data)
#define INTERPRET(n, v) interpret(n, v, rpc_data->line, rpc_data->rpc_aliases)
#define ENDRPCENT()	endrpcent_r(rpc_data)

#define RPC_FP		(rpc_data->rpc_fp)
#define STAYOPEN	(rpc_data->_rpc_stayopen)
#define CURRENT		(rpc_data->current)
#define CURRENTLEN	(rpc_data->currentlen)
#define LINE		(rpc_data->line)

#else

#define SETRPCENT(f)	setrpcent(f)
#define INTERPRET(n, v)	interpret(n, v, line, rpc_aliases)
#define ENDRPCENT()	endrpcent()

#define RPC_FP		rpc_fp
#define STAYOPEN	_rpc_stayopen
#define CURRENT		current
#define CURRENTLEN	currentlen
#define LINE		line

static int		_rpc_stayopen;
static int		currentlen;
static char		*current;
static char		line[BUFSIZ+1];
static FILE		*rpc_fp;
static char		*rpc_aliases[_MAXALIASES];
static struct rpcent	rpc_rpc;

#endif _THREAD_SAFE

#define	MAXADDRSIZE	14

static int yellowup();
static char domain[256];
static int interpret();
static char *any();

#ifdef _THREAD_SAFE
void endrpcent_r(struct rpcent_data *rpc_data)
#else
void
endrpcent(void)
#endif _THREAD_SAFE
{
#ifdef _THREAD_SAFE
	if (rpc_data == 0) return;
#endif _THREAD_SAFE
	
	if (CURRENT) {
		free((void *)CURRENT);
		CURRENT = NULL;
	}
	if (RPC_FP) {
		fclose(RPC_FP);
		RPC_FP = NULL;
	}
	STAYOPEN = 0;
}

#ifdef _THREAD_SAFE
int
getrpcbynumber_r(register int number, struct rpcent *rpcent,
		struct rpcent_data *rpc_data)
{
#else
struct rpcent *
getrpcbynumber(register int number)
{
	register struct rpcent *rpcent = &rpc_rpc;
#endif _THREAD_SAFE

	int is_yp_up, reason;
	char adrstr[10], *val;
	int vallen;

	TS_EINVAL((rpcent == 0 || rpc_data == 0));

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (SETRPCENT(0) != TS_SUCCESS)
		return (TS_FAILURE);
	if (!is_yp_up) {
		while((fgets(LINE, BUFSIZ, RPC_FP)) != NULL) {
			if (INTERPRET(rpcent, strlen(LINE))) {
				if (rpcent->r_number == number) {
					if (!STAYOPEN)
						ENDRPCENT();
					return(TS_FOUND(rpcent));
				}
			}
		}
	}
	else {
		sprintf(adrstr, "%d", number);
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_match(domain, "rpc.bynumber", adrstr, strlen(adrstr), &val, &vallen);

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_match failed is %d\n", 
					reason);
#endif
		}
		else {
			strncpy(LINE, val, vallen);
			INTERPRET(rpcent, vallen);
			free((void *)val);
			if (!STAYOPEN)
				ENDRPCENT();
			return (TS_FOUND(rpcent));
		}
	}
	if (!STAYOPEN)
		ENDRPCENT();
	return(TS_FAILURE);
}

#ifdef _THREAD_SAFE
int
getrpcbyname_r(const char *name, struct rpcent *rpcent, 
		struct rpcent_data *rpc_data)
{
#else
struct rpcent *
getrpcbyname(const char *name)
{
	struct rpcent *rpcent = &rpc_rpc;
#endif _THREAD_SAFE
	char **rp;

	TS_EINVAL((rpcent == 0 || rpc_data == 0));

	if (SETRPCENT(0) != TS_SUCCESS)
		return (TS_FAILURE);
	while((fgets(LINE, BUFSIZ, RPC_FP)) != NULL) {
		if (INTERPRET(rpcent, strlen(LINE))) {
			if (strcmp(rpcent->r_name, name) == 0) {
				if (!STAYOPEN)
					ENDRPCENT();
				return(TS_FOUND(rpcent));
			}
			for (rp = rpcent->r_aliases; *rp != NULL; rp++) 
				if (strcmp(*rp, name) == 0) {
					if (!STAYOPEN)
						ENDRPCENT();
					return(TS_FOUND(rpcent));
				}
		}
	}
	if (!STAYOPEN)
		ENDRPCENT();
	return(TS_FAILURE);
}
#ifdef _THREAD_SAFE
int
setrpcent_r(int f, struct rpcent_data *rpc_data)
#else
setrpcent(int f)
#endif _THREAD_SAFE
{
	int 	flags;

	TS_EINVAL((rpc_data == 0));

	STAYOPEN |= f;

	if (CURRENT)
		free((void *)CURRENT);
	CURRENT = NULL;
		
	if (RPC_FP) {
		rewind(RPC_FP);
		return(TS_SUCCESS);
	}

	if ((RPC_FP = fopen(_PATH_RPCDB, "r")) != NULL) {
		flags = fcntl(fileno(RPC_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(RPC_FP), F_SETFD, flags) == 0)
			return(TS_SUCCESS);
		(void)fclose(RPC_FP);
	}
	return(TS_FAILURE);
}
	
#ifdef _THREAD_SAFE
int
getrpcent_r(struct rpcent *rpcent, struct rpcent_data *rpc_data)
{
#else
struct rpcent *
getrpcent(void)
{
	struct rpcent *rpcent = &rpc_rpc;
#endif _THREAD_SAFE
	int reason, is_yp_up;
	char *key, *val;
	int keylen, vallen;

	TS_EINVAL((rpcent == 0 || rpc_data == 0));

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (!is_yp_up) {
		if (!RPC_FP && SETRPCENT(STAYOPEN) != TS_SUCCESS)
			return (TS_FAILURE);
		while((fgets(LINE, BUFSIZ, RPC_FP)) != NULL) {
			if (INTERPRET(rpcent, strlen(LINE)))
				return(TS_FOUND(rpcent));
		}
		return(TS_FAILURE);
	}
	if (CURRENT == NULL) {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason =  yp_first(domain, "rpc.bynumber",
		    &key, &keylen, &val, &vallen);

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
			return(TS_FAILURE);
		}
	}
	else {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_next(domain, "rpc.bynumber",
		    CURRENT, CURRENTLEN, &key, &keylen, &val, &vallen);

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_next failed is %d\n",
			    reason);
#endif
			return(TS_FAILURE);
		}
	}
	if (CURRENT)
		free((void *)CURRENT);
	CURRENT = key;
	CURRENTLEN = keylen;
	strncpy(LINE, val, vallen);
	free((void *)val);
	if (INTERPRET(rpcent, vallen))
		return (TS_FOUND(rpcent));
	else
		return(TS_FAILURE);
}

static int 
interpret(struct rpcent *rpc, int len, char *line, char **rpc_aliases)
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
	cp = any(p, " \t");
	if (cp == NULL)
		return (0);
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	rpc->r_name = line;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	rpc->r_number = atoi(cp);
	q = rpc->r_aliases = rpc_aliases;
	cp = any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &rpc_aliases[_MAXALIASES - 1])
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
 *	This routine should be locked whenever used since it contains
 *	static data and calls to getdomainname() and yp_bind() which
 *	are not thread-safe.
 */
static
yellowup(flag)
{
	static int firsttime = 1;
	nl_catd catd;
	static int usingyellow;

	if (firsttime || flag) {
		firsttime = 0;
		if (domain[0] == 0) {
			if (getdomainname(domain, sizeof(domain)) < 0) {
                		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				fprintf(stderr, catgets(catd,LIBCNET,NET6,
			    "getrpcent: getdomainname system call missing\n"));
				exit(1);
			}
		}
		usingyellow = !yp_bind(domain);
	}	
	return(usingyellow);
}
