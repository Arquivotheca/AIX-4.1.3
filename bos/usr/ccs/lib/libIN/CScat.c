static char sccsid[] = "@(#)11	1.6  src/bos/usr/ccs/lib/libIN/CScat.c, libIN, bos411, 9428A410j 6/10/91 10:14:16";
/*
 * LIBIN: CScat
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

char *
CScat(dst, src)
	register char *dst;
	char *src; {
	register char **argp, *from;

	if (dst && src) {
		for (argp = &src; from = *argp++; ) {
			while (*dst++ = *from++)
				;
			--dst;
		}
		*dst = 0;
	}
	return dst;
}
