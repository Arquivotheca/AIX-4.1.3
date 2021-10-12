/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: abspath
 *		defined
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
 * $Log: abspath.c,v $
 * Revision 1.7.5.2  1993/04/27  20:43:15  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:43:05  damon]
 *
 * Revision 1.7.5.1  1993/04/27  20:43:14  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.7.2.5  1992/12/03  17:20:24  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:07:52  damon]
 * 
 * Revision 1.7.2.4  1992/12/02  20:25:42  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:06  damon]
 * 
 * Revision 1.7.2.3  1992/11/12  18:27:48  damon
 * 	CR 329. Changed PATH_MAX to MAXPATHLEN
 * 	[1992/11/12  18:09:53  damon]
 * 
 * Revision 1.7.2.2  1992/09/24  19:01:16  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:21:18  gm]
 * 
 * Revision 1.7  1991/12/05  21:04:21  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:10:47  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:22:59  dwm]
 * 
 * Revision 1.5  90/10/07  20:02:45  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:08:11  gm]
 * 
 * Revision 1.4  90/08/09  14:22:43  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:07  gm]
 * 
 * Revision 1.3  90/06/29  14:38:06  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:20:25  gm]
 * 
 * Revision 1.2  90/01/02  19:26:28  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:12:40  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Adapted for 4.2 BSD UNIX.  Changed to new getwd() routine.
 * 	[85/04/30            sas]
 * 
 * 	Redid the rest of the routine so that now it has been completely
 * 	retouched, although still conserving most of the original design.
 * 	Increased curwd to 1024 characters to match what getwd can
 * 	produce.  Per suggestions by Steve Shafer, added the ability to
 * 	flush the remembered current working directory (which you should
 * 	do after calling chdir) and improved the initial construction of
 * 	the result so that the current working directory is not obtained
 * 	if the given pathname is already an absolute pathname.
 * 	[82/11/15            tlr]
 * 
 * 	Redid compaction of the absolute path name so that leading steps
 * 	of ".." are preserved.  Also fixed so that the trailing slash
 * 	is not deleted if it is also the initial slash.
 * 	[82/11/14            tlr]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)71  1.1  src/bldenv/sbtools/libode/abspath.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:41";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: abspath.c,v $ $Revision: 1.7.5.2 $ (OSF) $Date: 1993/04/27 20:43:15 $";
#endif
/*
 * abspath -- determine absolute pathname
 *
 * Originally written sometime around 1980 by James Gosling.
 *
 *----------------------------------------------------------------------
 *
 *     abspath (name,result)
 *     char *name;
 *     char *result;
 *
 * Abspath places the absolute pathname of the string name into
 * the string result.
 *
 * Abspath takes a pathname and converts it to an absolute pathname by
 * prepending the name of the current working directory if necessary.
 * Then the absolute pathname is compacted by removing and resolving
 * superfluous steps.
 *
 * Steps of "" (two adjacent slashes) and steps of "." are removed
 * because they have no effect on the meaning of the pathname.
 *
 * Steps of ".." are resolved by removing them together with the
 * preceeding step.  However, resolution is not possible if the
 * preceeding step is also ".."
 *
 * Abspath calls getcwd to obtain the name of the current working
 * directory when needed.  To improve performance, the result from
 * getcwd is saved so that getcwd need not be invoked again during
 * subsequent calls on abspath.  If you change the current working
 * directory (via chdir) you must call abspath(0,0) which causes
 * abspath to flush its saved result from getcwd.  If you do not do
 * this abspath will continue to use its saved result from getcwd
 * and this will most likely cause it to produce erronious results.
 *
 * Abspath returns 0 on success and -1 on failure.  The only failure
 * that can happen is a failure of getcwd.  See the documentation on
 * getcwd.  Failures in getcwd are pretty catastrophic.
 */

#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <ode/util.h>
#include <sys/param.h>

#define TRUE   1
#define FALSE  0

static char havecurwd = FALSE;
static char curwd [MAXPATHLEN];    /* remember the current working directory */

int
abspath ( char * name, char * result )
{
    register char * src;    /* source pointer for copy operations */
    register char * dst;    /* destination pointer for copy operations */
    register char * fence;  /* pointer to slash that cannot be backed over */
    register char * t;      /* scanback pointer in dst when we hit a slash */



    fence = NULL;
    if (name == 0  ||  result == 0)
    {
	havecurwd = FALSE;  /* flush knowledge of current working directory */
	return (0);
    }


    /*
     * Construct the initial result pathname, which is basically just
     * a concatenation of the current working directory (if the name
     * is a relative pathname) and the name.  If we need to know the
     * current working directory but don't have it saved away, we call
     * getcwd to figure it out for us.
     */

    dst = result;

    if (name[0] != '/')
    {
	if (!havecurwd  &&  getcwd(curwd, sizeof(curwd)) == 0)  return (-1);
	havecurwd = TRUE;

	src = curwd;
	while ((*dst++ = *src++) != 0)  ;    /* copy curwd to result */
	dst[-1] = '/';                       /* tack on a trailing slash */
    }

    src = name;
    while ((*dst++ = *src++) != 0)  ;   /* copy name to result */
    dst[-1] = '/';                      /* tack on a trailing slash */
    *dst = 0;                           /* make it null-terminated */



    /*
     * Now scan through result and compact the pathname.
     *
     *   "//"      =>  "/"
     *   "/./"     =>  "/"
     *   "/x/../"  =>  "/"
     *
     * where x is a string without a slash.  Note that x
     * cannot be "", ".", or ".."
     *
     * There is guaranteed to be a trailing slash on result when
     * we start, so that we don't need any special cases to handle
     * trailing steps--all steps in the pathname end with a slash.
     *
     * The fence points to the most recent slash that ".." cannot
     * back over.  Basically, all steps to the left of the fence
     * are ".."  Initially the fence points to the first slash.  We
     * are paranoid so we scan for the first slash.  Any characters
     * coming before the first slash (which must be the result of
     * getcwd) are assumed to be magical incantations and we leave
     * them alone.  This is never expected to happen, but who knows?
     */

    src = result;
    dst = result;

    while (*src)
    {
	if ((*dst++ = *src++) == '/')
	{
	    fence = dst-1;  /* set fence to first slash */
	    break;
	}
    }

    while (*src)
    {
	if ((*dst++ = *src++) == '/')
	{
	    t = dst-1;      /* address of slash */


	    switch (*--t)
	    {
	    case '/':           /* found "//" */
		dst = t+1;      /* take off "/" */
		break;

	    case '.': 
		switch (*--t)
		{
		case '/':           /* found "/./" */
		    dst = t+1;      /* take off "./" */
		    break;

		case '.': 
		    if (*--t == '/')
		    {                   /* found "/../" */
			if (t == fence)
			{                   /* it is a leading ".." */
			    fence = dst-1;  /* move fence over it */
			}
			else
			{
			    while (*--t != '/')  ;
			    dst = t+1;      /* take off "x/../" */
			}
		    }
		    break;
		}
		break;
	    }
	}
    }

    *dst = 0;      /* null-terminate the result */


    /*
     * Now get rid of a trailing slash provided it is not also an
     * initial slash.
     *
     * Note that we tacked on a trailing slash originally and the
     * compaction shouldn't affect it so it should still be there,
     * but we check anyway because we're paranoid.
     */

    if (--dst > result  &&  *dst == '/')  *dst = 0;


    return (0);
}
