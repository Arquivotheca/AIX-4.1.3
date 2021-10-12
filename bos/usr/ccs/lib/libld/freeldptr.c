/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: freeldptr
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
static char *sccsid = "@(#)63  1.4  src/bos/usr/ccs/lib/libld/freeldptr.c, libld, bos411, 9428A410j 4/16/91 05:16:11";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "lddef.h"
#include "takes.h"

int
freeldptr(ldptr)
LDFILE	*ldptr;
{
	extern void	free();
	extern LDLIST	*_ldhead;
	LDLIST		*ldindx;

	if (ldptr != NULL)
	{
		if (_ldhead == (LDLIST *) ldptr)
		{
			_ldhead = _ldhead->ld_next;
			free(ldptr);
			return(SUCCESS);
		}

		for (ldindx = _ldhead; ldindx != NULL; ldindx = ldindx->ld_next)
		{
			if (ldindx->ld_next == (LDLIST *) ldptr)
			{
				ldindx->ld_next = ((LDLIST *) ldptr)->ld_next;
				free(ldptr);
				return(SUCCESS);
			}
		}
	}

	return(FAILURE);
}

/* static char ID[ ] = "freeldptr.c: 1.1 1/7/82"; */
