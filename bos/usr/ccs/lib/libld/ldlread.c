/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldlinit, ldlitem, ldlread
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
static char *sccsid = "@(#)71  1.7  src/bos/usr/ccs/lib/libld/ldlread.c, libld, bos411, 9428A410j 4/16/91 05:16:39";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <linenum.h>
#include <syms.h>
#include <ldfcn.h>
#include "takes.h"
#ifdef _IBMRT
#include <sys/types.h>
#endif

static long		lnnoptr = 0L;
static unsigned short	maxlnnos = 0;
static LDFILE		*saveldptr = NULL;

int
ldlread(ldptr, fcnindx, linenum, linent)
LDFILE		*ldptr;
long		fcnindx;
unsigned short	linenum;
LINENO		*linent;
{
	extern int	ldlinit TAKES((LDFILE *,long));
	extern int	ldlitem TAKES((LDFILE *,unsigned short,LINENO *));

	if (ldlinit(ldptr, fcnindx) == SUCCESS)
		return(ldlitem(ldptr, (unsigned short)linenum, (LINENO *)linent));

	return(FAILURE);
}

int
ldlinit(ldptr, fcnindx)
LDFILE	*ldptr;
long	fcnindx;
{
	extern size_t	fread TAKES((void *,size_t,size_t,FILE *));
	extern int	fseek TAKES((FILE *,long int,int));
	extern int	ldtbread TAKES((LDFILE *,long,SYMENT *));
	extern int	ldshread TAKES((LDFILE *,unsigned short,SCNHDR *));
	SCNHDR		secthead;
	SYMENT		symbol;
	AUXENT		aux;
	LINENO		line;
	long		endlnptr;
	long		num_lnno;
	unsigned short	i;

	saveldptr = ldptr;

	if (ldtbread(ldptr, fcnindx, &symbol) == SUCCESS
		&& ISFCN(symbol.n_type) && (symbol.n_numaux == 1)
		&& FREAD((void *)&aux, (size_t)AUXESZ, (size_t)1, ldptr) == 1
		&& ldshread(ldptr, (unsigned short) symbol.n_scnum,
			&secthead) == SUCCESS)
	{
		if (secthead.s_nlnno == 0xffff)
		{
			for (i=1; ldshread(ldptr,i,&secthead) == SUCCESS; i++)
			{
				if ((secthead.s_flags & 0x0000ffff) ==
				STYP_OVRFLO &&
				secthead.s_nlnno == symbol.n_scnum)
				{
					num_lnno = secthead.s_vaddr;
				}
			}
		}
		else
		{
			num_lnno = (long)secthead.s_nlnno;
		}

		if ((lnnoptr = aux.x_sym.x_fcnary.x_fcn.x_lnnoptr) != 0L)
		{
			endlnptr = secthead.s_lnnoptr
				+ (long) (num_lnno * LINESZ);

			if ((secthead.s_lnnoptr <= lnnoptr)
				&& (lnnoptr + LINESZ <= endlnptr))
			{
				/* lnnoptr should be greater or equal
				 * to s_lnnoptr and less than endlnnoptr
				 * by at least as much as LINESZ
				 */
				maxlnnos = (unsigned short) (((endlnptr-lnnoptr)
					/ LINESZ) - 1);

				if (FSEEK(ldptr, lnnoptr, BEGINNING) == OKFSEEK
					&& FREAD((void *)&line, (size_t)LINESZ, (size_t)1, ldptr) == 1
					&& (line.l_lnno == 0)
					&&(line.l_addr.l_symndx == fcnindx))
				{
					return(SUCCESS);
				}
			}
		}
		else if (FSEEK(ldptr, secthead.s_lnnoptr, BEGINNING) == OKFSEEK)
		{
			for (maxlnnos = num_lnno; maxlnnos != 0;
				--maxlnnos)
			{
				if (FREAD((void *)&line, (size_t)LINESZ, (size_t)1, ldptr) != 1)
				{
					lnnoptr = 0L;
					maxlnnos = 0;
					saveldptr = NULL;
					return(FAILURE);
				}

				if ((line.l_lnno == 0)
					&& (line.l_addr.l_symndx == fcnindx))
				{
					lnnoptr = secthead.s_lnnoptr
						+ (long) ((num_lnno
						- maxlnnos) * LINESZ);
					--maxlnnos;
					return(SUCCESS);
				}
			}
		}
	}

	lnnoptr = 0L;
	maxlnnos = 0;
	saveldptr = NULL;
	return(FAILURE);
}

#ifdef KRC
int
ldlitem(ldptr, linenum, linent)
LDFILE		*ldptr;
unsigned short	linenum;
LINENO		*linent;
#else
int ldlitem(LDFILE *ldptr, unsigned short linenum, LINENO *linent)
#endif
{
	extern size_t	fread TAKES((void *,size_t,size_t,FILE *));
	extern int	fseek TAKES((FILE *,long int,int));
	LINENO		line;
	int		lflag;
	unsigned short	i;

	lflag = FAILURE;

	if ((ldptr == saveldptr) && (lnnoptr != 0)
		&& FSEEK(ldptr, lnnoptr, BEGINNING) == OKFSEEK
		&& FREAD((void *)&line, (size_t)LINESZ, (size_t)1, ldptr) == 1)
	{
		if (line.l_lnno == linenum)
		{
			linent->l_lnno = line.l_lnno;
			linent->l_addr.l_paddr = line.l_addr.l_paddr;
			return(SUCCESS);
		}

		for (i = maxlnnos; i != 0; --i)
		{
			if (FREAD((void *)&line, (size_t)LINESZ, (size_t)1, ldptr) != 1)
				return(FAILURE);
			else if (line.l_lnno == 0)
				return(lflag);
			else if (line.l_lnno == linenum)
			{
				linent->l_lnno = line.l_lnno;
				linent->l_addr.l_paddr = line.l_addr.l_paddr;
				return(SUCCESS);
			}
			else if (line.l_lnno > linenum
				&& (lflag == FAILURE
					|| linent->l_lnno > line.l_lnno))
			{
				lflag = SUCCESS;
				linent->l_lnno = line.l_lnno;
				linent->l_addr.l_paddr = line.l_addr.l_paddr;
			}
		}
	}

	return(lflag);
}

/* static char	ID[ ] = "ldlread.c: 1.1 1/7/82"; */
