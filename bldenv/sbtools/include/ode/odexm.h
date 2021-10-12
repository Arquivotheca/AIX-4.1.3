/* @(#)52       1.1  src/bldenv/sbtools/include/ode/odexm.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:14
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
 * @OSF_FREE_COPYRIGHT@
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
 * $Log: odexm.h,v $
 * Revision 1.1.6.1  1993/11/04  20:17:24  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  20:16:48  damon]
 *
 * Revision 1.1.4.3  1993/05/26  18:07:43  damon
 * 	CR 553. Added port field
 * 	[1993/05/26  17:17:15  damon]
 * 
 * Revision 1.1.4.2  1993/04/28  20:44:39  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:44:09  damon]
 * 
 * Revision 1.1.2.3  1992/12/03  19:14:05  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:15  damon]
 * 
 * Revision 1.1.2.2  1992/08/20  19:38:19  damon
 * 	CR 240. Initial version
 * 	[1992/08/20  19:37:33  damon]
 * 
 * $EndLog$
 */
#ifndef _OXMINIT_H
#define _OXMINIT_H
typedef struct oxminit {
  int monitor;
  const char * host;
  const char * ident;
  const char * relay;
  const char * port;
} OXMINIT;
#endif
