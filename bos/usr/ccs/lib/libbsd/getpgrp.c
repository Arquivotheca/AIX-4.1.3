static char sccsid[] = "@(#)09  1.1  src/bos/usr/ccs/lib/libbsd/getpgrp.c, libbsd, bos411, 9428A410j 4/23/93 13:36:53";
/*
 * COMPONENT_NAME: (LIBBSD)
 *
 * FUNCTIONS: getpgrp
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:  getpgrp()
 *
 * FUNCTION: Get process group.
 *
 * RETURNS: Returns -1 if unsuccessful.
 */

#include <sys/types.h>
extern pid_t kgetpgrp(); /* undocumented kernel routine */

pid_t
getpgrp(pid_t pid)
{
	return( kgetpgrp(pid) );
}
