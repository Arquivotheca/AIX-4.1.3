/* @(#)69       1.1  src/bldenv/sbtools/include/ode/versions.h, bldprocess, bos412, GOLDA411a 1/19/94 17:37:20
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
 * $Log: versions.h,v $
 * Revision 1.2.14.2  1993/11/16  16:52:42  damon
 * 	submitted to cover 2.3.4 goof
 * 	[1993/11/16  16:52:35  damon]
 *
 * Revision 1.2.14.1  1993/10/21  17:58:27  damon
 * 	Updated version to 2.3.3
 * 	[1993/10/21  17:58:21  damon]
 * 
 * Revision 1.2.12.1  1993/08/31  21:27:14  damon
 * 	CR 635. Updated to version 2.3.2
 * 	[1993/08/31  21:27:07  damon]
 * 
 * Revision 1.2.10.1  1993/08/11  16:03:59  damon
 * 	CR 620. Updated version to 2.3.1
 * 	[1993/08/11  16:03:48  damon]
 * 
 * Revision 1.2.8.2  1993/01/13  20:45:49  damon
 * 	CR 393. Updated to version 2.3
 * 	[1993/01/13  20:44:29  damon]
 * 
 * Revision 1.2.6.4  1992/12/03  17:20:17  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:07:47  damon]
 * 
 * Revision 1.2.6.3  1992/07/14  15:41:31  damon
 * 	CR 221. Changed BUILD_VERSION to 2.2
 * 	[1992/07/14  15:40:53  damon]
 * 
 * Revision 1.2.6.2  1992/06/15  17:58:34  damon
 * 	Taken from 2.1.1
 * 	[1992/06/15  16:25:40  damon]
 * 
 * Revision 1.2.2.2  1992/03/25  22:47:45  damon
 * 	Added BUILD_VERSION
 * 	[1992/03/25  21:51:00  damon]
 * 
 * Revision 1.2  1991/12/05  21:04:06  devrcs
 * 	updated to match new revision numbers
 * 	[91/07/23  16:21:17  ezf]
 * 
 * 	created header file for ODE revision numbers
 * 	[91/07/19  14:56:51  ezf]
 * 
 * $EndLog$
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                                July 1991                                  **
*******************************************************************************
**
**  Description:
**	This header file contains version information for ODE tools.
**
**/

/* revision information */
typedef struct
{
    char *vprog;	/* program name */
    char *vermin;	/* minimum compatible revision level */
    char *vermax;	/* maximum compatible revision level */
} revinfo;


/*                       DEFINES					     */

#  define  BUILD_VERSION	"ODE 2.3.3"
#  define  MINVER_BCI		"1.12.1.8"
#  define  MAXVER_BCI		MINVER_BCI
#  define  MINVER_BCO		"1.13.1.4"
#  define  MAXVER_BCO		MINVER_BCO
#  define  MINVER_BCS		"1.11.1.4"
#  define  MAXVER_BCS		MINVER_BCS
#  define  MINVER_BDIFF		"1.8.1.3"
#  define  MAXVER_BDIFF		MINVER_BDIFF
#  define  MINVER_BLOG		"1.7.5.4"
#  define  MAXVER_BLOG		MINVER_BLOG
#  define  MINVER_BMERGE	"1.9.5.6"
#  define  MAXVER_BMERGE	MINVER_BMERGE
#  define  MINVER_BSTAT		"1.7.5.5"
#  define  MAXVER_BSTAT		MINVER_BSTAT
#  define  MINVER_BSUBMIT	"1.11.5.2"
#  define  MAXVER_BSUBMIT	"1.11.6.2"
#  define  MINVER_LOGSUBMIT	"1.1.2.11"
#  define  MAXVER_LOGSUBMIT	MINVER_LOGSUBMIT
#  define  MINVER_RCSACL	"1.11.2.3"
#  define  MAXVER_RCSACL	"1.11.2.5"
#  define  MINVER_RCSAUTH	"1.7.2.1"
#  define  MAXVER_RCSAUTH	"1.7.2.4"
#  define  MINVER_SRCACL	"1.4.2.3"
#  define  MAXVER_SRCACL	"1.4.2.5"
