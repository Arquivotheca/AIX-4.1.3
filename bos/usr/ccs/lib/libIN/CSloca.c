static char sccsid[] = "@(#)82	1.6  src/bos/usr/ccs/lib/libIN/CSloca.c, libIN, bos411, 9428A410j 6/10/91 10:52:25";
/*
 * LIBIN: CSloca
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
 * CSloca scans str until any of the chars in set or a NULL is encountered.
 *
 * RETURN VALUE DESCRIPTION: 
 * If str is NULL or set is NULL, str is returned.  Otherwise, the address in
 * str of the char found is returned; if no char was found, the address
 * of the terminating NULL is returned.  If any of the characters in str or set
 * are not legal multibyte characters, then str is returned.
 */

#include <stdlib.h>
#include <sys/types.h>

char *
CSloca(str, set)
register char *str, *set;
{
	register char *oldstr;
	register char *srchset;
	wchar_t wc;
	wchar_t ch;
	int wccount, chcount;
	int mb_cur_max = MB_CUR_MAX;

	if (str && set) {
		oldstr = str;
		for (;;) {
			wccount = mbtowc(&wc, str, mb_cur_max);
			if (wccount == (int)-1)  /* illegal character ? */
				return (oldstr);
			if (wc == (wchar_t)NULL) /* end of string ? */
				return (str);
			srchset = set;
			for (;;) {
				chcount = mbtowc(&ch, srchset, mb_cur_max);
				if (chcount == (int)-1)  /* illegal char ? */
					return (oldstr);
				if (ch == (wchar_t)NULL) /* end of set ? */
				    break;
				if (wc == ch)
				    return (str);
				srchset += chcount;
			}
			str += wccount;
		}
	}
	return str;
}
