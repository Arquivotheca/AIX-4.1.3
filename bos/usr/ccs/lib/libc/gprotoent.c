static char sccsid[] = "@(#)55        1.15  src/bos/usr/ccs/lib/libc/gprotoent.c, libcnet, bos41J, 9515A_all 4/5/95 09:18:24";
/* 
 * COMPONENT_NAME: LIBCNET gprotoent.c
 * 
 * FUNCTIONS: any, endprotoent, getprotobyname, getprotobynumber, 
 *            getprotoent, interpret, setprotoent, yellowup 
 *
 * ORIGINS: 24  27  71
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
#include <nl_types.h>
#include <fcntl.h>
#include "libc_msg.h"

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ypresolv_rmutex;
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE

#define	SETPROTOENT(f)		setprotoent_r(f, proto_data)
#define ENDPROTOENT()		endprotoent_r(proto_data)
#define INTERPRET(p, n)		interpret(p, n, proto_data->line, proto_data->proto_aliases)

#define	STAYOPEN		(proto_data->_proto_stayopen)
#define	PROTO_FP		(proto_data->proto_fp)
#define CURRENT			(proto_data->current)
#define CURRENTLEN		(proto_data->currentlen)
#define LINE			(proto_data->line)

#else

#define	SETPROTOENT(f)		setprotoent(f)
#define ENDPROTOENT()		endprotoent()
#define INTERPRET(p,n)		interpret(p, n, line, proto_aliases)

#define	STAYOPEN		_proto_stayopen
#define	PROTO_FP		proto_fp
#define CURRENT			current
#define CURRENTLEN		currentlen
#define LINE			line

static struct protoent	proto_protocols;
static FILE		*proto_fp = NULL;
static int		_proto_stayopen;
static char *current = NULL;	/* current entry, analogous to protof */
static int currentlen;
static char *proto_aliases[_MAXALIASES];
static char line[BUFSIZ+1];

#endif /* _THREAD_SAFE */

/*
 * Internet version.
 */

static int yellowup();
static char domain[256];

#ifdef _THREAD_SAFE
void
endprotoent_r(struct protoent_data *proto_data)
#else
void
endprotoent(void)
#endif /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (proto_data == 0) return;
#endif /* _THREAD_SAFE */
	if (CURRENT) {
		free((void *)CURRENT);
		CURRENT = NULL;
	}
	if (PROTO_FP) {
		(void)fclose(PROTO_FP);
		PROTO_FP = NULL;
	}
	STAYOPEN = 0;
}

#ifdef _THREAD_SAFE
int
getprotobynumber_r(register int proto, struct protoent *protoent, 
		   struct protoent_data *proto_data)
{
#else
struct protoent *
getprotobynumber(register int proto)
{
	register struct protoent  *protoent = &proto_protocols;
#endif /* _THREAD_SAFE */
	int is_yp_up, reason;
	char adrstr[12], *val;
	int vallen;

	TS_EINVAL((protoent == 0 || proto_data == 0));

	if (SETPROTOENT(STAYOPEN) != TS_SUCCESS) 
		return(TS_FAILURE);
	
	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0); 

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (is_yp_up) {
		sprintf(adrstr, "%d", proto);
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_match(domain, "protocols.bynumber", adrstr,
				  strlen(adrstr), &val, &vallen);

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
			INTERPRET(protoent, vallen);
			free((void *)val);
			if (!STAYOPEN)
				ENDPROTOENT();
			return(TS_FOUND(protoent));
		}
	}
	if (!is_yp_up || reason) { 
		while ((fgets(LINE, BUFSIZ, PROTO_FP)) != NULL) {
			if (INTERPRET(protoent, strlen(LINE))) {
				if (protoent->p_proto == proto) {
					if (!STAYOPEN)
						ENDPROTOENT();
					return(TS_FOUND(protoent));
				}
			}
		}
	}
	if (!STAYOPEN)
		ENDPROTOENT();
	return (TS_FAILURE);
}

