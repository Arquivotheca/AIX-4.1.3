/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldfhread
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
static char *sccsid = "@(#)69  1.5  src/bos/usr/ccs/lib/libld/ldfhread.c, libld, bos411, 9428A410j 4/16/91 05:16:33";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "takes.h"
#ifdef _IBMRT
#include <sys/types.h>
#endif

int
ldfhread(ldptr, filehead)
LDFILE	*ldptr;
FILHDR	*filehead;
{
	extern size_t	fread TAKES((void *,size_t,size_t,FILE *));
	extern int	fseek TAKES((FILE *,long int,int));
	extern int	vldldptr TAKES((LDFILE *));

	if (vldldptr(ldptr) == SUCCESS
		&& FSEEK(ldptr, (long int)0L, (int)BEGINNING) == OKFSEEK
		&& FREAD((void *)filehead, (size_t)FILHSZ, (size_t)1, ldptr) == 1)
	{
		return(SUCCESS);
	}

	return(FAILURE);
}

/* static char ID[ ] = "ldfhread.c: 1.1 1/7/82"; */
