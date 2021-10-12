static char sccsid[] = "@(#)27        1.5  src/bos/usr/bin/mh/mts/hosts.c, cmdmh, bos411, 9428A410j 10/4/90 15:40:48";
/* 
 * COMPONENT_NAME: CMDMH hosts.c
 * 
 * FUNCTIONS: OfficialName, init_hs 
 *
 * ORIGINS: 10  26  28  35 
 *
 */


/* hosts.c - find out the official name of a host */

/* LINTLIBRARY */

/* In the SendMail world, we really don't know what the valid hosts are.
   We could poke around in the sendmail.cf file, but that still isn't a
   guarantee.  As a result, we'll say that everything is a valid host, and
   let SendMail worry about it. */


#include "strings.h"
#include <stdio.h>
#include "mts.h"
#include <ctype.h>
#ifdef	BSD42
#include <netdb.h>
#endif	BSD42


#define	NOTOK	(-1)


static struct host {
    char   *h_name;
    char  **h_aliases;
    struct host   *h_next;
}                   hosts;

char   *getcpy ();



static int init_hs();

/*  */

char   *OfficialName (name)
register char   *name;
{
    register char  *p;
    char   *q,
            site[BUFSIZ];
#ifdef	BSD42
    register struct hostent *hp;
#endif	BSD42
    static char buffer[BUFSIZ];
    register char **r;
    register struct host   *h;

    for (p = name, q = site; *p; p++, q++)
	*q = isupper (*p) ? tolower (*p) : *p;
    *q = (int)NULL;
    q = site;

    if (uleq (LocalName (), site))
	return LocalName ();

#ifdef	BSD41A
    if (rhost (&q) != NOTOK) {
	(void) strcpy (buffer, q);
	free (q);
	return buffer;
    }
#endif	BSD41A
#ifdef	BSD42
    sethostent (1);
    if (hp = gethostbyname (q)) {
	(void) strcpy (buffer, hp -> h_name);
	return buffer;
    }
#endif	BSD42

    if (hosts.h_name || init_hs ())
	for (h = hosts.h_next; h; h = h -> h_next)
	    if (uleq (h -> h_name, q))
		return h -> h_name;
	    else
		for (r = h -> h_aliases; *r; r++)
		    if (uleq (*r, q))
			return h -> h_name;

    (void) strcpy (buffer, site);
    return buffer;
}

/*  */

/* Use hostable as an exception file for those hosts that aren't on the
   Internet (listed in /etc/hosts).  These are usually PhoneNet and UUCP
   sites. */


#define	NALIASES	50

static int  init_hs () {
    register char  *cp,
                   *dp,
                  **q,
                  **r;
    char    buffer[BUFSIZ],
           *aliases[NALIASES];
    register struct host   *h;
    register FILE  *fp;

    if ((fp = fopen (hostable, "r")) == NULL)
	return 0;

    h = &hosts;
    while (fgets (buffer, sizeof buffer, fp) != NULL) {
	if (cp = index (buffer, '#'))
	    *cp = (int)NULL;
	if (cp = index (buffer, '\n'))
	    *cp = (int)NULL;
	for (cp = buffer; *cp; cp++)
	    if (isspace (*cp))
		*cp = ' ';
	for (cp = buffer; isspace (*cp); cp++)
	    continue;
	if (*cp == (int)NULL)
	    continue;

	q = aliases;
	if (cp = index (dp = cp, ' ')) {
	    *cp = (int)NULL;
	    for (cp++; *cp; cp++) {
		while (isspace (*cp))
		    cp++;
		if (*cp == (int)NULL)
		    break;
		if (cp = index (*q++ = cp, ' '))
		    *cp = (int)NULL;
		else
		    break;
		if (q >= aliases + NALIASES)
		    break;
	    }
	}

	*q = NULL;

	h -> h_next = (struct host *) calloc (1, sizeof *h);
	h = h -> h_next;
	h -> h_name = getcpy (dp);
	r = h -> h_aliases =
		(char **) calloc ((unsigned) (q - aliases + 1), sizeof *q);
	for (q = aliases; *q; q++)
	    *r++ = getcpy (*q);
	*r = NULL;
    }

    (void) fclose (fp);
    return 1;
}
