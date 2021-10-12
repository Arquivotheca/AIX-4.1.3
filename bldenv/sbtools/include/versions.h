/* @(#)26 1.1 src/bldenv/sbtools/include/versions.h, bldprocess, bos412, GOLDA411a 93/04/29 12:19:15 */
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

#  define  BUILD_VERSION	"ODE 2.1.1"
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
