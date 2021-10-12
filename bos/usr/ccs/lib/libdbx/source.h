/* @(#)81	1.2.1.3  src/bos/usr/ccs/lib/libdbx/source.h, libdbx, bos411, 9428A410j 3/23/93 10:22:19 */
#ifndef _h_source
#define _h_source
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
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
typedef int Lineno;


#define LASTLINE 0		/* recognized by printlines */

#include "lists.h"

extern String cursource;
#define basefile(name)  \
  ((rindex ((name), '/') == NULL) ? name : rindex ((name), '/') + 1)
extern char *filecmdcursrc;
#define FILE_CMD 0x01
#define EDIT_CMD 0x02

extern Lineno curline;
extern Lineno cursrcline;
extern int cursrclang;
extern int windowsize;
extern int inst_windowsize;
extern int windowtop;
extern List sourcepath;
extern boolean canReadSource (/*  */);
extern printlines(/* l1, l2 */);
extern String findsource(/* filename , ptr to flag */);
extern File opensource(/* filename */);
extern setsource(/* filename */);
extern getsrcpos(/*  */);
extern printsrcpos(/*  */);
extern edit(/* filename */);
extern search (/* direction, pattern */);
extern getsrcwindow (/* line, l1, l2 */);
extern srclang (/* filename */);
#endif /* _h_source */
