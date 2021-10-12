static char sccsid[] = "@(#)77  1.3  src/bos/usr/ccs/lib/libc/crt0msg.c, libcgen, bos411, 9428A410j 11/10/93 15:24:13";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: start
 *
 * ORIGINS: 3 10 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>             /* for error messages */
#include <mon.h>               /* for _PROF_IS_P and _PROF_IS_PG */
#include <nl_types.h>

#define MSGIDP "mcrt0"
#define MSGIDPG "gcrt0"

#include "libc_msg.h"
#define MSG01 catgets(catd, MS_LIBC, M_CRT0, \
			"%s: Profiling setup call to %s failed. rc: %d\n")

/*
 * NAME: crt0msg
 *
 * FUNCTION:
 *      Routine to output error message for crt0.s
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Called by mcrt0/gcrt0 if error return from monstartup.
 *
 * (NOTES:)
 *
 *      Written in C to properly use system message facilities.
 *
 *
 * (RECOVERY OPERATION:) None
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: void
 */

void crt0msg( int prof,              /* +1 is gcrt0 -1 is mcrt0      */
              int rc                 /* rc for message */
              )
{
	nl_catd catd;

	catd = catopen (MF_LIBC, NL_CAT_LOCALE);
        if ( prof == _PROF_TYPE_IS_P )
                fprintf(stderr, MSG01,MSGIDP,"monstartup", rc ) ;
        if ( prof == _PROF_TYPE_IS_PG )
                fprintf(stderr, MSG01,MSGIDPG,"monstartup", rc ) ;
	catclose (catd);
        return;

} /* end of function start */
