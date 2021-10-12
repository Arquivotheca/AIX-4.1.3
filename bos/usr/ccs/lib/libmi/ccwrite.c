static char sccsid[] = "@(#)76	1.1  src/bos/usr/ccs/lib/libmi/ccwrite.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:01";
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
 ** ccwrite.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>

/* 'C' style character write. */
ccwrite (str, ch)
	char	* str;
reg	int	ch;
{
	switch (ch) {
	case '\n':
		ch = 'n';
		break;
	case '\r':
		ch = 'r';
		break;
	case '\t':
		ch = 't';
		break;
	case '\033':
		ch = 'E';
		break;
	case '\v':
		ch = 'v';
		break;
	case '\b':
		ch = 'b';
		break;
	default:
		if (!isprint(ch)) {
			char	tbuf[5];

			(void)sprintf(tbuf, "\\%03o", ch);
			(void)memcpy(str, tbuf, 4);
		} else
			*str = ch;
		return ch;
	}
	str[0] = '\\';
	str[1] = ch;
	return ch;
}
