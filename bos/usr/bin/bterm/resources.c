static char sccsid[] = "@(#)82	1.1  com/XTOP/lib/X/Xrm.c, gos, x11r5_320 5/13/92 14:42:43";
/*
 *   COMPONENT_NAME: XLIB
 *
 *   FUNCTIONS: RawValue
 *		LeafHash
 *		mbnoop
 *		lcname
 *		lcname
 *		NewDatabase
 *		MoveValues
 *		if
 *		XrmCombineDatabase
 *		XrmMergeDatabases
 *		sizeof
 *		if
 *		XrmQPutResource
 *		XrmPutResource
 *		XrmQPutStringResource
 *		GetDatabase
 *		DumpEntry
 *		GTIGHTLOOSE
 *		if
 *		while
 *		XrmQGetResource
 *		XrmGetResource
 *
 *   ORIGINS: 27,40,
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * $XConsortium: Xrm.c,v 1.72 92/01/10 14:21:12 rws Exp $
 */

/***********************************************************
Copyright 1987, 1988, 1990 by Digital Equipment Corporation, Maynard,
Massachusetts, and the Massachusetts Institute of Technology, Cambridge,
Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include	<stdio.h>
#include	<ctype.h>

#include	"X/Xlibint.h"  
#include 	"X/XrmI.h" 
#include	"X/Xlcint.h"  

#if __STDC__
#define Const const
#else
#define Const /**/
#endif
#if __STDC__ && !defined(VMS)
#define RConst const
#else
#define RConst /**/
#endif

/*

These Xrm routines allow very fast lookup of resources in the resource
database.  Several usage patterns are exploited:

(1) Widgets get a lot of resources at one time.  Rather than look up each from
scratch, we can precompute the prioritized list of database levels once, then
search for each resource starting at the beginning of the list.

(2) Many database levels don't contain any leaf resource nodes.  There is no
point in looking for resources on a level that doesn't contain any.  This
information is kept on a per-level basis.

(3) Sometimes the widget instance tree is structured such that you get the same
class name repeated on the fully qualified widget name.  This can result in the
same database level occuring multiple times on the search list.  The code below
only checks to see if you get two identical search lists in a row, rather than
look back through all database levels, but in practice this removes all
duplicates I've ever observed.

Joel McCormack

*/

/*

The Xrm representation has been completely redesigned to substantially reduce
memory and hopefully improve performance.

The database is structured into two kinds of tables: LTables that contain
only values, and NTables that contain only other tables.

Some invariants:

The next pointer of the top-level node table points to the top-level leaf
table, if any.

Within an LTable, for a given name, the tight value always precedes the
loose value, and if both are present the loose value is always right after
the tight value.

Within an NTable, all of the entries for a given name are contiguous,
in the order tight NTable, loose NTable, tight LTable, loose LTable.

Bob Scheifler

*/

typedef unsigned long Signature;

static XrmQuark XrmQString, XrmQANY;

typedef	Bool (*DBEnumProc)(
#if NeedNestedPrototypes    /* this is Nested on purpose, to match Xlib.h */
    XrmDatabase*	/* db */,
    XrmBindingList	/* bindings */,
    XrmQuarkList	/* quarks */,
    XrmRepresentation*	/* type */,
    XrmValue*		/* value */,
    XPointer		/* closure */
#endif
);

typedef struct _VEntry {
    struct _VEntry	*next;		/* next in chain */
    XrmQuark		name;		/* name of this entry */
    unsigned int	tight:1;	/* 1 if it is a tight binding */
    unsigned int	string:1;	/* 1 if type is String */
    unsigned int	size:30;	/* size of value */
} VEntryRec, *VEntry;


typedef struct _DEntry {
    VEntryRec		entry;		/* entry */
    XrmRepresentation	type;		/* representation type */
} DEntryRec, *DEntry;

/* the value is right after the structure */
#define StringValue(ve) (XPointer)((ve) + 1)
#define RepType(ve) ((DEntry)(ve))->type
/* the value is right after the structure */
#define DataValue(ve) (XPointer)(((DEntry)(ve)) + 1)
#define RawValue(ve) (char *)((ve)->string ? StringValue(ve) : DataValue(ve))

typedef struct _NTable {
    struct _NTable	*next;		/* next in chain */
    XrmQuark		name;		/* name of this entry */
    unsigned int	tight:1;	/* 1 if it is a tight binding */
    unsigned int	leaf:1;		/* 1 if children are values */
    unsigned int	hasloose:1;	/* 1 if has loose children */
    unsigned int	hasany:1;	/* 1 if has ANY entry */
    unsigned int	pad:4;		/* unused */
    unsigned int	mask:8;		/* hash size - 1 */
    unsigned int	entries:16;	/* number of children */
} NTableRec, *NTable;

/* the buckets are right after the structure */
#define NodeBuckets(ne) ((NTable *)((ne) + 1))
#define NodeHash(ne,q) NodeBuckets(ne)[(q) & (ne)->mask]

/* leaf tables have an extra level of indirection for the buckets,
 * so that resizing can be done without invalidating a search list.
 * This is completely ugly, and wastes some memory, but the Xlib
 * spec doesn't really specify whether invalidation is OK, and the
 * old implementation did not invalidate.
 */
typedef struct _LTable {
    NTableRec		table;
    VEntry		*buckets;
} LTableRec, *LTable;

#define LeafHash(le,q) (le)->buckets[(q) & (le)->table.mask]

/* An XrmDatabase just holds a pointer to the first top-level table.
 * The type name is no longer descriptive, but better to not change
 * the Xresource.h header file.  This type also gets used to define
 * XrmSearchList, which is a complete crock, but we'll just leave it
 * and caste types as required.
 */
typedef struct _XrmHashBucketRec {
    NTable table;
    XPointer mbstate;
    XrmMethods methods;
} XrmHashBucketRec;

/* closure used in get/put resource */
typedef struct _VClosure {
    XrmRepresentation	*type;		/* type of value */
    XrmValuePtr		value;		/* value itself */
} VClosureRec, *VClosure;

/* closure used in get search list */
typedef struct _SClosure {
    LTable		*list;		/* search list */
    int			idx;		/* index of last filled element */
    int			limit;		/* maximum index */
} SClosureRec, *SClosure;

/* placed in XrmSearchList to indicate next table is loose only */
#define LOOSESEARCH ((LTable)1)

/* closure used in enumerate database */
typedef struct _EClosure {
    XrmDatabase db;			/* the database */
    DBEnumProc proc;			/* the user proc */
    XPointer closure;			/* the user closure */
    XrmBindingList bindings;		/* binding list */
    XrmQuarkList quarks;		/* quark list */
    int mode;				/* XrmEnum<kind> */
} EClosureRec, *EClosure;

/* predicate to determine when to resize a hash table */
#define GrowthPred(n,m) ((unsigned)(n) > (((m) + 1) << 2))

#define GROW(prev) \
    if (GrowthPred((*prev)->entries, (*prev)->mask)) \
	GrowTable(prev)

/* pick a reasonable value for maximum depth of resource database */
#define MAXDBDEPTH 100

/* macro used in get/search functions */
#define NFIND(ename) \
    q = ename; \
    entry = NodeHash(table, q); \
    while (entry && entry->name != q) \
        entry = entry->next; \
    if (leaf && entry && !entry->leaf) { \
        entry = entry->next; \
        if (entry && !entry->leaf) \
            entry = entry->next; \
        if (entry && entry->name != q) \
            entry = (NTable)NULL; \
    }

/* find entries named ename, leafness leaf, tight or loose, and call get */
#define GTIGHTLOOSE(ename,looseleaf) \
    NFIND(ename); \
    if (entry) { \
        if (leaf == entry->leaf) { \
            if (!leaf && !entry->tight && entry->next && \
                entry->next->name == q && entry->next->tight && \
                entry->next->hasloose && \
                looseleaf((LTable)entry->next, names+1, classes+1, closure)) \
                return True; \
            if ((*get)(entry, names+1, classes+1, closure)) \
                return True; \
            if (entry->tight && (entry = entry->next) && \
                entry->name == q && leaf == entry->leaf && \
                (*get)(entry, names+1, classes+1, closure)) \
                return True; \
        } else if (entry->leaf) { \
            if (entry->hasloose && \
                looseleaf((LTable)entry, names+1, classes+1, closure)) \
                return True; \
            if (entry->tight && (entry = entry->next) && \
                entry->name == q && entry->hasloose && \
                looseleaf((LTable)entry, names+1, classes+1, closure)) \
                return True; \
        } \
    }

