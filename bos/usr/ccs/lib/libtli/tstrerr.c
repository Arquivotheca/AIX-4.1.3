static char sccsid[] = "@(#)81  1.3  src/bos/usr/ccs/lib/libtli/tstrerr.c, libtli, bos411, 9434A411a 8/19/94 14:07:26";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: _tstrerror, t_strerror, Get_tstrerror_Ref
 *
 *   ORIGINS: 18 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
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
 * OSF/1 Release 1.2
 */

#include "common.h"
#include <stdio.h>
#include <nl_types.h>
#include <locale.h>
#include "libtli_msg.h"

static	char	*strerror;

#ifndef	_THREAD_SAFE

#define	_tstrerror()	(&strerror)

#else	/* _THREAD_SAFE */

extern	lib_data_functions_t	__t_data_funcs;
extern	void			*__tstrerror_hdl;

#define	Get_tstrerror_Ref	((char **)lib_data_ref(__t_data_funcs, __tstrerror_hdl))

/*
 * Function:
 *      _tstrerror
 *
 * Return value:
 *      address of the per-thread strerror if available or else
 *      address of the global strerror
 *
 * Description:
 *      Allow access to a per-thread strerror. Returning the address enables
 *      existing l/r-value rules to get the correct value. Default to
 *      the global strerror in case per-thread data is not available.
 */
char **
_tstrerror()
{
	char    **err;

	return ((err = Get_tstrerror_Ref) ? err : &strerror);
}
#endif  /* _THREAD_SAFE */

char *
t_strerror (errnum)
	int	errnum;
{
	char	*c, **strptr;
	extern	int	t_nerr;
	extern	char	*t_errlist[];
	nl_catd	catd;

	setlocale (LC_ALL,"");    /* Designates native locale */
	catd = catopen(MF_LIBTLI, NL_CAT_LOCALE);

	strptr = _tstrerror();
	if (*strptr)
		free(*strptr);

	if (errnum >= 0  &&  errnum < t_nerr) {
		int len;

		c = catgets(catd, MS_LIBTLI, errnum, t_errlist[errnum]);

		len = strlen(c) + 1;
		*strptr = (char *)malloc(len);
		memcpy(*strptr, c, len);
	} else {
		c = catgets(catd,MS_LIBTLI,TERROR_STRERR, "<%d>: error unknown");
		*strptr = (char *)malloc(strlen(c) + 64);
		sprintf(*strptr, c, errnum);
	}

	catclose(catd);
	return *strptr;
}

