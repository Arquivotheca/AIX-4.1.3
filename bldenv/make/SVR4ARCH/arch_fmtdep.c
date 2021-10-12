/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ArchFixMembName
 *		ArchReadHdr
 *		ArchReadMember
 *		ArchTOCTime
 *		ArchToNextMember
 *		ArchTouchTOC
 *		IS_AR_LONG_NAME
 *		IS_AR_STR_TABLE
 *		IS_AR_SYM_TABLE
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
 * Revision 1.1.2.3  1992/12/03  19:04:48  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:44  damon]
 *
 * Revision 1.1.2.2  1992/09/24  19:22:56  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:51:49  gm]
 * 
 * 	New SVR4 file
 * 
 * $EndLog$
 */
/*
 *  (c) Copyright 1991, 1992 Siemens-Nixdorf Information Systems, Burlington, MA, USA
 *  All Rights Reserved
 */

#ifndef lint
static char sccsid[] = "@(#)38  1.1  src/bldenv/make/SVR4ARCH/arch_fmtdep.c, bldprocess, bos412, GOLDA411a 1/19/94 15:57:41";
#endif /* not lint */

#include    <sys/types.h>
#include    <sys/time.h>
#include    <ctype.h>
#include    <ar.h>
#include    <stdio.h>
#include    "make.h"
#include    "hash.h"

/*
 * In SYSV.4:
 *
 * Archives contain no table of contents member.
 * The symbol table is the first archive member and has the name '/'.
 *	It is created if the archive contains any object files.
 * The string table is the second archive member if it exists.  It has the name '//'.
 *	It exists only if the name of one or more of the other 
 *	archive members is greater than 15 characters.
 * An archive member with a name longer than 15 characters has in its name field
 *	a '/', followed by the decimal representation of the offset
 *	into the string table of the name.  In the string table,
 *	each name is terminated by a '/' followed by a '\n' character.
 */
#define AR_MAX_NAME_LEN	    255
#define IS_AR_SYM_TABLE(a)	((a)->ar_name[0] == '/' && (a)->ar_name[1] == ' ')
#define IS_AR_STR_TABLE(a)	((a)->ar_name[0] == '/' && (a)->ar_name[1] == '/')
#define IS_AR_LONG_NAME(a)	((a)->ar_name[0] == '/' && (a)->ar_name[1] != ' ' && \
					(a)->ar_name[1] != '/')

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
	int i, size, offset;
	long whereami;
	static char *string_tbl = NULL;

reread:
	if (fread ((char *)arhPtr, sizeof(struct ar_hdr), 1, arch) != 1) {
		/*
		 * reinitialize string table for next archive
		 */
		if (string_tbl != NULL) {
			free(string_tbl);
			string_tbl = NULL;
		}
		return 0;
	}
	if (strncmp (arhPtr->ar_fmag, ARFMAG, sizeof(arhPtr->ar_fmag)) != 0) {
		/*
		 * The header is bogus, so the archive is bad
		 * and there's no way we can recover.  Reinitialize string table.
		 */
		if (DEBUG(ARCH))
			printf("ArchReadMember: bad archive header\n");
		if (string_tbl != NULL) {
			free(string_tbl);
			string_tbl = NULL;
		}
		return -1;

	}
	/*
	 * we don't cache symbol table or string table
	 */
	if (IS_AR_SYM_TABLE(arhPtr)) {
		if (DEBUG(ARCH))
			printf("ArchReadMember: read symbol table\n");
		ArchToNextMember(arch, arhPtr, hdrInfo);
		goto reread;
	} 
	if (IS_AR_STR_TABLE(arhPtr)) {
		/*
		 * If this is the string table, read it in for later use
		 */
		whereami = ftell (arch);
		(void) sscanf (arhPtr->ar_size, "%10d", &size);
		string_tbl = emalloc(size);
		if (DEBUG(ARCH) || DEBUG(MAKE)) {
			printf("ArchReadMember: read archive string table (%d bytes)\n", size);
		}
		if ((fread (string_tbl, size, 1, arch) != 1)) {
			fclose (arch);
			return 0;
		}
		fseek(arch, whereami, 0);
		ArchToNextMember(arch, arhPtr, hdrInfo);
		goto reread;
	}
	if (IS_AR_LONG_NAME(arhPtr)) {
		/*
		 * If this is a long name, find the offset and copy out of 
		 * the string table.  Else, copy it out of the header.
	    	 * If we have a long name and no a string table, error.
	    	 */
	    	if (string_tbl == NULL) {
	    		fclose (arch);
	    		return 0;
	    	}
	    	(void) sscanf (&arhPtr->ar_name[1], "%10d", &offset);
	   	for (size = 0, cp = string_tbl + offset; *cp != '/'; cp++, size++)
			memName[size] = *cp;
		memName[size] = '\0';
	} else {
	   	/*
	    	 * Copy name into memName array and NULL-terminate it.  
		 * The '/' character terminates member names in SYSV.4
	    	 */
		for (cp = arhPtr->ar_name, i = 0; i < sizeof(arhPtr->ar_name); i++, cp++) {
			if ((memName[i] = *cp) == '/') {
				memName[i] = '\0';
				break;
			}
		}
	}
	*memNamePtr = string_create(memName);
	if (DEBUG(ARCH) || DEBUG(MAKE)) {
		printf("Archiving %s\n", memName);
	}
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

void
ArchTouchTOC(GNode *gn)
{
}

void
ArchTOCTime(GNode *gn, Boolean *oodatePtr)
{
	*oodatePtr = FALSE;
}