/* find entries named ename, leafness leaf, loose only, and call get */
#define GLOOSE(ename,looseleaf) \
    NFIND(ename); \
    if (entry && entry->tight && (entry = entry->next) && entry->name != q) \
        entry = (NTable)NULL; \
    if (entry) { \
        if (leaf == entry->leaf) { \
            if ((*get)(entry, names+1, classes+1, closure)) \
                return True; \
        } else if (entry->leaf && entry->hasloose) { \
            if (looseleaf((LTable)entry, names+1, classes+1, closure)) \
                return True; \
        } \
    }

/* resourceQuarks keeps track of what quarks have been associated with values
 * in all LTables.  If a quark has never been used in an LTable, we don't need
 * to bother looking for it.
 */

static unsigned char *resourceQuarks = (unsigned char *)NULL;
static XrmQuark maxResourceQuark = -1;

/* determines if a quark has been used for a value in any database */
#define IsResourceQuark(q)  ((q) > 0 && (q) <= maxResourceQuark && \
			     resourceQuarks[(q) >> 3] & (1 << ((q) & 7)))

typedef unsigned char XrmBits;

#define BSLASH  ((XrmBits) (1 << 5))
#define NORMAL	((XrmBits) (1 << 4))
#define EOQ	((XrmBits) (1 << 3))
#define SEP	((XrmBits) (1 << 2))
#define ENDOF	((XrmBits) (1 << 1))
#define SPACE	(NORMAL|EOQ|SEP|(XrmBits)0)
#define RSEP	(NORMAL|EOQ|SEP|(XrmBits)1)
#define EOS	(EOQ|SEP|ENDOF|(XrmBits)0)
#define EOL	(EOQ|SEP|ENDOF|(XrmBits)1)
#define BINDING	(NORMAL|EOQ)
#define ODIGIT	(NORMAL|(XrmBits)1)

#define next_char(ch,str) xrmtypes[(unsigned char)((ch) = *(++(str)))]
#define next_mbchar(ch,len,str) xrmtypes[(unsigned char)(ch = (*db->methods->mbchar)(db->mbstate, str, &len), str += len, ch)]

#define is_space(bits)		((bits) == SPACE)
#define is_EOQ(bits)		((bits) & EOQ)
#define is_EOF(bits)		((bits) == EOS)
#define is_EOL(bits)		((bits) & ENDOF)
#define is_binding(bits)	((bits) == BINDING)
#define is_odigit(bits)		((bits) == ODIGIT)
#define is_separator(bits)	((bits) & SEP)
#define is_nonpcs(bits)		(!(bits))
#define is_normal(bits)		((bits) & NORMAL)
#define is_simple(bits)		((bits) & (NORMAL|BSLASH))
#define is_special(bits)	((bits) & (ENDOF|BSLASH))

/* parsing types */
static XrmBits Const xrmtypes[256] = {
    EOS,0,0,0,0,0,0,0,
    0,SPACE,EOL,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    SPACE,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,BINDING,NORMAL,NORMAL,NORMAL,BINDING,NORMAL,
    ODIGIT,ODIGIT,ODIGIT,ODIGIT,ODIGIT,ODIGIT,ODIGIT,ODIGIT,
    NORMAL,NORMAL,RSEP,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,BSLASH,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,
    NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,NORMAL,0
    /* The rest will be automatically initialized to zero. */
};

void XrmInitialize()
{
    XrmQString = XrmPermStringToQuark("String");
    XrmQANY = XrmPermStringToQuark("?");
}

/*ARGSUSED*/
static void mbnoop(state)
    XPointer state;
{
}

#if NeedFunctionPrototypes
void XrmStringToQuarkList(
    register _Xconst char  *name,
    register XrmQuarkList quarks)   /* RETURN */
#else
void XrmStringToQuarkList(name, quarks)
    register char        *name;
    register XrmQuarkList quarks;   /* RETURN */
#endif
{
    register XrmBits            bits;
    register Signature          sig = 0;
    register char               ch, *tname;
    register int                i = 0;

    if (tname = (char *)name) {
        tname--;
        while (!is_EOF(bits = next_char(ch, tname))) {
            if (is_binding (bits)) {
                if (i) {
                    /* Found a complete name */
                    *quarks++ = _XrmInternalStringToQuark(name,tname - name,
                                                          sig, False);
                    i = 0;
                    sig = 0;
                }
                name = tname+1;
            }
            else {
                sig = (sig << 1) + ch; /* Compute the signature. */
                i++;
            }
        }
        *quarks++ = _XrmInternalStringToQuark(name, tname - name, sig, False);
    }
    *quarks = NULLQUARK;
}

#if NeedFunctionPrototypes
void XrmStringToBindingQuarkList(
    register _Xconst char   *name,
    register XrmBindingList bindings,   /* RETURN */
    register XrmQuarkList   quarks)     /* RETURN */
#else
void XrmStringToBindingQuarkList(name, bindings, quarks)
    register char           *name;
    register XrmBindingList bindings;   /* RETURN */
    register XrmQuarkList   quarks;     /* RETURN */
#endif
{
    register XrmBits            bits;
    register Signature          sig = 0;
    register char               ch, *tname;
    register XrmBinding         binding;
    register int                i = 0;

    if (tname = (char *)name) {
        tname--;
        binding = XrmBindTightly;
        while (!is_EOF(bits = next_char(ch, tname))) {
            if (is_binding (bits)) {
                if (i) {
                    /* Found a complete name */
                    *bindings++ = binding;
                    *quarks++ = _XrmInternalStringToQuark(name, tname - name,
                                                          sig, False);

                    i = 0;
                    sig = 0;
                    binding = XrmBindTightly;
                }
                name = tname+1;

                if (ch == '*')
                    binding = XrmBindLoosely;
            }
            else {
                sig = (sig << 1) + ch; /* Compute the signature. */
                i++;
            }
        }
        *bindings = binding;
        *quarks++ = _XrmInternalStringToQuark(name, tname - name, sig, False);
    }
    *quarks = NULLQUARK;
}

/*ARGSUSED*/
static char mbchar(state, str, lenp)
    XPointer state;
    char *str;
    int *lenp;
{
    *lenp = 1;
    return *str;
}

/*ARGSUSED*/
static char *lcname(state)
    XPointer state;
{
    return "C";
}

static RConst XrmMethodsRec mb_methods = {
    mbnoop,
    mbchar,
    mbnoop,
    lcname,
    mbnoop
};

static XrmDatabase NewDatabase()
{
    register XrmDatabase db;

    db = (XrmDatabase) Xmalloc(sizeof(XrmHashBucketRec));
    if (db) {
	db->table = (NTable)NULL;
/*	db->methods = _XrmInitParseInfo(&db->mbstate);    */
	if (!db->methods)
	    db->methods = (XrmMethods)&mb_methods;
    }
    return db;
}

/* move all values from ftable to ttable, and free ftable's buckets.
 * ttable is quaranteed empty to start with.
 */
static void MoveValues(ftable, ttable)
    LTable ftable;
    register LTable ttable;
{
    register VEntry fentry, nfentry;
    register VEntry *prev;
    register VEntry *bucket;
    register VEntry tentry;
    register int i;

    for (i = ftable->table.mask, bucket = ftable->buckets; i >= 0; i--) {
	for (fentry = *bucket++; fentry; fentry = nfentry) {
	    prev = &LeafHash(ttable, fentry->name);
	    tentry = *prev;
	    *prev = fentry;
	    /* chain on all with same name, to preserve invariant order */
	    while ((nfentry = fentry->next) && nfentry->name == fentry->name)
		fentry = nfentry;
	    fentry->next = tentry;
	}
    }
    Xfree((char *)ftable->buckets);
}

/* move all tables from ftable to ttable, and free ftable.
 * ttable is quaranteed empty to start with.
 */
static void MoveTables(ftable, ttable)
    NTable ftable;
    register NTable ttable;
{
    register NTable fentry, nfentry;
    register NTable *prev;
    register NTable *bucket;
    register NTable tentry;
    register int i;

    for (i = ftable->mask, bucket = NodeBuckets(ftable); i >= 0; i--) {
	for (fentry = *bucket++; fentry; fentry = nfentry) {
	    prev = &NodeHash(ttable, fentry->name);
	    tentry = *prev;
	    *prev = fentry;
	    /* chain on all with same name, to preserve invariant order */
	    while ((nfentry = fentry->next) && nfentry->name == fentry->name)
		fentry = nfentry;
	    fentry->next = tentry;
	}
    }
    Xfree((char *)ftable);
}

