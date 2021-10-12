static char sccsid[] = "@(#)79	1.3  src/bos/usr/bin/uucp/uucpadm/strrfcpy.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:39";
/* 
 * COMPONENT_NAME: CMDUUCP strrfcpy.c
 * 
 * FUNCTIONS: strrfcpy 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  Uucpadm
 *
 *  Copyright (c) 1988 IBM Corporation.  All rights reserved.
 *
 *
 */

#include <stdio.h>
#include "defs.h"

int derror();

extern  char  File[FILES][MAXFILE];

int strrfcpy (target,dest,src)
struct files *target;
register char  *dest, *src;
{
    register  int  c;
    register  char  *p;

    p = src;			/* save origin of source	*/

    src = File[target->spot] + target->length - 1;
		/* push to end of file		*/

    c = src - p;
    dest += src - p;		/* update dest ptr same distance */

    if (*src != 0xff)
    {
	derror (EX_SOFTWARE, "strrfcpy: buffer sync error");
    }

    *dest = 0xff;			/* begin loop to copy string backwords*/
    while (src != p)
    {
	src--;
	c = *src;
        dest--;
	*dest = c;
    }

    return (EX_OK);
}
