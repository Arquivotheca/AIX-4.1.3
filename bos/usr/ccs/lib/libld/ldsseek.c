/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldsseek
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
static char *sccsid = "@(#)81  1.5  src/bos/usr/ccs/lib/libld/ldsseek.c, libld, bos411, 9428A410j 4/16/91 05:17:13";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldsseek(ldptr, sectnum)
LDFILE		*ldptr;
unsigned short	sectnum;
{
	extern int	ldshread TAKES((LDFILE *,unsigned short,SCNHDR *));
	extern int	fseek TAKES((FILE *,long int,int));
	SCNHDR	shdr;

	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS
		&& shdr.s_scnptr != 0
		&& FSEEK(ldptr, (long int)shdr.s_scnptr, BEGINNING) == OKFSEEK)
	{
		return(SUCCESS);
	}

	return(FAILURE);
}

/* static char ID[ ] = "ldsseek.c: 1.1 1/7/82"; */
