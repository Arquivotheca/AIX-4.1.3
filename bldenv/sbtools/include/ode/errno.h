/* @(#)46       1.1  src/bldenv/sbtools/include/ode/errno.h, bldprocess, bos412, GOLDA411a 1/19/94 17:35:47
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
 * $Log: errno.h,v $
 * Revision 1.1.4.3  1993/03/16  21:49:18  damon
 * 	CR 436. Added MERGEREQ
 * 	[1993/03/16  21:47:56  damon]
 *
 * Revision 1.1.4.2  1993/01/25  21:28:09  damon
 * 	CR 396. Converted history.c to err_log
 * 	[1993/01/25  21:26:44  damon]
 * 
 * Revision 1.1.2.4  1992/12/03  19:13:56  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:10  damon]
 * 
 * Revision 1.1.2.3  1992/08/07  19:39:08  damon
 * 	CR 236. First full version
 * 	[1992/08/07  19:37:06  damon]
 * 
 * Revision 1.1.2.2  1992/08/07  19:04:08  damon
 * 	CR 236. Initial Version
 * 	[1992/08/07  19:03:07  damon]
 * 
 * $EndLog$
 */

#define RECOVERABLE 1
#define FATAL 2

#define OE_OK	NULL

#define OE_SYSERROR	-1	/* System error, error is in errno */
#define OE_INTERNAL	1	/* Internal error, no further details */
#define OE_READ		2	/* File read failed */
#define OE_WRITE	3	/* File write failed */
#define OE_RENAME	4	/* File rename failed */
#define OE_ALLOC	5	/* Memory alloc failed */
#define OE_EXEC		6	/* exec failed */
#define OE_MKDIR	7	/* mkdir failed */
#define OE_SIGNAL	8	/* Killed by signal */
#define OE_STAT		9	/* stat failed */
#define OE_LSTAT	10	/* lstat failed */
#define OE_CHDIR	11	/* chdir failed */
#define OE_FORK		12	/* fork failed */
#define OE_GETWD	13	/* getwd failed */
#define OE_CHMOD	14	/* chmod failed */
#define OE_CLOSE	15	/* close failed */
#define OE_PIPEOPEN	16	/* Pipe open failed */
#define OE_PIPEREAD	17	/* Read of pipe failed */
#define OE_WAIT		18	/* Wait call failed */
#define OE_SETENV	19	/* setenv call failed */

#define OE_NOTDIR	20	/* Not a directory */
#define OE_NOTEXEC	21	/* File not executable */

#define OE_ENVVAR	22	/* Environment variable not set */

#define	OE_REVISION	23	/* Revision does not exist */
#define OE_ANCESTOR	24	/* No common ancestor */

#define OE_FILECOPY	25	/* filecopy call failed */

#define OE_LOG		26	/* No Log marker */
#define OE_ENDLOG	27	/* Missing ENDLOG */
#define OE_HISTORY	28	/* No HISTORY marker */
#define OE_LEADERLINE	29	/* Improper leader line */
#define OE_MARKERFILE	30	/* Could not open marker file */
#define OE_BADMARKER	31	/* Invalid marker */
#define OE_EMPTYMARKER	32	/* Empty distribution marker in marker file */
#define OE_EMPTYLOG	33	/* Empty log message */

#define OE_RCVARDEF	34	/* Variable not defined */
#define OE_PARSERC	35	/* Syntax error in resource file */

#define OE_OUTOFSB	36	/* Not in sandbox */
#define OE_NOMARKERS	37	/* No valid markers */
#define OE_INCMARKER	38	/* Bad include syntax */
#define OE_OPEN		39	/* Could not open file */
#define OE_BADHISTORY	40	/* Bad history log */
#define OE_BADREV	41	/* Bad revision format */
#define OE_NOLEADER	42	/* No comment leader */
#define OE_HASIBR	43	/* File contains IBR marker */
#define OE_CMTINLOG	44	/* Comment terminator in log */
#define OE_EOF		45	/* Unexpected EOF */
#define OE_MERGEREQ	46	/* Error reported was invalid */
#define OE_INVALID	47	/* Error reported was invalid */
