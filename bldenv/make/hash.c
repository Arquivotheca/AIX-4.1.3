/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Hash_CreateEntry
 *		Hash_CreateString
 *		Hash_DeleteEntry
 *		Hash_DeleteTable
 *		Hash_EnumFirst
 *		Hash_EnumNext
 *		Hash_FindEntry
 *		Hash_FindString
 *		Hash_InitTable
 *		RebuildTable
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
 * $Log: hash.c,v $
 * Revision 1.2.2.3  1992/12/03  19:05:24  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:14  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:24:04  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:54:06  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:42  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  16:00:21  mckeen]
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
 */

#ifndef lint
static char sccsid[] = "@(#)38  1.4  src/bldenv/make/hash.c, bldprocess, bos412, GOLDA411a 1/19/94 16:27:26";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)hash.c	5.5 (Berkeley) 12/28/90";
#endif /* not lint */

/* hash.c --
 *
 * 	This module contains routines to manipulate a hash table.
 * 	See hash.h for a definition of the structure of the hash
 * 	table.  Hash tables grow automatically as the amount of
 * 	information increases.
 */

#include "make.h"
#include "hash.h"

/*
 * Forward references to local procedures that are used before they're
 * defined:
 */

static void		RebuildTable(Hash_Table *);

/* 
 * The following defines the ratio of # entries to # buckets
 * at which we rebuild the table to make it larger.
 */

#define rebuildLimit 8

/*
 *---------------------------------------------------------
 * 
 * Hash_InitTable --
 *
 *	This routine just sets up the hash table.
 *
 * Results:	
 *	None.
 *
 * Side Effects:
 *	Memory is allocated for the initial bucket area.
 *
 *---------------------------------------------------------
 */

void
Hash_InitTable(
	register Hash_Table *t,	/* Structure to use to hold table. */
	int numBuckets)		/* How many buckets to create for starters.
				 * This number is rounded up to a power of
				 * two.   If <= 0, a reasonable default is
				 * chosen. The table will grow in size later
				 * as needed. */
{
	register int i;
	register struct Hash_Entry **hp;

	/*
	 * Round up the size to a power of two. 
	 */
	if (numBuckets <= 0)
		i = 16;
	else {
		for (i = 2; i < numBuckets; i <<= 1)
			 /* void */ ;
	}
	t->numEntries = 0;
	t->size = i;
	t->mask = i - 1;
	t->bucketPtr = hp = (struct Hash_Entry **)calloc(sizeof(*hp), i);
	if (hp == NULL)
		enomem();
}

/*
 *---------------------------------------------------------
 *
 * Hash_DeleteTable --
 *
 *	This routine removes everything from a hash table
 *	and frees up the memory space it occupied (except for
 *	the space in the Hash_Table structure).
 *
 * Results:	
 *	None.
 *
 * Side Effects:
 *	Lots of memory is freed up.
 *
 *---------------------------------------------------------
 */

void
Hash_DeleteTable(Hash_Table *t)
{
	register struct Hash_Entry **hp, *h, *nexth;
	register int i;

	for (hp = t->bucketPtr, i = t->size; --i >= 0;) {
		for (h = *hp++; h != NULL; h = nexth) {
			nexth = h->next;
			free((char *)h);
		}
	}
	free((char *)t->bucketPtr);

	/*
	 * Set up the hash table to cause memory faults on any future access
	 * attempts until re-initialization. 
	 */
	t->bucketPtr = NULL;
}

/*
 *---------------------------------------------------------
 *
 * Hash_FindEntry --
 *
 * 	Searches a hash table for an entry corresponding to key.
 *
 * Results:
 *	The return value is a pointer to the entry for key,
 *	if key was present in the table.  If key was not
 *	present, NULL is returned.
 *
 * Side Effects:
 *	None.
 *
 *---------------------------------------------------------
 */

Hash_Entry *
Hash_FindEntry(
	Hash_Table *t,		/* Hash table to search. */
	string_t key)		/* A hash key. */
{
	register Hash_Entry *e;
	register unsigned h;
	register string_t p;

	h = key->hashval;
	p = key;
	for (e = t->bucketPtr[h & t->mask]; e != NULL; e = e->next)
		if (e->name == p)
			return (e);
	return (NULL);
}

