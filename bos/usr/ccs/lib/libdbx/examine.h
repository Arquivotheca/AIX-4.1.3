/* @(#)45	1.3  src/bos/usr/ccs/lib/libdbx/examine.h, libdbx, bos411, 9428A410j 2/8/93 17:04:09 */
#ifndef _h_examine
#define _h_examine
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
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

#include "decode.h"

/*
 * Return type for getskip()
 */
typedef enum {
    skipfunc,
    skipmodule,
    skipnone
}	skiptype;

extern skiptype stepignore;

extern printinst (/* lowaddr, highaddr */);
extern printninst (/* count, addr */);
extern Address printdata (/* lowaddr, highaddr, format */);
extern printndata (/* count, startaddr, format */);
extern printvalue (/* v, format */);
extern printerror (/*  */);
extern printsig (/* signo */);
extern endprogram (/*  */);
extern dostep (/* isnext */);
extern setbp (/* addr */);
extern unsetbp (/* addr */);
#endif /* _h_examine */
