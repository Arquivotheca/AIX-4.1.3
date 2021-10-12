static char sccsid[] = "@(#)38	1.9  src/bos/usr/bin/mh/uip/ali.c, cmdmh, bos411, 9428A410j 11/9/93 09:40:03";
/* 
 * COMPONENT_NAME: CMDMH ali.c
 * 
 * FUNCTIONS: MSGSTR, Mali, print_aka, print_usr 
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
/* static char sccsid[] = "ali.c	7.1 87/10/13 17:22:20"; */

/* ali.c - the new ali */

#include "mh.h"
#include "addrsbr.h"
#include "aliasbr.h"
#include <stdio.h>
#include <locale.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#define	NVEC	50		/* maximum number of names */

/*  */

static struct swit switches[] = {
#define	ALIASW	0
    "alias aliasfile", 0,
#define	NALIASW	1
    "noalias", -7,

#define	LISTSW	2
    "list", 0,
#define	NLISTSW	3
    "nolist", 0,

#define	NORMSW	4
    "normalize", 0,
#define	NNORMSW	5
    "nonormalize", 0,

#define	USERSW	6
    "user", 0,
#define	NUSERSW	7
    "nouser", 0,

#define	HELPSW	8
    "help", 4,

    NULL, (int)NULL
};

/*  */

static	int     pos = 1;

extern struct aka  *akahead;

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     i,
            vecp = 0,
	    inverted = 0,
            list = 0,
	    noalias = 0,
	    normalize = AD_NHST;
    char   *cp,
          **ap,
          **argp,
            buf[100],
           *vec[NVEC],
           *arguments[MAXARGS];
    char    hlds[NL_TEXTMAX];
    struct aka *ak;

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    mts_init (invo_name);
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(HELPSW1, "%s [switches] aliases ..."), invo_name); /*MSG*/
		    switches[ALIASW].sw = MSGSTR(ALIFILE, switches[ALIASW].sw);
		    help (buf, switches);
		    done (1);

		case ALIASW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((i = alias (cp)) != AK_OK) {
			strcpy(hlds, MSGSTR(ALIASERROR, "aliasing error in %s - %s")); /*MSG*/
			adios (NULLCP, hlds, cp, akerror (i));
		    }
		    continue;
		case NALIASW: 
		    noalias++;
		    continue;

		case LISTSW: 
		    list++;
		    continue;
		case NLISTSW: 
		    list = 0;
		    continue;

		case NORMSW: 
		    normalize = AD_HOST;
		    continue;
		case NNORMSW: 
		    normalize = AD_NHST;
		    continue;

		case USERSW: 
		    inverted++;
		    continue;
		case NUSERSW: 
		    inverted = 0;
		    continue;
	    }
	vec[vecp++] = cp;
    }

    if (!noalias)
	(void) alias (AliasFile);

/*  */

    if (vecp)
	for (i = 0; i < vecp; i++)
	    if (inverted)
		print_usr (vec[i], list, normalize);
	    else
		print_aka (akvalue (vec[i]), list, 0);
    else {
	if (inverted)
	    adios (NULLCP, MSGSTR(USAGE, "usage: %s addresses ...  (you forgot the addresses)"), invo_name); /*MSG*/

	for (ak = akahead; ak; ak = ak -> ak_next) {
	    printf ("%s: ", ak -> ak_name);
	    pos += strlen (ak -> ak_name) + 1;
	    print_aka (akresult (ak), list, pos);
	}
    }

    done (0);
}

/*  */

print_aka (p, list, margin)
register char  *p;
int     list,
        margin;
{
    register char   c;

    if (p == NULL) {
	printf (MSGSTR(EMPTY2, "<empty>\n")); /*MSG*/
	return;
    }

    while (c = *p++)
	switch (c) {
	    case ',': 
		if (*p)
		    if (list)
			printf ("\n%*s", margin, "");
		    else
			if (pos >= 68) {
			    printf (",\n ");
			    pos = 2;
			}
			else {
			    printf (", ");
			    pos += 2;
			}

	    case '\0': 
		break;

	    default: 
		pos++;
		(void) putchar (c);
	}

    (void) putchar ('\n');
    pos = 1;
}

/*  */

print_usr (s, list, norm)
register char  *s;
int     list,
        norm;
{
    register char  *cp,
                   *pp,
                   *vp;
    register struct aka *ak;
    register struct mailname   *mp,
                               *np;

    if ((pp = getname (s)) == NULL)
	adios (NULLCP, MSGSTR(NOADDRESS, "no address in \"%s\""), s); /*MSG*/
    if ((mp = getm (pp, NULLCP, 0, norm, NULLCP)) == NULL)
	adios (NULLCP, MSGSTR(BADADDRESS, "bad address \"%s\""), s); /*MSG*/
    while (getname (""))
	continue;

    vp = NULL;
    for (ak = akahead; ak; ak = ak -> ak_next) {
	pp = akresult (ak);
	while (cp = getname (pp)) {
	    if ((np = getm (cp, NULLCP, 0, norm, NULLCP)) == NULL)
		continue;
	    if (uleq (mp -> m_host, np -> m_host)
		    && uleq (mp -> m_mbox, np -> m_mbox)) {
		vp = vp ? add (ak -> ak_name, add (",", vp))
		    : getcpy (ak -> ak_name);
		mnfree (np);
		while (getname (""))
		    continue;
		break;
	    }
	    mnfree (np);
	}
    }
    mnfree (mp);

    printf ("%s: ", s);
    print_aka (vp ? vp : s, list, pos += strlen (s) + 1);
    if (vp)
	free (vp);
}
