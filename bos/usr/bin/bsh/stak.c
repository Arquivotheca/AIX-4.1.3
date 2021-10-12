static char sccsid[] = "@(#)16	1.20  src/bos/usr/bin/bsh/stak.c, cmdbsh, bos41J, 9521A_all 5/23/95 17:56:47";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: getstak locstak savstak endstak tdystak stakchk cpystak growstak
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
 * 1.11  com/cmd/sh/sh/stak.c, cmdsh, bos324 
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include    "defs.h"
#include    <sys/access.h>
#include    <sys/param.h>

/* ========    storage allocation    ======== */

uchar_t *
getstak(int asize)            /* allocate requested stack */
{
    register uchar_t    *oldstak;
    register ptrdiff_t    size;

#ifdef  _SHRLIB /* shared library */
    locstak();    /* make sure stack is big enough */
#endif /* _SHRLIB */

    size = round(asize, BYTESPERWORD);
    oldstak = stakbot;
    staktop = stakbot += size;
    needmem (stakbot + BRKINCR);
    return(oldstak);
}

/*
 * set up stack for local use
 * should be followed by `endstak'
 */
uchar_t *
locstak()
{
#ifdef  _SHRLIB /* shared library */

    /*
     * In shared case we must have enough for any possible 
     * glob since malloc is the standard library version 
     * - so get BRKMAX always. N.B. this must be larger 
     * than the (NLS) max path name and the CPYSTR in io.c
     */
    if (brkend - stakbot < BRKMAX)
        if (growstak())
            error(MSGSTR(M_NOSPACE,(char *) nospace));
#else

    if (brkend - stakbot < BRKINCR)
        if (setbrk(brkincr)==-1)
            error(MSGSTR(M_NOSPACE,(char *)nospace));

#endif /* _SHRLIB */
    needmem (stakbot + BRKINCR);
    return(stakbot);
}

uchar_t *
savstak()
{
    assert(staktop == stakbot);
    return(stakbot);
}

uchar_t *
endstak(register uchar_t *argp)        /* tidy up after `locstak' */
{
    register uchar_t    *oldstak;

    *argp++ = 0;
    oldstak = stakbot;
    stakbot = staktop = (uchar_t *)round(argp, BYTESPERWORD);
    return(oldstak);
}

void
tdystak(register uchar_t *x)        /* try to bring stack back to x */
{

#ifndef _SHRLIB /* NOT shared library */

    while ((uchar_t *)stakbsy > x)
    {
        free(stakbsy);
        stakbsy = stakbsy->word;
    }

#endif /* NOT _SHRLIB */

    staktop = stakbot = MAX(x, stakbas);
    rmtemp(x);

}

void
stakchk()
{

#ifdef  _SHRLIB /* shared library */

    if (brkend - stakbas > BRKMAX+BRKMAX)
        stakbas = realloc(stakbas,(BRKMAX+BRKMAX));

#else /* NOT _SHRLIB */

    if (brkend - stakbot > BRKINCR+BRKINCR)
        setbrk(-BRKINCR);

#endif /* _SHRLIB */

}

uchar_t *
cpystak(uchar_t *x)
{
    register uchar_t    *argp = locstak ();

#ifdef  _SHRLIB /* shared library */

    assert(argp+length(x)+1 < brkend);
    locstak();    /* make sure its big enough */

#else /* NOT _SHRLIB */

    needmem (argp + length (x) + BRKINCR);

#endif /* _SHRLIB */

    return (endstak(movstr(x,argp)));
}

#ifdef  _SHRLIB /* shared library */

int
growstak()
{
    uchar_t    *oldbase;
    unsigned int    size;
    if (stakbot == 0)
    {
        size = BRKMAX+BRKMAX;
        stakbot = (uchar_t *)malloc(size);
        brkend = stakbot + size;
        stakbas = staktop = stakbot;
        return stakbas?0:-1;
    }
    size = (Rcheat(brkend) - Rcheat(stakbas));
    size += BRKMAX;
    oldbase = stakbas;
    stakbas = (uchar_t *) realloc(stakbas,size);
    if (stakbas){
        unsigned int    reloc;
        reloc = Rcheat(stakbas) - Rcheat(oldbase);
        stakbot += reloc;
        staktop += reloc;
        brkend = stakbas + size;
        return 0;
    }
    else return -1;
}

#endif /* _SHRLIB */
