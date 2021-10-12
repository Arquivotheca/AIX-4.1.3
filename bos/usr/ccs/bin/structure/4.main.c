static char sccsid[] = "@(#)87	1.4  src/bos/usr/ccs/bin/structure/4.main.c, cmdprog, bos411, 9428A410j 3/9/94 13:21:36";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: output
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include "def.h"
#include "4.def.h"

LOGICAL *brace;
output()
	{
	VERT w;
	int i;
	brace = (LOGICAL *) challoc(nodenum * sizeof(*brace));
	for (i = 0; i < nodenum; ++i)
		brace[i] = FALSE;
	if (progress) fprintf(stderr,MSGSTR(NDBRACE, "ndbrace:\n")); /*MSG*/
	for (w = START; DEFINED(w); w = RSIB(w))
		ndbrace(w);
	if (progress) fprintf(stderr,MSGSTR(OUTRAT, "outrat:\n")); /*MSG*/
	for (w = START; DEFINED(w); w = RSIB(w))
		outrat(w,0,YESTAB);
	OUTSTR("END\n");
	chfree(brace,nodenum * sizeof(*brace));
	brace = 0;
	}
