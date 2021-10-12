/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldtbindex
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
static char *sccsid = "@(#)82  1.5  src/bos/usr/ccs/lib/libld/ldtbindex.c, libld, bos411, 9428A410j 4/16/91 05:17:16";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <syms.h>
#include <ldfcn.h>
#include "takes.h"

long
ldtbindex(ldptr)
LDFILE	*ldptr;
{
	extern long	ftell TAKES((FILE *));
	extern int	vldldptr TAKES((LDFILE *));
	long		position;

	if (vldldptr(ldptr) == SUCCESS
		&& (position = FTELL(ldptr) - OFFSET(ldptr)
			- HEADER(ldptr).f_symptr) >= 0
		&& (position % SYMESZ) == 0)
	{
		return(position / SYMESZ);
	}

	return(BADINDEX);
}

/* static char	ID[ ] = "ldtbindex.c: 1.1 1/7/82"; */
