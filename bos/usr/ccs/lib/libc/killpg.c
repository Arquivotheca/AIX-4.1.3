static char sccsid[] = "@(#)56	1.2  src/bos/usr/ccs/lib/libc/killpg.c, libcsys, bos411, 9428A410j 6/16/90 01:33:30";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: killpg 
 *
 * ORIGINS: 26, 27 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include	<errno.h>

/*
 * FUNCTION:
 * killpg() is BSD kill process group.  It is emulated by
 * sys V kill(2) with a negative (but not -1) number.
 *
 * NOTE: 
 * The INCOMPATIBILITY with BSD killpg() is that killpg( 1, SIGXXX )
 * would send SIGXXX to all descendants of init(1).  This emulation will
 * send SIGXXX to ALL processes (except init(1)).
 *
 * RETURN VALUE:
 *	0 if OK
 *	-1 if error
 */

killpg(pgrp, sig)
int 	pgrp;
int 	sig;
{
	extern int errno;

	if (pgrp < 0) {
		errno = ESRCH;
		return(-1);
	}
	return(kill(-pgrp, sig));
}
