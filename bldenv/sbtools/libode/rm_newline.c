/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: rm_newline
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
 * $Log: rm_newline.c,v $
 * Revision 1.1.6.4  1993/04/28  14:36:05  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:48  damon]
 *
 * Revision 1.1.6.3  1993/04/09  17:16:04  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:14  damon]
 * 
 * Revision 1.1.6.2  1993/04/08  21:45:28  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  21:45:14  damon]
 * 
 * Revision 1.1.2.4  1992/12/03  17:22:15  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:57  damon]
 * 
 * Revision 1.1.2.3  1992/09/24  19:02:28  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:28  gm]
 * 
 * Revision 1.1.2.2  1992/02/18  22:04:36  damon
 * 	Initial Version. Needed for LBE removal
 * 	[1992/02/18  22:02:47  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)15  1.1  src/bldenv/sbtools/libode/rm_newline.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:17";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include <ode/odedefs.h>
#include <ode/util.h>

void
rm_newline ( char * string )

        /*
         * Removes the last new line in a string.
         */

{
    char        * scratch;                  /* points to location of newline */

  if (( scratch = ( strchr ( string, NEWLINE ))) != NULL )
    *scratch = NUL;
}
