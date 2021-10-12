static char sccsid[] = "@(#)93	1.3  src/bos/usr/ccs/lib/libc/setjmper.c, libcproc, bos411, 9428A410j 11/10/93 15:26:46";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: longjmperror
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: longjmperror()		(BSD)
 *
 * FUNCTION: writes a message to file descriptor 2 (nominally stderr)
 *	     that longjmp() failed.
 *
 * NOTES: 
 * 	This routine is called from longjmp() when an error occurs.
 * 	Programs that wish to exit gracefully from this error may
 * 	write their own versions.
 * 	If this routine returns, the program is aborted.
 *
 * RETURN VALUES: 
 *	NONE
 */

	/* message for longjmp error */
#define ERRMSG	"longjmp or siglongjmp function used outside of saved context\n"

#include "libc_msg.h"
nl_catd catd;
#define E_ERRMSG catgets(catd, MS_LIBC, M_SETJMPER, ERRMSG)

/*
 * This routine is called from longjmp() when an error occurs.
 * Programs that wish to exit gracefully from this error may
 * write their own versions.
 * If this routine returns, the program is aborted.
 */
void longjmperror()
{
	extern	int	write();	/* system call to write data */
	char	*errmsg = E_ERRMSG;	/* pointer to message text */

	catd = catopen(MF_LIBC, NL_CAT_LOCALE);

	/* write longjmp error message to file descriptor 2, hoping that
 	   file descriptor 2 is stderr */
	write(2, errmsg, strlen(errmsg));
}
