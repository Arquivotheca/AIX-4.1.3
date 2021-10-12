/* @(#)09	1.5  src/bos/kernel/db/dbtrb.h, sysdb, bos411, 9428A410j 7/22/93 09:54:49 */
#ifndef _h_DBTRB
#define _h_DBTRB

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

char	CONT_PROMPT[]  = "\nPlease press \"ENTER\" to continue:";
char	BAD_OPT_MSG[]  =	"\nInvalid option.\n";
char	SELECT_PROMPT[]  =
		"\nPlease enter an option number, or \"x\" to quit.\n";
#ifndef _THREADS
char	PID_PROMPT[]   =	"\nEnter a process id or \"x\" to quit.\n";
char	BAD_PID_MSG[]  =	"\nInvalid process id: 0x%x\n";
#else /* _THREADS */
char	TID_PROMPT[]   =	"\nEnter a thread id or \"x\" to quit.\n";
char	BAD_TID_MSG[]  =	"\nInvalid thread id: 0x%x\n";
#endif
char	NO_ACTIVE[]    =	"\nTHERE ARE NO TRB's ON THE ACTIVE LIST.\n";
char	NO_FREE[]      =	"\nTHERE ARE NO TRB's ON THE FREE LIST.\n";

#endif  /* _h_DBTRB */
