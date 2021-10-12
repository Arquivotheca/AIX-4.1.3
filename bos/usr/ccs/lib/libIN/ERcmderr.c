static char sccsid[] = "@(#)42	1.7  src/bos/usr/ccs/lib/libIN/ERcmderr.c, libIN, bos411, 9428A410j 6/10/91 10:15:39";
/*
 * LIBIN: ERcmderr, ERvcmderr
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
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
 *     Report program error to stderr, including error message from kernel.
 *     These routines are called as follows:
 *    
 *          ERcmderr(error_num, fmt, args...);
 *    
 *              or
 *    
 *          ERvcmderr(error_num, vargs);
 *    
 *     They write, on stderr, a message of the following form:
 *    
 *          <progname>: <formatted-message>  <kernel-message>.
 *    
 *     where <progname> is the name of the program, obtained by examining the
 *     global variable "char *progname"; <formatted-message> is the message
 *     described by the "fmt" and "args" ("vargs") arguments, which are treated
 *     exactly the way printf (vprintf) treats them; and <kernel-message> is
 *     the string corresponding to the error contained in the global variable
 *     "errno".
 *    
 *     error_num controls the printing of kernel errors.  If it is positive,
 *     it is assumed to be a kernel error number, and the string it corresponds
 *     to is obtained from the error table and appended to the user's formatted
 *     string as described above.  If it is -1, the current value of errno is
 *     used to obtain the string.  If it is 0, no string is appended.
 * 
 * RETURN VALUE DESCRIPTION: 
 */

#include <stdio.h>
#include <stdarg.h>

extern char *progname;

/* VARARGS */
ERcmderr(int error, char *fmt, ...)
{
	va_list         ap;

	va_start(ap, fmt);
	ERvcmderr(error, fmt, ap);
	va_end(ap);
}

ERvcmderr(error, fmt, ap)
	char    *fmt;
	va_list ap;
{
	extern int errno;
	extern char *ERsysmsg();

	if (error == -1)
		error = errno;  /* Save it before stdio smashes it */
	fflush(stdout);
	if (progname != NULL)
	    fprintf(stderr, "%s: ", progname);

	vfprintf(stderr, fmt, ap);
	if (error != 0)
		fprintf(stderr, "  %s.", ERsysmsg(error));
	putc('\n', stderr);
	fflush(stderr);
}
