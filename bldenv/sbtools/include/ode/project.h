/* @(#)58       1.1  src/bldenv/sbtools/include/ode/project.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:40
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
 * $Log: project.h,v $
 * Revision 1.1.4.1  1993/11/04  19:52:07  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  19:51:37  damon]
 *
 * Revision 1.1.2.4  1993/05/14  16:50:26  damon
 * 	CR 518. Changed prj_read and prj_write to take full sb path
 * 	[1993/05/14  16:49:00  damon]
 * 
 * Revision 1.1.2.3  1993/05/12  19:51:39  damon
 * 	CR 517. Added subprojects
 * 	[1993/05/12  19:48:44  damon]
 * 
 * Revision 1.1.2.2  1993/04/28  19:21:38  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  19:21:23  damon]
 * 
 * $EndLog$
 */

#include <ode/odedefs.h>
#include <ode/public/error.h>

ERR_LOG
prj_write ( const char * sb_path, const char * dir, const char * project );
BOOLEAN
prj_read (

    const char * sb_path,                             /* name of sandbox */
    const char * dir,                           /* name of base directory */
    char     ** project,
    char     ** sub_project );
