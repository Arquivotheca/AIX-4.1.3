static char sccsid[] = "@(#)40	1.6  src/bos/usr/ccs/lib/libbsd/flock.c, libbsd, bos411, 9428A410j 6/16/90 01:06:52";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: flock
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

/*
 * NAME: flock
 *                                                                    
 * FUNCTION:  Apply or remove an advisory lock on an open file
 *                                                                   
 * NOTES:  This is a BSD compatibility routine that invokes the fcntl()
 *	   system call.
 *
 *
 * RETURNS:  Returns 0 for a successful operation; on an error -1 is
 *	     returned and errno is set to the appropriate error code.
 */  

flock(fd, operation)
int   fd, operation;
{
	struct flock arg;
	int fval;

	if (operation & LOCK_UN)
		arg.l_type = F_UNLCK;

	else if (operation & LOCK_EX)
		arg.l_type = F_WRLCK;

	else if (operation & LOCK_SH)
		arg.l_type = F_RDLCK;
	
	else	/* following BSD flock functionality: "Just say OK" */
		return(0);

	arg.l_whence = 0;
	arg.l_start = 0;
	arg.l_len = 0;

	if (operation & LOCK_NB) {
			fval = fcntl(fd, F_SETLK, &arg); 
			if (fval == -1 && errno == EAGAIN || errno == EACCES) 
				errno = EWOULDBLOCK;
			return(fval);
			}

	return(fcntl(fd, F_SETLKW, &arg));
}
