/* @(#)90	1.3  src/bos/usr/bin/diff/diff.h, cmdfiles, bos411, 9428A410j 8/13/91 15:32:48 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "diff_msg.h"
#define MSGSTR(Num,Str) NLcatgets(catd,MS_DIFF,Num,Str)

#define	DI_NORMAL	0	/* Normal output */
#define	DI_EDIT		-1	/* Editor script out */
#define	DI_REVERSE	1	/* Reverse editor script */
#define	DI_CONTEXT	2	/* Diff with context */
#define	DI_IFDEF	3	/* Diff with merged #ifdef's */
#define	DI_NREVERSE	4	/* Reverse ed script with numbered */

