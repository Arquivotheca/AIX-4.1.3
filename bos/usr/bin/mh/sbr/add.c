static char sccsid[] = "@(#)31	1.6  src/bos/usr/bin/mh/sbr/add.c, cmdmh, bos411, 9428A410j 3/27/91 17:43:46";
/* 
 * COMPONENT_NAME: CMDMH add.c
 * 
 * FUNCTIONS: MSGSTR, add 
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
/* static char sccsid[] = "add.c	7.1 87/10/13 17:02:22"; */

/* add.c - concatenate two strings in managed memory */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

char   *add (this, that)
register char  *this,
               *that;
{
    register char  *cp;

    if (!this)
	this = "";
    if (!that)
	that = "";
    if ((cp = (char *)malloc ((unsigned) (strlen (this) + strlen (that) + 1))) == NULL)
	adios (NULLCP, MSGSTR(NOSTOR, "unable to allocate string storage")); /*MSG*/

    (void) sprintf (cp, "%s%s", that, this);
    if (*that)
	free (that);
    return cp;
}
