static char sccsid[] = "@(#)39  1.7  src/bos/usr/bin/mh/uip/aliasbr.c, cmdmh, bos411, 9428A410j 3/28/91 14:48:36";
/* 
 * COMPONENT_NAME: CMDMH aliasbr.c
 * 
 * FUNCTIONS: MSGSTR, add_aka, addall, addfile, addgroup, addmember, 
 *            akalloc, akerror, akresult, akval, akvalue, akvisible, 
 *            aleq, alias, getalias, getp, hmalloc, init_pw, scanp, 
 *            seek_home, seekp 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "aliasbr.c	7.1 87/10/13 17:22:38"; */

/* aliasbr.c - new aliasing mechanism */

#include "mh.h"
#include "aliasbr.h"
#include <ctype.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


static int  akvis;
static char *akerrst;

struct aka *akahead = NULL;
struct aka *akatail = NULL;

struct home *homehead = NULL;
struct home *hometail = NULL;

static add_aka (register struct aka *, register char *);
static struct aka *akalloc (register char   *);
static struct home *hmalloc (struct passwd *);

static char   *scanp (register char *),
	      *getp (register char *p),
	      *seekp (register char *, register char *,
		      register char **),
              *getalias (register char *),
              *akval (register struct aka *, register char *);

struct passwd  *getpwent ();
static	int addfile (register struct aka *, register char *),
	    addall (register struct aka *),
	    addmember (register struct aka *, register char *),
            aleq (register char *, register char *),
            addgroup (register struct aka *, register char *);


/*  */

char   *akvalue (s)
register char   *s;
{
    register char  *v;

    if (akahead == NULL)
	(void) alias (AliasFile);

    akvis = -1;
    v = akval (akahead, s);
    if (akvis == -1)
	akvis = 0;
    return v;
}


int     akvisible () {
    return akvis;
}

/*  */

char   *akresult (ak)
register struct aka *ak;
{
    register char  *cp = NULL,
                   *dp,
                   *pp;
    register struct adr *ad;

    for (ad = ak -> ak_addr; ad; ad = ad -> ad_next) {
	pp = ad -> ad_local ? akval (ak -> ak_next, ad -> ad_text)
	    : getcpy (ad -> ad_text);

	if (dp = cp) {
	    cp = concat (cp, ",", pp, NULLCP);
	    free (dp);
	    free (pp);
	}
	else
	    cp = pp;
    }

    if (akvis == -1)
	akvis = ak -> ak_visible;
    return cp;
}


static	char   *akval (	register struct aka *ak,
		       register char   *s)
{
    for (; ak; ak = ak -> ak_next)
	if (aleq (s, ak -> ak_name))
	    return akresult (ak);

    return getcpy (s);
}


static	int aleq (register char *string,
		  register char *aliasent)
{
    register char    c;

    while (c = *string++)
	if (*aliasent == '*')
	    return 1;
	else
	    if ((c | 040) != (*aliasent | 040))
		return 0;
	    else
		aliasent++;

    return (*aliasent == (char)NULL || *aliasent == '*');
}

/*  */

