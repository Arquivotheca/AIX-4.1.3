static char sccsid[] = "@(#)12	1.6  src/bos/usr/ccs/lib/libIN/CScmp.c, libIN, bos411, 9428A410j 6/10/91 10:14:20";
/*
 * LIBIN: CScmp
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

CScmp(s1, s2)
	register char *s1, *s2; {

	if (s1 && s2) {
		while (*s1 == *s2++)
			if (!*s1++)
				return 0;
		return (*s1 > *--s2)? 1: -1;
	} else  return 0;
}
