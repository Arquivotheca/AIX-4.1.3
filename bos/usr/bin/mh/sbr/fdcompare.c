static char sccsid[] = "@(#)51	1.3  src/bos/usr/bin/mh/sbr/fdcompare.c, cmdmh, bos411, 9428A410j 6/15/90 22:11:57";
/* 
 * COMPONENT_NAME: CMDMH fdcompare.c
 * 
 * FUNCTIONS: fdcompare 
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


/* fdcompare.c - are two files identical? */

#include "mh.h"
#include <stdio.h>

long lseek();


fdcompare (fd1, fd2)
register int fd1,
	     fd2;
{
    register int    i,
		    n1,
                    n2,
                    resp;
    register char  *c1,
                   *c2;
    char    b1[BUFSIZ],
            b2[BUFSIZ];

    resp = 1;
    while ((n1 = read (fd1, b1, sizeof b1)) >= 0
	    && (n2 = read (fd2, b2, sizeof b2)) >= 0
	    && n1 == n2) {
	c1 = b1;
	c2 = b2;
	for (i = n1 < sizeof b1 ? n1 : sizeof b1; i--;)
	    if (*c1++ != *c2++) {
		resp = 0;
		goto leave;
	    }
	if (n1 < sizeof b1)
	    goto leave;
    }
    resp = 0;

leave: ;
    (void) lseek (fd1, 0L, 0);
    (void) lseek (fd2, 0L, 0);
    return resp;
}
