static char sccsid[] = "@(#)78	1.3  src/bos/usr/bin/uucp/uucpadm/strfcpy.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:35";
/* 
 * COMPONENT_NAME: CMDUUCP strfcpy.c
 * 
 * FUNCTIONS: strfcpy 
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

extern char  File[FILES][MAXFILE];

int derror();

int strfcpy (target,dest, src)
struct files *target;
register char  *dest, *src;
{
    register  int  c;

    do
    {
	c = *src; src++;
	*dest = c; dest++;
    } while (c != 0xff);

    if (src - File[target->spot] != target->length)
    {
	derror (EX_SOFTWARE, "strfcpy: buffer sync error");
    }

    return (EX_OK);
}
