/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldahread
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
static char *sccsid = "@(#)65  1.7  src/bos/usr/ccs/lib/libld/ldahread.c, libld, bos411, 9428A410j 4/16/91 05:16:18";
#endif	lint

/*
* ldahread - fill archive file member header info.
*
* #ifdef PORTAR		printable ascii header archive version
* #else #ifdef PORT5AR	UNIX 5.0 semi-portable archive version
* #else			pre-UNIX 5.0 (old) archive version
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
ldahread(ldptr, arhead) 
LDFILE	*ldptr;
ARCHDR	*arhead;
{
	extern int fseek TAKES((FILE *, long int, int));
	extern size_t fread TAKES((void *, size_t, size_t, FILE *)); 
/*	extern long sgetl(); */
	extern int vldldptr();
	extern char *strncpy();
#ifdef AIXV3AR
	extern long atol TAKES((const char *));
#endif AIXV3AR

#ifdef PORTAR
	struct ar_hdr arbuf;

	if (vldldptr(ldptr) == SUCCESS && TYPE(ldptr) == ARTYPE
		&& FSEEK(ldptr, -((long)sizeof(arbuf)), BEGINNING) == OKFSEEK
		&& FREAD((char *)&arbuf, sizeof(arbuf), 1, ldptr) == 1
		&& !strncmp(arbuf.ar_fmag, ARFMAG, sizeof(arbuf.ar_fmag)))
	{
		register char *cp = arbuf.ar_name + sizeof(arbuf.ar_name);

		while (*--cp == ' ')
			;

		if (*cp == '/')
			*cp = '\0';
		else
			*++cp = '\0';

		(void) strncpy(arhead->ar_name, arbuf.ar_name,
			sizeof(arbuf.ar_name));

		if (sscanf(arbuf.ar_date, "%ld", &arhead->ar_date) == 1
			&& sscanf(arbuf.ar_uid, "%d", &arhead->ar_uid) == 1
			&& sscanf(arbuf.ar_gid, "%d", &arhead->ar_gid) == 1
			&& sscanf(arbuf.ar_mode, "%o", &arhead->ar_mode) == 1
			&& sscanf(arbuf.ar_size, "%ld", &arhead->ar_size) == 1)
		{
			return(SUCCESS);
		}
	}
#else
#ifdef AIXV3AR
	struct ar_hdr arbuf;

	if (vldldptr(ldptr) == SUCCESS && TYPE(ldptr) == ARTYPE
		&& fseek(IOPTR(ldptr), AR_OFF(ldptr), 0) == OKFSEEK
		&& FREAD((char *)&arbuf, (size_t)sizeof(arbuf), 
			(size_t)1, (FILE *)ldptr) == 1
		&& FREAD((char *)&arhead->ar_name[2],
			(size_t)(atol(arbuf.ar_namlen) - 2), 
			(size_t)1, (FILE *)ldptr) == 1)
	{
		strncpy(arhead->ar_name, arbuf._ar_name.ar_name, 2);
		arhead->ar_name[atol(arbuf.ar_namlen)] = '\0';

		if (sscanf(arbuf.ar_size, "%ld", &arhead->ar_size) == 1
			&& sscanf(arbuf.ar_nxtmem, "%ld",
				&arhead->ar_nxtmem) == 1
			&& sscanf(arbuf.ar_prvmem, "%ld",
				&arhead->ar_prvmem) == 1
			&& sscanf(arbuf.ar_date, "%ld", &arhead->ar_date) == 1
			&& sscanf(arbuf.ar_uid, "%ld", &arhead->ar_uid) == 1
			&& sscanf(arbuf.ar_gid, "%ld", &arhead->ar_gid) == 1
			&& sscanf(arbuf.ar_mode, "%o", &arhead->ar_mode) == 1
			&& sscanf(arbuf.ar_namlen, "%d",
				&arhead->ar_namlen) == 1)
		{
			return(SUCCESS);
		}
	}
#else
#ifdef PORT5AR
	struct arf_hdr arbuf;

	if (vldldptr(ldptr) == SUCCESS && TYPE(ldptr) == ARTYPE
		&& FSEEK(ldptr, -((long)sizeof(arbuf)), BEGINNING) == OKFSEEK
		&& FREAD((char *)&arbuf, sizeof(arbuf), 1, ldptr) == 1)
	{
		(void)strncpy(arhead->ar_name, arbuf.arf_name,
			sizeof(arbuf.arf_name));
		arhead->ar_date = sgetl(arbuf.arf_date);
		arhead->ar_uid = sgetl(arbuf.arf_uid);
		arhead->ar_gid = sgetl(arbuf.arf_gid);
		arhead->ar_mode = sgetl(arbuf.arf_mode);
		arhead->ar_size = sgetl(arbuf.arf_size);
		return(SUCCESS);
	}
#else
	if (vldldptr(ldptr) == SUCCESS && TYPE(ldptr) == ARTYPE
		&& FSEEK(ldptr, -((long) ARCHSZ), BEGINNING) == OKFSEEK
		&& FREAD((char *)arhead, ARCHSZ, 1, ldptr) == 1)
	{
		return(SUCCESS);
	}
#endif
#endif
#endif
	return(FAILURE);
}

/* static char ID[] = "@(#) ldahread.c: 1.4 3/3/83"; */
