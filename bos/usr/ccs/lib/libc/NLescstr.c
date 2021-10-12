static char sccsid[] = "@(#)28	1.8  src/bos/usr/ccs/lib/libc/NLescstr.c, libcnls, bos411, 9428A410j 6/11/91 09:46:13";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLescstr
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
#include <stdlib.h>

/*
 * NAME: NLescstr
 *
 * FUNCTION: Translate each NLS code point in src into a printable esc 
 *	     sequence in standard ASCII unique to that code point.
 *
 * RETURN VALUE DESCRIPTION: The length of a resulting string

 */
/*
 *  Translate each NLS code point in src into a printable escape sequence
 *  in standard ASCII unique to that code point.  Return length of resulting
 *  string.
 */
int
NLescstr(src, dest, dlen)
register unsigned char *src, *dest;	/* a source and destination strings */
register int dlen;       	/* the length of dest string */
{
	register int n;
	NLchar nlc;		        
	unsigned char *odest = dest;	/* the next char position */

	/*  Always NUL-terminate output string, if any; but never count
	 *  NUL as part of length.
	 */
	for ( ; dlen && *src; --dlen)   
		if ((n = mbtowc(&nlc, src, MB_CUR_MAX)) > 0) {
			if (dlen < NLESCMAX)
				break;
			src += n;
			n = NCesc(&nlc, dest);	/* Esc sequence */
			dest += n;
			dlen -= n - 1;
		} else
			*dest++ = *src++;
	if (odest < dest) {			/* more char ? */
		if (!dlen)			/* dlen = 0 ? */
			--dest;
		*dest = '\0';
	}
	return (dest - odest);
}
