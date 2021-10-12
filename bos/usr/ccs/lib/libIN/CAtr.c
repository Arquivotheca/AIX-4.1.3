static char sccsid[] = "@(#)10	1.6  src/bos/usr/ccs/lib/libIN/CAtr.c, libIN, bos411, 9428A410j 6/10/91 10:14:13";
/*
 * LIBIN: CAtr
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
CAtr(dst, tab, cnt)
	register unsigned char *dst, *tab;
	register cnt; {

	if (dst)
		while (cnt > 0) {
			*dst = tab[*dst];
			++dst;
			--cnt;
		}
	return dst;
}
