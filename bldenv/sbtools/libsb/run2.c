static char sccsid[] = "@(#)53  1.1  src/bldenv/sbtools/libsb/run2.c, bldprocess, bos412, GOLDA411a 4/29/93 12:23:11";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 *  School of Computer Science 
 *  Carnegie Mellon University 
 *  Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * ODE 2.1.1
 */

/*
 * Lifted from subrs.c in ./ode/bin/bcs
 */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
extern int errno;
#include <varargs.h>
#include <stdio.h>
#include <ode/odedefs.h>
#ifdef INC_TIME
#include <time.h>
#endif
#ifdef NO_UTIMES
/*
 * Note, the S on the end of NO_UTIMES is signifigant!
 */
#include <utime.h>
#endif

extern char *errmsg();
extern char *concat();

typedef void (*func_ptr)();

#ifndef	WTERMSIG
#define	WTERMSIG(x)	(int)(WIFSIGNALED(x) ? (x.w_status & 0xff) : -1)
#endif

#define STATIC static

echov(av)
char **av;
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
yy_runcmdv(cmd, av, withpath, child_init, ap)
char *cmd;
char **av;
int withpath;
func_ptr child_init;
va_list ap;
{
    int child;
    int fd;

    if ( ui_ver_level () >= VDEBUG )
	echov(av);
    if ((child = vfork()) == -1) {
	ui_print ( VWARN, "%s vfork failed", av[0]);
	return(-1);
    }
    if (child == 0) {
	(*child_init)(ap);
	if (withpath)
	    execvp(cmd, av);
	else
	    execv(cmd, av);
	exit(0377);
    }
    return(child);
}

full_runcmdv(va_alist)
va_dcl
{
    int status;
    va_list ap;
    char *cmd;
    char **av;
    int withpath;
    func_ptr child_init;

    va_start(ap);
    cmd = va_arg(ap, char *);
    av = va_arg(ap, char **);
    withpath = va_arg(ap, int);
    child_init = va_arg(ap, func_ptr);
    status = yy_runcmdv(cmd, av, withpath, child_init, ap);
    va_end(ap);
    return(status);
}
