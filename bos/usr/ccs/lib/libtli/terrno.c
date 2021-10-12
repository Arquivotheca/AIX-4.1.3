static char sccsid[] = "@(#)02  1.2  src/bos/usr/ccs/lib/libtli/terrno.c, libtli, bos411, 9428A410j 12/20/93 17:16:12";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: _terrno, _Get_terrno, _Set_terrno, Get_terrno_Ref
 *
 *   ORIGINS: 18 27 63
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

#include "common.h"

#ifdef	t_errno
#undef	t_errno
#endif	/* t_errno */
int	t_errno;

#ifdef	_THREAD_SAFE

extern lib_data_functions_t	__t_data_funcs;
extern void			*__terrno_hdl;

#define	Get_terrno_Ref	((int *)lib_data_ref(__t_data_funcs, __terrno_hdl))

/*
 * Function:
 *	_terrno
 *
 * Return value:
 *	address of the per-thread t_errno if available or else
 *	address of the global t_errno
 *
 * Description:
 *	Allow access to a per-thread t_errno. Returning the address enables
 *	existing l/r-value rules to set/get the correct value. Default to
 *	the global errno in case per-thread data is not available.
 */
int *
_terrno(void)
{
	int	*err;

	return ((err = Get_terrno_Ref) ? err : &t_errno);
}
#endif	/* _THREAD_SAFE */

/*
 * Function:
 *	_Get_terrno
 *
 * Return value:
 *	value of the per-thread terrno if available or else
 *	value of the global errno
 *
 * Description:
 *	For libraries only. Retrieve the value of terrno from the per-thread
 *	or glabal variable.
 */
int
_Get_terrno(void)
{
#ifdef	_THREAD_SAFE
	int	*err;

	return ((err = Get_terrno_Ref) ? *err : t_errno);
#else	/* _THREAD_SAFE */
	return (t_errno);
#endif	/* _THREAD_SAFE */
}

/*
 * Function:
 *	_Set_terrno
 *
 * Parameters:
 *	error	- value to set terrno to
 *
 * Return value:
 *	new value of t_errno
 *
 * Description:
 *	For libraries only. Set both the per-thread and global t_errnos.
 *	_THREAD_SAFE case helps code which still uses the global t_errno.
 */
int
_Set_terrno(int error)
{
#ifdef	_THREAD_SAFE
	int	*err;

	if ((err = Get_terrno_Ref))
		*err = error;
#endif	/* _THREAD_SAFE */
	t_errno = error;
	return (error);
}

