static char sccsid[] = "@(#)15  1.4  src/bos/usr/ccs/lib/libtli/topen.c, libtli, bos411, 9428A410j 3/8/94 19:12:53";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_open
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
 ** topen.c 1.1, last change 8/9/89
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <fcntl.h>

int
t_open (name, oflag, tinfo)
	char	* name;
	int	oflag;
	struct t_info	* tinfo;
{
	int			fd;
        int    			code;

        code = -1;
#ifdef XTI
	if (!oflag || oflag & ~(O_RDWR | O_NONBLOCK)) {
#else
	if (!oflag || oflag & ~(O_RDWR | O_NONBLOCK | O_NDELAY)) {
#endif
		t_errno = TBADFLAG;
		goto rtn;
	}
	if ((fd = open(name, oflag)) != -1) {
		if ( ioctl(fd, I_PUSH, "timod") != -1 ) {
#ifdef XTI
			if (tli_ioctl(fd, TI_XTI_HELLO, nilp(char), 0) == -1) {
				t_close(fd);
				goto rtn;
			}
#endif
			if (tinfo  &&  t_getinfo(fd, tinfo) == -1) {
				t_close(fd);
				goto rtn;
			}
			(void)t_sync(fd);
			code = fd;
			goto rtn;
		}
		close(fd);
	}
	t_errno = (errno == ENOENT) ? TBADNAME : TSYSERR;

rtn:
#ifdef XTIDBG
        tr_open (name, oflag, tinfo, code);
#endif
	return code;
}

