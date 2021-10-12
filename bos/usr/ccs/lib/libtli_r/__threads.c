static char sccsid[] = "@(#)23  1.1  src/bos/usr/ccs/lib/libtli_r/__threads.c, libtli, bos411, 9428A410j 12/20/93 18:11:57";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	This is a non-shared part of libtli_r.a/libxti_r.a.

	We intend to use a binder trick involving uninitialized variables that
	works in a non-shared environment, where if two modules are found that
	declare the same symbol, and one is initialized and the other isn't, 
	the symbols will be overlaid and initialized to the initialized one's 
	value. This will require making one module in libxti_r.a non-shared, 
	and modifying libc/POWER/crt0main.s and libc_r/__threads_init.c.

	Since the uninitialized global symbol in __threads_init() will begin 
	with a zero value unless overlaid if libxti_r.a is linked in, it can 
	be used as a test to access the xti_init function. Whenever this
	library gets linked in the value of _xti_tli_init_routine gets
	initialized and __threads_init() will call __xti_tli_init() routine
	for initialization.

*/

#ifdef _THREAD_SAFE

void __xti_tli_init(void);

void (*_xti_tli_init_routine)(void) = __xti_tli_init;

#endif	/* _THREAD_SAFE */
