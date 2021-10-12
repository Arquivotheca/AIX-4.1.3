/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: vldldptr
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
static char *sccsid = "@(#)87  1.4  src/bos/usr/ccs/lib/libld/vldldptr.c, libld, bos411, 9428A410j 4/16/91 05:17:30";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "lddef.h"

int
vldldptr(ldptr)
	LDFILE	*ldptr;
{
	extern LDLIST	*_ldhead;
	LDLIST		*ldindx;

	for (ldindx = _ldhead; ldindx != NULL; ldindx = ldindx->ld_next)
	{
		if (ldindx == (LDLIST *) ldptr)
			return(SUCCESS);
	}

	return(FAILURE);
}

/* static char ID[ ] = "vldldptr.c: 1.1 1/8/82"; */
