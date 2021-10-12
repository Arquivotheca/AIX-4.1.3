/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldaclose
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef	lint
static char *sccsid = "@(#)64  1.5  src/bos/usr/ccs/lib/libld/ldaclose.c, libld, bos411, 9428A410j 4/16/91 05:16:14";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldaclose(ldptr)
LDFILE	*ldptr;
{
	extern 		fclose TAKES((FILE *));
	extern int	vldldptr TAKES((LDFILE *));
	extern	    	freeldptr TAKES((LDFILE *));

	if (vldldptr(ldptr) == FAILURE)
	{
		return(FAILURE);
	}

	(void) fclose(IOPTR(ldptr));
	(void) freeldptr(ldptr);

	return(SUCCESS);
}

/* static char ID[ ] = "ldaclose.c: 1.1 1/7/82"; */
