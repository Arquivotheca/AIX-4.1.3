/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ArchFixMembName
 *		ArchReadHdr
 *		ArchReadMember
 *		ArchTOCTime
 *		ArchToNextMember
 *		ArchTouchTOC
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: arch_fmtdep.c,v $
 * Revision 1.2.4.4  1992/12/03  19:04:46  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:42  damon]
 *
 * Revision 1.2.4.3  1992/09/24  19:22:46  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:56:24  gm]
 * 
 * Revision 1.2.4.2  1992/03/16  14:54:03  mckeen
 * 	Fixed archive format changes with code from OSC
 * 	[1992/03/16  14:53:29  mckeen]
 * 
 * Revision 1.2  1991/12/05  20:41:52  devrcs
 * 	Fixes to long archive name support.
 * 	[91/09/12  15:06:09  marty]
 * 
 * 	Added support for new archive format
 * 	[91/09/12  13:18:37  mckeen]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  15:35:26  mckeen]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)26  1.3  src/bldenv/make/OSFARCH/arch_fmtdep.c, bldprocess, bos412, GOLDA411a 1/19/94 16:26:00";
#endif /* not lint */

#include    <sys/types.h>
#include    <sys/time.h>
#include    <ctype.h>
#include    <ar.h>
#include    <ranlib.h>
#include    <stdio.h>
#include    "make.h"
#include    "hash.h"

#ifndef AR_EFMT1
#define AR_EFMT1        "#1/"   /* extended format #1 */
#endif

typedef struct {
    int size;
} hdrInfo_t;

void
ArchFixMembName(string_t *memberPtr)
{
    if (DEBUG(ARCH)) {
	printf("ArchFixMembName(%s)\n", (*memberPtr)->data);
    }
}

int
ArchReadHdr(FILE *arch, void **vHdrInfoPtr)
{
    char	  magic[SARMAG];

    if ((fread (magic, SARMAG, 1, arch) != 1) ||
    	(strncmp (magic, ARMAG, SARMAG) != 0))
	return 0;
    *vHdrInfoPtr = (void *)malloc(sizeof(hdrInfo_t));
    return 1;
}

int
ArchReadMember(FILE *arch,
	       string_t *memNamePtr,
	       struct ar_hdr *arhPtr,
	       void *vHdrInfo)
{
    hdrInfo_t *hdrInfo = (hdrInfo_t *)vHdrInfo;
    char memName[NAME_MAX+1];
    char size[sizeof(arhPtr->ar_size)+1];
    char *cp;
    int  retval;
    int  count;

    if (fread ((char *)arhPtr, sizeof(struct ar_hdr), 1, arch) != 1)
	return 0;
    if (strncmp (arhPtr->ar_fmag, ARFMAG, sizeof(arhPtr->ar_fmag)) != 0) {
	/*
	 * The header is bogus, so the archive is bad
	 * and there's no way we can recover...
	 */
	return -1;
    }
    bcopy(arhPtr->ar_size, size, sizeof(size)-1);
    size[sizeof(size)] = 0;
    (void) sscanf (size, "%10d", &hdrInfo->size);
    if (hdrInfo->size&1) hdrInfo->size++;
    if (bcmp(arhPtr->ar_name, AR_EFMT1, sizeof(AR_EFMT1)-1) == 0 &&
	arhPtr->ar_name[sizeof(AR_EFMT1)-1] != ' ') {
	count = atoi(arhPtr->ar_name+sizeof(AR_EFMT1)-1);
	if (count <= 0 || count > NAME_MAX)
	    return -1;
	retval = fread(memName, (size_t) 1, (size_t) count, arch);
	if (retval != count)
	    return -1;
	memName[count] = 0;
	hdrInfo->size -= count;
    } else {
	bcopy(arhPtr->ar_name, memName, sizeof(arhPtr->ar_name));
	memName[sizeof(arhPtr->ar_name)] = 0;
	cp = strchr(memName, '/');
	if (cp)
	    *cp = 0;		/* Mark end of member */
	else {
	    cp = memName + sizeof(arhPtr->ar_name) - 1;
	    while (cp > memName && *cp == ' ')
	      cp--;
	    *++cp = 0;
	}
    }
    *memNamePtr = string_create(memName);
    return 1;
}

void
ArchToNextMember(FILE *arch, struct ar_hdr *arhPtr, void *vHdrInfo)
{
    hdrInfo_t *hdrInfo = (hdrInfo_t *) vHdrInfo;

    fseek (arch, hdrInfo->size, 1);
}

static string_t sRANLIBMAG = NULL;

void
ArchTouchTOC(GNode *gn)
{
    FILE *	    arch;	/* Stream open to archive */
    struct ar_hdr   arh;      	/* Header describing table of contents */

    if (sRANLIBMAG == NULL)
	sRANLIBMAG = string_create(RANLIBMAG);
    arch = ArchFindMember (gn->path, sRANLIBMAG, &arh, "r+");
    if (arch != (FILE *) NULL) {
	sprintf(arh.ar_date, "%-12ld", now);
	(void)fwrite ((char *)&arh, sizeof (struct ar_hdr), 1, arch);
	fclose (arch);
    }
}

void
ArchTOCTime(GNode *gn, Boolean *oodatePtr)
{
    struct ar_hdr *arhPtr;    /* Header for __.SYMDEF */
    int 	  modTimeTOC; /* The table-of-contents's mod time */

    if (sRANLIBMAG == NULL)
	sRANLIBMAG = string_create(RANLIBMAG);
    arhPtr = ArchStatMember (gn->path, sRANLIBMAG, FALSE);
    if (arhPtr != (struct ar_hdr *)NULL) {
	(void)sscanf (arhPtr->ar_date, "%12d", &modTimeTOC);

	if (DEBUG(ARCH) || DEBUG(MAKE)) {
	    printf("%s modified %s...", RANLIBMAG, Targ_FmtTime(modTimeTOC));
	}
	*oodatePtr = (gn->mtime > modTimeTOC);
    } else {
	/*
	 * A library w/o a table of contents is out-of-date
	 */
	if (DEBUG(ARCH) || DEBUG(MAKE)) {
	    printf("No t.o.c....");
	}
	*oodatePtr = TRUE;
    }
}
