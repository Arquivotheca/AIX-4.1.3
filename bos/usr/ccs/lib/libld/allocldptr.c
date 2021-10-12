/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: allocldptr
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
static char *sccsid = "@(#)62  1.4  src/bos/usr/ccs/lib/libld/allocldptr.c, libld, bos411, 9428A410j 4/16/91 05:16:07";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "lddef.h"
#include "takes.h"

LDLIST	*_ldhead = NULL;

LDFILE *
allocldptr()
{
	extern char *calloc();
	extern LDLIST *_ldhead;
	LDLIST *ldptr, *ldindx;
	static int last_fnum_ = 0;

	if ((ldptr = (LDLIST *) calloc(1, LDLSZ)) == NULL)
		return(NULL);

	ldptr->ld_next = NULL;

	if (_ldhead == NULL)
		_ldhead = ldptr;
	else
	{
		for (ldindx = _ldhead; ldindx->ld_next != NULL;)
		{
			ldindx = ldindx->ld_next;
		}

		ldindx->ld_next = ldptr;
	}

	ldptr->ld_item._fnum_ = ++last_fnum_;
	return((LDFILE *)ldptr);
}

/* static char ID[] = "allocldptr.c: 1.2 2/16/83"; */
