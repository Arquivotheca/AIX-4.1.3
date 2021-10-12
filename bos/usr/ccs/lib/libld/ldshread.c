/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldshread
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
static char *sccsid = "@(#)80  1.5  src/bos/usr/ccs/lib/libld/ldshread.c, libld, bos411, 9428A410j 4/16/91 05:17:10";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldshread(ldptr, sectnum, secthdr)
LDFILE		*ldptr;
unsigned short	sectnum;
SCNHDR		*secthdr;
{
	extern int	fseek TAKES((FILE *,long int,int));
	extern size_t	fread TAKES((void *,size_t,size_t,FILE *));
	extern int	vldldptr TAKES((LDFILE *));

	if (vldldptr(ldptr) == SUCCESS
		&& (sectnum != 0) && (sectnum <= HEADER(ldptr).f_nscns)
		&& FSEEK(ldptr, (long int) (FILHSZ + HEADER(ldptr).f_opthdr
			+ (sectnum - 1L) * SCNHSZ), BEGINNING) == OKFSEEK
		&& FREAD((void *)secthdr, (size_t)SCNHSZ, (size_t)1, ldptr) == 1)
	{
		return(SUCCESS);
	}

	return(FAILURE);
}

/* static char ID[ ] = "ldshread.c: 1.1 1/7/82"; */
