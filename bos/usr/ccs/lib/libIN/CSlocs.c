static char sccsid[] = "@(#)06	1.7  src/bos/usr/ccs/lib/libIN/CSlocs.c, libIN, bos411, 9428A410j 6/10/91 10:52:38";
/*
 * LIBIN: CSlocs
 *
 * ORIGIN: 9,10
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
 * CSlocs scans str until substr or a NULL is encountered.
 *
 * RETURN VALUE DESCRIPTION: 
 * If str is NULL or substr is NULL, str is returned.  Otherwise, the address in
 * str of the first occurrence of substr is returned; if substr was not found,
 * the address of the terminating NULL is returned.  Note: "" is a substring of
 * any string, and will cause str to be returned.
 */

 #include <stdlib.h>

char *
CSlocs(str, pat)
register char *str, *pat;
{
	if (str)
		while (*str) {
			if (CScmpp(pat, str) == 0)
				break;
			str += mblen(str, MB_CUR_MAX);
		}
	return str;
}
