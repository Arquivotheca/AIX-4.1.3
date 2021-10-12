static char sccsid[] = "@(#)81	1.4  src/bos/usr/ccs/bin/structure/3.main.c, cmdprog, bos411, 9428A410j 3/9/94 13:21:13";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: structure
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

structure()
	{
	VERT v, *head;

	if (progress)
		fprintf(stderr,MSGSTR(GTRCH, "	getreach:\n")); /*MSG*/
	getreach();
	if (routerr) return;
	if (progress)
		fprintf(stderr,MSGSTR(GTFLW, "	getflow:\n")); /*MSG*/
	getflow();
	if (progress)
		fprintf(stderr,MSGSTR(GTTHN, "	getthen:\n")); /*MSG*/
	getthen(START);
	head = (int *) challoc(nodenum * sizeof(*head));
	for (v = 0; v < nodenum; ++v)
		head[v] = UNDEFINED;
	for (v = START; DEFINED(v); v = RSIB(v))
		fixhd(v,UNDEFINED,head);
			/* fixhd must be called before getloop so that
				it gets applied to IFVX which becomes NXT(w) for UNTVX w */
	if (progress)
		fprintf(stderr,MSGSTR(GTLP, "	getloop:\n")); /*MSG*/
	getloop();
	if (progress)
		fprintf(stderr,MSGSTR(GTBRNCH, "	getbranch:\n")); /*MSG*/
	getbranch(head);
	chfree(head,nodenum * sizeof(*head));
	head = 0;
	}
