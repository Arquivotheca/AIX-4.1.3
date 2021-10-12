static char sccsid[] = "@(#)53        1.20  src/bos/usr/ccs/lib/libc/getservent.c, libcnet, bos41J, 9509A_all 2/23/95 18:23:20";
/* 
 * COMPONENT_NAME: LIBCNET getservent.c
 * 
 * FUNCTIONS: any, endservent, getservbyname, getservbyport, 
 *            getservent, interpret, setservent, yellowup 
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

/* 
 * unlike gethost, getpw, etc, this doesn't implement getservbyxxx
 * directly
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <rpcsvc/ypclnt.h>
#include <ctype.h>
#include <netinet/in.h>		/* for htons() macro (need types.h) */
#include <fcntl.h>
#include <nl_types.h>
#include "libc_msg.h"

#include "ts_supp.h"
#include "push_pop.h"

/*
 * Internet version.
 */
#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ypresolv_rmutex;
#endif _THREAD_SAFE

#ifdef _THREAD_SAFE

#define	SERV_FP			(serv_data->serv_fp)

#define	SETSERVENT(f)		setservent_r(f, serv_data)
#define	ENDSERVENT()		endservent_r(serv_data)
#define INTERPRET(s, l)		interpret(s, l, serv_data->line, serv_data->serv_aliases)

#define STAYOPEN		(serv_data->_serv_stayopen)
#define CURRENT			(serv_data->current)
#define CURRENTLEN		(serv_data->currentlen)
#define LINE			(serv_data->line)

#else

#define	SERV_FP			serv_fp

#define	SETSERVENT(f)		setservent(f)
#define ENDSERVENT()		endservent()
#define INTERPRET(s, l)		interpret(s, l, line, serv_aliases)


#define	STAYOPEN		_serv_stayopen
#define CURRENT			current
#define CURRENTLEN		currentlen
#define	LINE			line

static struct servent		serv_services;
static FILE			*serv_fp = NULL;
static int			_serv_stayopen;
static char 			*current = NULL;
static int 			currentlen;
static char			line[BUFSIZ+1];
static char			*serv_aliases[_MAXALIASES];

#endif _THREAD_SAFE	

static int yellowup();
static char domain[256];
static int interpret();
static struct servent *jk;      /* tmp struct for multiple client fix */
static struct ypall_callback incallback;
static int parsedb();
static char *globname, *globproto; /* because of yp_all oddities */
static char *any();

/*
 * Internet version.
 */

static char *YPMAP = "services.byname";

#ifdef _THREAD_SAFE
void
endservent_r(struct servent_data *serv_data)
#else
void
endservent(void)
#endif _THREAD_SAFE
{
#ifdef _THREAD_SAFE
	if (serv_data == 0) return;
#endif _THREAD_SAFE
	if (CURRENT) {
		free((void *)CURRENT);
		CURRENT = NULL;
	}
	if (SERV_FP) {
		(void)fclose(SERV_FP);
		SERV_FP = NULL;
	}
	STAYOPEN = 0;
}

#ifdef _THREAD_SAFE
int
getservbyport_r(int port, const char *proto, struct servent *servent,
		struct servent_data *serv_data) 
{
#else
struct servent *
getservbyport(int port, const char *proto)
{
	register struct servent *servent = &serv_services;
#endif _THREAD_SAFE
	register struct servent *p = NULL;
	register int status;
	int reason, is_yp_up;

	TS_EINVAL((servent == 0 || serv_data == 0));

	/*
	 * first try to do very fast NIS lookup.
	 */
	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (is_yp_up && (proto != 0)) {
		char portstr[12];
		char *key, *val;
		int keylen, vallen;
 
		sprintf(portstr, "%d", ntohs(port));
		keylen = strlen(portstr) + 1 + strlen(proto);
		if (!(key = malloc(keylen + 1)))
			perror("getservbyport: malloc() failed");
		sprintf(key, "%s/%s", portstr, proto);
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		status = yp_match(domain, YPMAP, key, keylen, &val, &vallen);

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (status) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_match failed is %d\n", reason);
#endif DEBUG
		}
		else {
			strncpy(LINE, val, vallen);
			INTERPRET(servent, vallen);
			free(val);
			free(key);
			if (!STAYOPEN)
				ENDSERVENT();
			return(TS_FOUND(servent));
		}
	}
	/*
	 * Defect 171380 -- if NIS fails, check the local services file.
	 */
	if (SETSERVENT(STAYOPEN) == TS_SUCCESS) {
	    while ((fgets(LINE, BUFSIZ, SERV_FP)) != NULL) {
		if (INTERPRET(servent, strlen(LINE))) {
			if (servent->s_port != port)
				continue;
			if (proto == 0 ||
			    strcmp(servent->s_proto,proto) == 0) {
				if (!STAYOPEN)
					ENDSERVENT();
				return(TS_FOUND(servent));
			}
		}
	    }
	}
	if (!STAYOPEN)
		ENDSERVENT();
	return(TS_FAILURE);
}
		
