static char sccsid[] = "@(#)Perror.c	1.1 11/30/89 14:39:56";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	Perror
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <errno.h>
#include <nl_types.h>

/*
 * NAME: Perror
 *
 * FUNCTION: Print error message as popup
 *
 * NOTES:
 *
 * RETURNS:
 *
 */

extern	int	errno;
extern	char	*sys_errlist[];
extern	int	sys_nerr;

Perror(fdes, err_set, err_msg, s)
nl_catd	fdes;
int err_set;
int err_msg;
char *s;
{
	diag_hmsg(fdes, err_set, err_msg, 
			s, sys_errlist[ (errno <= sys_nerr? errno:0) ]);
	return(0);
}