/* grow the table, based on current number of entries */
static void GrowTable(prev)
    NTable *prev;
{
    register NTable table;
    register int i;

    table = *prev;
    i = table->mask;
    if (i == 255) /* biggest it gets */
	return;
    while (i < 255 && GrowthPred(table->entries, i))
	i = (i << 1) + 1;
    i++; /* i is now the new size */
    if (table->leaf) {
	register LTable ltable;
	LTableRec otable;

	ltable = (LTable)table;
	/* cons up a copy to make MoveValues look symmetric */
	otable = *ltable;
	ltable->buckets = (VEntry *)Xmalloc(i * sizeof(VEntry));
	if (!ltable->buckets) {
	    ltable->buckets = otable.buckets;
	    return;
	}
	ltable->table.mask = i - 1;
	bzero((char *)ltable->buckets, i * sizeof(VEntry));
	MoveValues(&otable, ltable);
    } else {
	register NTable ntable;

	ntable = (NTable)Xmalloc(sizeof(NTableRec) + i * sizeof(NTable));
	if (!ntable)
	    return;
	*ntable = *table;
	ntable->mask = i - 1;
	bzero((char *)NodeBuckets(ntable), i * sizeof(NTable));
	*prev = ntable;
	MoveTables(table, ntable);
    }
}

/* merge values from ftable into *pprev, destroy ftable in the process */
static void MergeValues(ftable, pprev, override)
    LTable ftable;
    NTable *pprev;
    Bool override;
{
    register VEntry fentry, tentry;
    register VEntry *prev;
    register LTable ttable;
    VEntry *bucket;
    int i;
    register XrmQuark q;

    ttable = (LTable)*pprev;
    if (ftable->table.hasloose)
	ttable->table.hasloose = 1;
    for (i = ftable->table.mask, bucket = ftable->buckets;
	 i >= 0;
	 i--, bucket++) {
	for (fentry = *bucket; fentry; ) {
	    q = fentry->name;
	    prev = &LeafHash(ttable, q);
	    tentry = *prev;
	    while (tentry && tentry->name != q)
		tentry = *(prev = &tentry->next);
	    /* note: test intentionally uses fentry->name instead of q */
	    /* permits serendipitous inserts */
	    while (tentry && tentry->name == fentry->name) {
		/* if tentry is earlier, skip it */
		if (!fentry->tight && tentry->tight) {
		    tentry = *(prev = &tentry->next);
		    continue;
		}
		if (fentry->tight != tentry->tight) {
		    /* no match, chain in fentry */
		    *prev = fentry;
		    prev = &fentry->next;
		    fentry = *prev;
		    *prev = tentry;
		    ttable->table.entries++;
		} else if (override) {
		    /* match, chain in fentry, splice out and free tentry */
		    *prev = fentry;
		    prev = &fentry->next;
		    fentry = *prev;
		    *prev = tentry->next;
		    /* free the overridden entry */
		    Xfree((char *)tentry);
		    /* get next tentry */
		    tentry = *prev;
		} else {
		    /* match, discard fentry */
		    prev = &tentry->next;
		    tentry = fentry; /* use as a temp var */
		    fentry = fentry->next;
		    /* free the overpowered entry */
		    Xfree((char *)tentry);
		    /* get next tentry */
		    tentry = *prev;
		}
		if (!fentry)
		    break;
	    }
	    /* at this point, tentry cannot match any fentry named q */
	    /* chain in all bindings together, preserve invariant order */
	    while (fentry && fentry->name == q) {
		*prev = fentry;
		prev = &fentry->next;
		fentry = *prev;
		*prev = tentry;
		ttable->table.entries++;
	    }
	}
    }
    Xfree((char *)ftable->buckets);
    Xfree((char *)ftable);
    /* resize if necessary, now that we're all done */
    GROW(pprev);
}

/* merge tables from ftable into *pprev, destroy ftable in the process */
static void MergeTables(ftable, pprev, override)
    NTable ftable;
    NTable *pprev;
    Bool override;
{
    register NTable fentry, tentry;
    NTable nfentry;
    register NTable *prev;
    register NTable ttable;
    NTable *bucket;
    int i;
    register XrmQuark q;

    ttable = *pprev;
    if (ftable->hasloose)
	ttable->hasloose = 1;
    if (ftable->hasany)
	ttable->hasany = 1;
    for (i = ftable->mask, bucket = NodeBuckets(ftable);
	 i >= 0;
	 i--, bucket++) {
	for (fentry = *bucket; fentry; ) {
	    q = fentry->name;
	    prev = &NodeHash(ttable, q);
	    tentry = *prev;
	    while (tentry && tentry->name != q)
		tentry = *(prev = &tentry->next);
	    /* note: test intentionally uses fentry->name instead of q */
	    /* permits serendipitous inserts */
	    while (tentry && tentry->name == fentry->name) {
		/* if tentry is earlier, skip it */
		if ((fentry->leaf && !tentry->leaf) ||
		    (!fentry->tight && tentry->tight &&
		     (fentry->leaf || !tentry->leaf))) {
		    tentry = *(prev = &tentry->next);
		    continue;
		}
		nfentry = fentry->next;
		if (fentry->leaf != tentry->leaf ||
		    fentry->tight != tentry->tight) {
		    /* no match, just chain in */
		    *prev = fentry;
		    *(prev = &fentry->next) = tentry;
		    ttable->entries++;
		} else {
		    if (fentry->leaf)
			MergeValues((LTable)fentry, prev, override);
		    else
			MergeTables(fentry, prev, override);
		    /* bump to next tentry */
		    tentry = *(prev = &(*prev)->next);
		}
		/* bump to next fentry */
		fentry = nfentry;
		if (!fentry)
		    break;
	    }
	    /* at this point, tentry cannot match any fentry named q */
	    /* chain in all bindings together, preserve invariant order */
	    while (fentry && fentry->name == q) {
		*prev = fentry;
		prev = &fentry->next;
		fentry = *prev;
		*prev = tentry;
		ttable->entries++;
	    }
	}
    }
    Xfree((char *)ftable);
    /* resize if necessary, now that we're all done */
    GROW(pprev);
}

void XrmCombineDatabase(from, into, override)
    XrmDatabase	from, *into;
    Bool override;
{
    register NTable *prev;
    register NTable ftable, ttable, nftable;

    if (!*into) {
	*into = from;
    } else if (from) {
	if (ftable = from->table) {
	    prev = &(*into)->table;
	    ttable = *prev;
	    if (!ftable->leaf) {
		nftable = ftable->next;
		if (ttable && !ttable->leaf) {
		    /* both have node tables, merge them */
		    MergeTables(ftable, prev, override);
		    /* bump to into's leaf table, if any */
		    ttable = *(prev = &(*prev)->next);
		} else {
		    /* into has no node table, link from's in */
		    *prev = ftable;
		    *(prev = &ftable->next) = ttable;
		}
		/* bump to from's leaf table, if any */
		ftable = nftable;
	    } else {
		/* bump to into's leaf table, if any */
		if (ttable && !ttable->leaf)
		    ttable = *(prev = &ttable->next);
	    }
	    if (ftable) {
		/* if into has a leaf, merge, else insert */
		if (ttable)
		    MergeValues((LTable)ftable, prev, override);
		else
		    *prev = ftable;
	    }
	}
	Xfree((char *)from);
    }
}

void XrmMergeDatabases(from, into)
    XrmDatabase	from, *into;
{
    XrmCombineDatabase(from, into, True);
}

