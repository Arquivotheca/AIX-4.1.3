static char sccsid[] = "@(#)53	1.8  src/bos/usr/ccs/lib/libIN/ERsysmsg.c, libIN, bos411, 9428A410j 11/10/93 15:13:03";
/*
 * LIBIN: ERsysmsg
 *
 * ORIGIN: 9,10
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1993
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 *     Return system error message string, given errno value.
 *     If argument is -1, errno itself is used, thus avoiding the need
 *     to declare it in most programs.
 *
 *     The returned value, in some cases, is a pointer to a static area
 *     that may be overwritten by later calls.
 *
 * RETURN VALUE DESCRIPTION: 
 */
#include <limits.h>
#include <nl_types.h>
#include "libc_msg.h"
#include "libIN_msg.h"

char *ERsysmsg(error)
    register int error;
{
    extern char *sys_errlist[];
    extern int sys_nerr;
    extern int errno;
    nl_catd catd;	/* message catalog file descriptor */

    static char badnum[NL_TEXTMAX];

    if (error == -1)
	error = errno;
    if (error < 0 || error >= sys_nerr) {
	catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
	sprintf(badnum, catgets(catd, MS_LIBIN, M_ERSYSMSG1, "Error %d"),error);
	catclose(catd);
	return(badnum);
    } else if (error == 0) {
	catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
	sprintf(badnum, catgets(catd, MS_LIBIN, M_ERSYSMSG2,
		"No system error"));
	catclose(catd);
	return(badnum);
	}
    else {
	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
	sprintf(badnum, catgets(catd, MS_LIBC, error, sys_errlist[error]));
	catclose(catd);
	return(badnum);
	}
}
