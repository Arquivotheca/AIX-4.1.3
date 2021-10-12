static char sccsid[] = "@(#)82	1.1  src/bos/usr/ccs/lib/libc/load.c, libcproc, bos411, 9428A410j 2/26/91 17:46:42";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS:  load
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<sys/types.h>
#include	<sys/ldr.h>

int
(*load(char *filenameparm,uint flags,char *libpathparm))()
{
	if (!libpathparm){
		libpathparm = getenv("LIBPATH");
	}
	return _load(filenameparm,flags,libpathparm);
}
