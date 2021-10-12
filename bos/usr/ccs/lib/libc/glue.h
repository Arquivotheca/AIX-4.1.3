/* @(#)07	1.1  src/bos/usr/ccs/lib/libc/glue.h, libcio, bos411, 9428A410j 10/20/93 14:18:51 */
/*
 *   COMPONENT_NAME: LIBC
 *
 *   FUNCTIONS: inuse
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#include <stdio.h>
#include "ts_supp.h"
#ifdef _THREAD_SAFE
#define inuse(iop)	((iop)->_flag & (_IOINUSE|_IOREAD|_IOWRT|_IORW))
#else
#define inuse(iop)	((iop)->_flag & (_IOREAD|_IOWRT|_IORW))
#endif /* _THREAD_SAFE */

#define _NIOBRW		16
#define _NROWSIZE	(_NIOBRW * sizeof(FILE))
#define _NROWSTART	4
#define _NROWEXTEND	(_NROWSTART << 2)

struct glued {
        unsigned   lastfile;
        unsigned   freefile;
        int     nfiles;
        int     nrows;
        int     crow;
        FILE    **iobptr;
        };

extern struct glued _glued;
