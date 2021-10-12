static char sccsid[] = "@(#)18	1.7  src/bos/usr/ccs/lib/libIN/CScpyu.c, libIN, bos411, 9428A410j 6/10/91 10:52:02";
/*
 * LIBIN: CScpyu
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
 * CScpyu copies the string at src to dst until a NULL or the character
 * specified by term is encountered, whichever comes first.  A terminating NULL
 * is then copied.
 *
 * RETURN VALUE DESCRIPTION: 
 * If either dst or src is NULL, dst is returned and nothing is copied.
 * Otherwise the address of the NULL terminated dst is returned.
 */
#include <stdlib.h>
#include <sys/limits.h>
#include <sys/types.h>

char *
CScpyu(dst, src, term)
	register char *dst, *src;
	register wchar_t term;
{
	char mbstr[MB_LEN_MAX];
	char *pmb = mbstr;
	wchar_t wcstr;
	int n;
	int mb_cur_max = MB_CUR_MAX;

	if (src && dst) {
		if (mb_cur_max == 1) {
			wctomb(mbstr, term);
			while (*src != (char)NULL && *src != mbstr[0])
				*dst++ = *src++;
		}
		else
			for (;;) {
				n = mbtowc(&wcstr, src, MB_CUR_MAX);
				if (wcstr != term && *src != (char)NULL)
					for (;n > 0; --n) {
						*dst++ = *src++;
					}
				else
					break;
			}
		*dst = 0;	/* NULL terminate destination string */
	}
	return dst;
}
