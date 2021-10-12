static char sccsid[] = "@(#)10	1.4  src/bos/kernel/lfs/umask.c, syslfs, bos411, 9428A410j 8/27/93 16:23:16";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: umask, get_umask
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/user.h"

/*
 * mode mask for creation of files
 */
umask(mask)
int	mask;
{
	register t;

	t = U.U_cmask;
	U.U_cmask = mask & 0777;
	return t;
}

/* 
 * NAME: get_umask
 *
 * FUNCTION: This encapsulation routine is supplied to give subsystems
 *	     outside the file system the ability to access the umask.
 *
 * RETURN VALUES: This routine always completes successfully.
 *
 */

int
get_umask(void)
{
	return U.U_cmask;
}