#ifdef _THREAD_SAFE
int
getservbyname_r(const char *name, const char *proto, 
		struct servent *servent, struct servent_data *serv_data)
{
#else
struct servent *
getservbyname(const char *name, const char *proto)
{
	register struct servent *servent = &serv_services;
#endif _THREAD_SAFE
	register char **cp;
	int is_yp_up, rc ;

	TS_EINVAL((servent == 0 || serv_data == 0));

	if (SETSERVENT(STAYOPEN) != TS_SUCCESS) 
		return(TS_FAILURE);

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (is_yp_up) {
	    /* 
	    ** to make it this far, yellow pages must be up.  we'll do
	    ** a yp_all and parse through the database in search of the
	    ** name/proto match passed to getservbyname().
	    **
	    ** THREAD_SAFE_NOTE:
	    ** THIS PORTION OF THE CODE IS NOT REALLY THREAD_SAFE DUE TO
	    ** PARSEDB(). THE PARAMETERS FOR PARSEDB() ARE SET BY YP_ALL()
	    ** AND CANNOT BE MODIFIED OR ADDED TO IN ORDER TO BE 
	    ** THREAD_SAFE. THEREFORE IT RETURNS STATIC DATA WHICH IS THEN
	    ** ASSIGNED TO OUR SERVENT STRUCTURE. THE BEST WE CAN DO IS LOCK IT.
	    */

	    TS_LOCK(&_ypresolv_rmutex);
	    TS_PUSH_CLNUP(&_ypresolv_rmutex);

	    incallback.foreach = parsedb; /* parse the dbase from YP server */
	    globname = name;              /* needed in parsedb() */
	    globproto = proto;              /* needed in parsedb() */
	    jk = (struct servent *)0;
	    rc = yp_all(domain, "services.byname", &incallback);
	    if(rc == YPERR_RPC){
		    yp_bind(domain);
		    yp_all(domain, "services.byname", &incallback);
	    }
	    if (jk == (struct servent *)0 ) {
		    rc = TS_FAILURE;	/* close up shop */
	    }
	    else {
	    
		    if (!STAYOPEN)
			    ENDSERVENT();                 /* close up shop */
		    servent = jk;
		    rc = TS_FOUND(servent);
			/*
			 ** if yp_all [actually parsedb()] failed, servent will
			 ** be NULL, otherwise it will be set correctly.
			 */
	    }

	    TS_POP_CLNUP(0);
	    TS_UNLOCK(&_ypresolv_rmutex);
	}
	if (!is_yp_up || rc == TS_FAILURE) {
		/*
		 * Defect 171380 -- check the local services file.
		 */
		rc = TS_FAILURE;
		while ((fgets(LINE, BUFSIZ, SERV_FP)) != NULL) {
			if (INTERPRET(servent, strlen(LINE))) {
				if (strcmp(name, servent->s_name) == 0)
					goto gotname;
				for (cp = servent->s_aliases; *cp; cp++)
					if (strcmp(name, *cp) == 0)
						goto gotname;
				continue;

gotname:
				if (proto == 0 || 
				    strcmp(servent->s_proto, proto) == 0){
					if (!STAYOPEN)
						ENDSERVENT();
					rc = TS_FOUND(servent);
				}	
			}
		}
		if (!STAYOPEN)
			ENDSERVENT();
	}

	return(rc);
}

/*
 * THREAD_SAFE NOTE:
 * 	The parameters for this routine are determined by yp_all()
 *	and cannot be modified for thread_safe purposes. So, just take
 *	a lock wherever yp_all calls it.
 */
static int
parsedb(int instatus, char *inkey, int inkeylen, char *inval, 
	int invallen, char *indata)
{
	static char *serv_aliases[_MAXALIASES];
	static struct servent serv;
	static char line[BUFSIZ+1];
	char *p;
	register char *cp, **q;
	int foundit = 0;

	/* interpret inval and then compare name & proto with globname and
	   globproto.  NOTE: This interpret code is yanked from interpret(). */
	strncpy(line, inval, invallen);
	p = line;
	line[invallen] = '\n';
	if (*p == '#')
		return (0);
	cp = any(p, "#\n");
	if (cp == NULL)
		return (0);
	*cp = '\0';
	serv.s_name = p;
	p = any(p, " \t");
	if (p == NULL)
		return (0);
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		p++;
	cp = any(p, ",/");
	if (cp == NULL)
		return (0);
	*cp++ = '\0';
	serv.s_port = htons((u_short)atoi(p));
	serv.s_proto = cp;
	q = serv.s_aliases = serv_aliases;
	cp = any(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &serv_aliases[_MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	/* if the names compare, fine...if they don't, then search the
	   alias list.  If no aliases match, the return(0) else continue. */
	if (strcmp(globname, serv.s_name) != 0) 
		for (q = serv_aliases; *q; q++)  {
			if (strcmp(globname, *q) == 0) {
				foundit++;
				break;
			}
		}
	else 
		foundit++;
	if (!foundit) 
		return(0);
	/* If a protocol was specified, then see if it matches, else we've 
	   got a match already. */
	if ((globproto != (char *) 0) && 
	    (strcmp(globproto, serv.s_proto) != 0))
		return(0);
	jk = &serv;
	return(1);
}

#ifdef _THREAD_SAFE
int
setservent_r(int f, struct servent_data *serv_data)
#else
int
setservent(int f)
#endif _THREAD_SAFE
{
	int is_yp_up, flags;
	
	TS_EINVAL((serv_data == 0));

	STAYOPEN |= f;

	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	is_yp_up = yellowup(1);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);

	if (is_yp_up) {
		if (CURRENT)
			free((void *)CURRENT);
		CURRENT = NULL;
	}

	if (SERV_FP) {
		rewind(SERV_FP);
		return(TS_SUCCESS);
	}

	if ((SERV_FP = fopen(_PATH_SERVICES, "r")) != NULL) {
		flags = fcntl(fileno(SERV_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(SERV_FP), F_SETFD, flags) == 0)
			return(TS_SUCCESS);
		(void)fclose(SERV_FP);
	}
	return (TS_FAILURE);
}

#ifdef _THREAD_SAFE
getservent_r(struct servent *servent, struct servent_data *serv_data)
{
#else
struct servent *
getservent(void)
{
	register struct servent *servent = &serv_services;
#endif _THREAD_SAFE

	int is_yp_up, reason;
	char *key, *val;
	int keylen, vallen;

	TS_EINVAL((servent == 0 || serv_data == 0));

	
	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

 	is_yp_up = yellowup(0);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (!is_yp_up) {
		if (!SERV_FP && SETSERVENT(STAYOPEN) != TS_SUCCESS)
			return (TS_FAILURE);
		while ((fgets(LINE, BUFSIZ, SERV_FP)) != NULL) {
			if (INTERPRET(servent, strlen(LINE)))
				return(TS_FOUND(servent));
		}
		return(TS_FAILURE);
	}
		
	if (CURRENT == NULL) {
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		reason = yp_first(domain, "services.byname", &key, &keylen,
				  &val, &vallen);

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

		reason = yp_next(domain, "services.byname", CURRENT,
			 CURRENTLEN, &key, &keylen, &val, &vallen);

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
	INTERPRET(servent, vallen);
	return(TS_FOUND(servent));
}

static int
interpret(struct servent *serv, int len, char *line, char ** serv_aliases)
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
	serv->s_name = p;
	p = any(p, " \t");
	if (p == NULL)
		return (0); 
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		p++;
	cp = any(p, ",/");
	if (cp == NULL)
		return (0); 
	*cp++ = '\0';
	serv->s_port = htons((u_short)atoi(p));
	serv->s_proto = cp;
	q = serv->s_aliases = serv_aliases;
	cp = any(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &serv_aliases[_MAXALIASES - 1])
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
 *	This routine must be locked whenever it is used since it contains
 *	static data and calls to getdomainname() and yp_bind() which are
 *	not thread-safe.
 */
static
yellowup(flag)
{
	static int firsttime = 1;
	static int usingyellow;		/* are yellow pages up? */
	nl_catd catd;

	if (firsttime || flag) {
		firsttime = 0;
		if (domain[0] == 0) {
			if (getdomainname(domain, sizeof(domain)) < 0) {
                		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				fprintf(stderr, catgets(catd,LIBCNET,NET8,
			    "getservent: getdomainname system call missing\n"));
				exit(1);
			}
		}
		usingyellow = !yp_bind(domain);
	}	
	return(usingyellow);
}
