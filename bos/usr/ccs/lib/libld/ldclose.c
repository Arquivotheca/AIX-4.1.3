/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldclose
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
static char *sccsid = "@(#)67  1.6  src/bos/usr/ccs/lib/libld/ldclose.c, libld, bos411, 9428A410j 4/16/91 05:16:25";
#endif	lint

/*
* ldclose - close current object file.
*		if current object file is an archive member,
*		set up for next object file from archive.
*
* #ifdef PORTAR		printable ascii headers archive version
* #else #ifdef PORT5AR	UNIX 5.0 semi-portable archive version
* #else			pre UNIX 5.0 (old) archive version
* #endif
*/
#include <stdio.h>
#include <ar.h>
#include <filehdr.h>
#include <ldfcn.h>
#include "takes.h"
#ifdef _IBMRT
#include <sys/types.h>
#endif

int
ldclose(ldptr)
LDFILE	*ldptr;
{
	extern int fseek TAKES((FILE *,long int,int));
	extern size_t fread TAKES((void *,size_t,size_t,FILE *));
	extern int fclose TAKES((FILE *));
	extern int vldldptr();
	extern int freeldptr();
	extern long sgetl();
#ifdef AIXV3AR
	extern long atol();
#endif AIXV3AR


#ifdef PORTAR
	struct ar_hdr arhdr;
	long ar_size;

	if (vldldptr(ldptr) == FAILURE)
		return(SUCCESS);

	if (TYPE(ldptr) == ARTYPE
		&& FSEEK(ldptr, -((long)sizeof(arhdr)), BEGINNING) == OKFSEEK
		&& FREAD((char *)&arhdr, sizeof(arhdr), 1, ldptr) == 1
		&& !strncmp(arhdr.ar_fmag, ARFMAG, sizeof(arhdr.ar_fmag))
		&& sscanf(arhdr.ar_size, "%ld", &ar_size) == 1)
	{
		/*
		* Be sure OFFSET is even
		*/
		OFFSET(ldptr) += ar_size + sizeof(arhdr) + (ar_size & 01);

		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
			FREAD((char *)&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1)
		{
			return(FAILURE);
		}
	}
#else
#ifdef AIXV3AR
	struct	ar_hdr arhdr;
	long	ar_offset,
		ar_namesz;

	if (vldldptr(ldptr) == FAILURE)
		return(SUCCESS);

	if (TYPE(ldptr) == ARTYPE
		&& fseek(IOPTR(ldptr), AR_OFF(ldptr), 0) == OKFSEEK
		&& FREAD((char *)&arhdr, (size_t)sizeof(arhdr), (size_t)1, ldptr) == 1
		&& (ar_offset = atol(arhdr.ar_nxtmem)) != AR_END_OFF(ldptr)
		&& fseek(IOPTR(ldptr), ar_offset, 0) == OKFSEEK
		&& FREAD((char *)&arhdr, (size_t)sizeof(arhdr), (size_t)1, ldptr) == 1
		&& atol(arhdr.ar_namlen) != 0L)
	{
		/*
		* Be sure OFFSET is even
		*/
		if (fseek(IOPTR(ldptr), (ar_namesz = 
			((atol(arhdr.ar_namlen) + 01) & ~01)), 1) == OKFSEEK &&
			FREAD((char *)&(HEADER(ldptr)), (size_t)FILHSZ, (size_t)1, ldptr) == 1)
		{
			OFFSET(ldptr) = ar_offset + sizeof(arhdr) + ar_namesz;
			AR_OFF(ldptr) = ar_offset;
			return(FAILURE);
		}
	}
#else
#ifdef PORT5AR
	struct arf_hdr arhdr;
	long ar_size, nsyms;

	if (vldldptr(ldptr) == FAILURE)
		return(SUCCESS);

	if (TYPE(ldptr) == ARTYPE
		&& FSEEK(ldptr, -((long)sizeof(arhdr)), BEGINNING) == OKFSEEK
		&& FREAD((char *)&arhdr, sizeof(arhdr), 1, ldptr) == 1)
	{
		ar_size = sgetl(arhdr.arf_size);
		/*
		* Be sure offset is even
		*/
		OFFSET(ldptr) += ar_size + sizeof(arhdr) + (ar_size & 01);

		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
			FREAD((char *)&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1)
		{
			return(FAILURE);
		}
	}
#else
	ARCHDR arhdr;

	if (vldldptr(ldptr) == FAILURE)
		return(SUCCESS);

	if (TYPE(ldptr) == ARTYPE
		&& FSEEK(ldptr, -((long)ARCHSZ), BEGINNING) == OKFSEEK
		&& FREAD((char *)&arhdr, ARCHSZ, 1, ldptr) == 1)
	{
		/*
		* Be sure OFFSET is even
		*/
		OFFSET(ldptr) += arhdr.ar_size + ARCHSZ + (arhdr.ar_size & 01);

		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
			FREAD((char *)&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1)
		{
			return(FAILURE);
		}
	}
#endif
#endif
#endif
	(void) fclose(IOPTR(ldptr));
	(void) freeldptr(ldptr);
	return(SUCCESS);
}

/* static char ID[] = "ldclose.c: 1.3 2/16/83"; */
