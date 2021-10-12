/* @(#)66	1.8  src/bos/kernel/db/POWER/pr_proc.h, sysdb, bos411, 9428A410j 6/5/91 09:41:29 */

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_PR_PROC
#define _h_PR_PROC

/*
  * Proc display definitions; goes with proc.c
 */

#define Get_from_memory(a1,a2,a3,a4) 	get_put_data((a1),(a2),(a3),(a4),FALSE)
#define VIRT 1


/****************** DEBUG Stuff ******************/
/* #define Debug */			/* to include debug code */

/* Values for DBG_LVL */
#define DBGGENERAL	1

#ifdef Debug
extern int jr_debug();
extern int DBG_LVL;
#endif /* Debug */

#endif /* h_PR_PROC */
