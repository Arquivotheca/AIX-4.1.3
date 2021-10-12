static char sccsid[] = "@(#)58	1.6  src/bos/usr/ccs/lib/libIN/CSskpc.c, libIN, bos411, 9428A410j 6/10/91 10:52:55";
/*
 * LIBIN: CSskpc
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
 * CSskpc scans str until a NULL or a character other than chr is encountered.
 *
 * RETURN VALUE DESCRIPTION: 
 * If str is NULL, NULL is returned.  Otherwise, the address in str of the
 * character (which is not chr) is returned; if str consists entirely of chrs,
 * the address of the terminating NULL is returned.
 */

#include <stdlib.h>
#include <sys/types.h>

char *
CSskpc(str, chr)
register char *str;
register wchar_t chr;
{
	wchar_t wc;
	char mbchar;
	int strcount;
	int mb_cur_max = MB_CUR_MAX;

	if (str && chr) {
		if (mb_cur_max == 1) {
			wctomb(&mbchar, chr);
			while (*str)
				if (*str++ != mbchar)
					return (--str);
		} else {
			for (;*str != (char)NULL ;str += strcount) {
				strcount = mbtowc(&wc, str, mb_cur_max);
				if (wc != chr)
					return (str);
			}
		}
	}
	return str;
}
