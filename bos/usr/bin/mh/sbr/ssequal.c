static char sccsid[] = "@(#)11	1.2  src/bos/usr/bin/mh/sbr/ssequal.c, cmdmh, bos411, 9428A410j 6/15/90 22:15:31";
/* 
 * COMPONENT_NAME: CMDMH ssequal.c
 * 
 * FUNCTIONS: ssequal 
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


/* ssequal.c - initially equal? */


ssequal (substr, str)
register char  *substr,
               *str;
{
    if (!substr)
	substr = "";
    if (!str)
	str = "";

    while (*substr)
	if (*substr++ != *str++)
	    return 0;
    return 1;
}
