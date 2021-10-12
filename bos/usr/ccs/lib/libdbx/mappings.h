/* @(#)61	1.10  src/bos/usr/ccs/lib/libdbx/mappings.h, libdbx, bos411, 9428A410j 3/23/93 10:21:26 */
#ifndef _h_mappings
#define _h_mappings
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros)
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
#include "machine.h"
#include "source.h"
#include "symbols.h"

typedef enum { EXACT, NEXT, PREVIOUS } line_loc;  /* for findline() */

typedef struct {
    Address addr;
    Lineno line;
} Linetab;

typedef struct lchunk {
    Linetab *lp;
    Linetab *lend;
    unsigned int line_reloc;	/* Base line for relatively numbered lines. */
    struct lchunk *next;
} Linechunk;

typedef struct {
    String filename;
    Linechunk *lineptr;		/* list of line number sections */
    struct includefile *incl_chain;	/* list of include files */
} Filetab;

typedef struct {
    Symbol func;
    Address addr;	/* Possibly use func->symvalue.funcv... here */
    Address tbsearch;	/* Lowest search address for finding trace table */
    Address tbfound;	/* Address where trace table was found */
    Filetab *filep;	/* File table entry for this file. */
    unsigned int tbcontents[TBSIZE];	/* Contents of tracetable */
    unsigned int exceptptr; /* ptr to .except table */
} AddrOfFunc;

typedef struct {
    Linetab *lp;
    Linetab *lend;
    Address addr;
} Lineaux;

typedef struct includefile {
    String filename;
    struct includefile *incl_chain;	/* Next include file at same level */
    struct includefile *incl_parent;	/* Includer of include file */
    struct includefile *incl_child;	/* Include files included by this. */
    Linetab *lp;		/* First line number within include. */
    Linetab *lend;		/* Last line number within this include. */
    Filetab *src_file;		/* Top level source file. */
} Incfile;

#define NOADDR ((Address) -1)	/* no address for line or procedure */
#define TBINIT 0xffeeddcc	/* Implausible starting word to TB table */

extern Filetab **filetab;
extern Linetab **linetab;
extern Lineaux *lineaux;
extern String srcfilename(/* addr */);
extern Lineno srcline (/* addr */);
extern Lineno linelookup(/* addr */);
extern Address nextline(/* addr */);
extern Linetab *findline(/* addr, location */);
extern Address objaddr(/* line, name */);
extern newfunc(/* f, addr */);
extern Symbol whatblock(/* addr */);
extern ordfunctab(/*  */);
extern clrfunctab(/*  */);
extern dumpfunctab(/*  */);
extern Incfile *addrtoincl();
#endif /* _h_mappings */
