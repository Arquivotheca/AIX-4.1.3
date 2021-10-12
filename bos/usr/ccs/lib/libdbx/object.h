/* @(#)67	1.7  src/bos/usr/ccs/lib/libdbx/object.h, libdbx, bos411, 9428A410j 3/21/94 17:07:25 */
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: 
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

struct obj_stat {
    unsigned int stringsize;	/* size of the dumped string table */
    unsigned int nsyms;		/* number of symbols */
    unsigned int nfiles;	/* number of files */
    unsigned int nlines;	/* number of lines */
} *nlhdr;
extern int nfiles_total;	/* total number of files encountered */
extern int nlines_total;	/* total number of lines encountered */

struct reloc_table
{
  unsigned int old_addr;
  unsigned int new_addr;
};

struct heat_shrink {
    unsigned int num_elements;
    struct reloc_table *map;
};

#include "languages.h"
#include "symbols.h"

/* Return the address of a symbol table entry. */

#define TABLECHUNK  10000	/* Number of entries in a single symtab chunk */
#define sym_addr(x)  \
        (&namelist[(unsigned)(x) / TABLECHUNK][(unsigned)(x) % TABLECHUNK])

/*
 * Move the object address for a procedure or function up the
 * appropriate amount.
 */

#define findbeginning(f) ( (f)->symvalue.funcv.beginaddr += FUNCOFFSET )
#define DEFAULTOBJNM	 "a.out"


#ifndef N_MOD2
#endif
extern String objname ;
extern integer objsize;
extern Language curlang;
extern Symbol curmodule;
extern Symbol curparam;
extern Symbol curcomm;
extern Address curstat;
extern Symbol commchain;
extern short stab_compact_level;        /* stabstring compaction level */
extern String curfilename (/*  */);
extern Symbol curblock;
extern File openfd();
extern pushBlock (/* b */);
extern changeBlock (/* b */);
extern enterblock (/* b */);
extern exitblock (/*  */);
extern readobj (/* file */);
extern String getcont (/*  */);
extern objfree (/*  */);
extern chkUnnamedBlock (/*  */);
