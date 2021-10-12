static char sccsid[] = "@(#)93	1.1  src/bos/usr/ccs/lib/libmi/strecpy.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:35";
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
 ** strecpy.c 2.1, last change 11/14/90
 **/


char *
strecpy (dst, src)
register char	* dst;
register char	* src;
{
	while (*dst++ = *src++)
		;
	return &dst[-1];
}