/* store a value in the database, overriding any existing entry */
static void PutEntry(db, bindings, quarks, type, value)
    XrmDatabase		db;
    XrmBindingList	bindings;
    XrmQuarkList	quarks;
    XrmRepresentation	type;
    XrmValuePtr		value;
{
    register NTable *pprev, *prev;
    register NTable table;
    register XrmQuark q;
    register VEntry *vprev;
    register VEntry entry;
    NTable *nprev, *firstpprev;

#define NEWTABLE(q,i) \
    table = (NTable)Xmalloc(sizeof(LTableRec)); \
    if (!table) \
	return; \
    table->name = q; \
    table->hasloose = 0; \
    table->hasany = 0; \
    table->mask = 0; \
    table->entries = 0; \
    if (quarks[i]) { \
	table->leaf = 0; \
	nprev = NodeBuckets(table); \
    } else { \
	table->leaf = 1; \
	if (!(nprev = (NTable *)Xmalloc(sizeof(VEntry *)))) \
	    return; \
	((LTable)table)->buckets = (VEntry *)nprev; \
    } \
    *nprev = (NTable)NULL; \
    table->next = *prev; \
    *prev = table

    if (!db || !*quarks)
	return;
    table = *(prev = &db->table);
    /* if already at leaf, bump to the leaf table */
    if (!quarks[1] && table && !table->leaf)
	table = *(prev = &table->next);
    pprev = prev;
    if (!table || (quarks[1] && table->leaf)) {
	/* no top-level node table, create one and chain it in */
	NEWTABLE(NULLQUARK,1);
	table->tight = 1; /* arbitrary */
	prev = nprev;
    } else {
	/* search along until we need a value */
	while (quarks[1]) {
	    q = *quarks;
	    table = *(prev = &NodeHash(table, q));
	    while (table && table->name != q)
		table = *(prev = &table->next);
	    if (!table)
		break; /* not found */
	    if (quarks[2]) {
		if (table->leaf)
		    break; /* not found */
	    } else {
		if (!table->leaf) {
		    /* bump to leaf table, if any */
		    table = *(prev = &table->next);
		    if (!table || table->name != q)
			break; /* not found */
		    if (!table->leaf) {
			/* bump to leaf table, if any */
			table = *(prev = &table->next);
			if (!table || table->name != q)
			    break; /* not found */
		    }
		}
	    }
	    if (*bindings == XrmBindTightly) {
		if (!table->tight)
		    break; /* not found */
	    } else {
		if (table->tight) {
		    /* bump to loose table, if any */
		    table = *(prev = &table->next);
		    if (!table || table->name != q ||
			!quarks[2] != table->leaf)
			break; /* not found */
		}
	    }
	    /* found that one, bump to next quark */
	    pprev = prev;
	    quarks++;
	    bindings++;
	}
	if (!quarks[1]) {
	    /* found all the way to a leaf */
	    q = *quarks;
	    entry = *(vprev = &LeafHash((LTable)table, q));
	    while (entry && entry->name != q)
		entry = *(vprev = &entry->next);
	    /* if want loose and have tight, bump to next entry */
	    if (entry && *bindings == XrmBindLoosely && entry->tight)
		entry = *(vprev = &entry->next);
	    if (entry && entry->name == q &&
		(*bindings == XrmBindTightly) == entry->tight) {
		/* match, need to override */
		if ((type == XrmQString) == entry->string &&
		    entry->size == value->size) {
		    /* update type if not String, can be different */
		    if (!entry->string)
			RepType(entry) = type;
		    /* identical size, just overwrite value */
		    bcopy((char *)value->addr, RawValue(entry), value->size);
		    return;
		}
		/* splice out and free old entry */
		*vprev = entry->next;
		Xfree((char *)entry);
		(*pprev)->entries--;
	    }
	    /* this is where to insert */
	    prev = (NTable *)vprev;
	}
    }
    /* keep the top table, because we may have to grow it */
    firstpprev = pprev;
    /* iterate until we get to the leaf */
    while (quarks[1]) {
	/* build a new table and chain it in */
	NEWTABLE(*quarks,2);
	if (*quarks++ == XrmQANY)
	    (*pprev)->hasany = 1;
	if (*bindings++ == XrmBindTightly) {
	    table->tight = 1;
	} else {
	    table->tight = 0;
	    (*pprev)->hasloose = 1;
	}
	(*pprev)->entries++;
	pprev = prev;
	prev = nprev;
    }
    /* now allocate the value entry */
    entry = (VEntry)Xmalloc(((type == XrmQString) ?
			     sizeof(VEntryRec) : sizeof(DEntryRec)) +
			    value->size);
    if (!entry)
	return;
    entry->name = q = *quarks;
    if (*bindings == XrmBindTightly) {
	entry->tight = 1;
    } else {
	entry->tight = 0;
	(*pprev)->hasloose = 1;
    }
    /* chain it in, with a bit of type cast ugliness */
    entry->next = *((VEntry *)prev);
    *((VEntry *)prev) = entry;
    entry->size = value->size;
    if (type == XrmQString) {
	entry->string = 1;
    } else {
	entry->string = 0;
	RepType(entry) = type;
    }
    /* save a copy of the value */
    bcopy((char *)value->addr, RawValue(entry), value->size);
    (*pprev)->entries++;
    /* this is a new leaf, need to remember it for search lists */
    if (q > maxResourceQuark) {
	unsigned oldsize = maxResourceQuark + 1;
	unsigned size = (q | 0x7f) + 1; /* reallocate in reasonable chunks */
	if (resourceQuarks)
	    resourceQuarks = (unsigned char *)Xrealloc((char *)resourceQuarks,
						       size);
	else
	    resourceQuarks = (unsigned char *)Xmalloc(size);
	if (resourceQuarks) {
	    bzero((char *)&resourceQuarks[oldsize], size - oldsize);
	    maxResourceQuark = size - 1;
	} else {
	    maxResourceQuark = -1;
	}
    }
    if (q > 0 && resourceQuarks)
	resourceQuarks[q >> 3] |= 1 << (q & 0x7);
    GROW(firstpprev);

#undef NEWTABLE
}

#if NeedFunctionPrototypes
void XrmQPutStringResource(
    XrmDatabase     *pdb,
    XrmBindingList  bindings,
    XrmQuarkList    quarks,
    _Xconst char    *str)
#else
void XrmQPutStringResource(pdb, bindings, quarks, str)
    XrmDatabase     *pdb;
    XrmBindingList  bindings;
    XrmQuarkList    quarks;
    char            *str;
#endif
{
    XrmValue    value;

    if (!*pdb) *pdb = NewDatabase();
    value.addr = (XPointer) str;
    value.size = strlen(str)+1;
    PutEntry(*pdb, bindings, quarks, XrmQString, &value);
}

/*	Function Name: GetDatabase
 *	Description: Parses a string and stores it as a database.
 *	Arguments: db - the database.
 *                 str - a pointer to the string containing the database.
 *                 filename - source filename, if any.
 *                 doall - whether to do all lines or just one
 */

/*
 * This function is highly optimized to inline as much as possible. 
 * Be very careful with modifications, or simplifications, as they 
 * may adversely affect the performance.
 *
 * Chris Peterson, MIT X Consortium		5/17/90.
 */

#define LIST_SIZE 101
#define BUFFER_SIZE 100

static void GetIncludeFile();

