/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldopen
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
static char *sccsid = "@(#)78  1.7  src/bos/usr/ccs/lib/libld/ldopen.c, libld, bos411, 9428A410j 4/16/91 05:17:04";
#endif	lint

/*
* ldopen - get LDFILE, header info for object file.
*		if it is an archive, get the first file from the archive.
*		if it is an already opened archive, assume ldclose() set
*		up everything already.
*
* #ifdef PORTAR		printable ascii header archive version
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

LDFILE *
ldopen(filename, ldptr)
char	*filename;
LDFILE	*ldptr;
{
	extern FILE *fopen TAKES((const char *,const char *));
	extern int fclose TAKES((FILE *));
	extern int fseek TAKES((FILE *,long int,int));
	extern size_t fread TAKES((void *,size_t,size_t,FILE *));
	extern int vldldptr TAKES((LDFILE *));
	extern LDFILE *allocldptr();
	extern int freeldptr TAKES((LDFILE *));
	extern long sgetl();
#ifdef AIXV3AR
	extern long atol();
#endif AIXV3AR
	FILE *ioptr;
	unsigned short type;
#ifdef PORTAR
	struct ar_hdr arbuf;
	char buf[SARMAG];
#else
#ifdef AIXV3AR
	struct ar_hdr arbuf;
	char buf[SAIAMAG];
	struct fl_hdr flbuf;
#else
#ifdef PORT5AR
	struct ar_hdr arbuf;
	long nsyms;
#else
	unsigned short atype = 0;
#endif
#endif
#endif

	if (vldldptr(ldptr) == FAILURE)
	{
		if ((ioptr = fopen(filename, "r")) == NULL)
			return(NULL);

#ifdef PORTAR
		if (fread(buf, sizeof(char) * SARMAG, 1, ioptr) != 1)
			buf[0] = '\0';

		(void) fseek(ioptr, 0L, 0);
#else
#ifdef AIXV3AR
		if (fread(buf, sizeof(char) * SAIAMAG, 1, ioptr) != 1)
			buf[0] = '\0';

		(void) fseek(ioptr, 0L, 0);
#else
#ifdef PORT5AR
		if (fread((char *)&arbuf, sizeof(arbuf), 1, ioptr) != 1)
			arbuf.ar_magic[0] = '\0';

		(void) fseek(ioptr, 0L, 0);
#endif
#endif
#endif
		if (fread((char *)&type, (size_t)sizeof(type), (size_t)1, ioptr) != 1
			|| (ldptr = allocldptr()) == NULL)
		{
			(void) fclose(ioptr);
			return(NULL);
		}
#ifdef PORTAR
		if (strncmp(buf, ARMAG, SARMAG) == 0)
		{
			long ar_size;

			TYPE(ldptr) = (unsigned short) ARTYPE;

			if (fseek(ioptr, (long int) (sizeof(char) * SARMAG), 0)
					== OKFSEEK
				&& fread((char *)&arbuf, (size_t)sizeof(arbuf),(size_t)1,
					ioptr) == 1
				&& !strncmp(arbuf.ar_fmag, ARFMAG,
					sizeof(arbuf.ar_fmag))
				&& arbuf.ar_name[0] == '/'
				&& sscanf(arbuf.ar_size, "%ld", &ar_size) == 1)
			{
				OFFSET(ldptr) = sizeof(char) * SARMAG +
					2 * sizeof(struct ar_hdr) +
					((ar_size + 01) & ~01);
			}
			else
				OFFSET(ldptr) = sizeof(char) * SARMAG +
					sizeof(struct ar_hdr);
		}
		else
		{
			TYPE(ldptr) = type;
			OFFSET(ldptr) = 0L;
		}
#else
#ifdef AIXV3AR
		if (strncmp(buf, AIAMAG, SAIAMAG) == 0)
		{
			TYPE(ldptr) = (unsigned short)ARTYPE;

			if (fseek(ioptr, 0L, 0) == OKFSEEK
				&& fread((char *) &flbuf, (size_t)sizeof(flbuf), (size_t)1,
					ioptr) == 1
				&& (AR_OFF(ldptr) = atol(flbuf.fl_fstmoff))
					!= 0L
				&& fseek(ioptr, (long int)atol(flbuf.fl_fstmoff), 0)
					== OKFSEEK
				&& fread((char *)&arbuf, (size_t)sizeof(arbuf), (size_t)1,
					ioptr) == 1
				&& fseek(ioptr, (long int)((atol(arbuf.ar_namlen)+01)
					& ~01), 1) == OKFSEEK)
			{
				OFFSET(ldptr) = (long)ftell(ioptr);
				AR_OFF(ldptr) = (long)atol(flbuf.fl_fstmoff);
				AR_END_OFF(ldptr)= 0L;
			}
			else
			{
				OFFSET(ldptr) = 0L;
				AR_OFF(ldptr) = 0L;
				AR_END_OFF(ldptr)= 0L;
			}
		}
		else
		{
			TYPE(ldptr) = type;
			OFFSET(ldptr) = 0L;
			AR_OFF(ldptr) = 0L;
		}
#else
#ifdef PORT5AR
		if (strncmp(arbuf.ar_magic, ARMAG, SARMAG) == 0)
		{
			TYPE(ldptr) = (unsigned short)ARTYPE;
			nsyms = sgetl(arbuf.ar_syms);

			OFFSET(ldptr) = (nsyms * sizeof(struct ar_sym)) +
				sizeof(struct arf_hdr) + sizeof(arbuf);
		}
		else
		{
			TYPE(ldptr) = type;
			OFFSET(ldptr) = 0L;
		}
#else
		if (sizeof(ARTYPE) == sizeof(type)
			|| fread((char *)&atype, (size_t)sizeof(atype), (size_t)1, ioptr) != 1)
		{
			atype = type;
		}

		if (atype == (unsigned short)ARTYPE
			|| type == (unsigned short)ARTYPE)
		{
			TYPE(ldptr) = (unsigned short)ARTYPE;
			OFFSET(ldptr) = ARCHSZ + sizeof(ARMAG);
		}
		else
		{
			TYPE(ldptr) = type;
			OFFSET(ldptr) = 0L;
		}
#endif
#endif
#endif
		IOPTR(ldptr) = ioptr;

		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
			FREAD((char *)&(HEADER(ldptr)), (size_t)FILHSZ, (size_t)1, ldptr) == 1)
		{
			return(ldptr);
		}
	}
	else if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK)
		return(ldptr);

	(void) fclose(IOPTR(ldptr));
	(void) freeldptr(ldptr);
	return(NULL);
}

/* static char ID[] = "ldopen.c: 1.3 2/16/83"; */
