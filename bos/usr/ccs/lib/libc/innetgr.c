#ifndef lint
static char sccsid[] = "@(#)57        1.15  src/bos/usr/ccs/lib/libc/innetgr.c, libcnet, bos411, 9428A410j 4/20/94 18:45:49";
#endif
/* 
 * COMPONENT_NAME: LIBCNET innetgr.c
 * 
 * FUNCTIONS: any, doit, inlist, innetgr, lookup, makekey 
 *
 * ORIGINS: 24  27 
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
#include <ctype.h>
#include <rpcsvc/ypclnt.h>
#include <netdb.h>
#include <nl_types.h>
#include "libc_msg.h"

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ypresolv_rmutex;
#endif /* _THREAD_SAFE */
 
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	32
#endif

#ifdef _THREAD_SAFE

#define DOIT(g)		doit_r(g, netgr_data) 

#define NAME		netgr_data->name
#define MACHINE		netgr_data->machine
#define DOMAIN		netgr_data->domain
#define LIST		netgr_data->list
#define LISTP		netgr_data->listp

#else

#define DOIT(g)		doit(g)

#define NAME		name
#define MACHINE		machine
#define DOMAIN		domain
#define LIST		list
#define LISTP		listp

static char 		*name;
static char		*machine;
static char		*domain;
static char		*list[200];
static char		**listp;

#endif /* _THREAD_SAFE */

/* 
 * innetgr: test whether I'm in /etc/netgroup
 * 
 */

static char *any();
static int doit();
static int lookup();
static int makekey();
static char thisdomain[256];
static int dns_strncmp();


#ifdef _THREAD_SAFE
innetgr_r(char *grp, char *mach, char *nm, char *dom, 
	  struct innetgr_data *netgr_data)
#else
innetgr(char *grp, char *mach, char *nm, char *dom)
#endif /* _THREAD_SAFE */
{
	int res;
	nl_catd catd;
	char tmp_mach[MAXHOSTNAMELEN * 4]; /* This is 128 */
	int rc, i;

	TS_EINVAL((netgr_data == 0));
	TS_LOCK(&_ypresolv_rmutex);
	if (getdomainname(thisdomain, sizeof(thisdomain)) < 0) {
		TS_PUSH_CLNUP(&_ypresolv_rmutex);
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
		(void) fprintf(stderr, catgets(catd,LIBCNET,NET18,
		    "innetgr: getdomainname system call missing\r\n"));
		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
	    exit(1);
	}
	TS_UNLOCK(&_ypresolv_rmutex);
	LISTP = LIST;
	MACHINE = mach;
	NAME = nm;
	DOMAIN = dom;
	if (DOMAIN) {
		if (NAME && !MACHINE) {
			if (lookup("netgroup.byuser",grp,NAME,DOMAIN,&res)) {
				return(res);
			}
		} else if (MACHINE && !NAME) {
			rc = lookup("netgroup.byhost",grp,MACHINE,DOMAIN,&res);
			 /* If lookup failed, we need to force the machine name
			    to lower case, try one more time.
			  */
			if (rc != 1 || res != 1) {
				strcpy(tmp_mach, mach);
				for (i = 0; tmp_mach[i] != NULL; i++)
					tmp_mach[i] = tolower(tmp_mach[i]);
				if (lookup("netgroup.byhost", grp, tmp_mach, DOMAIN, &res)) {
					return(res);
				}
			}
			else
				return(res);
		}
	}
	return DOIT(grp);
}
	

	

/* 
 * calls itself recursively
 */