int     alias (file)
register char   *file;
{
    int     i;
    register char  *bp,
		   *cp,
                   *pp;
    char    lc,
	   *ap;
    register struct aka *ak = NULL;
    register    FILE *fp;

    if (*file != '/' && *file != '.')
	file = libpath (file);
    if ((fp = fopen (file, "r")) == NULL) {
	akerrst = file;
	return AK_NOFILE;
    }

    while (vfgets (fp, &ap) == OK) {
	bp = ap;
	switch (*(pp = scanp (bp))) {
	    case '<': 		/* recurse a level */
		if (!*(cp = getp (pp + 1))) {
		    akerrst = MSGSTR(NEEDAF, "'<' without alias-file"); /*MSG*/
		    (void) fclose (fp);
		    return AK_ERROR;
		}
		if ((i = alias (cp) != AK_OK)) {
		    (void) fclose (fp);
		    return i;
		}

	    case ':': 		/* comment */
	    case ';': 
	    case '\0': 
		continue;
	}

	akerrst = bp;
	if (!*(cp = seekp (pp, &lc, &ap))) {
	    (void) fclose (fp);
	    return AK_ERROR;
	}
	if (!(ak = akalloc (cp))) {
	    (void) fclose (fp);
	    return AK_LIMIT;
	}
	switch (lc) {
	    case ':': 
		ak -> ak_visible = 0;
		break;

	    case ';': 
		ak -> ak_visible = 1;
		break;

	    default: 
		(void) fclose (fp);
		return AK_ERROR;
	}

	switch (*(pp = scanp (ap))) {
	    case '\0': 		/* EOL */
		(void) fclose (fp);
		return AK_ERROR;

	    case '<': 		/* read values from file */
		if (!*(cp = getp (pp + 1))) {
		    (void) fclose (fp);
		    return AK_ERROR;
		}
		if (!addfile (ak, cp)) {
		    (void) fclose (fp);
		    return AK_NOFILE;
		}
		break;

	    case '=': 		/* UNIX group */
		if (!*(cp = getp (pp + 1))) {
		    (void) fclose (fp);
		    return AK_ERROR;
		}
		if (!addgroup (ak, cp)) {
		    (void) fclose (fp);
		    return AK_NOGROUP;
		}
		break;

	    case '+': 		/* UNIX group members */
		if (!*(cp = getp (pp + 1))) {
		    (void) fclose (fp);
		    return AK_ERROR;
		}
		if (!addmember (ak, cp)) {
		    (void) fclose (fp);
		    return AK_NOGROUP;
		}
		break;

	    case '*': 		/* Everyone */
		(void) addall (ak);
		break;

	    default: 		/* list */
		while (cp = getalias (pp))
		    add_aka (ak, cp);
		break;
	}
    }

    (void) fclose (fp);
    return AK_OK;
}

/*  */

char   *akerror (i)
int     i;
{
    static char buffer[BUFSIZ];
    char hlds[NL_TEXTMAX];

    switch (i) {
	case AK_NOFILE: 
	    (void) sprintf (buffer, MSGSTR(NOREAD2, "unable to read '%s'"), akerrst); /*MSG*/
	    break;

	case AK_ERROR: 
    	    strcpy(hlds, akerrst);
	    (void) sprintf (buffer, MSGSTR(LERROR, "error in line '%s'"), hlds); /*MSG*/
	    break;

	case AK_LIMIT: 
	    (void) sprintf (buffer, MSGSTR(OUTOFMEM, "out of memory while on '%s'"), akerrst); /*MSG*/
	    break;

	case AK_NOGROUP: 
	    (void) sprintf (buffer, MSGSTR(NOGRP, "no such group as '%s'"), akerrst); /*MSG*/
	    break;

	default: 
	    (void) sprintf (buffer, MSGSTR(UNKERROR, "unknown error (%d)"), i); /*MSG*/
	    break;
    }

    return buffer;
}

/*  */

static char   *scanp (register char *p)
{
    while (isspace (*p))
	p++;
    return p;
}


static char   *getp (register char   *p)
{
    register char  *cp = scanp (p);

    p = cp;
    while (!isspace (*cp) && *cp)
	cp++;
    *cp = (char)NULL;

    return p;
}


static char   *seekp (register char   *p,
		      register char   *c,
		      register char   **a)
{
    register char  *cp = scanp (p);

    p = cp;
    while (!isspace (*cp) && *cp && *cp != ':' && *cp != ';')
	cp++;
    *c = *cp;
    *cp++ = (char)NULL;
    *a = cp;

    return p;
}

/*  */

static	int addfile (register struct aka *ak,
		     register char   *file)
{
    register char  *cp;
    char    buffer[BUFSIZ];
    register    FILE *fp;

    if ((fp = fopen (libpath (file), "r")) == NULL) {
	akerrst = file;
	return (int)NULL;
    }

    while (fgets (buffer, sizeof buffer, fp) != NULL)
	while (cp = getalias (buffer))
	    add_aka (ak, cp);

    (void) fclose (fp);
    return 1;
}

/*  */

static	int addgroup (register struct aka *ak,
		      register char   *grp)
{
    register char  *gp;
    register struct group  *gr = getgrnam (grp);
    register struct home   *hm = NULL;

    if (!gr)
	gr = getgrgid (atoi (grp));
    if (!gr) {
	akerrst = grp;
	return (int)NULL;
    }

    if (homehead == NULL)
	init_pw ();

    while (gp = *gr -> gr_mem++)
	for (hm = homehead; hm; hm = hm -> h_next)
	    if (!strcmp (hm -> h_name, gp)) {
		add_aka (ak, hm -> h_name);
		break;
	    }

    return 1;
}

