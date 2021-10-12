/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldtbseek
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
static char *sccsid = "@(#)84  1.5  src/bos/usr/ccs/lib/libld/ldtbseek.c, libld, bos411, 9428A410j 4/16/91 05:17:22";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldtbseek(ldptr)
LDFILE	*ldptr;
{
	extern int	fseek TAKES((FILE *,long int,int));
	extern int	vldldptr TAKES((LDFILE *));

	if (vldldptr(ldptr) == SUCCESS
		&& HEADER(ldptr).f_symptr != 0L
		&& FSEEK(ldptr, (long int)HEADER(ldptr).f_symptr, BEGINNING) == OKFSEEK)
	{
		return(SUCCESS);
	}

	return(FAILURE);
}

/* static char ID[ ] = "ldtbseek.c: 1.1 1/7/82"; */
