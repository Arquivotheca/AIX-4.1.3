/* @(#)84	1.2  src/bos/usr/ccs/lib/libc/push_pop.h, libc, bos411, 9433B411a 8/8/94 17:44:23 */
/*
 *   COMPONENT_NAME: libc
 *
 *   FUNCTIONS: TS_POP_CLNUP
 *		TS_PUSH_CLNUP
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/**********
  TS_PUSH_CLNUP  pushes a cleanup handler for a cancellation point onto the
                 calling thread's cleanup stack.
  TS_POP_CLNUP   pops a cleanup handler from the calling thread's cleanup stack.
**********/	 

#ifndef _PUSH_POP_H_
#define _PUSH_POP_H_

#ifdef _THREAD_SAFE
#include <lib_lock.h>
extern lib_lock_functions_t	_libc_lock_funcs;

#define TS_PUSH_CLNUP(lock)     lib_cleanup_push(_libc_lock_funcs, _rec_mutex_unlock, lock)
#define TS_POP_CLNUP(flg)       lib_cleanup_pop(_libc_lock_funcs, flg)

#else

#define TS_PUSH_CLNUP(lock)
#define TS_POP_CLNUP(flg)

#endif
#endif /* _PUSH_POP_H_ */
