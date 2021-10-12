/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldlseek
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
static char *sccsid = "@(#)72  1.7  src/bos/usr/ccs/lib/libld/ldlseek.c, libld, bos411, 9428A410j 4/16/91 05:16:44";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldlseek(ldptr, sectnum)
LDFILE		*ldptr;
unsigned short	sectnum;
{
	extern int	ldshread();
	extern int	fseek TAKES((FILE *,long int,int));
	unsigned short	i;
	SCNHDR	shdr;

	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS)
	{
		if ((shdr.s_nlnno != 0 && (shdr.s_nlnno != 0xffff))
			&& FSEEK(ldptr, shdr.s_lnnoptr, BEGINNING) == OKFSEEK)
		{
			return(SUCCESS);
		}
		else if(shdr.s_nlnno == 0xffff)
		{
			for (i = 1; ldshread(ldptr, i, &shdr) == SUCCESS; i++)
			{
				if ((shdr.s_flags & 0x0000ffff) == STYP_OVRFLO
					&& shdr.s_nlnno == sectnum &&
					shdr.s_vaddr &&
					FSEEK(ldptr, shdr.s_lnnoptr, BEGINNING)
					== OKFSEEK)
				{
					return(SUCCESS);
				}
			}

			return(FAILURE);
		}
	}
				
	return(FAILURE);
}

/* static char ID[ ] = "ldlseek.c: 1.1 1/7/82"; */
