static char sccsid[] = "@(#)74	1.4  src/bos/usr/ccs/bin/structure/2.main.c, cmdprog, bos411, 9428A410j 3/9/94 13:20:57";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: build
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
#include "2.def.h"

VERT *after;
int *ntobef, *ntoaft;
build()
	{
	VERT v, *dom, *head;
	int type;
	struct list **inarc;
	dfs(START);
	if (routerr) return;
	for (v = 0; v < nodenum; ++v)
		{
		type = NTYPE(v);
		if (type == LOOPVX || type == DOVX)
			FATH(ARC(v,0)) = v;
		}

	head = (int *) challoc(sizeof(*head) * nodenum);
	if (progress) fprintf(stderr,MSGSTR(GETHEAD, "	gethead:\n")); /*MSG*/
	gethead(head);	/* sets head[v] to ITERVX heading smallest loop containing v or UNDEFINED */

	if (routerr) return;
	inarc = (struct list **) challoc(nodenum * sizeof(*inarc));
	if (progress) fprintf(stderr,MSGSTR(GETINARC, "	getinarc:\n")); /*MSG*/
	getinarc(inarc,head);		/* sets inarc[v] to list of forward arcs entering v */

	dom = (int *) challoc(nodenum * sizeof(*dom));
	if (progress) fprintf(stderr,MSGSTR(GETDOM, "	getdom:\n")); /*MSG*/
	getdom(inarc,dom);	/* sets dom[v] to immediate dominator of v or UNDEFINED */
	if (routerr) return;
	if (progress) fprintf(stderr,MSGSTR(GETTREE, "	gettree:\n")); /*MSG*/
	gettree(inarc, dom, head);
	if (routerr) return;

	chfree(head, nodenum * sizeof(*head)); head = 0;
	chfree(dom,nodenum * sizeof(*dom)); dom = 0;
	for (v = 0; v < nodenum; ++v)
		{
		freelst(inarc[v]);
		inarc[v] = 0;
		}
	chfree(inarc,sizeof(*inarc) * nodenum); inarc = 0;
	chfree(ntoaft,sizeof(*ntoaft) * nodenum); ntoaft = 0;
	chfree(ntobef,sizeof(*ntobef) * nodenum); ntobef = 0;
	chfree(after, sizeof(*after) * accessnum); after = 0;
	}
