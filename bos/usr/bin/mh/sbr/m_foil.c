static char sccsid[] = "@(#)69	1.5  src/bos/usr/bin/mh/sbr/m_foil.c, cmdmh, bos411, 9428A410j 3/27/91 17:48:20";
/* 
 * COMPONENT_NAME: CMDMH m_foil.c
 * 
 * FUNCTIONS: MSGSTR, m_foil 
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
/* static char sccsid[] = "m_foil.c	7.1 87/10/13 17:08:24"; */

/* m_foil.c - foil search of .mh_profile */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


void m_foil (path)
char   *path;
{
    register struct node *np;

    defpath = context = "/dev/null";

    if (path) {
	np = m_defs = (struct node *) malloc (sizeof *np);
	if (np == NULL)
	    adios (NULLCP, MSGSTR(NOAPSTOR, "unable to allocate profile storage")); /*MSG*/

	np -> n_name = getcpy ("Path");
	np -> n_field = getcpy (path);
	np -> n_context = 0;
	np -> n_next = NULL;
    }
}
