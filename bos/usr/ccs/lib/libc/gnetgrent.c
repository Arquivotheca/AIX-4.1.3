#ifndef lint
static char sccsid[] = "@(#)54        1.12.1.3  src/bos/usr/ccs/lib/libc/gnetgrent.c, libcnet, bos411, 9428A410j 4/20/94 18:45:26";
#endif
/* 
 * COMPONENT_NAME: LIBCNET gnetgrent.c
 * 
 * FUNCTIONS: any, doit, endnetgrent, fill, getgroup, getnetgrent, 
 *            match, setnetgrent 
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <rpcsvc/ypclnt.h>
#include <nl_types.h>
#include "libc_msg.h"

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _getnetgrent_rmutex;
#endif _THREAD_SAFE

/* 
 * access members of a netgroup
 */

struct grouplist {		/* also used by pwlib */
	char *gl_machine;
	char *gl_name;
	char *gl_domain;
	struct grouplist *gl_nxt;
};

static char *any();

static struct list {			/* list of names to check for loops */
	char *name;
	struct list *nxt;
};

static struct grouplist *grouplist, *grlist;

static void getgroup();
static void doit();
static char *fill();
static char *match();
static char domain[256];
static char oldgrp[256];	/* global so endnetgrent() can get to it */


/*
 * THREAD_SAFE NOTE:
 *	The grouplists are maintained per process, therefore
 *	only locks are taken around the code which modifies or
 *	reads the groups. It is not per thread.
 */

setnetgrent(grp)
	char *grp;
{
	TS_LOCK(&_getnetgrent_rmutex);
	if (strcmp(oldgrp, grp) == 0)
		grlist = grouplist;
	else {
		if (grouplist != NULL)
			endnetgrent();
		TS_PUSH_CLNUP(&_getnetgrent_rmutex);
		getgroup(grp);
		TS_POP_CLNUP(0);
		grlist = grouplist;
		(void) strcpy(oldgrp, grp);
	}
	TS_UNLOCK(&_getnetgrent_rmutex);
}

endnetgrent() {
	struct grouplist *gl;
	
	TS_LOCK(&_getnetgrent_rmutex);
	for (gl = grouplist; gl != NULL; gl = gl->gl_nxt) {
		if (gl->gl_name)
			free((void *)gl->gl_name);
		if (gl->gl_domain)
			free((void *)gl->gl_domain);
		if (gl->gl_machine)
			free((void *)gl->gl_machine);
		free((void *) gl);
	}
	grouplist = NULL;
	grlist = NULL;
	oldgrp[0] = '\0';	/* wipe out old group name */
	TS_UNLOCK(&_getnetgrent_rmutex);
}

getnetgrent(machinep, namep, domainp)
	char **machinep, **namep, **domainp;
{
	TS_LOCK(&_getnetgrent_rmutex);
	if (grlist) {
		*machinep = grlist->gl_machine;
		*namep = grlist->gl_name;
		*domainp = grlist->gl_domain;
		grlist = grlist->gl_nxt;
		TS_UNLOCK(&_getnetgrent_rmutex);
		return (1);
	}
	else {
		TS_UNLOCK(&_getnetgrent_rmutex);
		return (0);
	}
}


/*
 * THREAD_SAFE NOTE:
 *	This routine should be locked whenever called since getdomainname()
 *	is not thread-safe.
 */
static void
getgroup(grp)
	char *grp;
{
	nl_catd catd;

	if (getdomainname(domain, sizeof(domain)) < 0) {
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
		(void) fprintf(stderr, catgets(catd,LIBCNET,NET9,
		    "getnetgrent: getdomainname system call missing\n"));
	    exit(1);
	}
	doit(grp,(struct list *) NULL);
}
		

/*
 * recursive function to find the members of netgroup "group". "list" is
 * the path followed through the netgroups so far, to check for cycles.
 */
static void
doit(group,list)
    char *group;
    struct list *list;
{
    register char *p, *q;
    register struct list *ls;
    char *val;
    struct grouplist *gpls;
    nl_catd catd;
 
 
    /*
     * check for non-existing groups
     */
    if ((val = match(group)) == NULL) {
        return;
    }
 
 
    /*
     * check for cycles
     */
    for (ls = list; ls != NULL; ls = ls->nxt) {
        if (strcmp(ls->name, group) == 0) {
            catd = catopen(MF_LIBC,NL_CAT_LOCALE);
            (void) fprintf(stderr, catgets(catd,LIBCNET,NET10,
			"Cycle detected in /etc/netgroup: %s.\n"),group);
            return;
        }
    }
 
 
    if (!(ls = (struct list *) malloc((size_t)sizeof(struct list))))
	perror("malloc failed");
    ls->name = group;
    ls->nxt = list;
    list = ls;
    
    p = val;
    while (p != NULL) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == 0 || *p =='#')
            break;
        if (*p == '(') {
            gpls = (struct grouplist *) malloc((size_t)sizeof(struct grouplist));
            p++;
 
            if (!(p = fill(p,&gpls->gl_machine,',')))  {
                goto syntax_error;
            }
            if (!(p = fill(p,&gpls->gl_name,','))) {
                goto syntax_error;
            }
            if (!(p = fill(p,&gpls->gl_domain,')'))) {
                goto syntax_error;
            }
            gpls->gl_nxt = grouplist;
            grouplist = gpls;
        } else {
            q = any(p, " \t\n#");
            if (q && *q == '#')
                break;
            *q = 0;
            doit(p,list);
            *q = ' ';
        }
        p = any(p, " \t");
    }
    return;
 
syntax_error:
    catd = catopen(MF_LIBC,NL_CAT_LOCALE);
    (void) fprintf(stderr, catgets(catd,LIBCNET,NET11,
				"syntax error in /etc/netgroup\n"));
    (void) fprintf(stderr,"--- %s\n",val);
    return;
}



/*
 * Fill a buffer "target" selectively from buffer "start".
 * "termchar" terminates the information in start, and preceding
 * or trailing white space is ignored. The location just after the
 * terminating character is returned.  
 */
static char *
fill(start,target,termchar)
    char *start;
    char **target;
    char termchar;
{
    register char *p;
    register char *q;
    char *r;
	unsigned size;
	
 
    for (p = start; *p == ' ' || *p == '\t'; p++)
        ;
    r = index(p, termchar);
    if (r == NULL) {
        return(NULL);
    }
    if (p == r) {
		*target = NULL;	
    } else {
        for (q = r-1; *q == ' ' || *q == '\t'; q--)
            ;
		size = q - p + 1;
		*target = malloc((size_t)(size+1));
		(void) strncpy(*target,p,(size_t)size);
		(*target)[size] = 0;
	}
    return(r+1);
}

/*
 * THREAD_SAFE NOTE:
 *	This function should be locked when used since it contains a 
 *	call to yp_match() which is not thread-safe.
 */

static char *
match(group)
	char *group;
{
	char *val;
	int vallen;
	int err;

	err = yp_match(domain,"netgroup",group,strlen(group),&val,&vallen);
	if (err) {
#ifdef DEBUG
		(void) fprintf(stderr,"yp_match(netgroup,%s) failed: %s\n",group
				,yperr_string(err));
#endif
		return(NULL);
	}
	return(val);
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
