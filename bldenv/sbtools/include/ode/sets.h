/* @(#)66       1.1  src/bldenv/sbtools/include/ode/sets.h, bldprocess, bos412, GOLDA411a 1/19/94 17:37:08
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
 * $Log: sets.h,v $
 * Revision 1.1.4.1  1993/11/04  19:52:11  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  19:51:38  damon]
 *
 * Revision 1.1.2.3  1993/04/28  20:06:43  damon
 * 	CR 463. Prototypes
 * 	[1993/04/28  20:06:31  damon]
 * 
 * Revision 1.1.2.2  1993/04/28  19:21:42  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  19:21:22  damon]
 * 
 * $EndLog$
 */

BOOLEAN
sets_access ( char * basedir, char * sb );

void
sets_fullname ( char * setn, char * user, char ** symbolic_name );

void
sets_write  ( 
    const char      * sb,                                    /* sandbox name */
    const char      * sbbase,                       /* holds path to sandbox */
    const char      * setname,                         /* holds full setname */
    const char      * setdir );                     /* name of set directory */

int
current_set (
    char     ** setname,                             /* the set name to fill */
    char     ** setdir,                              /* set directory t fill */
    char     ** sbname,                               /* the current sandbox */
    char     ** rc_file );                                 /* rc file to use */
