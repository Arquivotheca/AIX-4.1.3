static char sccsid[] = "@(#)94	1.1  src/bos/usr/ccs/lib/libmi/strtohex.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:37";
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
 ** strtohex.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>

	boolean	strtohex(   char * to, char * from, int len   );

static int
tohex (ch)
	char	ch;
{
	if ( ch >= '0'  &&  ch <= '9' )
		return ch - '0';
	if ( ch >= 'a'  &&  ch <= 'f' )
		return 10 + ch - 'a';
	if ( ch >= 'A'  &&  ch <= 'F' )
		return 10 + ch - 'A';
	return -1;
}

boolean
strtohex (to, from, len)
	char	* to;
	char	* from;
	int	len;
{
	int	i1;
	
	if ( strncmp(from, "0x", 2) == 0 )
		from += 2;
	while ( len-- ) {
		if ( (i1 = tohex(*from++)) == -1 )
			return false;
		*to = i1 << 4;
		if ( (i1 = tohex(*from++)) == -1 )
			return false;
		*to++ |= i1;
	}
	return true;
}