#ifdef _THREAD_SAFE
static
doit_r(char *group, struct innetgr_data *netgr_data)
#else
static
doit(char *group)
#endif /* _THREAD_SAFE */
{
	char *key, *val;
	int vallen,keylen;
	char *r;
	int match;
	register char *p, *q;
	register char **lp;
	int err;
	nl_catd catd;
	
	*LISTP++ = group;
	if (LISTP > LIST + sizeof(LIST)) {
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
		(void) fprintf(stderr, catgets(catd,LIBCNET,NET19,
			"innetgr: recursive overflow\r\n"));
		LISTP--;
		return (0);
	}
	key = group;
	keylen = strlen(group);
	TS_LOCK(&_ypresolv_rmutex);
	TS_PUSH_CLNUP(&_ypresolv_rmutex);

	err = yp_match(thisdomain, "netgroup", key, keylen, &val, &vallen);

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_ypresolv_rmutex);
	if (err) {
#ifdef DEBUG
		if (err == YPERR_KEY)
			(void) fprintf(stderr,
			    "innetgr: no such netgroup as %s\n", group);
		else
			(void) fprintf(stderr, "innetgr: yp_match, %s\n",yperr_string(err));
#endif
		LISTP--;
		return(0);
	}
	/* 
	 * check for recursive loops
	 */
	for (lp = LIST; lp < LISTP-1; lp++)
		if (strcmp(*lp, group) == 0) {
                	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			(void) fprintf(stderr, catgets(catd,LIBCNET,NET20,
			    "innetgr: netgroup %s called recursively\r\n"),
			    group);
			LISTP--;
			return(0);
		}
	
	p = val;
	p[vallen] = 0;
	while (p != NULL) {
		match = 0;
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == 0 || *p == '#')
			break;
		if (*p == '(') {
			p++;
			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
                		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				(void) fprintf(stderr, catgets(catd,LIBCNET,NET21,
				    "innetgr: syntax error in /etc/netgroup\r\n"));
				LISTP--;
				return(0);
			}
			if (p == q || MACHINE == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (dns_strncmp(MACHINE, p, q-p) == 0)
					match++;
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
                		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				(void) fprintf(stderr, catgets(catd,LIBCNET,NET21,
				    "innetgr: syntax error in /etc/netgroup\r\n"));
				LISTP--;
				return(0);
			}
			if (p == q || NAME == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(NAME, p, q-p) == 0)
					match++;
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ')');
			if (q == NULL) {
                		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				(void) fprintf(stderr, catgets(catd,LIBCNET,NET21,
				    "innetgr: syntax error in /etc/netgroup\r\n"));
				LISTP--;
				return(0);
			}
			if (p == q || DOMAIN == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(DOMAIN, p, q-p) == 0)
					match++;
			}
			p = r+1;
			if (match == 3) {
				free((void *)val);
				LISTP--;
				return 1;
			}
		}
		else {
			q = any(p, " \t\n#");
			if (q && *q == '#')
				break;
			if (q)
				*q = 0;
			if (DOIT(p)) {
				free((void *)val);
				LISTP--;
				return 1;
			}
			if (q)
				*q = ' ';
		}
		p = any(p, " \t");
	}
	free((void *)val);
	LISTP--;
	return 0;
}

/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
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
 * return 1 if "what" is in the comma-separated, newline-terminated "list"
 */
static
inlist(what,list)
	char *what;
	char *list;
{
#	define TERMINATOR(c)    (c == ',' || c == '\n')

	register char *p;
	int len;
         
	len = strlen(what);     
	p = list;
	do {             
		if (strncmp(what,p,len) == 0 && TERMINATOR(p[len])) {
			return(1);
		}
		while (!TERMINATOR(*p)) {
			p++;
		}
		p++;
	} while (*p);
	return(0);
}




/*
 * Lookup a host or user name in a yp map.  Set result to 1 if group in the 
 * lookup-up list of groups. Return 1 if the map was found.
 */
static
lookup(map,group,name,domain,res)
	char *map;
	char *group;
	char *name;
	char *domain;
	int *res;
{
	int err;
	char *val;
	int vallen;
	char key[256];
	char *wild = "*";
	int i;

	for (i = 0; i < 4; i++) {
		switch (i) {
		case 0: makekey(key,name,domain); break;
		case 1: makekey(key,wild,domain); break;	
		case 2: makekey(key,name,wild); break;
		case 3: makekey(key,wild,wild); break;	
		}
		TS_LOCK(&_ypresolv_rmutex);
		TS_PUSH_CLNUP(&_ypresolv_rmutex);

		err  = yp_match(thisdomain,map,key,strlen(key),&val,&vallen); 

		TS_POP_CLNUP(0);
		TS_UNLOCK(&_ypresolv_rmutex);
		if (!err) {
			*res = inlist(group,val);
			free((void *)val);
			if (*res) {
				return(1);
			}
		} else {
#ifdef DEBUG
			(void) fprintf(stderr,
				"yp_match(%s,%s) failed: %s.\n",map,key,yperr_string(err));
#endif
			if (err != YPERR_KEY)  {
				return(0);
			}
		}
	}
	*res = 0;
	return(1);
}



/*
 * Generate a key for a netgroup.byXXXX yp map
 */
static
makekey(key,name,domain)
	register char *key;
	register char *name;
	register char *domain;
{
	while (*key++ = *name++)
		;
	*(key-1) = '.';
	while (*key++ = *domain++)
		;
}	

/*
 * FUNCTION: Aaccording to DNS rules, compares at most n pairs of
 * characters from the strings pointed to by s1 and s2, returning
 * an integer as follows:
 *              Less than 0     If s1 is less than s2
 *              Equal to 0      If s1 is equal to s2
 *              Greater than 0  If s1 is greater than s2.
 *
 * NOTES:    Handles the pathological case where the value of n equals
 *           the maximum value of an unsigned long integer.
 *
 * PARAMETERS:
 *           char *s1 - first string
 *           char *s2 - second string
 *           size_t n - number of characters to compare
 *
 * RETURN VALUE DESCRIPTION: Returns a negative, zero, or positive value
 *           as described above.
 *
 * DNS rules specify that host names should be treated in a case insensitive
 * manner.
 */

static int
dns_strncmp(const char *s1, const char *s2, size_t n)
{
	register c1, c2;
	size_t i = 0;
	
	if (s1 == s2)
		return(0);
	for(i = 0; i < n && *s1 &&
	    (c1 = tolower(*s1)) == (c2 = tolower(*s2++)); i++)
		if (*s1++ == '\0')
			return(0);
	return((i == n)? 0: (*s1 - *--s2));
}
