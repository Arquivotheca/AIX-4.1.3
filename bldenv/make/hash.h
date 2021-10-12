/* @(#)39       1.3  src/bldenv/make/hash.h, bldprocess, bos412, GOLDA411a 1/19/94 16:27:31
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Hash_GetValue
 *		Hash_SetValue
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
 * $Log: hash.h,v $
 * Revision 1.2.2.3  1992/12/03  19:05:26  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:15  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:24:10  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:54:13  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:45  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  16:00:27  mckeen]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)hash.h	5.4 (Berkeley) 12/28/90
 */

/* hash.h --
 *
 * 	This file contains definitions used by the hash module,
 * 	which maintains hash tables.
 */

#ifndef	_HASH
#define	_HASH

#include "sprite.h"
#include "str.h"

/* 
 * The following defines one entry in the hash table.
 */

typedef struct Hash_Entry {
    struct Hash_Entry *next;		/* Used to link together all the
    					 * entries associated with the same
					 * bucket. */
    ClientData	      clientData;	/* Arbitrary piece of data associated
    					 * with key. */
    string_t	      name;		/* key string */
} Hash_Entry;

typedef struct Hash_Table {
    struct Hash_Entry **bucketPtr;/* Pointers to Hash_Entry, one
    				 * for each bucket in the table. */
    int 	size;		/* Actual size of array. */
    int 	numEntries;	/* Number of entries in the table. */
    int 	mask;		/* Used to select bits for hashing. */
} Hash_Table;

/* 
 * The following structure is used by the searching routines
 * to record where we are in the search.
 */

typedef struct Hash_Search {
    Hash_Table  *tablePtr;	/* Table being searched. */
    int 	nextIndex;	/* Next bucket to check (after current). */
    Hash_Entry 	*hashEntryPtr;	/* Next entry to check in current bucket. */
} Hash_Search;

/*
 * Macros.
 */

/*
 * ClientData Hash_GetValue(h) 
 *     Hash_Entry *h; 
 */

#define Hash_GetValue(h) ((h)->clientData)

/* 
 * Hash_SetValue(h, val); 
 *     Hash_Entry *h; 
 *     char *val; 
 */

#define Hash_SetValue(h, val) ((h)->clientData = (ClientData) (val))

/*
 * The following procedure declarations and macros
 * are the only things that should be needed outside
 * the implementation code.
 */

extern Hash_Entry *	Hash_CreateEntry(Hash_Table *, string_t, Boolean *);
extern void		Hash_CreateString(Hash_Table *, string_t);
extern void		Hash_DeleteEntry(Hash_Table *, Hash_Entry *);
extern void		Hash_DeleteTable(Hash_Table *);
extern Hash_Entry *	Hash_EnumFirst(Hash_Table *, Hash_Search *);
extern Hash_Entry *	Hash_EnumNext(Hash_Search *);
extern Hash_Entry *	Hash_FindEntry(Hash_Table *, string_t);
extern string_t		Hash_FindString(Hash_Table *, string_t);
extern void		Hash_InitTable(Hash_Table *, int);

#endif /* _HASH */
