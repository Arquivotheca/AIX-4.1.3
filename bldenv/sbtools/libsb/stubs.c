static char sccsid[] = "@(#)65  1.1  src/bldenv/sbtools/libsb/stubs.c, bldprocess, bos412, GOLDA411a 4/29/93 12:25:07";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 * ODE 2.1.1
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

print_usage ()

	/* This is a dummy routine for uquit.  User should make
	   real one in his/her own program. */

{
}                                                             /* print usage */
