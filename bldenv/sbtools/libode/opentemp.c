/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: opentemp
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
 * COPYRIGHT NOTICE
 * Copyright (c) 1993, 1992, 1991, 1990
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
 * $Log: opentemp.c,v $
 * Revision 1.1.7.1  1993/09/16  17:36:32  damon
 * 	Fixed COPYRIGHT NOTICE section
 * 	[1993/09/16  17:36:00  damon]
 *
 * Revision 1.1.5.1  1993/08/19  18:35:24  damon
 * 	CR 622. Added typecast
 * 	[1993/08/19  18:30:23  damon]
 * 
 * Revision 1.1.2.5  1993/05/05  18:51:20  marty
 * 	Defind first argument to opentemp() as
 * 	const.
 * 	[1993/05/05  18:51:10  marty]
 * 
 * Revision 1.1.2.4  1993/04/27  21:08:42  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  21:08:32  damon]
 * 
 * Revision 1.1.2.3  1993/03/31  19:06:20  damon
 * 	CR 443. opentemp now just creates directory, no file
 * 	[1993/03/31  19:05:45  damon]
 * 
 * Revision 1.1.2.2  1993/03/15  17:46:22  damon
 * 	CR 443. Added opentemp.o
 * 	[1993/03/15  17:44:27  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)97  1.1  src/bldenv/sbtools/libode/opentemp.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:38";
#endif /* not lint */

#include <fcntl.h>
#include <unistd.h>
#include <ode/util.h>
#include <sys/param.h>
#include <sys/stat.h>

int
opentemp( const char *dir, char *tempdir )
{
    int pid;
    char *p;
    const char *q;

    pid = getpid();

    p = tempdir;
    q = dir;
    do {
        *p++ = *q;
    } while (*q++ != '\0');
    p--;
    while (*--p == 'X') {
        *p = (pid % 10) + '0';
        pid /= 10;
    }
    p++;
    for (;;) {
        if (mkdir(tempdir, 0700) >= 0)
            break;
        if (mkdir(tempdir, 0700) >= 0)
            break;
        if (*p == 'z')
            return(-1);
        if (*p == '\0')
            return(-1);
        if (*p < 'a' && *p > 'z')
            *p = 'a';
        else
            (*p)++;
    }
  return ( 0 );
}
