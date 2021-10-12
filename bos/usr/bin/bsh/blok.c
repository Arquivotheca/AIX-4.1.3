static char sccsid[] = "@(#)86	1.23  src/bos/usr/bin/bsh/blok.c, cmdbsh, bos41J, 9521B_all 5/25/95 10:55:01";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: alloc alloc_free addblok checkbptr checkmem
 *            free setup_mem realloc
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.14  com/cmd/sh/sh/blok.c, cmdsh, bos320, 9125320 6/6/91 23:10:09
 * 
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 *
 * OSF/1 1.1 
 */

#include    "defs.h"

/*
 *    storage allocator
 *    (circular first fit strategy)
 */

#define BUSY 01

#define busy(x)    (Rcheat((x)->word) & BUSY)

unsigned	brkincr = BRKINCR;          /* 5 pages */


#ifndef _SHRLIB /* NOT shared library */
void 		setup_mem(); /* Perform pre-malloc setup. */
struct blk 	*blokp = (struct blk *)NULL; /* current search pointer */
struct blk 	*bloktop;                    /* top of arena (last blok) */
uchar_t		*brkbegin;
uchar_t		*setbrk();


#ifdef TRACK_BSH_MALLOC

#define   MLOG_SIZE   1023
#define   REALLOC_ID  0x20000000

struct {
    unsigned int	locatingString,
			loggingIndex,
			loggingCounter;
    struct {
        unsigned int	callerAddress,
			bufferAddress,   /* watch for REALLOC_ID */
			bufferSize;
    } entry[MLOG_SIZE];

} loggingMalloc = {0xAAAABBBB, 0, 0};


#define MALLOC_LOG(AAA, BBB, CCC)                                        \
        loggingMalloc.entry[loggingMalloc.loggingIndex].callerAddress =  \
                           (unsigned int) (AAA);                         \
        loggingMalloc.entry[loggingMalloc.loggingIndex].bufferAddress =  \
                           (unsigned int) (BBB);                         \
        loggingMalloc.entry[loggingMalloc.loggingIndex].bufferSize    =  \
                           (unsigned int) (CCC);                         \
        loggingMalloc.loggingCounter++;                                  \
        if (++loggingMalloc.loggingIndex >= MLOG_SIZE)                   \
            loggingMalloc.loggingIndex = 0

#endif /* TRACK_BSH_MALLOC */


uchar_t *
alloc(unsigned nbytes)
{
    register unsigned	rbytes = round(nbytes+BYTESPERWORD, BYTESPERWORD);

#ifdef TRACK_BSH_MALLOC
    unsigned int	*callerAddress = (int *) &nbytes ;
#endif /* TRACK_BSH_MALLOC */

    /* Initialize memory allocation on 1st call. */
    if (blokp == (struct blk *)NULL) setup_mem();

    for (;;) {
        int    c = 0;
        register struct blk *p = blokp;
        register struct blk *q;

        do {
            if (!busy(p)) {
                while (!busy(q = p->word))
                    p->word = q->word;
                if ((char *)q - (char *)p >= rbytes) {
                    blokp = (struct blk *)((uchar_t *)p + rbytes);
                    if (q > blokp)
                        blokp->word = p->word;
                    p->word = (struct blk *)(Rcheat(blokp) | BUSY);

#ifdef TRACK_BSH_MALLOC
                    MALLOC_LOG(callerAddress[-4], p +1, nbytes);
#endif /* TRACK_BSH_MALLOC */

                    return((uchar_t *)(p + 1));
                }
            }
            q = p;
            p = (struct blk *)(Rcheat(p->word) & ~BUSY);
        } while (p > q || (c++) == 0);
        addblok(rbytes);
    }
}

addblok(reqd)
    unsigned reqd;
{
    if (stakbot == 0) {
        brkbegin = setbrk(BRKINCR);
        bloktop = (struct blk *)brkbegin;
    }

    if (stakbas != staktop) {
        register uchar_t *rndstak;
        register struct blk *blokstak;

        pushstak(0);
        rndstak = (uchar_t *)round(staktop, BYTESPERWORD);
	if ((Rcheat(rndstak) + sizeof(char *)) >= brkend)
		(void) setbrk(BRKINCR);
        blokstak = (struct blk *)(stakbas) - 1;
        blokstak->word = stakbsy;
        stakbsy = blokstak;
        bloktop->word = (struct blk *)(Rcheat(rndstak) | BUSY);
        bloktop = (struct blk *)(rndstak);
    }

    reqd += brkincr;
    reqd &= ~(brkincr - 1);
    needmem (Rcheat(bloktop)+reqd+BRKINCR+
	    (staktop ? (int)(staktop-stakbot) : 0));

    if ((Rcheat(bloktop) + reqd + sizeof(char *)) >= brkend)
        (void) setbrk(BRKINCR);

    blokp = bloktop;
    bloktop = bloktop->word = (struct blk *)(Rcheat(bloktop) + reqd);
    bloktop->word = (struct blk *)(brkbegin + 1);

    {
        register uchar_t	*stakadr = (uchar_t *)(bloktop + 2);

        if (stakbot != staktop)
            staktop = movstr(stakbot, stakadr);
        else
            staktop = stakadr;

        stakbas = stakbot = stakadr;
    }
}

