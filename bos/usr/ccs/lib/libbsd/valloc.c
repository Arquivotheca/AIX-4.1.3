static char sccsid[] = "@(#)93	1.4  src/bos/usr/ccs/lib/libbsd/valloc.c, libbsd, bos411, 9428A410j 3/4/94 10:13:18";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: valloc
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdlib.h>

/*
 * NAME: valloc
 * FUNCTION:  allocates size bytes aligned on a page boundary.
 */
void *
valloc(size_t i)
{
	int valsiz = getpagesize(), j;
	char *cp = malloc((size_t)(i + (valsiz-1)));

	if (cp == NULL)
		return(NULL);

	j = ((int)cp + (valsiz-1)) &~ (valsiz-1);
	return ((void *)j);
}
