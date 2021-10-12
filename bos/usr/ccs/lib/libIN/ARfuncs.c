static char sccsid[] = "@(#)06	1.12  src/bos/usr/ccs/lib/libIN/ARfuncs.c, libIN, bos411, 9428A410j 11/10/93 15:12:56";
/*
 * LIBIN: ARfuncs
 *
 * ORIGIN: 9,10
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1993
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Functions to process archives without the user having to
 *	     know what format the archive is in.  Handles System III,
 *	     System V.0, Berkeley 4BSD and System V.2 formats.
 * IMPORTANT NOTE: This file is used in the creation of the build environment
 *	     tools(xnm and xstrip).  The definition AIXV2 is used to indicate
 *           code required to build in the version 2 environment.  After
 *           making any changes to this file,  it is necessary to also test
 *           the building and execution of the build environment tools.
 */

#include <stdio.h>
#ifndef AIXV2
#include <stdlib.h>
#include <string.h>
#endif /* not AIXV2 */
#include <IN/CSdefs.h>
#include <IN/ARdefs.h>
#include <nl_types.h>
#include "libIN_msg.h"

extern long	sgetl(); 

static long	next_off;	/* Used for AIX indexed archive format */
static char	name_buf[260];	/* Name buffer for member names */
static char	SeekError[] = "AR: unable to seek in archive (?)\n";
static char	NameError[] = "AR: bad member name in archive (?)\n";


/* System III definitions */
#define	S3ARMAG	0177545
#define S3SARMAG (sizeof(int))
typedef struct
{       char ar_name[14];
	long ar_date;
	char ar_uid;
	char ar_gid;
	int ar_mode;
	long ar_size;
} S3ar_hdr;


/* System V.0 definitions */
#define	S5ARMAG	"<ar>"
#define S5SARMAG 4
typedef struct  /* archive header */
{       char ar_magic[4];
	char ar_name[16];
	char ar_date[4];
	char ar_syms[4];
} S5ar_hdr;
typedef struct
{       char sym_name[8];
	char sym_ptr[4];
} S5ar_sym;
typedef struct  /* archive file member header */
{       char arf_name[16];
	char arf_date[4];
	char arf_uid[4];
	char arf_gid[4];
	char arf_mode[4];
	char arf_size[4];
} S5arf_hdr;


/* SysV.2 and Berkeley 4BSD definitions */
#define BKARMAG	"!<arch>\n"
#define BKSARMAG 8
#define BKARFMAG "`\n"
typedef struct
{       char ar_name[16];
	char ar_date[12];
	char ar_uid[6];
	char ar_gid[6];
	char ar_mode[8];
	char ar_size[10];
	char ar_fmag[2];
} BKar_hdr;


/* AIX 3.1 definitions */
#define AIAFARMAG	"<aiaff>\n"
#define	AIAFSARMAG	8
typedef struct
{	char fl_magic[AIAFSARMAG];
	char fl_memoff[12];
	char fl_gstoff[12];
	char fl_fstmoff[12];
	char fl_lstmoff[12];
	char fl_freeoff[12];
} AIAFfl_hdr;

typedef struct
{	
	char ar_size[12];
	char ar_nxtmem[12];
	char ar_prvmem[12];
	char ar_date[12];
	char ar_uid[12];
	char ar_gid[12];
	char ar_mode[12];
	char ar_namlen[4];
	union
	{	char ar_name[2];
		char ar_fmag[2];
	}	_ar_name;
} AIAFar_hdr;


/*
 * NAME: ARisarchive
 *
 * FUNCTION: Given a FILE, determine an archive starts at the current
 *	     position in the file.  Leave the file positioned as it was
 *	     when this function was entered. 
 *
 * PARAMETERS: File descriptor of file to check.
 *
 * RETURN VALUE DESCRIPTION:
 *	      0 if not an archive,
 *	      3 for System III,
 *	      5 for System V, or
 *	     42 for Berkeley 4.2 and for System V.2.
 *	     47 for AIX Indexed archive file format
 */
