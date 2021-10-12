/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldnsseek
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
static char *sccsid = "@(#)76  1.5  src/bos/usr/ccs/lib/libld/ldnsseek.c, libld, bos411, 9428A410j 4/16/91 05:16:57";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldnsseek(ldptr, sectname)
LDFILE	*ldptr;
char 	*sectname;
{
	extern int	ldnshread TAKES((LDFILE *,char *,SCNHDR *));
	extern int	fseek TAKES((FILE *,long int,int));
	SCNHDR	shdr;

	if (ldnshread(ldptr, sectname, &shdr) == SUCCESS
		&& shdr.s_scnptr != 0
		&& FSEEK(ldptr, (long int)shdr.s_scnptr, BEGINNING) == OKFSEEK)
	{
		return(SUCCESS);
	}

	return(FAILURE);
}

/* static char ID[ ] = "ldnsseek.c: 1.1 1/7/82"; */
