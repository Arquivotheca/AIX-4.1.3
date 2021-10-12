static char sccsid[] = "@(#)13	1.4  src/bos/usr/bin/mh/sbr/trimcpy.c, cmdmh, bos411, 9428A410j 10/10/90 16:18:52";
/* 
 * COMPONENT_NAME: CMDMH trimcpy.c
 * 
 * FUNCTIONS: trimcpy 
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


/* trimcpy.c - strip [lt]wsp and replace newlines with spaces */

#include "mh.h"
#include <ctype.h>
#include <stdio.h>


char *trimcpy (cp)
register char *cp;
{
    register char  *sp;

    while (isspace (*cp))
	cp++;
    for (sp = cp + strlen (cp) - 1; sp >= cp; sp--)
	if (isspace (*sp))
	    *sp = (char)NULL;
	else
	    break;
    for (sp = cp; *sp; sp++)
	if (isspace (*sp))
	    *sp = ' ';

    return getcpy (cp);
}
