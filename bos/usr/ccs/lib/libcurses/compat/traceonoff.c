static char sccsid[] = "@(#)38  1.5  src/bos/usr/ccs/lib/libcurses/compat/traceonoff.c, libcurses, bos411, 9428A410j 6/16/90 01:53:58";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   traceon, traceoff
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

/*
 * NAME:        traceon
 */

traceon()
{
#ifdef DEBUG
	if (outf == NULL) {
		outf = fopen("trace", "a");
		if (outf == NULL) {
			perror("trace");
			exit(-1);
		}
		fprintf(outf, "trace turned on\n");
	}
#endif
}

/*
 * NAME:        traceoff
 */

traceoff()
{
#ifdef DEBUG
	if (outf != NULL) {
		fprintf(outf, "trace turned off\n");
		fclose(outf);
		outf = NULL;
	}
#endif
}
