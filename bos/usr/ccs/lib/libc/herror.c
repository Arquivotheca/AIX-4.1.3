static char sccsid[] = "@(#)56	1.9  src/bos/usr/ccs/lib/libc/herror.c, libcnet, bos411, 9428A410j 3/14/94 16:30:26";
/*
 *   COMPONENT_NAME: LIBC
 *
 *   FUNCTIONS: _Get_h_errno, _Set_h_errno, _h_errno, herror
 *
 *   ORIGINS: 26,27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* herror.c,v $ $Revision: 2.5.2.3 $ (OSF) */

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <nl_types.h>
#include "libc_msg.h"
#ifdef	_THREAD_SAFE
#include <lib_data.h>
#endif	/* _THREAD_SAFE */
#include <netdb.h>
static nl_catd catd;

char	*h_errlist[] = {
	"Error 0",
	"Unknown host",				/* 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/* 2 TRY_AGAIN */
	"Unknown server error",			/* 3 NO_RECOVERY */
	"No address associated with name",	/* 4 NO_ADDRESS */
};
const int	h_nerr = { sizeof(h_errlist)/sizeof(h_errlist[0]) };

/*
 * herror --
 *	print the error indicated by the h_errno value.
 */
int
herror(s)
	char *s;
{
	struct iovec iov[4];
	register struct iovec *v = iov;
	int _Get_h_errno();

	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
	if (s && *s) {
		v->iov_base = s;
		v->iov_len = strlen(s);
		v++;
		v->iov_base = ": ";
		v->iov_len = 2;
		v++;
	}

	switch (_Get_h_errno()) {
		case 0:
			v->iov_base = catgets(catd,LIBCNET,NET13, h_errlist[0]);
			break;
		case 1:
			v->iov_base = catgets(catd,LIBCNET,NET14, h_errlist[1]);
			break;
		case 2:
			v->iov_base = catgets(catd,LIBCNET,NET15, h_errlist[2]);
			break;
		case 3:
			v->iov_base = catgets(catd,LIBCNET,NET16, h_errlist[3]);
			break;
		case 4:
			v->iov_base = catgets(catd,LIBCNET,NET17, h_errlist[4]);
			break;
		default:
			v->iov_base =
			catgets(catd,LIBCNET,NET12, "Unknown error");
			break;
	}
	v->iov_base = _Get_h_errno() < h_nerr ? h_errlist[_Get_h_errno()] : 
			"Unknown error";
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = "\n";
	v->iov_len = 1;
	writev(fileno(stderr), iov, (v - iov) + 1);
	catclose(catd);
	return (0);
}

/*
 * Following are the functions for setting/getting h_errno.
 */

#ifdef  h_errno
#undef  h_errno
#endif  /* h_errno */

int	h_errno;

#ifdef	_THREAD_SAFE

extern lib_data_functions_t	_libc_data_funcs;
extern void			*_h_errno_hdl;

#define Get_Error_Ref	((int *)lib_data_ref(_libc_data_funcs, _h_errno_hdl))

/*
 * Function:
 *	_h_errno
 *
 * Return value:
 *	address of the per-thread h_errno if available or else
 *	address of the global h_errno
 *
 * Description:
 *	Allow access to a per-thread h_errno. Returning the address enables
 *	existing l/r-value rules to set/get the correct value. Default to
 *	the global h_errno in case per-thread data is not available.
 */
int *
_h_errno()
{
	int	*err;

	return ((err = Get_Error_Ref) ? err : &h_errno);
}
#endif  /* _THREAD_SAFE */

/*
 * Function:
 *	_Get_h_errno
 *
 * Return value:
 *	value of the per-thread h_errno if available or else
 *	value of the global h_errno
 *
 * Description:
 *	For libraries only. Retrieve the value of h_errno from the per-thread
 *	or glabal variable.
 */
int
_Get_h_errno()
{
#ifdef  _THREAD_SAFE
	int	*err;

	return ((err = Get_Error_Ref) ? *err : h_errno);
#else
	return (h_errno);
#endif  /* _THREAD_SAFE */
}

/*
 * Function:
 *	_Set_h_errno
 *
 * Parameters:
 *	h_error	- value to set h_errno to
 *
 * Return value:
 *	new value of h_errno
 *
 * Description:
 *	For libraries only. Set both the per-thread and global h_errnos.
 *	_THREAD_SAFE case helps code which still uses the global h_errno.
 */
int
_Set_h_errno(int error)
{
#ifdef  _THREAD_SAFE
	int	*err;

	if ((err = Get_Error_Ref))
		*err = error;
#endif  /* _THREAD_SAFE */
	h_errno = error;
	return (error);
}
