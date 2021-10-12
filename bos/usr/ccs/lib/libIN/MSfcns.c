static char sccsid[] = "@(#)00	1.7  src/bos/usr/ccs/lib/libIN/MSfcns.c, libIN, bos411, 9428A410j 11/10/93 15:13:27";
/*
 * LIBIN: MSsegment, MSinit, MSterm, MSsavetop, MSrestoretop, MSgetblock
 *
 * ORIGIN: 9
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
 * FUNCTION: Segmented memory stack allocator functions.
 *
 */


/* a crock to get around some bogus compilers, e.g. SystemV.2 */
#define void int

#include <stdio.h>
#include <IN/standard.h>
#include <IN/MSdefs.h>
#include <nl_types.h>
#include "libIN_msg.h"

/* VARARGS */
static void error(f,a,b) char *f; {fprintf(stderr,f,a,b); exit(EXITFATAL);}

/* allocator to use if use did not supply one */
static char *alloc(sz) register unsigned sz; {
    extern char *malloc();
    nl_catd catd;
    register char *blk;
    if ((blk=malloc(sz))==(char *)NULL) {
	catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
	error(catgets(catd, MS_LIBIN, M_ALLOC, "memory allocation failed\n"));
	catclose(catd);
    }
    return(blk);}

#define getfirst(ms) ((MSsegment *)(LS1first(&(ms)->segs)))
#define freefirst(ms) ((*(ms)->free)((char *)LS1rmvfirst(&(ms)->segs)))
#define roundup(sz) (((sz) + ms->align) & ~ms->align)
#define fits(ms,sz) ((ms)->nextbyte+(sz) <= &getfirst(ms)->blk[ms->size])

/* Allocate a new Memory Stack segment */
static MSsegment *newseg(ms) register MSheader *ms; {
    register MSsegment *seg = 
		(MSsegment *)((*ms->alloc)(ms->size + sizeof(MSsegment)-1));
    ms->nextbyte = &(seg->blk[0]);
    return(seg);}

/* initialize a memory stack descriptor */
void MSinit(ms,size,align,al,fr,er) register MSheader *ms;
				register unsigned size,align;
				char *(*al)(); void (*fr)(), (*er)();{
    extern void free();
    if (align==0) align = sizeof(int);
    ms->align = align - 1;
    if (size==0) size = 2048;
    ms->size = roundup(size);
    ms->alloc = (al!=(char *(*)())NULL ? al : alloc);
    ms->error = (er!=(int (*)())NULL ? er : error);
    ms->free = (fr!=(void (*)())NULL ? fr : free);
    LS1init(&ms->segs);
    LS1append(&ms->segs,newseg(ms));}

/* finish up with a memory stack */
void MSterm(ms) register MSheader *ms; {
    while (!LS1isempty(&ms->segs)) freefirst(ms);}

/* Save a pointer to the top of a Memory Stack in a MSpointer
   which can be later passed to MSrestoretop to delete all blocks
   allocated with MSgetblock since the pointer was saved. */
MSpointer MSsavetop(ms) register MSheader *ms; {
    MSpointer retval;
    retval.ms = ms;
    retval.seg = getfirst(ms);
    retval.byte = ms->nextbyte;
    return(retval);}

/* Restore a stack pointer saved by MSsavetop. This has the effect of
   deleting all blocks allocated with MSgetblock since the MSsavetop. */
void MSrestoretop(msp) MSpointer msp; {
    register MSheader *ms = msp.ms;
    while (getfirst(ms) != msp.seg) freefirst(ms);
    ms->nextbyte = msp.byte;}

/* Extend stack by adding a block and readjusting MStop to point to
   the beginning of the new block.  A blank space may be left in
   previous block. */
static extend(ms) register MSheader *ms; {LS1prepend(&ms->segs,newseg(ms));}

/* Allocate a block on the top of the Memory Stack and return a     *
* pointer to it.  Calls extend if necessary to extend the stack. */
char *MSgetblock(ms,sz) register MSheader *ms; register unsigned sz; {
    register char *retval;
    nl_catd catd;
    sz = roundup(sz);
    if (sz > ms->size) {
	catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
	(*ms->error)(catgets(catd, MS_LIBIN, M_MSGETBLOCK,
	    "invalid allocation size: %d\n"), sz);
	catclose(catd);
    } else {
	  if (!fits(ms,sz)) extend(ms);
	  retval = ms->nextbyte;
	  ms->nextbyte += sz;}
    return(retval);}