int ARisarchive(file)
FILE *file;
{       union
	{
		int magic3;
		char magic5[S5SARMAG];
		char magicB[BKSARMAG];
		char magicI[AIAFSARMAG];
	} magic;
	long curloc = ftell(file);
	nl_catd catd;	/* descriptor for message catalog file */

#ifdef AIXV2
	if (fread((void *)&magic, 1, sizeof(magic), file) < S3SARMAG)
#else /* AIXV2 */
	if (fread((void *)&magic, (size_t)1, (size_t)sizeof(magic), file) < S3SARMAG)
#endif /* AIXV2 */
		return(0);

	if (fseek(file,(long)curloc,0) != 0)
	{
		catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
		fprintf(stderr, catgets(catd, MS_LIBIN,
			M_SEEKERROR, SeekError));
		catclose(catd);
		return(0);
	}

	return(	magic.magic3 == S3ARMAG ? S3_AR_ID :
#ifdef AIXV2
		(strncmp(magic.magic5, S5ARMAG, S5SARMAG) == 0 ? S5_AR_ID :
		(strncmp(magic.magicI, AIAFARMAG, AIAFSARMAG) == 0 ? AIAF_AR_ID:
		(strncmp(magic.magicB, BKARMAG, BKSARMAG) == 0 ? BK_AR_ID :
#else /* AIXV2 */
		(strncmp(magic.magic5, S5ARMAG, (size_t)S5SARMAG) == 0 ? S5_AR_ID :
		(strncmp(magic.magicI, AIAFARMAG, (size_t)AIAFSARMAG) == 0 ? AIAF_AR_ID:
		(strncmp(magic.magicB, BKARMAG, (size_t)BKSARMAG) == 0 ? BK_AR_ID :
#endif /* AIXV2 */
		NO_AR_ID))));
}


/*
 * NAME: ARforeach
 *
 * FUNCTION: Given a FILE, call the function parameter once for each
 *	     member of the archive, passing the FILE (positioned to the
 *	     beginning of the archive), a pointer to the name of the
 *	     member, and the size. 
 *
 * PARAMETERS: File descriptor for archive file.
 *	     Pointer to function to call for each member.
 *
 * RETURN VALUE DESCRIPTION:
 *	      0 for success,
 *	     -1 for internal problems, or
 *	     fcn return code if it is ever non-zero.
 */
int ARforeach(file,fcn)
FILE *file;
int (*fcn)();
{
	nl_catd catd;	/* descriptor for message catalog file */
	register artype = ARisarchive(file);
	register ahdrlen,
		fhdrlen;
	int	retval,
		done = 0;
	ARparm	parm;
	union
	{
		S5ar_hdr S5arhdr;
		AIAFfl_hdr AIAFflhdr;
	} header;
	union
	{
		S3ar_hdr S3hdr; 
		S5arf_hdr S5hdr;
		BKar_hdr BKhdr;
		AIAFar_hdr AIAFhdr;
	} fileheader;

	/* Set variables for read length based on archive format */
	switch (artype)
	{
		case NO_AR_ID:
			return(-1);
		case S3_AR_ID:
			ahdrlen = S3SARMAG;
			fhdrlen = sizeof(S3ar_hdr);
			break;
		case S5_AR_ID:
			ahdrlen = sizeof(S5ar_hdr);
			fhdrlen = sizeof(S5arf_hdr);
			break;
		case AIAF_AR_ID:
			ahdrlen = sizeof(AIAFfl_hdr);
			fhdrlen = sizeof(AIAFar_hdr);
			break;
		case BK_AR_ID:
			ahdrlen = BKSARMAG;
			fhdrlen = sizeof(BKar_hdr);
			break;
	}

	/* Read in identifying information at beginning of archive */
#ifdef AIXV2
	fread((void *)&header, ahdrlen, 1, file);
#else /* AIXV2 */
	fread((void *)&header, (size_t)ahdrlen, (size_t)1, file);
#endif /* AIXV2 */

	/* System V and AIX 3.1 need special positioning to first member */
	if (artype == S5_AR_ID)
		fseek(file,(long int)(sgetl(header.S5arhdr.ar_syms)*sizeof(S5ar_sym)),1);
	else if(artype == AIAF_AR_ID)
	{
		next_off = atol(header.AIAFflhdr.fl_fstmoff);
		fseek(file, (long)next_off, 0);
	}

	/* Initialize variables in the ARparm structure */
	parm.file = file;
	parm.name = name_buf;
	parm.name[14] = 0;

	/* For each member of the archive */
	while (!feof(file) && !done)
	{
		long newpos;

		parm.oldpos = ftell(file);	/* Save current position */

		/* Read in the member header */
#ifdef AIXV2
		if (fread((void *)&fileheader,fhdrlen,1,file)!=1) break;
#else /* AIXV2 */
		if (fread((void *)&fileheader,(size_t)fhdrlen,(size_t)1,file)!=1) break;
#endif /* AIXV2 */

		/* Process according to type */
		switch (artype)
		{
		case S3_AR_ID:
#ifdef AIXV2
			strncpy(parm.name,fileheader.S3hdr.ar_name,14);
#else /* AIXV2 */
			strncpy(parm.name,fileheader.S3hdr.ar_name,(size_t)14);
#endif /* AIXV2 */
			parm.date=fileheader.S3hdr.ar_date;
			parm.uid=fileheader.S3hdr.ar_uid;
			parm.gid=fileheader.S3hdr.ar_gid;
			parm.mode=(unsigned)fileheader.S3hdr.ar_mode;
			parm.size=fileheader.S3hdr.ar_size;
			break;
		case S5_AR_ID:
#ifdef AIXV2
			strncpy(parm.name,fileheader.S5hdr.arf_name,14);
#else /* AIXV2 */
			strncpy(parm.name,fileheader.S5hdr.arf_name,(size_t)14);
#endif /* AIXV2 */
			parm.date=sgetl(fileheader.S5hdr.arf_date);
			parm.uid=sgetl(fileheader.S5hdr.arf_uid);
			parm.gid=sgetl(fileheader.S5hdr.arf_gid);
			parm.mode=sgetl(fileheader.S5hdr.arf_mode);
			parm.size=sgetl(fileheader.S5hdr.arf_size);
			break;
		case AIAF_AR_ID:
		  { register long name_len;

			parm.size=atol(fileheader.AIAFhdr.ar_size);
			next_off=atol(fileheader.AIAFhdr.ar_nxtmem);
		    	parm.date=atol(fileheader.AIAFhdr.ar_date);
		  	parm.uid=atol(fileheader.AIAFhdr.ar_uid);
			parm.gid=atol(fileheader.AIAFhdr.ar_gid);
			parm.mode=strtol(fileheader.AIAFhdr.ar_mode,(char **)NULL,8);
			name_len = atol(fileheader.AIAFhdr.ar_namlen);
			name_len++; name_len &= ~0x01;
			strncpy(parm.name,
#ifdef AIXV2
				fileheader.AIAFhdr._ar_name.ar_name, 2);
#else /* AIXV2 */
				fileheader.AIAFhdr._ar_name.ar_name, (size_t)2);
#endif /* AIXV2 */
			if(name_len > 2)
			{
#ifdef AIXV2
	  			if(fread((void *)&parm.name[2], 1, name_len, file)
#else /* AIXV2 */
	  			if(fread((void *)&parm.name[2], (size_t)1, (size_t)name_len, file)
#endif /* AIXV2 */
				!= name_len)
				{
					catd = catopen (MF_LIBIN, NL_CAT_LOCALE);
					fprintf(stderr, catgets(catd,
						MS_LIBIN, M_NAMEERROR,
						NameError));
					catclose(catd);
					return(-1);
				}
			}
			parm.name[atol(fileheader.AIAFhdr.ar_namlen)] = '\0';
		  }
		  break;
		case BK_AR_ID:
		  {   register char *cp;

#ifdef AIXV2
		    strncpy(parm.name,fileheader.BKhdr.ar_name,15);
#else /* AIXV2 */
		    strncpy(parm.name,fileheader.BKhdr.ar_name,(size_t)15);
#endif /* AIXV2 */
		    for (cp = &parm.name[14]; cp>=parm.name && *cp==' '; cp--)
			*cp=0;
		    if (*cp=='/') *cp=0;        /* System V.2 */
		    parm.date=atol(fileheader.BKhdr.ar_date);
		    parm.uid=atol(fileheader.BKhdr.ar_uid);
		    parm.gid=atol(fileheader.BKhdr.ar_gid);
		    parm.mode=strtol(fileheader.BKhdr.ar_mode,(char **)NULL,8);
		    parm.size=atol(fileheader.BKhdr.ar_size);
		  }
		  break;
		} /* switch */

		/* Member size is rounded up to long word */ 
		parm.size++; parm.size &= ~1;

		/* AIX 3.1 locates next member by offset in previous header */
		if(artype == AIAF_AR_ID)
		{
			if(!(newpos = next_off))
				done++;
	    	}
		else	/* All others add size to previous offset */
		{
			newpos = ftell(file) + parm.size;
		}

		/* Call specified routine and check for errors */
		if ((retval=(*fcn)(&parm)) != 0)
			return(retval);

		/* Go to next member in archive */
		if (fseek(file,(long)newpos,0) != 0)
		{
			catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
			fprintf(stderr, catgets(catd, MS_LIBIN,
				M_SEEKERROR, SeekError));
			catclose(catd);
			return(-1);
		}
	} /* while */

	return(0);
}
