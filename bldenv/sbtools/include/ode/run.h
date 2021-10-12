/* @(#)62       1.1  src/bldenv/sbtools/include/ode/run.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:54
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990
 * Open Software Foundation, Inc.
 *
 * Permission is hereby granted to use, copy, modify and freely distribute
 * the software in this file and its documentation for any purpose without
 * fee, provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.  Further, provided that the name of Open
 * Software Foundation, Inc. ("OSF") not be used in advertising or
 * publicity pertaining to distribution of the software without prior
 * written permission from OSF.  OSF makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */
/*
 * HISTORY
 * $Log: run.h,v $
 * Revision 1.1.6.3  1993/11/08  20:17:58  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:19  damon]
 *
 * Revision 1.1.6.2  1993/11/05  22:43:13  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  22:41:20  damon]
 * 
 * Revision 1.1.6.1  1993/11/03  20:40:10  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:39:12  damon]
 * 
 * Revision 1.1.4.1  1993/08/19  16:40:26  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  16:39:56  damon]
 * 
 * Revision 1.1.2.2  1993/04/28  18:06:25  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  18:06:09  damon]
 * 
 * $EndLog$
 */
#ifndef _RUN_H
#define _RUN_H
#ifdef __STDC__
int
full_runcmdv(const char * , ... );
int
runcmd(const char *, const char *, ... );
int
fd_runcmd(const char *, const char *, int , int , int , ... );
int
runp (const char *, ...);
#endif
int
endcmd( int );
int
runcmdv( const char *, const char *, char **);
int
runv ( const char *, char * const * );
int
runvp ( const char *, char * const *);
#endif
