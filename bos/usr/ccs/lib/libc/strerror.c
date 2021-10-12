static char sccsid[] = "@(#)71	1.11.1.5  src/bos/usr/ccs/lib/libc/strerror.c, libcstr, bos411, 9428A410j 11/12/93 09:35:34";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strerror
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * FUNCTION: Maps the error number in errnum to an error message string.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS:
 *	     int errnum - error number to be associated with error message
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the error message
 *	     string associated with the error number.
 */
/*LINTLIBRARY*/
#include "libc_msg.h"
#include <limits.h>

#ifdef _THREAD_SAFE
#include <sys/errno.h>
#endif /* _THREAD_SAFE */

extern int sys_nerr;
extern char *sys_errlist[];

char *__strerror();

/*
 * ANSI and XPG say the behaviour is as if no library function calls
 * the strerror function because the user has access to the internal
 * array strerrbuf after the call to strerror(), and calling perror
 * would modify strerrbuf.  ANSI says the user can't modify the string
 * and that no library function will modify it either by calling strerror.
 * This is a work around so that perror() can use strerror too without
 * modifying the array to which the user has access.
 */
#ifdef _THREAD_SAFE
int
strerror_r(int errnum, char *buf, int buflen)
#else /* _THREAD_SAFE */
char	*
strerror(int errnum)
#endif _THREAD_SAFE
{
#ifdef _THREAD_SAFE
	char strerrbuf[NL_TEXTMAX];

	if ((buf == NULL) || (buflen <= 1)) {
		errno = EINVAL;
		return -1;
	}

	(void) __strerror(errnum, strerrbuf);
	(void) strncpy(buf, strerrbuf, buflen);
	buf[buflen-1]='\0';
	return 0;
#else /* _THREAD_SAFE */
	static char strerrbuf[NL_TEXTMAX];
	return (__strerror(errnum, strerrbuf));
#endif /* _THREAD_SAFE */
}

/* NOTE:  assuming that buf is a valid pointer to an array of characters */
char *
__strerror(int errnum, char *buf)
{
	nl_catd catd = 0;

	catd = catopen(MF_LIBC, NL_CAT_LOCALE);

	if((errnum >= 0) && (errnum < sys_nerr))
		strcpy(buf,catgets(catd, MS_LIBC, errnum, sys_errlist[errnum]));
	else
		(void) sprintf(buf, catgets(catd, MS_LIBC, M_PERROR, 
				       "Error %d occurred."), errnum);

	catclose(catd);
	return (buf);
}