#ifdef _THREAD_SAFE
int 
getprotobyname_r(register const char *name, struct protoent *protoent,
		 struct protoent_data *proto_data)
{
#else
struct protoent *
getprotobyname(register const char *name)
{
	register struct protoent *protoent = &proto_protocols;
#endif /* _THREAD_SAFE */
	register char **cp;
	int reason, is_yp_up;
	char *val;
	int vallen;

	TS_EINVAL((protoent == 0 || proto_data == 0));

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (SETPROTOENT(STAYOPEN) != TS_SUCCESS) 
		return(TS_FAILURE);
	
	if (is_yp_up) {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_match(domain, "protocols.byname", name,
			 strlen(name), &val, &vallen); 

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (reason) {
#ifdef DEBUG
		    fprintf(stderr, 
			"reason yp_match failed is %d\n", reason);
#endif
		}
		else {
			strncpy(LINE, val, vallen);
			INTERPRET(protoent, vallen);	
			free((void *)val);
			if (!STAYOPEN)
				ENDPROTOENT();
			return(TS_FOUND(protoent));
		}
	}
	if (!is_yp_up || reason) {
		while ((fgets(LINE, BUFSIZ, PROTO_FP)) != NULL) {
			if (INTERPRET(protoent, strlen(LINE))) {
				if (strcmp(protoent->p_name, name) == 0) {
					if (!STAYOPEN)
						ENDPROTOENT();
					return(TS_FOUND(protoent));
				}
				for (cp = protoent->p_aliases; *cp != 0; cp++)
					if (strcmp(*cp, name) == 0) {
						if (!STAYOPEN)
							ENDPROTOENT();
						return(TS_FOUND(protoent));
					}
			}
		}
	}
	if (!STAYOPEN)
		ENDPROTOENT();
	return (TS_FAILURE);
}

#ifdef _THREAD_SAFE
int
setprotoent_r(int f, struct protoent_data *proto_data)
#else
setprotoent(int f)
#endif /* _THREAD_SAFE */
{
	int flags;
	int is_yp_up;

	TS_EINVAL((proto_data == 0));
	STAYOPEN |= f;

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(1);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (is_yp_up) {
		if (CURRENT)
			free(CURRENT);
		CURRENT = NULL;
	}
	
	if (PROTO_FP) {
		rewind(PROTO_FP);
		return(TS_SUCCESS);
	}
	if ((PROTO_FP = fopen(_PATH_PROTOCOLS, "r")) != NULL) {
		flags = fcntl(fileno(PROTO_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(PROTO_FP), F_SETFD, flags) == 0)
			return(TS_SUCCESS);
		(void)fclose(PROTO_FP);
	}

	return(TS_FAILURE);
}

#ifdef _THREAD_SAFE
int
getprotoent_r(struct protoent *protoent, struct protoent_data *proto_data)
{
#else
struct protoent *
getprotoent(void)
{
	register struct protoent *protoent = &proto_protocols;
#endif /* _THREAD_SAFE */
	int is_yp_up, reason;
	char *key, *val;
	int keylen, vallen;
	struct protoent *pp;
	
	TS_EINVAL((protoent == 0 || proto_data == 0));

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (!is_yp_up) {
		if (!PROTO_FP && SETPROTOENT(STAYOPEN) != TS_SUCCESS) 
			return(TS_FAILURE);
		while((fgets(LINE, BUFSIZ, PROTO_FP)) != NULL) {
			if (INTERPRET(protoent, strlen(LINE)))
				return(TS_FOUND(protoent));
		}
		return(TS_FAILURE);
	}
	if (CURRENT == NULL) {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_first(domain, "protocols.bynumber", &key,
			 &keylen, &val, &vallen);

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

		reason = yp_next(domain, "protocols.bynumber", CURRENT,
			 CURRENTLEN, &key, &keylen, &val, &vallen);

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
	if (INTERPRET(protoent, vallen))
		return (TS_FOUND(protoent));
	else
		return (TS_FAILURE);
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

static int
interpret(struct protoent *proto, int len, char *line, char **proto_aliases)
{
	char *p;
	register char *cp, **q;

	p = line;
	line[len] = '\n';
	if (*p == '#')
		return(0);
	cp = any(p, "#\n");
	if (cp == NULL)
		return(0);
	*cp = '\0';
	proto->p_name = p;
	cp = any(p, " \t");
	if (cp == NULL)
		return(0);
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	proto->p_proto = atoi(cp);
	q = proto->p_aliases = proto_aliases;
	if (p != NULL) {
		cp = p;
		while (cp && *cp) {
			if (*cp == ' ' || *cp == '\t') {
				cp++;
				continue;
			}
			if (q < &proto_aliases[_MAXALIASES - 1])
				*q++ = cp;
			cp = any(cp, " \t");
			if (cp != NULL)
				*cp++ = '\0';
		}
	}
	*q = NULL;
	return (1);
}

/* 
 * check to see if yellow pages are up, and store that fact in usingyellow.
 * The check is performed once at startup and thereafter if flag is set
 *
 * THREAD_SAFE NOTE:
 *	This routine should be locked when called since it contains
 *	static data and calls to getdomainname() and yp_bind() which
 *	are not thread-safe.
 */
static int
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
				fprintf(stderr, catgets(catd,LIBCNET,NET36,
			   "getprotoent: getdomainname system call missing\n"));
				exit(1);
			}
		}
		usingyellow = !yp_bind(domain);
	}	
	return(usingyellow);
}
