static char sccsid[] = "@(#)20	1.1  src/bos/usr/ccs/lib/libc_r/libc_data.c, libcthrd, bos411, 9428A410j 10/20/93 14:41:28";
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: _libc_declare_data_functions
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* libc_data.c,v $ $Revision: 1.2.2.2 */

/*
 *  This file contains the declarations of all thread data used in libc.
 */

#include	<lib_data.h>

/*
 * set of functions for accessing per-thread data
 */
lib_data_functions_t	_libc_data_funcs;

/*
 * handle for errno & h_errno
 */
void	*_errno_hdl;
void	*_h_errno_hdl;
void	*_alloca_hdl;

/*
 * Function:
 *	_libc_declare_data_functions
 *
 * Parameters:
 *	f	- address of exported data functions
 *
 * Description:
 *	Called from the data functions provider at startup to
 *	allow libc to initialise its set of data functions.
 *	Eg. called from pthread_init()
 */
void
_libc_declare_data_functions(lib_data_functions_t *f)
{
	_libc_data_funcs = *f;
	lib_data_hdl(_libc_data_funcs, &_errno_hdl);
	lib_data_hdl(_libc_data_funcs, &_h_errno_hdl);
	lib_data_hdl(_libc_data_funcs, &_alloca_hdl);
}
