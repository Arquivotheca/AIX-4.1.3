static char sccsid[] = "@(#)86	1.1  src/bos/usr/ccs/lib/libmi/hextostr.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:22";
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
 ** hextostr.c 2.1, last change 11/14/90
 **/


static	char	hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	char	* hextostr(   char * str, char * cp, int len   );

char	*
hextostr (str, cp, len)
	char	* str;
	char	* cp;
	int	len;
{
	char	* cp1 = str;
	
	while ( len-- ) {
		*cp1++ = hex[(*cp >> 4) & 0xF];
		*cp1++ = hex[*cp++ & 0xF];
	}
	*cp1 = '\0';
	return str;
}
