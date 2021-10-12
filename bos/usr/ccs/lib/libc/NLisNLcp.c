static char sccsid[] = "@(#)83	1.4  src/bos/usr/ccs/lib/libc/NLisNLcp.c, libcnls, bos411, 9428A410j 6/11/91 09:46:32";
/*
* COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
*
* FUNCTIONS: NLisNLcp
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1989, 1991
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

#include <stdlib.h>
#include <NLchar.h>

int NLisNLcp(char *c)
{
    int rc;

    rc = mblen(c, MB_CUR_MAX);

    if (rc == 1) {
	if ((unsigned char)*c >0x7f)
	    return(1);
	else
	    return(0);
    }
    else
	return(rc);
}




