static char sccsid[] = "@(#)64	1.3.1.3  src/bos/usr/ccs/lib/libdbx/names.c, libdbx, bos411, 9428A410j 10/5/93 12:08:47";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: identname, names_free, Demangle, EraseDemangledName
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

/*
 * Name are the internal representation for identifiers.
 *
 * A hash table is used to map identifiers to names.
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "defs.h"
#include "names.h"
#include <demangle.h>

/*
 * The hash table is a power of two, in order to make hashing faster.
 * Using a non-prime is ok since we use chaining instead of re-hashing.
 */

#define HASHTABLESIZE 8192

private Name nametable[HASHTABLESIZE];

/*
 * Names are allocated in large chunks to avoid calls to malloc
 * and to cluster names in memory so that tracing hash chains
 * doesn't cause many a page fault.
 */

#define CHUNKSIZE 1000

typedef struct Namepool {
    struct Name name[CHUNKSIZE];
    struct Namepool *prevpool;
} *Namepool;

private Namepool namepool = nil;
private Integer nleft = 0;

/*
 * Given an identifier, convert it to a name.
 * If it's not in the hash table, then put it there.
 *
 * The second argument specifies whether the string should be copied
 * into newly allocated space if not found.
 *
 * This routine is time critical when starting up the debugger
 * on large programs.
 */

public Name identname(s, isallocated)
String s;
Boolean isallocated;
{
    register unsigned h = 0;
    register char *p, *q;
    register Name n, *np;
    Namepool newpool;
    extern char* get_name();

    for (p = s; *p != '\0'; p++) {
	h = (h << 1) ^ (*p);
    }
    h &= (HASHTABLESIZE-1);
    np = &nametable[h];
    n = *np;
    while (n != nil) {
	if (!strcmp(s,n->identifier))
	   return n;
	n = n->chain;
    }

    /*
     * Now we know that name hasn't been found,
     * so we allocate a name, store the identifier, and
     * enter it in the hash table.
     */
    if (nleft <= 0) {
	newpool = new(Namepool);
	newpool->prevpool = namepool;
	namepool = newpool;
	nleft = CHUNKSIZE;
    }
    --nleft;
    n = &(namepool->name[nleft]);
    if (isallocated) {
	n->identifier = s;
    } else {
	/* this case doesn't happen very often */
        n->identifier = get_name(s, p);
    }
    n->chain = *np;
    *np = n;
    return n;
}

/*
 * Deallocate the name table.
 */

public names_free()
{
    Namepool n, m;
    register integer i;

    n = namepool;
    while (n != nil) {
	m = n->prevpool;
	dispose(n);
	n = m;
    }
    /* Zero out nametable */
    memset(nametable,0,4*HASHTABLESIZE);
    namepool = nil;
    nleft = 0;
}

/*
 * Demangle a mangled C++ name.
 */

DemangledName Demangle(Name name)
{
    char *rest;
    void *dName;
    DemangledName d;

    dName = demangle(ident(name), &rest, RegularNames);
    d = (DemangledName)malloc(sizeof(struct DemangledName));
    if (dName == nil)
    {
        d->name = name;
	d->shortName = d->fullName = d->qualName = ident(name);
        d->params = "";
    }
    else
    {
	char *s;
	int nlen;

        enum NameKind nameKind = kind(dName);
        if (nameKind == MemberVar)
	    s = varName(dName);
	else
	    s = functionName(dName);
	d->name = identname(s, true);

	nlen = strlen(ident(d->name));
	if (nameKind == MemberVar || nameKind == MemberFunction)
	{
	    int qlen = strlen(qualifier(dName));

	    s = malloc(qlen + nlen + 3);
	    strcpy (s, qualifier(dName));
	    strcpy (&s[qlen], "::");
	    strcpy (&s[qlen + 2], ident(d->name));
	    d->qualName = s;
	}
	else 
	    d->qualName = ident(d->name);

	d->fullName = text(dName);
	d->params = d->fullName + strlen(d->qualName);
	d->shortName = d->params - nlen;
    }
    d->mName = name;
    d->dName = dName;
    return d;
}

void EraseDemangledName(d)
/* Free all memory allocated to a demangled name. Recall that all but (maybe) */
/* the qualifier are merely pointers into the struct cName of the name.       */
DemangledName d;
{
    if (d->qualName != ident(d->name))
        free(d->qualName);
    if (d->dName != nil)
        erase(d->dName);
    free(d);
}
