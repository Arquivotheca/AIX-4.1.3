static char sccsid[] = "@(#)59	1.8  src/bos/usr/ccs/lib/libc/NLyesno.c, libcnls, bos411, 9428A410j 6/14/91 09:53:44";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLyesno, scan
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


/*
 * NAME: NLyesno
 *
 * FUNCTION: Determine obsolete interface to rpmatch
 */



int NLyesno (const char *s)
{
	return ( rpmatch(s) );
}
