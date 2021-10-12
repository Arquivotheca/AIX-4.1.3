static char sccsid[] = "@(#)56	1.3  src/bos/usr/bin/uucp/uucpadm/bupd.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:52";
/* 
 * COMPONENT_NAME: CMDUUCP bupd.c
 * 
 * FUNCTIONS: bupd 
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
 *  bupd - replace string in file buffer.
 */
#include "defs.h"
#include <string.h>
#include <stdio.h>

int derror();

extern char File[FILES][MAXFILE];

int bupd (target,l, s)
struct files *target;
char  *s, *l;
{
    int  ls, ll, del;

    ls = strlen (s);
    ll = strlen (l);
    del = ls - ll;			/* chg in string size old to new */

    if (target->length + del > MAXFILE)
    {
	derror (EX_SOFTWARE, "bupd: Out of space");
    }

    /*
     *  If new length is <= current length of current line.
     */
    if (del <= 0)
    {
        (void) strcpy (l, s);		/* copy in the new one		*/

        if (del < 0)			/* shrink the file?		*/
	{
	    (void) strfcpy (target,l + ls + 1, l + ll + 1);
	}
    }
    else	/* new length is greater */
    {
        (void) strrfcpy (target,l + ls + 1, l + ll + 1);
	(void) strcpy (l, s);
    }

    target->length += del;

    return (EX_OK);
}
