static char sccsid[] = "@(#)52	1.4  src/bos/usr/bin/uucp/uucpadm/bload.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:36";
/* 
 * COMPONENT_NAME: CMDUUCP bload.c
 * 
 * FUNCTIONS: bload 
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
 *  Copyright (c) 1987 IBM Corporation.  All rights reserved.
 *
 *
 */

/*
 *  bload - load the target->spot and process to internal format.
 *
 *  The internal format is as follows:  Each line of the input
 *  target->spot becomes a separate null terminated string.  This is
 *  accomplished simply by changing all newlines to nulls.
 *  The last null is followed by 0xff to mark the end of all strings.
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"

long lseek ();
/*void perror ();*/
extern char  File[FILES][MAXFILE];

int bload (target)
struct files *target;
{
    register int    i;
             int    err;
    register char  *p;
	char c2;
	c2 = '\0';

    /*
     *  Load the configuration target->spot.  The buffer is supposed to be initialized
     *  to zero by the system.
     */
    (void) lseek (target->fd, 0L, 0);

    if ((err = read (target->fd, File[target->spot], MAXFILE)) < 0)
        return (EX_IOERR);
    target->length = err;

    /*
     *  Must NOT fill the buffer.  This shows we got all the data.
     *  It also leaves room for a possible newline to be added, plus
     *  the 0xff multistring terminator.
     */
    if (target->length > MAXFILE - 2)
	return (EX_SOFTWARE);
	
    /*
     *  Make sure last line ends with newline, and add superterminator.
     */
    if (target->length == 0 || File[target->spot][target->length - 1] != '\n')
 /* last line has no newline? */
	File[target->spot][target->length++] = '\n';		/* add one */

    File[target->spot][target->length++] = 0xff;

    /*
     *  Make sure that no nulls and 0xff superterminators are part of user
     *  data.  Change newlines encountered to nulls.
     */
    p = File[target->spot];
    i = target->length - 1;			/* don't look at final 0xff */
    while (i-- > 0)
    {
	register int  c;

	c = *p;

	if (c == 0xff || c == '\0')	/* don't allow illegal metachars */
	    return (EX_SOFTWARE);

    if (c == '\n' && ((strchr(target->cont,(int) c2) == NULL) || c2 == '\0'))		/* replace newlines with null */
          *p = '\0';

	c2 = *p;

	p++;				/* to next char */
    }

    return (EX_OK);
}
