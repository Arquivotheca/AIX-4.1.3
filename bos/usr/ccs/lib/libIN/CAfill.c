static char sccsid[] = "@(#)08	1.7  src/bos/usr/ccs/lib/libIN/CAfill.c, libIN, bos411, 9428A410j 6/10/91 10:51:56";
/*
 * LIBIN: CAfill
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
 * CAfill sets the first cnt characters of dst to chr.  chr is of type wchar_t,
 * but it is stored at dst after first converting it to a multibyte character.
 *
 * RETURN VALUE DESCRIPTION: 
 * If dst is NULL or cnt<=0, dst is returned and nothing is stored.
 * Otherwise, the pointer to the byte following the last stored character is
 * returned.
 */

#include <sys/limits.h>
#include <sys/types.h>
char *
CAfill(dst, chr, cnt)
register char *dst;	/* where to put copies of character */
wchar_t chr;		/* character to be copied */
register cnt;		/* number of copies desired */
{
	char mbstr[MB_LEN_MAX];

	int mbcount;	/* # of bytes in mb representation of this character */
	int n;		/* temp version of mbcount */
	char *pmb;	/* pointer to mbstr buffer */

	if (dst)
		if ((mbcount = wctomb(mbstr, chr)) == 1)
			for (;cnt > 0; --cnt)
				*dst++ = mbstr[0];
		else
		{
			for (;cnt > 0; --cnt)
			{
				pmb = mbstr;
				for (n = mbcount; n > 0; --n)
					*dst++ = *pmb++;
			}
		}
	return(dst);
}
