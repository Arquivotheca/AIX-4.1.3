static char sccsid[] = "@(#)55	1.2.1.1  src/bos/usr/ccs/lib/libc/wstrtos.c, libcnls, bos411, 9428A410j 2/2/94 15:33:13";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wstrtos
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

#include <stdlib.h>
#include <ctype.h>

/*
 * NAME: wstrtos
 *
 * FUNCTION: Convert a string of NLchars to chars.
 *
 * NOTE:     Macro NLchrlen returns the length of NLchar.
 *           Macro NCenc does the same as NCencode().
 *
 * RETURN VALUE DESCRIPTION: Return the length of string converted.
 */

/*
 * Convert a string of NLchars to chars; return length of string produced.
 */

char * wstrtos (char *c, wchar_t *nlc)
{
    	wcstombs (c, nlc, wcslen(nlc)*MB_CUR_MAX + 1);
    	return (c);
}
