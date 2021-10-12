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
 * Revision 1.2.2.3  1992/12/03  19:04:42  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:39  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:22:41  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:56:18  gm]
 * 
 * Revision 1.2  1991/12/05  20:41:27  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  15:32:45  mckeen]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)37  1.1  src/bldenv/make/BSDARCH/arch_fmtdep.c, bldprocess, bos412, GOLDA411a 1/19/94 15:57:19";
#endif /* not lint */

#include    <sys/types.h>
#include    <sys/time.h>
#include    <ctype.h>
#include    <ar.h>
#include    <ranlib.h>
#include    <stdio.h>
#include    "make.h"
#include    "hash.h"

#ifndef RANLIBMAG
#define RANLIBMAG "__.SYMDEF"
#endif

static struct ar_hdr size_arh;
#define AR_MAX_NAME_LEN	    (sizeof(size_arh.ar_name)-1)

void
ArchFixMembName(string_t *memberPtr)
{
    int len;
    char *member;
    string_t old_member;

    if (DEBUG(ARCH)) {
	printf("ArchFixMembName(%s)\n", (*memberPtr)->data);
    }
    len = (*memberPtr)->len;
    if (len <= AR_MAX_NAME_LEN)
	return;
    old_member = *memberPtr;
    member = strndup((*memberPtr)->data, AR_MAX_NAME_LEN);
    *memberPtr = string_create(member);
    string_deref(old_member);
    if (DEBUG(ARCH)) {
	printf("ArchFixMembName: truncated to %s\n", (*memberPtr)->data);
    }
}

int
ArchReadHdr(FILE *arch, void **hdrInfoPtr)
{
    char	  magic[SARMAG];

    if ((fread (magic, SARMAG, 1, arch) != 1) ||
    	(strncmp (magic, ARMAG, SARMAG) != 0)) {
	if (DEBUG(ARCH))
	    printf("ArchReadHdr: bad header\n");
	return 0;
    }
    if (DEBUG(ARCH))
	printf("ArchReadHdr: header OK\n");
    return 1;
}

int
ArchReadMember(FILE *arch,
	       string_t *memNamePtr,
	       struct ar_hdr *arhPtr,
	       void *hdrInfo)
{
    char memName[AR_MAX_NAME_LEN+1];
    char *cp;

    if (fread ((char *)arhPtr, sizeof(struct ar_hdr), 1, arch) != 1)
	return 0;
    if (strncmp (arhPtr->ar_fmag, ARFMAG, sizeof(arhPtr->ar_fmag)) != 0) {
	/*
	 * The header is bogus, so the archive is bad
	 * and there's no way we can recover...
	 */
	return -1;
    }
    (void) strncpy (memName, arhPtr->ar_name, sizeof(arhPtr->ar_name));
    for (cp = &memName[AR_MAX_NAME_LEN]; *cp == ' '; cp--)
	continue;
    *++cp = '\0';
    *memNamePtr = string_create(memName);
    return 1;
}

void
ArchToNextMember(FILE *arch, struct ar_hdr *arhPtr, void *hdrInfo)
{
    int size;

    arhPtr->ar_size[sizeof(arhPtr->ar_size)-1] = '\0';
    (void) sscanf (arhPtr->ar_size, "%10d", &size);
    fseek (arch, (size + 1) & ~1, 1);
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
