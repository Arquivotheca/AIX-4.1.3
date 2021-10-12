static char sccsid[] = "@(#)48	1.3  src/bos/usr/bin/mh/uip/discard.c, cmdmh, bos411, 9428A410j 6/15/90 22:17:55";
/* 
 * COMPONENT_NAME: CMDMH discard.c
 * 
 * FUNCTIONS: discard 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* discard.c - discard output on a file pointer */

#include "mh.h"
#include <stdio.h>
#ifndef	SYS5
#include <sgtty.h>
#else	SYS5
#include <sys/types.h>
#include <termio.h>
#include <sys/ioctl.h>
#endif	SYS5


void	discard (io)
FILE   *io;
{
#ifndef	SYS5
    struct sgttyb   sg;
#else	SYS5
    struct termio   sg;
#endif	SYS5

    if (io == NULL)
	return;

#ifndef	SYS5
    if (ioctl (fileno (io), TIOCGETP, (char *) &sg) != NOTOK)
	(void) ioctl (fileno (io), TIOCSETP, (char *) &sg);
#else	SYS5
    if (ioctl (fileno (io), TCGETA, &sg) != NOTOK)
	(void) ioctl (fileno (io), TCSETA, &sg);
#endif	SYS5

    if (io -> _ptr = io -> _base)
	io -> _cnt = 0;
}