static void GetDatabase(db, str, filename, doall)
    XrmDatabase db;
    register char *str;
    char *filename;
    Bool doall;
{
    register char *ptr;
    register XrmBits bits = 0;
    register char c;
    int len;
    register Signature sig;
    register char *ptr_max;
    register XrmQuarkList t_quarks;
    register XrmBindingList t_bindings;

    int alloc_chars = BUFSIZ;
    char buffer[BUFSIZ], *value_str;
    XrmQuark quarks[LIST_SIZE];
    XrmBinding bindings[LIST_SIZE];
    XrmValue value;
    Bool only_pcs;
    Bool dolines;

    if (!db)
	return;

    if (!(value_str = Xmalloc(sizeof(char) * alloc_chars)))
	return;

    (*db->methods->mbinit)(db->mbstate);
    str--;
    dolines = True;
    while (!is_EOF(bits) && dolines) {
	dolines = doall;

	/*
	 * First: Remove extra whitespace. 
	 */

	do {
	    bits = next_char(c, str);
	} while is_space(bits);

	/*
	 * Ignore empty lines.
	 */

	if (is_EOL(bits))
	    continue;		/* start a new line. */

	/*
	 * Second: check the first character in a line to see if it is
	 * "!" signifying a comment, or "#" signifying a directive.
	 */

	if (c == '!') { /* Comment, spin to next newline */
	    while (is_simple(bits = next_char(c, str))) {}
	    if (is_EOL(bits))
		continue;
	    while (!is_EOL(bits = next_mbchar(c, len, str))) {}
	    str--;
	    continue;		/* start a new line. */
	}

	if (c == '#') { /* Directive */
	    /* remove extra whitespace */
	    only_pcs = True;
	    while (is_space(bits = next_char(c, str))) {};
	    /* only "include" directive is currently defined */
	    if (!strncmp(str, "include", 7)) {
		str += (7-1);
		/* remove extra whitespace */
		while (is_space(bits = next_char(c, str))) {};
		/* must have a starting " */
		if (c == '"') {
		    char *fname = str+1;
		    len = 0;
		    do {
			if (only_pcs) {
			    bits = next_char(c, str);
			    if (is_nonpcs(bits))
				only_pcs = False;
			}
			if (!only_pcs)
			    bits = next_mbchar(c, len, str);
		    } while (c != '"' && !is_EOL(bits));
		    /* must have an ending " */
		    if (c == '"')
			GetIncludeFile(db, filename, fname, str - len - fname);
		}
	    }
	    /* spin to next newline */
	    if (only_pcs) {
		while (is_simple(bits))
		    bits = next_char(c, str);
		if (is_EOL(bits))
		    continue;
	    }
	    while (!is_EOL(bits))
		bits = next_mbchar(c, len, str);
	    str--;
	    continue;		/* start a new line. */
	}

	/*
	 * Third: loop through the LHS of the resource specification
	 * storing characters and converting this to a Quark.
	 *
	 * If the number of quarks is greater than LIST_SIZE - 1.  This
	 * function will trash your memory.
	 *
	 * If the length of any quark is larger than BUFSIZ this function
	 * will also trash memory.
	 */
	
	t_bindings = bindings;
	t_quarks = quarks;

	sig = 0;
	ptr = buffer;
	*t_bindings = XrmBindTightly;	
	for(;;) {
	    if (!is_binding(bits)) {
		while (!is_EOQ(bits)) {
		    *ptr++ = c;
		    sig = (sig << 1) + c; /* Compute the signature. */
		    bits = next_char(c, str);
		}

		*t_quarks++ = _XrmInternalStringToQuark(buffer, ptr - buffer,
							sig, False);

		if (is_separator(bits))  {
		    if (!is_space(bits))
			break;

		    /* Remove white space */
		    do {
			*ptr++ = c;
			sig = (sig << 1) + c; /* Compute the signature. */
		    } while (is_space(bits = next_char(c, str)));

		    /* 
		     * The spec doesn't permit it, but support spaces
		     * internal to resource name/class 
		     */

		    if (is_separator(bits))
			break;
		    t_quarks--;
		    continue;
		}

		if (c == '.')
		    *(++t_bindings) = XrmBindTightly;
		else
		    *(++t_bindings) = XrmBindLoosely;

		sig = 0;
		ptr = buffer;
	    }
	    else {
		/*
		 * Magic unspecified feature #254.
		 *
		 * If two separators appear with no Text between them then
		 * ignore them.
		 *
		 * If anyone of those separators is a '*' then the binding 
		 * will be loose, otherwise it will be tight.
		 */

		if (c == '*')
		    *t_bindings = XrmBindLoosely;
	    }

	    bits = next_char(c, str);
	}

	*t_quarks = NULLQUARK;

	/*
	 * Make sure that there is a ':' in this line.
	 */

	if (c != ':') {
	    char oldc;

	    /*
	     * A parsing error has occured, toss everything on the line
	     * a new_line can still be escaped with a '\'.
	     */

	    while (is_normal(bits))
		bits = next_char(c, str);
	    if (is_EOL(bits))
		continue;
	    bits = next_mbchar(c, len, str);
	    do {
		oldc = c;
		bits = next_mbchar(c, len, str);
	    } while (c && (c != '\n' || oldc == '\\'));
	    str--;
	    continue;
	}

	/*
	 * I now have a quark and binding list for the entire left hand
	 * side.  "c" currently points to the ":" separating the left hand
	 * side for the right hand side.  It is time to begin processing
	 * the right hand side.
	 */

	/* 
	 * Fourth: Remove more whitespace
	 */

	for(;;) {
	    if (is_space(bits = next_char(c, str)))
		continue;
	    if (c != '\\')
		break;
	    bits = next_char(c, str);
	    if (c == '\n')
		continue;
	    str--;
	    bits = BSLASH;
	    c = '\\';
	    break;
	}

	/* 
	 * Fifth: Process the right hand side.
	 */

	ptr = value_str;
	ptr_max = ptr + alloc_chars - 4;
	only_pcs = True;
	len = 1;

	for(;;) {

	    /*
	     * Tight loop for the normal case:  Non backslash, non-end of value
	     * character that will fit into the allocated buffer.
	     */

	    if (only_pcs) {
		while (is_normal(bits) && ptr < ptr_max) {
		    *ptr++ = c;
		    bits = next_char(c, str);
		}
		if (is_EOL(bits))
		    break;
		if (is_nonpcs(bits)) {
		    only_pcs = False;
		    bits = next_mbchar(c, len, str);
		}
	    }
	    while (!is_special(bits) && ptr + len <= ptr_max) {
		len = -len;
		while (len)
		    *ptr++ = str[len++];
		bits = next_mbchar(c, len, str);
	    }

	    if (is_EOL(bits)) {
		str--;
		break;
	    }

	    if (c == '\\') {
		/*
		 * We need to do some magic after a backslash.
		 */

		if (only_pcs) {
		    bits = next_char(c, str);
		    if (is_nonpcs(bits))
			only_pcs = False;
		}
		if (!only_pcs)
		    bits = next_mbchar(c, len, str);

		if (is_EOL(bits)) {
		    if (is_EOF(bits))
			continue;
		} else if (c == 'n') {
		    /*
		     * "\n" means insert a newline.
		     */
		    *ptr++ = '\n';
		} else if (c == '\\') {
		    /*
		     * "\\" completes to just one backslash.
		     */
		    *ptr++ = '\\';
		} else {
		    /*
		     * pick up to three octal digits after the '\'.
		     */
		    char temp[3];
		    int count = 0;
		    while (is_odigit(bits) && count < 3) {
			temp[count++] = c;
			if (only_pcs) {
			    bits = next_char(c, str);
			    if (is_nonpcs(bits))
				only_pcs = False;
			}
			if (!only_pcs)
			    bits = next_mbchar(c, len, str);
		    }

		    /*
		     * If we found three digits then insert that octal code
		     * into the value string as a character.
		     */

		    if (count == 3) {
			*ptr++ = (unsigned char) ((temp[0] - '0') * 0100 +
						  (temp[1] - '0') * 010 +
						  (temp[2] - '0'));
		    }
		    else {
			int tcount;

			/* 
			 * Otherwise just insert those characters into the 
			 * string, since no special processing is needed on
			 * numerics we can skip the special processing.
			 */

			for (tcount = 0; tcount < count; tcount++) {
			    *ptr++ = temp[tcount]; /* print them in
						      the correct order */
			}
		    }
		    continue;
		}
		if (only_pcs) {
		    bits = next_char(c, str);
		    if (is_nonpcs(bits))
			only_pcs = False;
		}
		if (!only_pcs)
		    bits = next_mbchar(c, len, str);
	    }

	    /* 
	     * It is important to make sure that there is room for at least
	     * four more characters in the buffer, since I can add that
	     * many characters into the buffer after a backslash has occured.
	     */

	    if (ptr + len > ptr_max) {
		char * temp_str;

		alloc_chars += BUFSIZ/10;		
		temp_str = Xrealloc(value_str, sizeof(char) * alloc_chars);

		if (!temp_str) {
		    Xfree(value_str);
		    (*db->methods->mbfinish)(db->mbstate);
		    return;
		}

		ptr = temp_str + (ptr - value_str); /* reset pointer. */
		value_str = temp_str;
		ptr_max = value_str + alloc_chars - 4;
	    }
	}

	/*
	 * Lastly: Terminate the value string, and store this entry 
	 * 	   into the database.
	 */

	*ptr++ = '\0';

	/* Store it in database */
	value.size = ptr - value_str;
	value.addr = (XPointer) value_str;
	
	PutEntry(db, bindings, quarks, XrmQString, &value);
    }

    Xfree(value_str);
    (*db->methods->mbfinish)(db->mbstate);
}


#if NeedFunctionPrototypes
void XrmPutLineResource(
    XrmDatabase *pdb,
    _Xconst char*line)
#else
void XrmPutLineResource(pdb, line)
    XrmDatabase *pdb;
    char        *line;
#endif
{
    if (!*pdb) *pdb = NewDatabase();
    GetDatabase(*pdb, line, (char *)NULL, False);
}
/*	Function Name: ReadInFile
 *	Description: Reads the file into a buffer.
 *	Arguments: filename - the name of the file.
 *	Returns: An allocated string containing the contents of the file.
 */

static char *
ReadInFile(filename)
char * filename;
{
    register int fd, size;
    char * filebuf;

    if ( (fd = OpenFile(filename)) == -1 )
	return (char *)NULL;

    GetSizeOfFile(filename, size);
	
    if (!(filebuf = Xmalloc(size + 1))) { /* leave room for '\0' */
	close(fd);
	return (char *)NULL;
    }

    if (ReadFile(fd, filebuf, size) != size) { /* If we didn't read the
						  correct number of bytes. */
	CloseFile(fd);
	Xfree(filebuf);
	return (char *)NULL;
    }
    CloseFile(fd);

    filebuf[size] = '\0';	/* NULL terminate it. */
    return filebuf;
}

