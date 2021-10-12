static char sccsid[] = "@(#)54	1.2  src/bos/usr/bin/uucp/uucpadm/bstore.c, cmduucp, bos411, 9428A410j 6/16/90 00:04:51";
/* 
 * COMPONENT_NAME: UUCP bstore.c
 * 
 * FUNCTIONS: bstore 
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
 *  bstore - change file buffer back to legitimate string structure and
 *	     write the whole thing back.
 */

#include <stdio.h>
#include "defs.h"

long  lseek ();
int   derror ();

extern char  File[FILES][MAXFILE];

int bstore (target)
struct files *target;
{
    register int    i;
    register char  *p;

    /*
     *  Change embedded nulls back to newlines, and 0xff back to null.
     */
    if (*(File[target->spot] + target->length - 1) != 0xff)
    {
	derror (EX_SOFTWARE, "bstore: buffer sync error");
    }

    p = File[target->spot];
    while (1)
    {
	i = *p;

	if (i == 0xff)				/* end of all strings?	*/
	{
	    *p = '\0';				/* store null there */
	    break;
	}

        if (i == '\0')				/* chg null to newline */
	    *p = '\n';

	p++;
    }


    /*
     *  Write the file back.
     */
    (void) lseek (target->fd, 0L, 0);
    if (ftruncate (target->fd, 0))
    {
	derror (EX_IOERR, "bstore: Trouble truncating");
    }

    i = target->length - 1;

    if (write (target->fd, File[target->spot], (unsigned) i) != i)
    {
	derror (EX_IOERR, "bstore: Trouble writing back");
    }


    return (EX_OK);
}
