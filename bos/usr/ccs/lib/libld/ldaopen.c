/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: hdrassign, ldaopen
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
static char *sccsid = "@(#)66  1.5  src/bos/usr/ccs/lib/libld/ldaopen.c, libld, bos411, 9428A410j 4/16/91 05:16:22";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "takes.h"
#ifdef _IBMRT
#include <sys/types.h>
#endif

int hdrassign TAKES((LDFILE *,LDFILE *));

LDFILE *
ldaopen(filename, oldptr)
char	*filename;
LDFILE	*oldptr;
{
	extern FILE *fopen TAKES((const char *, const char *));
	extern int vldldptr();
	extern LDFILE *allocldptr();
	extern int freeldptr();
	FILE *ioptr;
	LDFILE *nldptr;

	if (vldldptr(oldptr) == FAILURE || (nldptr = allocldptr()) == NULL)
		return(NULL);

	if ((ioptr = fopen(filename, "r")) == NULL)
	{
		(void) freeldptr(nldptr);
		return(NULL);
	}

	IOPTR(nldptr) = ioptr;
	OFFSET(nldptr) = OFFSET(oldptr);
	TYPE(nldptr) = TYPE(oldptr);
	hdrassign(oldptr, nldptr);
	nldptr->_fnum_ = oldptr->_fnum_;	/* use same string table */
	return(nldptr);
}

int
hdrassign(oldptr, nldptr)
LDFILE	*oldptr;
LDFILE	*nldptr;
{
	(HEADER(nldptr)).f_magic = (HEADER(oldptr)).f_magic;
	(HEADER(nldptr)).f_nscns = (HEADER(oldptr)).f_nscns;
	(HEADER(nldptr)).f_timdat = (HEADER(oldptr)).f_timdat;
	(HEADER(nldptr)).f_symptr = (HEADER(oldptr)).f_symptr;
	(HEADER(nldptr)).f_nsyms = (HEADER(oldptr)).f_nsyms;
	(HEADER(nldptr)).f_opthdr = (HEADER(oldptr)).f_opthdr;
	(HEADER(nldptr)).f_flags = (HEADER(oldptr)).f_flags;
	return;
}

/* static char ID[] = "ldaopen.c: 1.2 2/16/83"; */
