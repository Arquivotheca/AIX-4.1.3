static char sccsid[] = "@(#)75	1.1  src/bos/usr/ccs/lib/libmi/ccread.c, cmdpse, bos411, 9428A410j 5/7/91 13:07:59";
/*
 *   COMPONENT_NAME: CMDPSE
 *
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
 ** ccread.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include <pse/clib.h>

/**'C' style character read from the string starting at *cpp.
** Standard 'C' character escapes are recognized and converted
** into their single character internal representations.
*/

char
ccread (cpp)
	char	** cpp;
{
	reg	char	* cp, ch;
		int	i1, cnt;

	cp = *cpp;
	ch = *cp++;
	if (ch != '\\') {
		*cpp = cp;
		return ch;
	}
	switch (ch = *cp++) {
	case 'n':
		ch = '\n';
		break;
	case 'r':
		ch = '\r';
		break;
	case 't':
		ch = '\t';
		break;
	case 'E':
		ch = '\033';
		break;
	case 'v':
		ch = '\v';
		break;
	case 'b':
		ch = '\b';
		break;
	default:
		if (ch != '0')	/* Return the current character */
			break;
		{
		char	num_buf[5], * cp1;
		int	base = 16;

		/* We return ch == 0 if the numeric char string
		** is bogus e.g. \0xyz yields 0 with the pointer
		** advanced to point at the 'x'.
		*/
		strncpy(num_buf, cp-1, 4);
		if (num_buf[1] == 'x' || num_buf[1] == 'X') {
			base = 16;
			num_buf[4] = '\0';
		} else {
			base = 8;
			num_buf[3] = '\0';
		}
		ch = (char)strtol(num_buf, &cp1, base);
		if (cp1 != num_buf)
			cp += ((cp1 - num_buf) - 1);
		}
		return ch;
	}
	*cpp = cp;
	return ch;
}
