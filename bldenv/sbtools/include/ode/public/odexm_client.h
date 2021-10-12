/* @(#)59       1.1  src/bldenv/sbtools/include/ode/public/odexm_client.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:43
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
 * $Log: odexm_client.h,v $
 * Revision 1.1.6.2  1993/11/05  20:34:12  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  20:33:29  damon]
 *
 * Revision 1.1.6.1  1993/11/04  20:17:27  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  20:16:49  damon]
 * 
 * Revision 1.1.4.7  1993/05/18  18:45:12  damon
 * 	CR 515. Added proto for oxm_write
 * 	[1993/05/18  18:42:18  damon]
 * 
 * Revision 1.1.4.6  1993/05/05  14:51:52  damon
 * 	CR 485. oxm_read now returns int
 * 	[1993/05/05  14:51:47  damon]
 * 
 * Revision 1.1.4.5  1993/05/05  14:28:30  damon
 * 	CR 485. Added oxm_read
 * 	[1993/05/05  14:28:25  damon]
 * 
 * Revision 1.1.4.4  1993/05/04  19:45:12  damon
 * 	CR 486. Added size parameter to oxm_gets
 * 	[1993/05/04  19:44:55  damon]
 * 
 * Revision 1.1.4.3  1993/04/28  18:06:19  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  18:06:06  damon]
 * 
 * Revision 1.1.4.2  1993/04/27  15:25:00  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:24:45  damon]
 * 
 * Revision 1.1.2.4  1992/12/03  19:14:17  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:24  damon]
 * 
 * Revision 1.1.2.3  1992/09/28  19:42:19  damon
 * 	CR 240. Added predeclarations
 * 	[1992/09/28  19:38:52  damon]
 * 
 * Revision 1.1.2.2  1992/08/20  19:38:25  damon
 * 	CR 240. Initial version
 * 	[1992/08/20  19:37:46  damon]
 * 
 * $EndLog$
 */

#ifndef _ODEXM_CLIENT_H_

#define _ODEXM_CLIENT_H_

#include <ode/odexm.h>
#include <ode/public/error.h>

typedef void * OXM_CNXTN;

ERR_LOG
oxm_init ( int initct, OXMINIT * init );
ERR_LOG oxm_open ( OXM_CNXTN *, int );
ERR_LOG oxm_runcmd ( OXM_CNXTN, int, const char **, const char * );
char *
oxm_gets ( OXM_CNXTN, char *, int, ERR_LOG * );
int
oxm_read ( OXM_CNXTN, char *, int, ERR_LOG * );
int
oxm_write ( OXM_CNXTN, const char *, int, ERR_LOG * );
ERR_LOG oxm_get_all ( OXM_CNXTN, int );
ERR_LOG oxm_endcmd ( OXM_CNXTN, int * );
ERR_LOG oxm_close ( OXM_CNXTN );
BOOLEAN
oxm_poll ( OXM_CNXTN oxm_cnxtn );
int
oxm_stdout ( OXM_CNXTN oxm_cnxtn );
#endif
