static char sccsid[] = "@(#)67	1.3  src/bos/usr/bin/mh/sbr/m_find.c, cmdmh, bos411, 9428A410j 6/15/90 22:12:54";
/* 
 * COMPONENT_NAME: CMDMH m_find.c
 * 
 * FUNCTIONS: m_find 
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


/* m_find.c - find an entry in the profile */

#include "mh.h"
#include <stdio.h>


char   *m_find (str)
register char  *str;
{
    register struct node   *np;

    m_getdefs ();
    for (np = m_defs; np; np = np -> n_next)
	if (uleq (np -> n_name, str))
	    return (np -> n_field);

    return NULL;
}