/*  */

static	int addmember (register struct aka *ak,
		       register char   *grp)
{
    int     gid;
    register struct group  *gr = getgrnam (grp);
    register struct home   *hm = NULL;

    if (gr)
	gid = gr -> gr_gid;
    else {
	gid = atoi (grp);
	gr = getgrgid (gid);
    }
    if (!gr) {
	akerrst = grp;
	return (int)NULL;
    }

    if (homehead == NULL)
	init_pw ();

    for (hm = homehead; hm; hm = hm -> h_next)
	if (hm -> h_gid == gid)
	    add_aka (ak, hm -> h_name);

    return 1;
}

/*  */

static	int addall (register struct aka *ak)
{
    int     noshell = NoShell == NULLCP || *NoShell == (char)NULL;
    register struct home   *hm;

    if (homehead == NULL)
	init_pw ();
    if (Everyone < 0)
	Everyone = EVERYONE;

    for (hm = homehead; hm; hm = hm -> h_next)
	if (hm -> h_uid > Everyone
		&& (noshell || strcmp (hm -> h_shell, NoShell)))
	    add_aka (ak, hm -> h_name);

    return homehead != NULL;
}

/*  */

static char   *getalias (register char *addrs)
{
    register char  *pp,
                   *qp;
    static char *cp = NULL;

    if (cp == NULL)
	cp = addrs;
    else
	if (*cp == (char)NULL)
	    return (cp = NULL);

    for (pp = cp; isspace (*pp); pp++)
	continue;
    if (*pp == (char)NULL)
	return (cp = NULL);
    for (qp = pp; *qp != (char)NULL && *qp != ','; qp++)
	continue;
    if (*qp == ',')
	*qp++ = (char)NULL;
    for (cp = qp, qp--; qp > pp; qp--)
	if (*qp != (char)NULL)
	    if (isspace (*qp))
		*qp = (char)NULL;
	    else
		break;

    return pp;
}

/*  */

static	add_aka (register struct aka *ak,
		 register char   *pp)
{
    register struct adr *ad,
			*ld;

    for (ad = ak -> ak_addr, ld = NULL; ad; ld = ad, ad = ad -> ad_next)
	if (!strcmp (pp, ad -> ad_text))
	    return;

    ad = (struct adr   *) malloc (sizeof *ad);
    if (ad == NULL)
	return;
    ad -> ad_text = getcpy (pp);
    ad -> ad_local = index (pp, '@') == (char)NULL && index (pp, '!') == (char)NULL;
    ad -> ad_next = NULL;
    if (ak -> ak_addr)
	ld -> ad_next = ad;
    else
	ak -> ak_addr = ad;
}


init_pw () {
    register struct passwd  *pw;

    (void) setpwent ();

    while (pw = getpwent ())
	if (!hmalloc (pw))
	    break;

    (void) endpwent ();
}

/*  */

static struct aka *akalloc (register char   *id)
{
    register struct aka *p = (struct aka   *) malloc (sizeof *p);

    if (!p)
	return NULL;

    p -> ak_name = getcpy (id);
    p -> ak_visible = 0;
    p -> ak_addr = NULL;
    p -> ak_next = NULL;
    if (akatail != NULL)
	akatail -> ak_next = p;
    if (akahead == NULL)
	akahead = p;
    akatail = p;

    return p;
}


static struct home *hmalloc (struct passwd *pw)
{
    register struct home   *p = (struct home   *) malloc (sizeof *p);

    if (!p)
	return NULL;

    p -> h_name = getcpy (pw -> pw_name);
    p -> h_uid = pw -> pw_uid;
    p -> h_gid = pw -> pw_gid;
    p -> h_home = getcpy (pw -> pw_dir);
    p -> h_shell = getcpy (pw -> pw_shell);
#ifdef	BSD42
    p -> h_ngrps = 0;
#endif	BSD42
    p -> h_next = NULL;
    if (hometail != NULL)
	hometail -> h_next = p;
    if (homehead == NULL)
	homehead = p;
    hometail = p;

    return p;
}

/*  */

#ifndef	MMDFMTS
struct home *seek_home (name)
register char   *name;
{
    register struct home *hp;

    if (homehead == NULL)
	init_pw ();

    for (hp = homehead; hp; hp = hp -> h_next)
	if (uleq (name, hp -> h_name))
	    return hp;
	
    return NULL;
}
#endif	MMDFMTS