static void
GetIncludeFile(db, base, fname, fnamelen)
    XrmDatabase db;
    char *base;
    char *fname;
    int fnamelen;
{
    int len;
    char *str;
    char realfname[BUFSIZ];

    if (fnamelen <= 0 || fnamelen >= BUFSIZ)
	return;
    if (*fname != '/' && base && (str = rindex(base, '/'))) {
	len = str - base + 1;
	if (len + fnamelen >= BUFSIZ)
	    return;
	strncpy(realfname, base, len);
	strncpy(realfname + len, fname, fnamelen);
	realfname[len + fnamelen] = '\0';
    } else {
	strncpy(realfname, fname, fnamelen);
	realfname[fnamelen] = '\0';
    }
    if (!(str = ReadInFile(realfname)))
	return;
    GetDatabase(db, str, realfname, True);
    Xfree(str);
}

#if NeedFunctionPrototypes
XrmDatabase XrmGetFileDatabase(
    _Xconst char    *filename)
#else
XrmDatabase XrmGetFileDatabase(filename)
    char 	    *filename;
#endif
{
    XrmDatabase db;
    char *str;

    if (!(str = ReadInFile(filename)))
	return (XrmDatabase)NULL;

    db = NewDatabase();
    GetDatabase(db, str, filename, True);
    Xfree(str);
    return db;
}

/* call the user proc for every value in the table, arbitrary order.
 * stop if user proc returns True.  level is current depth in database.
 */
/*ARGSUSED*/
static Bool EnumLTable(table, names, classes, level, closure)
    LTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    register int	level;
    register EClosure	closure;
{
    register VEntry *bucket;
    register int i;
    register VEntry entry;
    XrmValue value;
    XrmRepresentation type;
    Bool tightOk;

    closure->bindings[level] = (table->table.tight ?
				XrmBindTightly : XrmBindLoosely);
    closure->quarks[level] = table->table.name;
    level++;
    tightOk = !*names;
    closure->quarks[level + 1] = NULLQUARK;
    for (i = table->table.mask, bucket = table->buckets;
	 i >= 0;
	 i--, bucket++) {
	for (entry = *bucket; entry; entry = entry->next) {
	    if (entry->tight && !tightOk)
		continue;
	    closure->bindings[level] = (entry->tight ?
					XrmBindTightly : XrmBindLoosely);
	    closure->quarks[level] = entry->name;
	    value.size = entry->size;
	    if (entry->string) {
		type = XrmQString;
		value.addr = StringValue(entry);
	    } else {
		type = RepType(entry);
		value.addr = DataValue(entry);
	    }
	    if ((*closure->proc)(&closure->db, closure->bindings+1,
				 closure->quarks+1, &type, &value,
				 closure->closure))
		return True;
	}
    }
    return False;
}

static Bool EnumAllNTable(table, level, closure)
    NTable		table;
    register int	level;
    register EClosure	closure;
{
    register NTable *bucket;
    register int i;
    register NTable entry;
    XrmQuark empty = NULLQUARK;

    if (level >= MAXDBDEPTH)
	return False;
    for (i = table->mask, bucket = NodeBuckets(table);
	 i >= 0;
	 i--, bucket++) {
	for (entry = *bucket; entry; entry = entry->next) {
	    if (entry->leaf) {
		if (EnumLTable((LTable)entry, &empty, &empty, level, closure))
		    return True;
	    } else {
		closure->bindings[level] = (entry->tight ?
					    XrmBindTightly : XrmBindLoosely);
		closure->quarks[level] = entry->name;
		if (EnumAllNTable(entry, level+1, closure))
		    return True;
	    }
	}
    }
    return False;
}

/* recurse on every table in the table, arbitrary order.
 * stop if user proc returns True.  level is current depth in database.
 */

/* look for a tight/loose value */
static Bool GetVEntry(table, names, classes, closure)
    LTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    VClosure		closure;
{
    register VEntry entry;
    register XrmQuark q;

    /* try name first */
    q = *names;
    entry = LeafHash(table, q);
    while (entry && entry->name != q)
	entry = entry->next;
    if (!entry) {
	/* not found, try class */
	q = *classes;
	entry = LeafHash(table, q);
	while (entry && entry->name != q)
	    entry = entry->next;
	if (!entry)
	    return False;
    }
    if (entry->string) {
	*closure->type = XrmQString;
	closure->value->addr = StringValue(entry);
    } else {
	*closure->type = RepType(entry);
	closure->value->addr = DataValue(entry);
    }
    closure->value->size = entry->size;
    return True;
}

/* look for a loose value */
static Bool GetLooseVEntry(table, names, classes, closure)
    LTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    VClosure		closure;
{
    register VEntry	entry;
    register XrmQuark	q;

#define VLOOSE(ename) \
    q = ename; \
    entry = LeafHash(table, q); \
    while (entry && entry->name != q) \
	entry = entry->next; \
    if (entry && entry->tight && (entry = entry->next) && entry->name != q) \
	entry = (VEntry)NULL;

    /* bump to last component */
    while (names[1]) {
	names++;
	classes++;
    }
    VLOOSE(*names);  /* do name, loose only */
    if (!entry) {
	VLOOSE(*classes); /* do class, loose only */
	if (!entry)
	    return False;
    }
    if (entry->string) {
	*closure->type = XrmQString;
	closure->value->addr = StringValue(entry);
    } else {
	*closure->type = RepType(entry);
	closure->value->addr = DataValue(entry);
    }
    closure->value->size = entry->size;
    return True;

#undef VLOOSE
}

/* recursive search for a value */
static Bool GetNEntry(table, names, classes, closure)
    NTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    VClosure		closure;
{
    register NTable	entry;
    register XrmQuark	q;
    register unsigned int leaf;
    Bool		(*get)();
    NTable		otable;

    if (names[2]) {
	get = GetNEntry; /* recurse */
	leaf = 0;
    } else {
	get = GetVEntry; /* bottom of recursion */
	leaf = 1;
    }
    GTIGHTLOOSE(*names, GetLooseVEntry);   /* do name, tight and loose */
    GTIGHTLOOSE(*classes, GetLooseVEntry); /* do class, tight and loose */
    if (table->hasany) {
	GTIGHTLOOSE(XrmQANY, GetLooseVEntry); /* do ANY, tight and loose */
    }
    if (table->hasloose) {
	while (1) {
	    names++;
	    classes++;
	    if (!names[1])
		break;
	    if (!names[2]) {
		get = GetVEntry; /* bottom of recursion */
		leaf = 1;
	    }
	    GLOOSE(*names, GetLooseVEntry);   /* do name, loose only */
	    GLOOSE(*classes, GetLooseVEntry); /* do class, loose only */
	    if (table->hasany) {
		GLOOSE(XrmQANY, GetLooseVEntry); /* do ANY, loose only */
	    }
	}
    }
    /* look for matching leaf tables */
    otable = table;
    table = table->next;
    if (!table)
	return False;
    if (table->leaf) {
	if (table->tight && !otable->tight)
	    table = table->next;
    } else {
	table = table->next;
	if (!table || !table->tight)
	    return False;
    }
    if (!table || table->name != otable->name)
	return False;
    /* found one */
    if (table->hasloose &&
	GetLooseVEntry((LTable)table, names, classes, closure))
	return True;
    if (table->tight && table == otable->next) {
	table = table->next;
	if (table && table->name == otable->name && table->hasloose)
	    return GetLooseVEntry((LTable)table, names, classes, closure);
    }
    return False;
}

Bool XrmQGetResource(db, names, classes, pType, pValue)
    XrmDatabase         db;
    XrmNameList		names;
    XrmClassList 	classes;
    XrmRepresentation	*pType;  /* RETURN */
    XrmValuePtr		pValue;  /* RETURN */
{
    register NTable table;
    VClosureRec closure;

    if (db && *names) {
	closure.type = pType;
	closure.value = pValue;
	table = db->table;
	if (names[1]) {
	    if (table && !table->leaf) {
		if (GetNEntry(table, names, classes, &closure))
		    return True;
	    } else if (table && table->hasloose &&
		       GetLooseVEntry((LTable)table, names, classes, &closure))
		return True;
	} else {
	    if (table && !table->leaf)
		table = table->next;
	    if (table && GetVEntry((LTable)table, names, classes, &closure))
		return True;
	}
    }
    *pType = NULLQUARK;
    pValue->addr = (XPointer)NULL;
    pValue->size = 0;
    return False;
}