/*
 *---------------------------------------------------------
 *
 * Hash_FindString --
 *
 * 	Searches a hash table for an entry corresponding to string.
 *
 * Results:
 *	The return value is a pointer to the entry for key,
 *	if key was present in the table.  If key was not
 *	present, NULL is returned.
 *
 * Side Effects:
 *	None.
 *
 *---------------------------------------------------------
 */

string_t
Hash_FindString(
	Hash_Table *t,		/* Hash table to search. */
	string_t str)		/* A string. */
{
	register Hash_Entry *e;
	register unsigned h;
	register int l;
	register string_t en;

	h = str->hashval;
	l = str->len;
	for (e = t->bucketPtr[h & t->mask]; e != NULL; e = e->next) {
		en = e->name;
		if (en->hashval == h && en->len == l &&
		    memcmp(en->data, str->data, l) == 0)
			return (en);
	}
	return ((string_t) NULL);
}

/*
 *---------------------------------------------------------
 *
 * Hash_CreateEntry --
 *
 *	Searches a hash table for an entry corresponding to
 *	key.  If no entry is found, then one is created.
 *
 * Results:
 *	The return value is a pointer to the entry.  If *newPtr
 *	isn't NULL, then *newPtr is filled in with TRUE if a
 *	new entry was created, and FALSE if an entry already existed
 *	with the given key.
 *
 * Side Effects:
 *	Memory may be allocated, and the hash buckets may be modified.
 *---------------------------------------------------------
 */

Hash_Entry *
Hash_CreateEntry(
	register Hash_Table *t,	/* Hash table to search. */
	string_t key,		/* A hash key. */
	Boolean *newPtr)	/* Filled in with TRUE if new entry created,
				 * FALSE otherwise. */
{
	register Hash_Entry *e;
	register unsigned h;
	register string_t p;
	struct Hash_Entry **hp;

	/*
	 * Hash the key.  As a side effect, save the length (strlen) of the
	 * key in case we need to create the entry.
	 */
	h = key->hashval;
	p = key;
	for (e = t->bucketPtr[h & t->mask]; e != NULL; e = e->next) {
		if (e->name == p) {
			if (newPtr != NULL)
				*newPtr = FALSE;
			return (e);
		}
	}

	/*
	 * The desired entry isn't there.  Before allocating a new entry,
	 * expand the table if necessary (and this changes the resulting
	 * bucket chain). 
	 */
	if (t->numEntries >= rebuildLimit * t->size)
		RebuildTable(t);
	e = (Hash_Entry *) emalloc(sizeof(*e));
	hp = &t->bucketPtr[h & t->mask];
	e->next = *hp;
	*hp = e;
	e->clientData = NULL;
	e->name = string_ref(p);
	t->numEntries++;

	if (newPtr != NULL)
		*newPtr = TRUE;
	return (e);
}

/*
 *---------------------------------------------------------
 *
 * Hash_CreateString --
 *
 *	Create a new hash table entry for string .
 *
 * Results:
 *	An entry is added to the hash table for the string.
 *
 * Side Effects:
 *	Memory may be allocated, and the hash buckets may be modified.
 *---------------------------------------------------------
 */

void
Hash_CreateString(
	register Hash_Table *t,	/* Hash table. */
	string_t str)		/* A string. */
{
	register Hash_Entry *e;
	register unsigned h;
	struct Hash_Entry **hp;

	/*
	 * Before allocating a new entry, expand the table if necessary
	 * (and this changes the resulting bucket chain).
	 */
	if (t->numEntries >= rebuildLimit * t->size)
		RebuildTable(t);
	e = (Hash_Entry *) emalloc(sizeof(*e));
	h = str->hashval;
	hp = &t->bucketPtr[h & t->mask];
	e->next = *hp;
	*hp = e;
	e->clientData = NULL;
	e->name = str;
	t->numEntries++;
}

