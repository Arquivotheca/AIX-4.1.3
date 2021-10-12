static char sccsid[] = "@(#)17	1.6  src/bos/usr/ccs/lib/libIN/CSloct.c, libIN, bos411, 9428A410j 6/10/91 10:14:55";
/*
 * LIBIN: CSloct
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 *
 * RETURN VALUE DESCRIPTION: 
 */

unsigned char *
CSloct(str, tbl, mask)
	register unsigned char *str, *tbl;
	register mask; {

	if (str)
		while (*str && (tbl[*str] & mask) == 0)
			str++;
	return str;
}
