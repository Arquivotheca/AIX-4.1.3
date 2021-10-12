static char sccsid[] = "@(#)95	1.6  src/bos/usr/ccs/lib/libIN/CSlocc.c, libIN, bos411, 9428A410j 6/10/91 10:52:30";
/*
 * LIBIN: CSlocc
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
 * CSlocc scans str until chr or a NULL is encountered.
 *
 * RETURN VALUE DESCRIPTION: 
 * If str is NULL, NULL is returned.  Otherwise, the address in str of chr is
 * returned; if chr was not found, the address of the terminating NULL is
 * returned.  If any of the characters in str are not legal multibyte
 * characters, then str is returned.
 */

#include <stdlib.h>
#include <sys/types.h>

char *
CSlocc(str, chr)
register char *str;
register wchar_t chr;
{
	char mbchr;
	int mbcount;
	char *oldstr;
	wchar_t wc;
	int mb_cur_max = MB_CUR_MAX;

	if (str) {
		oldstr = str;
		if (mb_cur_max == 1) {
			wctomb(&mbchr, chr);
			while (*str) {
				if (*str == mbchr)
					break;
				str++;
			}
		} else {
			while (*str) {
				mbcount = mbtowc(&wc, str, mb_cur_max);
				if (mbcount == (int)-1)
					return(oldstr);
				if (wc == chr)
					break;
				str += mbcount;
			}
		}
	}
	return str;
}
