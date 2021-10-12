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
 * Revision 1.2.2.3  1992/12/03  19:04:40  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:37  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:22:33  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:56:13  gm]
 * 
 * Revision 1.2  1991/12/05  20:41:25  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  15:32:24  mckeen]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)25  1.3  src/bldenv/make/AIXARCH/arch_fmtdep.c, bldprocess, bos412, GOLDA411a 1/19/94 16:25:47";
#endif /* not lint */

#include    <sys/types.h>
#include    <sys/time.h>
#include    <ctype.h>
#include    <ar.h>
#include    <stdio.h>
#include    "make.h"
#include    "hash.h"

#define AR_MAX_NAME_LEN	    255

struct hdrInfo {
    long next_member, last_member, found_last;
};

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
    struct fl_hdr flhdr;
    struct hdrInfo *hi;

    if ((fread (&flhdr, FL_HSZ, 1, arch) != 1) ||
    	(strncmp (flhdr.fl_magic, AIAMAG, SAIAMAG) != 0)) {
	if (DEBUG(ARCH))
	    printf("ArchReadHdr: bad header\n");
	return 0;
    }
    hi = (struct hdrInfo *) emalloc(sizeof(struct hdrInfo));
    hi->last_member = atol(flhdr.fl_lstmoff);
    hi->next_member = atol(flhdr.fl_fstmoff);
    hi->found_last = 0;
    *hdrInfoPtr = (void *)hi;
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
    struct hdrInfo *hi = (struct hdrInfo *) hdrInfo;
    char memName[AR_MAX_NAME_LEN+1];
    long len;

    if (!hi->next_member || hi->found_last)
	return 0;
    hi->found_last = (hi->next_member == hi->last_member);

    fseek(arch, hi->next_member, 0);
    fread(arhPtr, AR_HSZ, 1, arch);
    memName[0] = arhPtr->_ar_name.ar_name[0];
    memName[1] = arhPtr->_ar_name.ar_name[1];
    len = atol(arhPtr->ar_namlen);
    fread(&memName[2], len - 2, 1, arch);
    memName[len] = '\0';
    *memNamePtr = string_create(memName);
    return 1;
}

void
ArchToNextMember(FILE *arch, struct ar_hdr *arhPtr, void *hdrInfo)
{
    struct hdrInfo *hi = (struct hdrInfo *) hdrInfo;

    hi->next_member = atol(arhPtr->ar_nxtmem);
}

void
ArchTouchTOC(GNode *gn)
{
}

void
ArchTOCTime(GNode *gn, Boolean *oodatePtr)
{
    *oodatePtr = FALSE;
}
