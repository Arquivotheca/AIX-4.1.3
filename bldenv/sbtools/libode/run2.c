/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: WTERMSIG
 *		echov
 *		full_runcmdv
 *		yy_runcmdv
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: run2.c,v $
 * Revision 1.2.10.2  1993/11/08  20:18:19  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:36  damon]
 *
 * Revision 1.2.10.1  1993/11/03  20:40:45  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:16  damon]
 * 
 * Revision 1.2.8.1  1993/08/19  18:26:33  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  18:25:49  damon]
 * 
 * Revision 1.2.4.7  1993/05/05  14:42:41  marty
 * 	Add include file sys/socket.h for hp700_hpux.
 * 	[1993/05/05  14:42:31  marty]
 * 
 * Revision 1.2.4.6  1993/04/27  22:12:56  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  22:12:43  damon]
 * 
 * Revision 1.2.4.5  1993/04/12  17:24:12  damon
 * 	CR 446 Cleaned up include files
 * 	[1993/04/12  17:24:03  damon]
 * 
 * Revision 1.2.4.4  1993/04/08  20:25:59  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  20:22:32  damon]
 * 
 * Revision 1.2.4.3  1993/01/21  18:37:04  damon
 * 	CR 401. Added stdarg
 * 	[1993/01/21  18:34:11  damon]
 * 
 * Revision 1.2.4.2  1993/01/13  17:38:34  damon
 * 	CR 382. Removed NO_UTIMES
 * 	[1993/01/05  21:14:10  damon]
 * 
 * Revision 1.2.2.7  1992/12/17  20:01:49  marty
 * 	Remove external definition of concat().
 * 	[1992/12/17  20:01:32  marty]
 * 
 * Revision 1.2.2.6  1992/12/03  17:22:20  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:00  damon]
 * 
 * Revision 1.2.2.5  1992/12/02  20:26:38  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:32  damon]
 * 
 * Revision 1.2.2.4  1992/09/24  19:02:34  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:40  gm]
 * 
 * Revision 1.2.2.3  1992/06/22  21:26:32  damon
 * 	CR 181. Changed vfork to fork
 * 	[1992/06/22  21:24:51  damon]
 * 
 * 	Modified so that error messages make it back to the
 * 	terminal before exit.
 * 
 * 	DCE OT defect 2341
 * 	[1992/04/13  16:50:08  mhickey]
 * 
 * Revision 1.2.2.2  1992/06/15  19:23:38  damon
 * 	Synched with DCE changes
 * 	[1992/06/15  19:20:55  damon]
 * 
 * Revision 1.2  1991/12/05  21:05:44  devrcs
 * 	Removed dependence on bcs.h
 * 	[91/08/01  13:54:13  damon]
 * 
 * 	First version using library version of SCAPI
 * 	[91/08/01  07:32:56  damon]
 * 
 * 	First version using library version of SCAPI
 * 	[91/07/31  20:58:11  damon]
 * 
 * $EndLog$
 */

/*
 * Lifted from subrs.c in ./ode/bin/bcs
 */

#ifndef lint
static char sccsid[] = "@(#)17  1.1  src/bldenv/sbtools/libode/run2.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:21";
#endif /* not lint */

#include <unistd.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/run.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdio.h>
#ifdef INC_TIME
#include <time.h>
#endif

typedef void (*func_ptr)(va_list);

#ifndef	WTERMSIG
#define	WTERMSIG(x)	(int)(WIFSIGNALED(x) ? (x.w_status & 0xff) : -1)
#endif

void
echov( char **av )
{
    int i;

    for (i = 0; av[i] != NULL; i++) {
        if (i > 0)
            (void) putc(' ', stderr);
        (void) fputs(av[i], stderr);
    }
    (void) putc('\n', stderr);
}

STATIC
int
yy_runcmdv( const char *cmd, char **av, int withpath, func_ptr child_init,
            va_list ap )
{
    int child;

    if ( ui_ver_level () >= VDEBUG )
	echov(av);
        fflush(stdout);
        fflush(stderr);
    if ((child = fork()) == -1) {
	ui_print ( VWARN, "%s fork failed", av[0]);
	return(-1);
    }
    if (child == 0) {
	(*child_init)(ap);
	if (withpath)
	    execvp(cmd, av);
	else
	    execv(cmd, av);
	perror(cmd);
        _exit(0377);
    }
    return(child);
}

int
#ifdef __STDC__
full_runcmdv(const char * cmd, ... )
#else
full_runcmdv(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
    char *cmd;
#endif
    int status;
    va_list ap;
    char **av;
    int withpath;
    func_ptr child_init;

#ifdef __STDC__
    va_start(ap, cmd);
#else
    va_start(ap );
    cmd = va_arg(ap, char *);
#endif
    av = va_arg(ap, char **);
    withpath = va_arg(ap, int);
    child_init = va_arg(ap, func_ptr);
    status = yy_runcmdv(cmd, av, withpath, child_init, ap);
    va_end(ap);
    return(status);
}
