static char sccsid[] = "@(#)22  1.1  src/bos/usr/ccs/lib/libtli_r/threads.c, libtli, bos411, 9428A410j 12/20/93 18:11:51";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: __xti_tli_init
 *
 *   ORIGINS: 18 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#ifdef _THREAD_SAFE

#include <lib_lock.h>
#include <lib_data.h>

lib_lock_functions_t	__t_lock_funcs;
lib_data_functions_t	__t_data_funcs;
lib_mutex_t		__t_mutex;

void	*__terrno_hdl;
#ifdef	XTI
void	*__tstrerror_hdl;
#endif

/*
 * Function:
 *	__xti_tli_init
 *
 * Parameters:
 *
 * Description:
 *	For thread-safe libxti_r.a/libtli_r.a library just like libc_r.a,
 *	this initialization routine will be run from __thread_init(), which
 *	gets called from crt0_r() much like more general pthread init 
 *	routines are.
 *	
 *	These libraries will be linked in whenever someone wants them and
 *	so to avoid the initialization problem (when these are not linked in)
 *	we need non-shared file, __threads.c, which has declaration of
 *	this routine.
 *
 *	Called from __thread_init() to initialize t_errno handle and
 *	per thread data.
 */


void
__xti_tli_init()
{
	_lib_declare_lock_functions( &__t_lock_funcs);
	_lib_declare_data_functions( &__t_data_funcs);
	lib_data_hdl( __t_data_funcs, &__terrno_hdl);
#ifdef	XTI
	lib_data_hdl( __t_data_funcs, &__tstrerror_hdl);
#endif
	lib_mutex_create( __t_lock_funcs, &__t_mutex);
}
#endif /* _THREAD_SAFE */
