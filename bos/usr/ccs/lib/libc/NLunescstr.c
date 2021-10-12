static char sccsid[] = "@(#)26	1.7  src/bos/usr/ccs/lib/libc/NLunescstr.c, libcnls, bos411, 9428A410j 6/16/90 01:27:12";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLunescstr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <NLctype.h>

/*
 * NAME: NLunescstr
 *
 * FUNCTION: Translate any escape sequences found in src that indicate
 *	     an NLS code point back to the code point represented.
 *
 * RETURN VALUE DESCRIPTION: The number of chars converted.
 */
/*
 *  Translate any escape sequences found in src that indicate an NLS code
 *  point back to the code point represented.
 */
int
NLunescstr(src, dest, dlen)
register unsigned char *src, *dest;
register int dlen;	/* the length to be converted */
{
	register int n;	/* the length of NLchar converted to char */
	NLchar nc;
	unsigned char *odest = dest; /* the next char position */

	/*  Always NUL-terminate output string, if any; but never count
	 *  NUL as part of length.
	 */
	for ( ; dlen && *src; dest += n, dlen -= n) {
		src += NCunesc(src, &nc);
		if (dlen < NCchrlen(nc))
			break;
		n = NCenc(&nc, dest);
	}
	if (odest < dest) {
		if (!dlen)
			--dest;
		*dest = '\0';
	}
	return (dest - odest);
}
