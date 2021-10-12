static char sccsid[] = "@(#)64	1.5  src/bos/usr/bin/mh/sbr/m_delete.c, cmdmh, bos411, 9428A410j 3/27/91 17:48:15";
/* 
 * COMPONENT_NAME: CMDMH m_delete.c
 * 
 * FUNCTIONS: MSGSTR, m_delete 
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
/* static char sccsid[] = "m_delete.c	7.1 87/10/13 17:07:35"; */

/* m_delete.c - delete an entry from the profile */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


m_delete (key)
register char  *key;
{
    register struct node   *np,
                           *pp;

    m_getdefs ();
    for (np = m_defs, pp = NULL; np; pp = np, np = np -> n_next) {
	if (uleq (np -> n_name, key)) {
	    if (!np -> n_context)
		admonish (NULLCP, MSGSTR(BUG, "bug: m_delete(key=\"%s\")"), np -> n_name); /*MSG*/
	    if (pp)
		pp -> n_next = np -> n_next;
	    else
		m_defs = np -> n_next;
	    free (np -> n_name);
	    if (np -> n_field)
		free (np -> n_field);
	    free ((char *) np);
	    ctxflags |= CTXMOD;
	    return 0;
	}
    }

    return 1;
}