void
free(void *vap)
{
    struct blk		*ap = (struct blk *) vap;
    register struct blk	*p;

#ifdef TRACK_BSH_MALLOC
    unsigned int  *callerAddress = (int *) &vap;
    MALLOC_LOG(callerAddress[-4], vap, 0xfeeeeeee);
#endif /* TRACK_BSH_MALLOC */

    if ((p = ap) && (p < bloktop && p >= (struct blk *)brkbegin)) {

#ifdef DEBUG
        chkbptr(p);
#endif /* DEBUG */

        --p;
        p->word = (struct blk *)(Rcheat(p->word) & ~BUSY);
    }
}


#ifdef DEBUG /* debug routines */
void
chkbptr(struct bld *ptr)
{
    int			exf = 0;
    register struct blk	*p = (struct blk *)brkbegin;
    register struct blk	*q;
    int			us = 0,
			un = 0;

    for (;;) {
        q = (struct blk *)(Rcheat(p->word) & ~BUSY);

        if (p+1 == ptr)
            exf++;

        if (q < (struct blk *)brkbegin || q > bloktop)
            abort(3);

        if (p == bloktop)
            break;

        if (busy(p))
            us += q - p;
        else
            un += q - p;

        if (p >= q)
            abort(4);

        p = q;
    }

    if (exf == 0)
        abort(1);

}


void
chkmem()
{
    register struct blk	*p = (struct blk *)brkbegin;
    register struct blk	*q;
    int			us = 0,
			un = 0;

    for (;;) {
        q = (struct blk *)(Rcheat(p->word) & ~BUSY);

        if (q < (struct blk *)brkbegin || q > bloktop)
            abort(3);

        if (p == bloktop)
            break;

        if (busy(p))
            us += q - p;
        else
            un += q - p;

        if (p >= q)
            abort(4);

        p = q;
    }

    prs("un/used/avail ");
    prn(un);
    blank();
    prn(us);
    blank();
    prn((char *)bloktop - brkbegin - (un + us));
    newline();

}
#endif /* DEBUG */

/* Setup memory allocation for the shell's alloc (malloc) */
void
setup_mem()
{
    /* Only do this once. */
    if (blokp != (struct blk *)NULL) 
        return;

    /* initialise storage allocation */
    stakbot = 0;
    addblok((unsigned)0);

    /*    during testing it was discovered that signal
     *    handling had to be set before a call to setlocale
     *    setlocale was making a call to the shell's malloc
     *    which was causing a memory fault
    */
    stdsigs();
}

#else /* shared library */

    extern void		*end;
    uchar_t		*brkbegin = (uchar_t *) &end;
    uchar_t		*bloktop  = (uchar_t *) &end;

#undef free
/* in the shared library version, the library malloc is used to allocate.
 * However, some shell routines free pointers which where not malloc'd and
 * expect this to be ignored.  We test by checking for addresses within
 * the heap.  To reduce the cost of the test, we only use sbrk to check
 * for the true end of heap when the check fails.  Eventually, blktop
 * should converge to the high water mark of the heap.
 */

void
alloc_free(uchar_t *ap)
{
    if (ap >= (uchar_t *)brkbegin && ap <= (uchar_t *)bloktop)
        free(ap);
    else {
        bloktop = (uchar_t *)sbrk(0);
        if (ap >=(uchar_t *) brkbegin && ap <=(uchar_t *) bloktop)
            free(ap);
    }
    
}

/* use only to init stak */
addblok(reqd)
    unsigned	reqd;
{

    if (stakbot == 0)
    {
        growstak();
    }

}
#endif /* NOT _SHRLIB */

#ifndef	_SHRLIB /* NOT shared library */

	/*	The following code is for an unshared version of
	 *	the sh. This is a primitive version of realloc and
	 *	is not intended to be a final solution for anything.
	 *	This code should be deleted when the shared library
	 *	solution has been obtained.
	 */

void *
realloc(void *buf_adr, size_t nbytes)
{

    void	*new_adr;

#ifdef TRACK_BSH_MALLOC
    unsigned int *callerAddress = (int *) &buf_adr;
#endif /* TRACK_BSH_MALLOC */

    if (new_adr = malloc (nbytes))
        if (new_adr != buf_adr)
            bcopy (buf_adr, new_adr, nbytes);

#ifdef TRACK_BSH_MALLOC
    MALLOC_LOG(callerAddress[-4],(unsigned int)buf_adr | REALLOC_ID,nbytes);
#endif /* TRACK_BSH_MALLOC */

    return (new_adr);

}

#endif /* NOT _SHRLIB */
