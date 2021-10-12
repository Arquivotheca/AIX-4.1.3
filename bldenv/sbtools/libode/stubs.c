/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: print_usage
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
 * $Log: stubs.c,v $
 * Revision 1.2.5.2  1993/04/28  14:36:02  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:47  damon]
 *
 * Revision 1.2.5.1  1993/04/28  14:36:01  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.2.2.3  1992/12/03  17:22:58  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:26  damon]
 * 
 * Revision 1.2.2.2  1992/06/15  18:10:58  damon
 * 	Synched with 2.1.1
 * 	[1992/06/15  18:04:50  damon]
 * 
 * Revision 1.2.5.2  1992/06/15  16:34:54  damon
 * 	Taken from 2.1.1
 * 
 * Revision 1.2.2.2  1992/03/24  21:18:34  damon
 * 	print_revision is now a full routine in interface.c
 * 	[1992/03/24  21:17:50  damon]
 * 
 * Revision 1.2  1991/12/05  21:13:22  devrcs
 * 	This routine replaces print_usage.c.  It contains stubs for
 * 	print_usage and print_revision.
 * 	[91/01/08  12:22:48  randyb]
 * 
 * $EndLog$
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                		Randy Barbano 				     **
**                              December 1990                                **
*******************************************************************************
**
**    Description:
**	These are stub functions for library libsb.a.  If the user does
**	not provide real functions, these will be used and will do nothing.
**
**    lib functions and their usage:
**	1) print_usage ()
**	2) print_rev ()
**
*/

#ifndef lint
static char sccsid[] = "@(#)28  1.1  src/bldenv/sbtools/libode/stubs.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:45";
#endif /* not lint */

#include <ode/util.h>

void
print_usage ( void )

	/* This is a dummy routine for uquit.  User should make
	   real one in his/her own program. */

{
}                                                             /* print usage */
