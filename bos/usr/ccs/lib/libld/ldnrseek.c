/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldnrseek
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
static char *sccsid = "@(#)74  1.8  src/bos/usr/ccs/lib/libld/ldnrseek.c, libld, bos411, 9428A410j 7/26/91 13:42:32";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldnrseek(ldptr, sectname)
LDFILE	*ldptr;
char 	*sectname;
{
	extern int	ldnshread(),
			ldshread();
	extern int	fseek TAKES((FILE *,long int,int));
	unsigned short	i;
	SCNHDR	shdr,
		t_shdr;


	if (ldnshread(ldptr, sectname, &shdr) == SUCCESS)
	{
		if ((shdr.s_nreloc != 0 && (shdr.s_nreloc != 0xffff))
			&& FSEEK(ldptr, shdr.s_relptr, BEGINNING) == OKFSEEK)
		{
			return(SUCCESS);
		}
		else if(shdr.s_nreloc == 0xffff)
		{
			for (i=1; ldshread(ldptr, i, &t_shdr) == SUCCESS; i++)
			{
				if ((t_shdr.s_flags&0x0000ffff) == STYP_OVRFLO)
				{
					if (ldshread(ldptr, t_shdr.s_nreloc,
						&shdr) == SUCCESS &&
						!strncmp(shdr.s_name, sectname,
						8)
						&& t_shdr.s_paddr &&
						FSEEK(ldptr, shdr.s_relptr,
						BEGINNING) == OKFSEEK)
					{
						return(SUCCESS);
					}
				}
			}

			return(FAILURE);
		}
	}
				
	return(FAILURE);
}

/* static char ID[ ] = "ldnrseek.c: 1.1 1/7/82"; */
