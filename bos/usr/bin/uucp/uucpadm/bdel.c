static char sccsid[] = "@(#)49	1.3  src/bos/usr/bin/uucp/uucpadm/bdel.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:22";
/* 
 * COMPONENT_NAME: CMDUUCP bdel.c
 * 
 * FUNCTIONS: bdel 
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
 *  bdel - delete string in file buffer.
 */
#include "defs.h"
#include <string.h>
#include <stdio.h>

int bdel (target,l)
struct files *target;
char  *l;
{
    int  len;

    len = strlen (l) + 1;

    (void) strfcpy (target,l, l + len);

    target->length -= len;

    return (EX_OK);
}
