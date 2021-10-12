
/* @(#)45	1.8  src/bos/kernel/db/add_cmd.h, sysdb, bos411, 9428A410j 6/16/90 02:59:57 */
/*                                      */
#ifndef _h_ADD_CMD
#define _h_ADD_CMD
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

/*
  * Definitions to go with addition formatted control block displays
 */
#include "parse.h"

#define VIRT 1
#define Copyin(from,to,size) 	get_put_data((from),VIRT,(to),(size),FALSE)

/*
 * Low-level debugger control block buffer.
 *
 * This is a large area used for moving information into
 * before displaying it.
 *
 * Control block formatting routines should use this instead of
 * creating large automatic data areas to avoid stack overflow.
 */
#define	CBBUFSIZE 24*1024

#ifdef DEF_STORAGE
char	cbbuf[CBBUFSIZE];		/* The area		*/
#else
extern	cbbuf;				/* Ctl block buffer	*/
#endif	/* DEF_STORAGE */

#endif