#if NeedFunctionPrototypes
Bool XrmGetResource(db, name_str, class_str, pType_str, pValue)
    XrmDatabase         db;
    _Xconst char	*name_str;
    _Xconst char	*class_str;
    XrmString		*pType_str;  /* RETURN */
    XrmValuePtr		pValue;      /* RETURN */
#else
Bool XrmGetResource(db, name_str, class_str, pType_str, pValue)
    XrmDatabase         db;
    XrmString		name_str;
    XrmString		class_str;
    XrmString		*pType_str;  /* RETURN */
    XrmValuePtr		pValue;      /* RETURN */
#endif
{
    XrmName		names[MAXDBDEPTH+1];
    XrmClass		classes[MAXDBDEPTH+1];
    XrmRepresentation   fromType;
    Bool		result;

    XrmStringToNameList(name_str, names);
    XrmStringToClassList(class_str, classes);
    result = XrmQGetResource(db, names, classes, &fromType, pValue);
    (*pType_str) = XrmQuarkToString(fromType);
    return result;
}

/* ********       Functions from ParseCmd.c         *********  */

#define Par_MESSAGE	"Error parsing argument \"%s\" (%s); %s\n"

static void _XReportParseError(arg, msg)
    XrmOptionDescRec *arg;
    char *msg;
{
    (void) fprintf(stderr, Par_MESSAGE,
		   arg->option, arg->specifier, msg);
    exit(1);
}

#if NeedFunctionPrototypes
void XrmParseCommand(
    XrmDatabase		*pdb,		/* data base */
    register XrmOptionDescList options, /* pointer to table of valid options */
    int			num_options,	/* number of options		     */
    _Xconst char	*prefix,	/* name to prefix resources with     */
    int			*argc,		/* address of argument count 	     */
    char		**argv)		/* argument list (command line)	     */
#else
void XrmParseCommand(pdb, options, num_options, prefix, argc, argv)
    XrmDatabase		*pdb;		/* data base */
    register XrmOptionDescList options; /* pointer to table of valid options */
    int			num_options;	/* number of options		     */
    char		*prefix;	/* name to prefix resources with     */
    int			*argc;		/* address of argument count 	     */
    char		**argv;		/* argument list (command line)	     */
