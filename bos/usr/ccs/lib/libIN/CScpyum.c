static char sccsid[] = "@(#)19	1.7  src/bos/usr/ccs/lib/libIN/CScpyum.c, libIN, bos411, 9428A410j 6/10/91 10:52:06";
/*
 * LIBIN: CScpyum
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
 * CScpyum(dst, src, term, cnt) copies up to cnt chars of string at src to dst
 * until a NULL or term is encountered, whichever comes first.  A terminating
 * NULL is then copied.  If cnt is <= 0, only the NULL is copied.

 * RETURN VALUE DESCRIPTION: 
 * If dst is NULL or src is NULL, dst is returned and nothing is copied.
 * Otherwise, the address of the NULL terminating dst is returned.  Dst must be
 * able to receive cnt+1 chars.
 */
#include <stdlib.h>
#include <sys/types.h>

char *
CScpyum(dst, src, term, cnt)
register char *dst, *src;
register wchar_t term;
register int cnt;
{
	char c;
	wchar_t wc;
	int mb_cur_max = MB_CUR_MAX;
	int n;	/* number of bytes in current character */

	if (dst && src) {
		if (mb_cur_max == 1) {
			wctomb(&c, term);
			while (cnt > 0 && *src != (char)NULL && *src != c) {
				*dst++ = *src++;
				cnt--;
			}
		}
		else
			for (;cnt > 0; cnt--) {
				n = mbtowc(&wc, src, mb_cur_max);
				/* copy one character */
				if (wc != term && *src != (char)NULL)
					for (;n > 0; --n) {
						*dst++ = *src++;
					}
				else
					break;
			}
		*dst = 0;
	}
	return dst;
}
