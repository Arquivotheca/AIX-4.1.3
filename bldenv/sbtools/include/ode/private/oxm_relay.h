/* @(#)57       1.1  src/bldenv/sbtools/include/ode/private/oxm_relay.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:37
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
 * Copyright (c) 1993, 1992, 1991, 1990 Open Software Foundation, Inc.
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
 * 
 */
/*
 * HISTORY
 * $Log: oxm_relay.h,v $
 * Revision 1.1.2.1  1993/11/09  20:08:22  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  20:08:01  damon]
 *
 * $EndLog$
 */
#ifndef _PRIVATE_OXM_RELAY_H
#define _PRIVATE_OXM_RELAY_H
#include <ode/odedefs.h>
int odexm_begin ( char *, char *, int );
int
odexm ( int , int , int , char **, BOOLEAN );
void
odexm_end (void);
#endif
