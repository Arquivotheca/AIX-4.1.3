static char sccsid[] = "@(#)45	1.2  src/bos/usr/bin/mh/uip/bzero.c, cmdmh, bos411, 9428A410j 6/15/90 22:17:42";
/* 
 * COMPONENT_NAME: CMDMH bzero.c
 * 
 * FUNCTIONS: bzero 
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




#include	<memory.h>


bzero(s, n)
register char	*s;
register int	n;
{
	memset(s, '\0', n);
}

