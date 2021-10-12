static char sccsid[] = "@(#)36	1.7  src/bos/usr/bin/mh/sbr/advertise.c, cmdmh, bos411, 9428A410j 3/27/91 17:43:57";
/* 
 * COMPONENT_NAME: CMDMH advertise.c
 * 
 * FUNCTIONS: MSGSTR, advertise 
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
/* static char sccsid[] = "advertise.c	7.1 87/10/13 17:02:56"; */

/* advertise.c - the heart of adios */

#include "mh.h"
#include <stdio.h>
#ifdef	BSD42
#include <sys/types.h>
#include <sys/uio.h>
#endif	BSD42

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/* For 4.2BSD systems, use writev() for slightly better performance.  Why?
   Well, there are a couple of reasons.  Primarily, it gives a smoother
   output...  More importantly though, it's a sexy syscall()...
 */

extern int  errno;
extern int  sys_nerr;

/*  */

/* VARARGS3 */

void advertise (what, tail, fmt, a, b, c, d, e, f)
char   *what,
       *tail,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    int	    eindex = errno;
#ifdef	BSD42
    char    buffer[BUFSIZ],
            err[BUFSIZ];
    struct iovec    iob[20];
    register struct iovec  *iov = iob;
#endif	BSD42

    (void) fflush (stdout);

#ifndef	BSD42
    fprintf (stderr, "%s: ", invo_name);
    fprintf (stderr, fmt, a, b, c, d, e, f);
    if (what) {
	if (eindex > 0 && eindex < sys_nerr)
	    fprintf (stderr, ": %s", strerror(eindex));
	else
	    fprintf (stderr, MSGSTR(ERROR, "Error %d"), eindex); /*MSG*/
    }
    if (tail)
	fprintf (stderr, ", %s", tail);
    (void) fputc ('\n', stderr);
#else	BSD42
    (void) fflush (stderr);

    iov -> iov_len = strlen (iov -> iov_base = invo_name);
    iov++;
    iov -> iov_len = strlen (iov -> iov_base = ": ");
    iov++;
    
    (void) sprintf (buffer, fmt, a, b, c, d, e, f);
    iov -> iov_len = strlen (iov -> iov_base = buffer);
    iov++;
    if (what) {
	if (*what) {
/*
**          The string in question (what) will already be in the fmt used
**          in the printf above.  Hence, we don't need what is below now.
**
**	    iov -> iov_len = strlen (iov -> iov_base = " ");
**	    iov++;
**	    iov -> iov_len = strlen (iov -> iov_base = what);
**	    iov++;
*/
	    iov -> iov_len = strlen (iov -> iov_base = ": ");
	    iov++;
	}
	if (eindex > 0 && eindex < sys_nerr)
	    iov -> iov_len = strlen (iov -> iov_base = strerror(eindex));
	else {
	    (void) sprintf (err, MSGSTR(ERROR, "Error %d"), eindex); /*MSG*/
	    iov -> iov_len = strlen (iov -> iov_base = err);
	}
	iov++;
    }
    if (tail && *tail) {
	iov -> iov_len = strlen (iov -> iov_base = ", ");
	iov++;
	iov -> iov_len = strlen (iov -> iov_base = tail);
	iov++;
    }
    iov -> iov_len = strlen (iov -> iov_base = "\n");
    iov++;
    (void) writev (fileno (stderr), iob, iov - iob);
#endif	BSD42
}
