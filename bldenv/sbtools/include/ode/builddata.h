/* @(#)44       1.1  src/bldenv/sbtools/include/ode/builddata.h, bldprocess, bos412, GOLDA411a 1/19/94 17:35:38
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
 * $Log: builddata.h,v $
 * Revision 1.1.4.1  1993/11/03  23:42:56  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  23:41:38  damon]
 *
 * Revision 1.1.2.2  1993/04/28  17:58:24  damon
 * 	CR 463. New prototypes for buildata.c
 * 	[1993/04/28  17:58:13  damon]
 * 
 * $EndLog$
 */
#ifndef _BUILDDATA_H
#define _BUILDDATA_H
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>

void
bld_conf_read ( char * sandbox_base, char * project, struct rcfile * rc );

char *
build_base_dir (
    char      * keyword,       /* identifying build's record in "build_list" */
    struct      rcfile  * contents,             /* holds contents of rc file */
    BOOLEAN     errors,            /* true if error messages should be given */
    BOOLEAN     defaultx );                        /* use default build_list */
#endif
