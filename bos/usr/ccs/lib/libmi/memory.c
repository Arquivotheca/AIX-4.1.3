static char sccsid[] = "@(#)87	1.1  src/bos/usr/ccs/lib/libmi/memory.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:24";
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
 ** memory.c 2.1, last change 11/14/90
 **/


char *
memset (s, c, n)
	char	* s;
	int	c;
	int	n;
{
register char	* cp1;
register int	i1;
register int	ch;

	cp1 = s;
	ch = c;
	for (i1 = n; i1 > 8; i1 -= 8) {
		cp1[0] = ch;
		cp1[1] = ch;
		cp1[2] = ch;
		cp1[3] = ch;
		cp1[4] = ch;
		cp1[5] = ch;
		cp1[6] = ch;
		cp1[7] = ch;
		cp1 += 8;
	}
	if (i1 > 0) {
		do {
			*cp1++ = ch;
		} while (--i1);
	}
	return s;
}

char *
memcpy (s1, s2, n)
	char * s1;
	char * s2;
	int	n;
{
register char	* cp1, * cp2;
register int	i1;

	cp1 = s1;
	cp2 = s2;
	for (i1 = n; i1 >= 8; i1 -= 8) {
		cp1[0] = cp2[0];
		cp1[1] = cp2[1];
		cp1[2] = cp2[2];
		cp1[3] = cp2[3];
		cp1[4] = cp2[4];
		cp1[5] = cp2[5];
		cp1[6] = cp2[6];
		cp1[7] = cp2[7];
		cp1 += 8;
		cp2 += 8;
	}
	if (i1 > 0) {
		do {
			*cp1++ = *cp2++;
		} while (--i1);
	}
	return s1;
}
