/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldrseek
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
static char *sccsid = "@(#)79  1.7  src/bos/usr/ccs/lib/libld/ldrseek.c, libld, bos411, 9428A410j 4/16/91 05:17:07";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldrseek(ldptr, sectnum)
LDFILE		*ldptr;
unsigned short	sectnum;
{
	extern int	ldshread TAKES((LDFILE *,unsigned short,SCNHDR *));
	extern int	fseek TAKES((FILE *,long int,int));
	unsigned short	i;
	SCNHDR	shdr;


	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS)
	{
		if ((shdr.s_nreloc != 0 && (shdr.s_nreloc != 0xffff))
			&& FSEEK(ldptr, shdr.s_relptr, BEGINNING) == OKFSEEK)
		{
			return(SUCCESS);
		}
		else if(shdr.s_nreloc == 0xffff)
		{
			for (i = 1; ldshread(ldptr, i, &shdr) == SUCCESS; i++)
			{
				if ((shdr.s_flags & 0x0000ffff) == STYP_OVRFLO
					&& shdr.s_nreloc == sectnum &&
					shdr.s_paddr &&
					FSEEK(ldptr, shdr.s_relptr, BEGINNING)
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

/* static char ID[ ] = "ldrseek.c: 1.1 1/7/82"; */
