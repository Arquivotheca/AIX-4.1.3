static char sccsid[] = "@(#)06	1.7  src/bos/usr/ccs/lib/libc/NCgetbuf.c, libcnls, bos411, 9428A410j 6/16/90 01:25:20";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCgetbuf
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

#include <NLchar.h>
#include <stdlib.h>

/*
 * NAME: _NCgetbuf
 *
 * FUNCTION: Malloc a buffer of dlen NLchar's and decode string src into it.
 *
 * RETURN VALUE DESCRIPTION: NLchar ptr to the decoded string.
 */
/* 
 *  Malloc a buffer of dlen NLchar's and decode string src into it.
 */

NLchar *
_NCgetbuf(src, dlen)
char *src;       	/* a char string */
register int dlen;	/* the length of converted NLchar string */
{
	register NLchar *dest = (NLchar *)
		malloc((size_t)(dlen = (strlen(src) + 2) * sizeof (NLchar)));
			/* get a buffer for NLchar string */
	dest[0] = 1;
	(void)NCdecstr(src, &dest[1], (dlen / sizeof (NLchar)) - 1);
			/* decode to NLchar string */
	return (&dest[1]);
}
