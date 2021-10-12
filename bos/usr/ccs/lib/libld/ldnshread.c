/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldnshread
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
static char *sccsid = "@(#)75  1.5  src/bos/usr/ccs/lib/libld/ldnshread.c, libld, bos411, 9428A410j 4/16/91 05:16:54";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "takes.h"

int
ldnshread(ldptr, sectname, secthdr)
LDFILE	*ldptr;
char	*sectname;
SCNHDR	*secthdr;
{
	extern int	fseek TAKES((FILE *,long int,int));
	extern size_t	fread TAKES((void *,size_t,size_t,FILE *));
	extern int	vldldptr TAKES((LDFILE *));
	unsigned short	i;
	int		j;
	unsigned short	numsects;

	if (vldldptr(ldptr) == SUCCESS)
	{
		if (FSEEK(ldptr, (long int) (FILHSZ + HEADER(ldptr).f_opthdr),
			BEGINNING) == OKFSEEK)
		{
			numsects = (HEADER(ldptr)).f_nscns;

			for (i = 0; (i < numsects)
				&& (FREAD((void *)secthdr, (size_t)SCNHSZ, 
				(size_t)1,ldptr) == 1); ++i)
			{
				for (j = 0; (j < 8)
					&& (secthdr->s_name[j] == sectname[j]);
					++j)
				{
					if (secthdr->s_name[j] == '\0')
						return(SUCCESS);
				}

				if (j == 8)
					return(SUCCESS);
			}
		}
	}

	return(FAILURE);
}

/* static char ID[ ] = "ldnshread.c: 1.1 1/7/82"; */
