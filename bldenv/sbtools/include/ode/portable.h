/* @(#)54       1.1  src/bldenv/sbtools/include/ode/portable.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:25
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: streq
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
 * $Log: portable.h,v $
 * Revision 1.2.6.6  1993/04/28  18:25:52  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  18:11:51  damon]
 *
 * Revision 1.2.6.5  1993/04/27  15:34:28  damon
 * 	CR 463. Added misc.h
 * 	[1993/04/27  15:34:18  damon]
 * 
 * Revision 1.2.6.4  1993/04/09  14:57:54  damon
 * 	CR 446. Removed S_ISDIR
 * 	[1993/04/09  14:56:30  damon]
 * 
 * Revision 1.2.6.3  1993/01/25  19:03:44  damon
 * 	CR 382. Moved more defines to here
 * 	[1993/01/25  19:03:33  damon]
 * 
 * Revision 1.2.6.2  1993/01/13  17:45:43  damon
 * 	CR 382. Moved defines from other code
 * 	[1993/01/13  17:44:26  damon]
 * 
 * Revision 1.2.4.11  1992/12/21  21:37:43  marty
 * 	Workaround for referring to "stat" without actually
 * 	including "usr/include/sys/stat.h".
 * 	[1992/12/21  21:37:30  marty]
 * 
 * Revision 1.2.4.10  1992/12/21  16:04:00  hester
 * 	Remove prototyping when NO_ANSI_CC is set.
 * 	[1992/12/21  16:03:49  hester]
 * 
 * Revision 1.2.4.9  1992/12/16  19:41:19  gm
 * 	CR366: convert to using full prototypes.
 * 	[1992/12/16  19:40:59  gm]
 * 
 * Revision 1.2.4.8  1992/12/03  19:14:10  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:18  damon]
 * 
 * Revision 1.2.4.7  1992/11/13  15:20:21  root
 * 	Changed NEED_LSTAT_DECL to NO_LSTAT_DECL
 * 	Added getwd decl for systems without
 * 	[1992/11/13  15:03:14  root]
 * 
 * Revision 1.2.4.6  1992/11/11  21:16:09  damon
 * 	CR 329. Added getopt vars, lstat decl.
 * 	[1992/11/11  21:15:32  damon]
 * 
 * Revision 1.2.4.5  1992/11/11  15:50:21  damon
 * 	CR 329. Removed PROTO stuff. Moved func decs to portable.h
 * 	[1992/11/11  15:49:54  damon]
 * 
 * Revision 1.2.4.4  1992/09/15  19:47:04  damon
 * 	CR 266. Removed define for salloc
 * 	[1992/09/15  19:46:39  damon]
 * 
 * Revision 1.2.4.3  1992/06/16  21:53:19  damon
 * 	2.1.1 touch-up
 * 	[1992/06/16  21:51:25  damon]
 * 
 * Revision 1.2.2.2  1992/04/02  21:01:06  damon
 * 	Added LOCK_EX
 * 	[1992/04/02  20:44:25  damon]
 * 
 * 	Added NEED_LOCK_DEFS
 * 	[1992/04/02  20:38:24  damon]
 * 
 * Revision 1.2  1991/12/05  21:03:59  devrcs
 * 	ODE portability declarations
 * 	(e.g. use libsb salloc if libc strdup doesn't exist)
 * 	[91/07/25  14:56:41  ezf]
 * 
 * $EndLog$
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

#ifdef NO_DIRENT
#define dirent direct
#endif

/******************************************************************************

		         DEFINES

******************************************************************************/

#ifdef NEED_LOCK_DEFS
#define LOCK_NB O_NONBLOCK
#define LOCK_SH F_RDLCK
#define LOCK_EX F_WRLCK
#endif

/******************************************************************************

                         VARIABLES

******************************************************************************/

#ifndef _GETOPT_H_
extern char *optarg;
extern int optind;
#endif

/******************************************************************************

                         RETURN VALUES OF FUNCTIONS

******************************************************************************/

#ifdef NO_STRDUP
#ifdef NO_ANSI_CC
  char * strdup ();
#else
  char * strdup (const char *);
#endif
#endif

#ifdef NO_LSTAT_DECL
#ifdef NO_ANSI_CC
  int lstat();
#else
  struct stat;
  int lstat(const char *, struct stat *);
#endif
#endif

/******************************************************************************

                         INCLUDES

******************************************************************************/
#ifdef INC_DIRENT
#  include <dirent.h>
#endif
