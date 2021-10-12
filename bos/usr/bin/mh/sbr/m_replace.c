static char sccsid[] = "@(#)80	1.5  src/bos/usr/bin/mh/sbr/m_replace.c, cmdmh, bos411, 9428A410j 3/27/91 17:50:52";
/* 
 * COMPONENT_NAME: CMDMH m_replace.c
 * 
 * FUNCTIONS: MSGSTR, m_replace 
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
/* static char sccsid[] = "m_replace.c	7.1 87/10/13 17:11:07"; */

/* m_replace.c - replace an entry in the profile */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


void m_replace (key, value)
register char  *key,
               *value;
{
    register struct node   *np;
    char hlds[NL_TEXTMAX];

    m_getdefs ();
    if (m_defs == NULL) {
	np = m_defs = (struct node *) malloc (sizeof *np);
	if (np == NULL)
	    adios (NULLCP, MSGSTR(NOAPSTOR, "unable to allocate profile storage")); /*MSG*/

	np -> n_name = getcpy (key);
	np -> n_field = getcpy (value);
	np -> n_context = 1;
	np -> n_next = NULL;
	ctxflags |= CTXMOD;
	return;
    }

    for (np = m_defs;; np = np -> n_next) {
	if (uleq (np -> n_name, key)) {
	    if (strcmp (value, np -> n_field) != 0) {
		if (!np -> n_context) {
		    sprintf (hlds, MSGSTR(BUG2, "bug: m_replace(key=\"%s\",value=\"%s\")")); /*MSG*/
		    admonish (NULLCP, hlds, key, value);
		}
		if (np -> n_field)
		    free (np -> n_field);
		np -> n_field = getcpy (value);
		ctxflags |= CTXMOD;
	    }
	    return;
	}
	if (!np -> n_next)
	    break;
    }
    np -> n_next = (struct node *) malloc (sizeof *np);
    if (np -> n_next == NULL)
	adios (NULLCP, MSGSTR(NOAPSTOR, "unable to allocate profile storage")); /*MSG*/

    np = np -> n_next;
    np -> n_name = getcpy (key);
    np -> n_field = getcpy (value);
    np -> n_context = 1;
    np -> n_next = NULL;
    ctxflags |= CTXMOD;
}
