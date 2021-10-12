static char sccsid[] = "@(#)65	1.3.1.3  src/bos/usr/ccs/lib/libc/NCcoluniq.c, libcnls, bos411, 9428A410j 3/10/94 11:20:45";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCcoluniq
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This function returns the secondary, or unique, collation value for 
 * the wchar_t argument. The returned value can be either 
 *	positive -	the real secondary (lc_coluniq) value 
 *	zero     -	the wchar_t is non-collating (ignore)
 *
 * Note that the coluniq value returned only applies to the "single
 * char"; to retrieve the coluniq value for multi-character collating
 * elements, use the NLxcolu function.
 */

#include <sys/localedef.h>
#include <ctype.h>
#include <sys/limits.h>

int NCcoluniq(wchar_t nlc)
{
char *str[MB_LEN_MAX+1];
int len;
char *dummy;

    len=wctomb(str,nlc);
    if (len<=0)
	return(0);	/* what else can we do? */
    str[len]=0;		/* set terminating null char */

    return(_mbucoll(__lc_collate,str,&dummy));
}
