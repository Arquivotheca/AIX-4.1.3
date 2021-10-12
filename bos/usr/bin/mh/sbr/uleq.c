static char sccsid[] = "@(#)14	1.2  src/bos/usr/bin/mh/sbr/uleq.c, cmdmh, bos411, 9428A410j 6/15/90 22:15:42";
/* 
 * COMPONENT_NAME: CMDMH uleq.c
 * 
 * FUNCTIONS: uleq 
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


/* uleq.c - "unsigned" lexical compare */


uleq (c1, c2)
register char  *c1,
               *c2;
{
    register int    c;

    if (!c1)
	c1 = "";
    if (!c2)
	c2 = "";

    while (c = *c1++)
	if ((c | 040) != (*c2 | 040))
	    return 0;
	else
	    c2++;
    return (*c2 == 0);
}
