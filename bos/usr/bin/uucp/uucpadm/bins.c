static char sccsid[] = "@(#)50	1.3  src/bos/usr/bin/uucp/uucpadm/bins.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:26";
/* 
 * COMPONENT_NAME: CMDUUCP bins.c
 * 
 * FUNCTIONS: bins 
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

/*
 *  bins - insert string in file buffer before current line.
 */
#include "defs.h"
#include <string.h>
#include <stdio.h>

int derror();

extern char File[FILES][MAXFILE];

int bins (target)
struct files *target;
{
    if (target->length + 1 > MAXFILE)
    {
	derror (EX_SOFTWARE, "bupd: Out of space on File");
    }

    File[target->spot][target->length - 1] = '\0';
    File[target->spot][target->length] = 0xff;
    target->length += 1;
    return (EX_OK);
}
