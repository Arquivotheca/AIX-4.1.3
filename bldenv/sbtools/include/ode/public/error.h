/* @(#)61       1.1  src/bldenv/sbtools/include/ode/public/error.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:51
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
 * $Log: error.h,v $
 * Revision 1.1.4.6  1993/04/28  19:25:00  damon
 * 	CR 463. Fixed another typo
 * 	[1993/04/28  19:24:54  damon]
 *
 * Revision 1.1.4.5  1993/04/28  18:32:57  damon
 * 	CR 463. Fixed typos
 * 	[1993/04/28  18:32:47  damon]
 * 
 * Revision 1.1.4.4  1993/04/28  18:21:47  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  18:21:31  damon]
 * 
 * Revision 1.1.4.3  1993/04/28  18:06:22  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  18:06:07  damon]
 * 
 * Revision 1.1.4.2  1993/04/27  15:24:56  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:24:44  damon]
 * 
 * Revision 1.1.2.5  1992/12/03  19:14:16  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:23  damon]
 * 
 * Revision 1.1.2.4  1992/09/28  19:43:14  damon
 * 	CR 236. Added multiple include check
 * 	[1992/09/28  19:38:26  damon]
 * 
 * Revision 1.1.2.3  1992/09/17  13:31:35  damon
 * 	CR 236. Added decl. of err_log
 * 	[1992/09/17  13:31:20  damon]
 * 
 * Revision 1.1.2.2  1992/08/25  21:48:15  damon
 * 	CR 236. Public definition of error type
 * 	[1992/08/25  21:47:14  damon]
 * 
 * $EndLog$
 */
#ifndef _PUBLIC_ERROR_H_
#define _PUBLIC_ERROR_H_
#include <ode/odedefs.h>

typedef void * ERR_LOG;

void
err_append ( ERR_LOG log, ERR_LOG log_append );
void
err_clear ( ERR_LOG log );
int
err_errno ( ERR_LOG log );
ERR_LOG
err_log ( int, ... );
BOOLEAN
err_occurred ( ERR_LOG log, int ode_errno );
int
err_ode_errno ( ERR_LOG log );
ERR_LOG
err_prev ( ERR_LOG log );
void
err_report ( ERR_LOG log );
char *
err_str ( ERR_LOG log );
int
err_total ( ERR_LOG log );
int
err_type ( ERR_LOG log );
#endif
