/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldtbread
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
static char *sccsid = "@(#)83  1.5  src/bos/usr/ccs/lib/libld/ldtbread.c, libld, bos411, 9428A410j 4/16/91 05:17:19";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <syms.h>
#include <ldfcn.h>
#include "takes.h"
#ifdef _IBMRT
#include <sys/types.h>
#endif

int
ldtbread(ldptr, symnum, symentry)
LDFILE	*ldptr;
long	symnum;
SYMENT	*symentry;
{
	extern int	fseek TAKES((FILE *,long int,int));
	extern size_t	fread TAKES((void *,size_t,size_t,FILE *));
	extern int	vldldptr TAKES((LDFILE *));

	if (vldldptr(ldptr) == SUCCESS
		&& (symnum >= 0) && (symnum <= (HEADER(ldptr)).f_nsyms)
		&& FSEEK(ldptr, HEADER(ldptr).f_symptr + symnum * SYMESZ,
			BEGINNING) == OKFSEEK
		&& FREAD((void *)symentry,(size_t)SYMESZ,(size_t)1,ldptr) == 1)
	{
		return(SUCCESS);
	}

	return(FAILURE);
}

/* static char ID[ ] = "ldtbread.c: 1.1 1/7/82"; */
