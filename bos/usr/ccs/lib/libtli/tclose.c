static char sccsid[] = "@(#)02  1.3  src/bos/usr/ccs/lib/libtli/tclose.c, libtli, bos411, 9428A410j 3/8/94 19:12:10";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_close
 *
 *   ORIGINS: 18 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/** Copyright (c) 1989  Mentat Inc.
 ** tclose.c 1.2, last change 1/29/90
 **/

#include "common.h"

int
t_close (fd)
	int	fd;
{
	int	retval;

	if (!iostate_lookup(fd, IOSTATE_FREE))
		return  -1;
	retval = close(fd);
	if (retval == -1)
		t_unix_to_tli_error();

#ifdef XTIDBG
        tr_close (fd, retval);
#endif
	return retval;
}
