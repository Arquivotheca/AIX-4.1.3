/* @(#)90  1.2  src/bos/kernext/x25/jsmalloc.h, sysxx25, bos411, 9428A410j 1/10/94 16:52:21 */
#ifndef _H_JSMALLOC
#define _H_JSMALLOC
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM corp.
 */

#ifdef QLLC
#include <x25/jsdefs.h>
#else
#include "jsdefs.h"
#endif
			       /* for generic_ptr_t, temp fix until new      */
                               /* compiler available for void * declaration  */
                               /* extension to ANSI C                        */
                               /* jsdefs.h also provides void_ptr_t, which   */
                               /* is only defined when AIX2_2 flag is defined*/

#ifdef _NO_PROTO
/*****************************************************************************/
/* void_ptr_t is typedeffed in jsdefs.h to be char *, but only when AIX2_2   */
/* is defined. Reason for this is that AIX2.2 compiler can't handle void *   */
/* properly, so you can't build scaffold code.                               */
/*****************************************************************************/
void_ptr_t xmalloc();
int xmfree();
extern generic_ptr_t  kernel_heap;
extern generic_ptr_t  pinned_heap;
#else
#include <sys/malloc.h>
#endif

#endif

