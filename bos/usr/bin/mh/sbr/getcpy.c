static char sccsid[] = "@(#)59	1.6  src/bos/usr/bin/mh/sbr/getcpy.c, cmdmh, bos411, 9428A410j 3/27/91 17:44:31";
/* 
 * COMPONENT_NAME: CMDMH getcpy.c
 * 
 * FUNCTIONS: MSGSTR, getcpy 
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
/* static char sccsid[] = "getcpy.c	7.1 87/10/13 17:06:36"; */

/* getcpy.c - copy a string in managed memory */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


char   *getcpy (str)
register char  *str;
{
    register char  *cp;

    if ((cp = (char *)malloc ((unsigned) (strlen (str) + 1))) == NULL)
	adios (NULLCP, MSGSTR(NOSTOR, "unable to allocate string storage")); /*MSG*/

    (void) strcpy (cp, str);
    return cp;
}
