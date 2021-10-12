#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)09  1.1  src/bos/usr/ccs/lib/libcurses/trace.c, libcurses, bos411, 9428A410j 9/3/93 15:14:29";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _asciify
 *		traceoff
 *		traceon
 *		
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/trace.c	1.8"		*/



#include "curses_inc.h"

traceon()
{
#ifdef DEBUG
    if (outf == NULL)
    {
	outf = fopen("trace", "a");
	if (outf == NULL)
	{
	    perror("trace");
	    exit(-1);
	}
	fprintf(outf, "trace turned on\n");
    }
#endif /* DEBUG */
    return (OK);
}

traceoff()
{
#ifdef DEBUG
    if (outf != NULL)
    {
	fprintf(outf, "trace turned off\n");
	fclose(outf);
	outf = NULL;
    }
#endif /* DEBUG */
    return (OK);
}

#ifdef DEBUG
#include <ctype.h>

char *
_asciify(str)
register char *str;
{
    static	char	string[1024];
    register	char	*p1 = string;
    register	char	*p2;
    register	char	c;

    while (c = *str++)
    {
	p2 = unctrl(c);
	while (*p1 = *p2++)
	    p1++;
    }
    return string;
}
#endif /* DEBUG */
