static char sccsid[] = "@(#)92	1.1  src/bos/usr/ccs/lib/libmi/strecat.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:33";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** strecat.c 2.1, last change 11/14/90
 **/


char *
strecat (dst, src)
register char	* dst;
register char	* src;
{
	while (*dst++)
		;
	for(--dst; *dst++ = *src++; )
		;
	return &dst[-1];
}
