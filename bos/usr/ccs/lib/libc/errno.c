static char sccsid[] = "@(#)11	1.2  src/bos/usr/ccs/lib/libc/errno.c, libc, bos411, 9438C411c 9/23/94 19:35:12";
/*
 *   COMPONENT_NAME: LIBC
 *
 *   FUNCTIONS: _Errno
 *		_Geterrno
 *		_Seterrno
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifdef	errno
#undef	errno
#endif	/* errno */
extern int	errno;

#ifdef	_THREAD_SAFE
#include	<lib_data.h>

extern lib_data_functions_t	_libc_data_funcs;
extern void			*_errno_hdl;

#define	Get_Error_Ref	((int *)lib_data_ref(_libc_data_funcs, _errno_hdl))

#endif /* _THREAD_SAFE */

/*
 * Function:
 *	_Errno
 *
 * Return value:
 *  Thread Safe
 *	address of the per-thread errno if available or else
 *	address of the global errno
 *  Regular
 *	returns the address of the global errno
 *
 * Description:
 *	Allow access to a per-thread errno. Returning the address enables
 *	existing l/r-value rules to set/get the correct value. Default to
 *	the global errno in case per-thread data is not available.
 */
int *
_Errno()
{
#ifdef _THREAD_SAFE
	int	*err;
	return ((err = Get_Error_Ref) ? err : &errno);
#else /* _THREAD_SAFE */
	return &errno;
#endif /* _THREAD_SAFE */
}

/*
 * Function:
 *	_Geterrno
 *
 * Return value:
 *	value of the per-thread errno if available or else
 *	value of the global errno
 *
 * Description:
 *	For libraries only. Retrieve the value of errno from the per-thread
 *	or glabal variable.
 */
int
_Geterrno()
{
#ifdef	_THREAD_SAFE
	int	*err;

	return ((err = Get_Error_Ref) ? *err : errno);
#else	/* _THREAD_SAFE */
	return (errno);
#endif	/* _THREAD_SAFE */
}

/*
 * Function:
 *	_Seterrno
 *
 * Parameters:
 *	error	- value to set errno to
 *
 * Return value:
 *	new value of errno
 *
 * Description:
 *	For libraries only. Set both the per-thread and global errnos.
 *	_THREAD_SAFE case helps code which still uses the global errno.
 */
int
_Seterrno(int error)
{
#ifdef	_THREAD_SAFE
	int	*err;

	if ((err = Get_Error_Ref))
		*err = error;
#endif	/* _THREAD_SAFE */
	errno = error;
	return (error);
}
