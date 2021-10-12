static char sccsid[] = "@(#)67	1.1  src/bos/usr/ccs/lib/libbsd/setpgrp.c, libbsd, bos411, 9428A410j 9/27/89 16:49:59";
/*
 * COMPONENT_NAME: (LIBBSD) descriptive name
 *
 * FUNCTIONS: setpgrp
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/proc.h>

/*
 * NAME:  setpgrp()
 *
 * FUNCTION: Set process group.
 *
 * RETURNS: Returns -1 if unsuccessful.
 */

setpgrp(pid_t pid, pid_t pgrp)
{
	return( setpgid( pid, pgrp ) );
}