/*
 *---------------------------------------------------------
 *
 * Hash_DeleteEntry --
 *
 * 	Delete the given hash table entry and free memory associated with
 *	it.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Hash chain that entry lives in is modified and memory is freed.
 *
 *---------------------------------------------------------
 */

void
Hash_DeleteEntry(Hash_Table *t, Hash_Entry *e)
{
	register Hash_Entry **hp, *p;

	if (e == NULL)
		return;
	for (hp = &t->bucketPtr[e->name->hashval & t->mask];
	     (p = *hp) != NULL; hp = &p->next) {
		if (p == e) {
			*hp = p->next;
			free((char *)p);
			t->numEntries--;
			return;
		}
	}
	(void) write(2, "bad call to Hash_DeleteEntry\n", 29);
	abort();
}

/*
 *---------------------------------------------------------
 *
 * Hash_EnumFirst --
 *	This procedure sets things up for a complete search
 *	of all entries recorded in the hash table.
 *
 * Results:	
 *	The return value is the address of the first entry in
 *	the hash table, or NULL if the table is empty.
 *
 * Side Effects:
 *	The information in searchPtr is initialized so that successive
 *	calls to Hash_Next will return successive HashEntry's
 *	from the table.
 *
 *---------------------------------------------------------
 */

Hash_Entry *
Hash_EnumFirst(
	Hash_Table *t,			/* Table to be searched. */
	register Hash_Search *searchPtr)/* Area in which to keep state 
					 * about search.*/
{
	searchPtr->tablePtr = t;
	searchPtr->nextIndex = 0;
	searchPtr->hashEntryPtr = NULL;
	return Hash_EnumNext(searchPtr);
}

/*
 *---------------------------------------------------------
 *
 * Hash_EnumNext --
 *    This procedure returns successive entries in the hash table.
 *
 * Results:
 *    The return value is a pointer to the next HashEntry
 *    in the table, or NULL when the end of the table is
 *    reached.
 *
 * Side Effects:
 *    The information in searchPtr is modified to advance to the
 *    next entry.
 *
 *---------------------------------------------------------
 */

Hash_Entry *
Hash_EnumNext(Hash_Search *searchPtr) /* Area used to keep state about 
					 search. */
{
	register Hash_Entry *e;
	Hash_Table *t = searchPtr->tablePtr;

	/*
	 * The hashEntryPtr field points to the most recently returned
	 * entry, or is nil if we are starting up.  If not nil, we have
	 * to start at the next one in the chain.
	 */
	e = searchPtr->hashEntryPtr;
	if (e != NULL)
		e = e->next;
	/*
	 * If the chain ran out, or if we are starting up, we need to
	 * find the next nonempty chain.
	 */
	while (e == NULL) {
		if (searchPtr->nextIndex >= t->size)
			return (NULL);
		e = t->bucketPtr[searchPtr->nextIndex++];
	}
	searchPtr->hashEntryPtr = e;
	return (e);
}

/*
 *---------------------------------------------------------
 *
 * RebuildTable --
 *	This local routine makes a new hash table that
 *	is larger than the old one.
 *
 * Results:	
 * 	None.
 *
 * Side Effects:
 *	The entire hash table is moved, so any bucket numbers
 *	from the old table are invalid.
 *
 *---------------------------------------------------------
 */

static void
RebuildTable(register Hash_Table *t)
{
	register Hash_Entry *e, *next, **hp, **xp;
	register int i, mask;
        register Hash_Entry **oldhp;
	int oldsize;

	oldhp = t->bucketPtr;
	oldsize = i = t->size;
	i <<= 1;
	t->size = i;
	t->mask = mask = i - 1;
	t->bucketPtr = hp = (struct Hash_Entry **) emalloc(sizeof(*hp) * i);
	while (--i >= 0)
		*hp++ = NULL;
	for (hp = oldhp, i = oldsize; --i >= 0;) {
		for (e = *hp++; e != NULL; e = next) {
			next = e->next;
			xp = &t->bucketPtr[e->name->hashval & mask];
			e->next = *xp;
			*xp = e;
		}
	}
	free((char *)oldhp);
}
