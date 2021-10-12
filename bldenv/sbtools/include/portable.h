/* @(#)24 1.1 src/bldenv/sbtools/include/portable.h, bldprocess, bos412, GOLDA411a 93/04/29 12:19:02 */
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
**                                July 1991                                  **
*******************************************************************************
**
**    Description:
**	This header file contains declarations for ODE portability
**	across various environments.
*/

/******************************************************************************

                         MACROS

******************************************************************************/

#  define  streq(A,B)   (strcmp ((A),(B)) == 0)     /* are two strings equal */

#ifdef NO_STRDUP
#    define	strdup(X)	salloc(X)	/* allocate and copy a string */
#endif

/******************************************************************************

		         RETURN VALUES OF FUNCTIONS

******************************************************************************/

extern	char	      * salloc();
#ifdef NEED_LOCK_DEFS
#define LOCK_NB O_NONBLOCK
#define LOCK_SH F_RDLCK
#define LOCK_EX F_WRLCK
#endif