#endif
{
    int 		foundOption;
    char		**argsave;
    register int	i, myargc;
    XrmBinding		bindings[100];
    XrmQuark		quarks[100];
    XrmBinding		*start_bindings;
    XrmQuark		*start_quarks;
    char		*optP, *argP, optchar, argchar;
    int			matches;
    enum {DontCare, Check, NotSorted, Sorted} table_is_sorted;
    char		**argend;

#define PutCommandResource(value_str)				\
    {								\
    XrmStringToBindingQuarkList(				\
	options[i].specifier, start_bindings, start_quarks);    \
    XrmQPutStringResource(pdb, bindings, quarks, value_str);    \
    } /* PutCommandResource */

    myargc = (*argc); 
    argend = argv + myargc;
    argsave = ++argv;

    /* Initialize bindings/quark list with prefix (typically app name). */
    quarks[0] = XrmStringToName(prefix);
    bindings[0] = XrmBindTightly;
    start_quarks = quarks+1;
    start_bindings = bindings+1;

    table_is_sorted = (myargc > 2) ? Check : DontCare;
    for (--myargc; myargc > 0; --myargc, ++argv) {
	foundOption = False;
	matches = 0;
	for (i=0; i < num_options; ++i) {
	    /* checking the sort order first insures we don't have to
	       re-do the check if the arg hits on the last entry in
	       the table.  Useful because usually '=' is the last entry
	       and users frequently specify geometry early in the command */
	    if (table_is_sorted == Check && i > 0 &&
		strcmp(options[i].option, options[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	    for (argP = *argv, optP = options[i].option;
		 (optchar = *optP++) &&
		 (argchar = *argP++) &&
		 argchar == optchar;);
	    if (!optchar) {
		if (!*argP ||
		    options[i].argKind == XrmoptionStickyArg ||
		    options[i].argKind == XrmoptionIsArg) {
		    /* give preference to exact matches, StickyArg and IsArg */
		    matches = 1;
		    foundOption = i;
		    break;
		}
	    }
	    else if (!argchar) {
		/* may be an abbreviation for this option */
		matches++;
		foundOption = i;
	    }
	    else if (table_is_sorted == Sorted && optchar > argchar) {
		break;
	    }
	    if (table_is_sorted == Check && i > 0 &&
		strcmp(options[i].option, options[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	}
	if (table_is_sorted == Check && i >= (num_options-1))
	    table_is_sorted = Sorted;
	if (matches == 1) {
		i = foundOption;
		switch (options[i].argKind){
		case XrmoptionNoArg:
		    --(*argc);
		    PutCommandResource(options[i].value);
		    break;
			    
		case XrmoptionIsArg:
		    --(*argc);
		    PutCommandResource(*argv);
		    break;

		case XrmoptionStickyArg:
		    --(*argc);
		    PutCommandResource(argP);
		    break;

		case XrmoptionSepArg:
		    if (myargc > 1) {
			++argv; --myargc; --(*argc); --(*argc);
			PutCommandResource(*argv);
		    } else
			(*argsave++) = (*argv);
		    break;
		
		case XrmoptionResArg:
		    if (myargc > 1) {
			++argv; --myargc; --(*argc); --(*argc);
			XrmPutLineResource(pdb, *argv);
		    } else
			(*argsave++) = (*argv);
		    break;
		
		case XrmoptionSkipArg:
		    if (myargc > 1) {
			--myargc;
			(*argsave++) = (*argv++);
		    }
		    (*argsave++) = (*argv); 
		    break;

		case XrmoptionSkipLine:
		    for (; myargc > 0; myargc--)
			(*argsave++) = (*argv++);
		    break;

		case XrmoptionSkipNArgs:
		    {
			register int j = 1 + (int) options[i].value;

			if (j > myargc) j = myargc;
			for (; j > 0; j--) {
			    (*argsave++) = (*argv++);
			    myargc--;
			}
			argv--;		/* went one too far before */
			myargc++;
		    }
		    break;

		default:
		    _XReportParseError (&options[i], "unknown kind");
		    break;
		}
	}
	else
	    (*argsave++) = (*argv);  /*compress arglist*/ 
    }

    if (argsave < argend)
	(*argsave)=NULL; /* put NULL terminator on compressed argv */
}

/* ********       Functions from Quarks.c         *********  */

typedef unsigned long Entry;
#ifdef PERMQ
typedef unsigned char Bits;
#endif

static XrmQuark nextQuark = 1;	/* next available quark number */
static unsigned long quarkMask = 0;
static Entry zero = 0;
static Entry *quarkTable = &zero; /* crock */
static unsigned long quarkRehash;
static XrmString **stringTable = NULL;
#ifdef PERMQ
static Bits **permTable = NULL;
#endif
static XrmQuark nextUniq = -1;	/* next quark from XrmUniqueQuark */

#define QUANTUMSHIFT	8
#define QUANTUMMASK	((1 << QUANTUMSHIFT) - 1)
#define CHUNKPER	8
#define CHUNKMASK	((CHUNKPER << QUANTUMSHIFT) - 1)

#define LARGEQUARK	((Entry)0x80000000L)
#define QUARKSHIFT	18
#define QUARKMASK	((LARGEQUARK - 1) >> QUARKSHIFT)
#define SIGMASK		((1L << QUARKSHIFT) - 1)

#define STRQUANTSIZE	(sizeof(XrmString) * (QUANTUMMASK + 1))
#ifdef PERMQ
#define QUANTSIZE	(STRQUANTSIZE + \
			 (sizeof(Bits) * ((QUANTUMMASK + 1) >> 3))
#else
#define QUANTSIZE	STRQUANTSIZE
#endif

#define HASH(sig) ((sig) & quarkMask)
#define REHASHVAL(sig) ((((sig) % quarkRehash) + 2) | 1)
#define REHASH(idx,rehash) ((idx + rehash) & quarkMask)
#define NAME(q) stringTable[(q) >> QUANTUMSHIFT][(q) & QUANTUMMASK]
#ifdef PERMQ
#define BYTEREF(q) permTable[(q) >> QUANTUMSHIFT][((q) & QUANTUMMASK) >> 3]
#define ISPERM(q) (BYTEREF(q) & (1 << ((q) & 7)))
#define SETPERM(q) BYTEREF(q) |= (1 << ((q) & 7))
#define CLEARPERM(q) BYTEREF(q) &= ~(1 << ((q) & 7))
#endif

/* Permanent memory allocation */

#define WALIGN sizeof(unsigned long)
#define DALIGN sizeof(double)

#define NEVERFREETABLESIZE ((8192-12) & ~(DALIGN-1))
static char *neverFreeTable = NULL;
static int  neverFreeTableSize = 0;

static char *permalloc(length)
    register unsigned int length;
{
    char *ret;

    if (neverFreeTableSize < length) {
	if (length >= NEVERFREETABLESIZE)
	    return Xmalloc(length);
	if (! (ret = Xmalloc(NEVERFREETABLESIZE)))
	    return (char *) NULL;
	neverFreeTableSize = NEVERFREETABLESIZE;
	neverFreeTable = ret;
    }
    ret = neverFreeTable;
    neverFreeTable += length;
    neverFreeTableSize -= length;
    return(ret);
}

char *Xpermalloc(length)
    unsigned int length;
{
    int i;

    if (neverFreeTableSize && length < NEVERFREETABLESIZE) {
#ifndef WORD64
	if ((sizeof(struct {char a; double b;}) !=
	     (sizeof(struct {char a; unsigned long b;}) -
	      sizeof(unsigned long) + sizeof(double))) &&
	    !(length & (DALIGN-1)) &&
	    (i = (NEVERFREETABLESIZE - neverFreeTableSize) & (DALIGN-1))) {
	    neverFreeTableSize -= DALIGN - i;
	    neverFreeTable += DALIGN - i;
	} else
#endif
	    if (i = (NEVERFREETABLESIZE - neverFreeTableSize) & (WALIGN-1)) {
		neverFreeTableSize -= WALIGN - i;
		neverFreeTable += WALIGN - i;
	    }
    }
    return permalloc(length);
}

static Bool
ExpandQuarkTable()
{
    unsigned long oldmask, newmask;
    register char c, *s;
    register Entry *oldentries, *entries;
    register Entry entry;
    register int oldidx, newidx, rehash;
    Signature sig;
    XrmQuark q;

    oldentries = quarkTable;
    if (oldmask = quarkMask)
	newmask = (oldmask << 1) + 1;
    else {
	if (!stringTable) {
	    stringTable = (XrmString **)Xmalloc(sizeof(XrmString *) *
						CHUNKPER);
	    if (!stringTable)
		return False;
	    stringTable[0] = (XrmString *)NULL;
	}
#ifdef PERMQ
	if (!permTable)
	    permTable = (Bits **)Xmalloc(sizeof(Bits *) * CHUNKPER);
	if (!permTable)
	    return False;
#endif
	stringTable[0] = (XrmString *)Xpermalloc(QUANTSIZE);
	if (!stringTable[0])
	    return False;
#ifdef PERMQ
	permTable[0] = (Bits *)((char *)stringTable[0] + STRQUANTSIZE);
#endif
	newmask = 0x1ff;
    }
    entries = (Entry *)Xmalloc(sizeof(Entry) * (newmask + 1));
    if (!entries)
	return False;
    bzero((char *)entries, sizeof(Entry) * (newmask + 1));
    quarkTable = entries;
    quarkMask = newmask;
    quarkRehash = quarkMask - 2;
    for (oldidx = 0; oldidx <= oldmask; oldidx++) {
	if (entry = oldentries[oldidx]) {
	    if (entry & LARGEQUARK)
		q = entry & (LARGEQUARK-1);
	    else
		q = (entry >> QUARKSHIFT) & QUARKMASK;
	    for (sig = 0, s = NAME(q); c = *s++; )
		sig = (sig << 1) + c;
	    newidx = HASH(sig);
	    if (entries[newidx]) {
		rehash = REHASHVAL(sig);
		do {
		    newidx = REHASH(newidx, rehash);
		} while (entries[newidx]);
	    }
	    entries[newidx] = entry;
	}
    }
    if (oldmask)
	Xfree((char *)oldentries);
    return True;
}

#if NeedFunctionPrototypes
XrmQuark _XrmInternalStringToQuark(
    register _Xconst char *name, register int len, register Signature sig,
    Bool permstring)
#else
XrmQuark _XrmInternalStringToQuark(name, len, sig, permstring)
    register XrmString name;
    register int len;
    register Signature sig;
    Bool permstring;
#endif
{
    register XrmQuark q;
    register Entry entry;
    register int idx, rehash;
    register int i;
    register char *s1, *s2;
    char *new;

    rehash = 0;
    idx = HASH(sig);
    while (entry = quarkTable[idx]) {
	if (entry & LARGEQUARK)
	    q = entry & (LARGEQUARK-1);
	else {
	    if ((entry - sig) & SIGMASK)
		goto nomatch;
	    q = (entry >> QUARKSHIFT) & QUARKMASK;
	}
	for (i = len, s1 = (char *)name, s2 = NAME(q); --i >= 0; ) {
	    if (*s1++ != *s2++)
		goto nomatch;
	}
	if (*s2) {
nomatch:    if (!rehash)
		rehash = REHASHVAL(sig);
	    idx = REHASH(idx, rehash);
	    continue;
	}
#ifdef PERMQ
	if (permstring && !ISPERM(q)) {
	    Xfree(NAME(q));
	    NAME(q) = (char *)name;
	    SETPERM(q);
	}
#endif
	return q;
    }
    if (nextUniq == nextQuark)
	return NULLQUARK;
    if ((nextQuark + (nextQuark >> 2)) > quarkMask) {
	if (!ExpandQuarkTable())
	    return NULLQUARK;
	return _XrmInternalStringToQuark(name, len, sig, permstring);
    }
    q = nextQuark;
    if (!(q & QUANTUMMASK)) {
	if (!(q & CHUNKMASK)) {
	    if (!(new = Xrealloc((char *)stringTable,
				 sizeof(XrmString *) *
				 ((q >> QUANTUMSHIFT) + CHUNKPER))))
		return NULLQUARK;
	    stringTable = (XrmString **)new;
#ifdef PERMQ
	    if (!(new = Xrealloc((char *)permTable,
				 sizeof(Bits *) *
				 ((q >> QUANTUMSHIFT) + CHUNKPER))))
		return NULLQUARK;
	    permTable = (Bits **)new;
#endif
	}
	new = Xpermalloc(QUANTSIZE);
	if (!new)
	    return NULLQUARK;
	stringTable[q >> QUANTUMSHIFT] = (XrmString *)new;
#ifdef PERMQ
	permTable[q >> QUANTUMSHIFT] = (Bits *)(new + STRQUANTSIZE);
#endif
    }
    if (!permstring) {
	s2 = (char *)name;
#ifdef PERMQ
	name = Xmalloc(len+1);
#else
	name = permalloc(len+1);
#endif
	if (!name)
	    return NULLQUARK;
	for (i = len, s1 = (char *)name; --i >= 0; )
	    *s1++ = *s2++;
	*s1++ = '\0';
#ifdef PERMQ
	CLEARPERM(q);
    }
    else {
	SETPERM(q);
#endif
    }
    NAME(q) = (char *)name;
    if (q <= QUARKMASK)
	entry = (q << QUARKSHIFT) | (sig & SIGMASK);
    else
	entry = q | LARGEQUARK;
    quarkTable[idx] = entry;
    nextQuark++;
    return q;
}

#if NeedFunctionPrototypes
XrmQuark XrmStringToQuark(
    _Xconst char *name)
#else
XrmQuark XrmStringToQuark(name)
    XrmString name;
#endif
{
    register char c, *tname;
    register Signature sig = 0;

    if (!name)
        return (NULLQUARK);
   
    for (tname = (char *)name; c = *tname++; )
        sig = (sig << 1) + c;

    return _XrmInternalStringToQuark(name, tname-(char *)name-1, sig, False);
}

#if NeedFunctionPrototypes
XrmQuark XrmPermStringToQuark(
    _Xconst char *name)
#else
XrmQuark XrmPermStringToQuark(name)
    XrmString name;
#endif
{
    register char c, *tname;
    register Signature sig = 0;

    if (!name)
	return (NULLQUARK);

    for (tname = (char *)name; c = *tname++; )
	sig = (sig << 1) + c;

    return _XrmInternalStringToQuark(name, tname-(char *)name-1, sig, True);
}

XrmString XrmQuarkToString(quark)
    register XrmQuark quark;
{
    if (quark <= 0 || quark >= nextQuark)
    	return NULLSTRING;
#ifdef PERMQ
    /* We have to mark the quark as permanent, since the caller might hold
     * onto the string pointer forver.
     */
    SETPERM(quark);
#endif
    return NAME(quark);
}
