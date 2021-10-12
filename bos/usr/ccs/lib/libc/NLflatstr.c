static char sccsid[] = "@(#)62	1.11  src/bos/usr/ccs/lib/libc/NLflatstr.c, libcnls, bos411, 9428A410j 6/11/91 09:46:24";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLflatstr
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
 * NAME: NLflatstr
 *
 * FUNCTION: Translate each NLS code point in src into the single 
 *           standard ASCII char most nearly matching the code 
 *           point's appearance.
 *
 * RETURN VALUE DESCRIPTION: The length of the converted char.
 */
/*
 *
 *  Translate each NLS code point in src into the single standard ASCII
 *  character most nearly matching the code point's appearance.
 */
int NLflatstr(unsigned char *src, unsigned char *dest, int dlen)
{
	register int n;	/* the length of NLS code */
	unsigned char *odest = dest;	/* the next char position */

	/**********
	  only valid in single byte codesets
	**********/
	if (MB_CUR_MAX > 1) {
	    strncpy(dest, src, dlen);
	    return(dlen);
	}
	    
	/*  Always NUL-terminate output string, if any; but never count
	 *  NUL as part of length.
	 */
	for ( ; dlen && *src; --dlen)
		if (n = NLisNLcp(src)) {		/* NLS ? */
			*dest++ = NCflatchr(NCdechr(src));
				/* decode to NLchar and convert to ASCII */
			src += n;
		} else
			*dest++ = *src++;
	if (odest < dest) {	/* more char? */
		if (!dlen)
			--dest;
		*dest = '\0';
	}
	return (dest - odest);
}
