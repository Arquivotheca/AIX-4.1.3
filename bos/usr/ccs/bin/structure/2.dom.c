static char sccsid[] = "@(#)71	1.2  src/bos/usr/ccs/bin/structure/2.dom.c, cmdprog, bos411, 9428A410j 6/15/90 22:55:00";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: comdom, getdom
 *
 * ORIGINS: 26; 27
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
#
/*
set dom[v] to immediate dominator of v, based on arcs as stored in inarcs
(i.e. pretending the graph is reducible if it isn't).
Algorithm is from Hecht and Ullman, Analysis of a simple algorithm for global
flow analysis problems, except bit vector operations replaced by search
through DOM to save quadratic blowup in space 
*/
#include "def.h"
#include "2.def.h"


getdom(inarc,dom)
struct list **inarc;
VERT *dom;
	{
	VERT v;
	int i;
	struct list *ls;
	for (v = 0; v < nodenum; ++v)
		dom[v] = UNDEFINED;
	for (i = 1; i < accessnum; ++i)
		{
		v = after[i];
		for (ls = inarc[v]; ls; ls = ls->nxtlist)
			{
			ASSERT(ntoaft[ls->elt] < i,getdom);
			dom[v] = comdom(dom[v],ls->elt,dom);
			}

		}
	}


comdom(u,v,dom)			/* find closest common dominator of u,v */
VERT u,v, *dom;
	{
	if (u == UNDEFINED) return(v);
	if (v == UNDEFINED) return(u);
	while(u != v)
		{
		ASSERT(u != UNDEFINED && v != UNDEFINED, comdom);
		if (ntoaft[u] < ntoaft[v])	
			v = dom[v];
		else
			u = dom[u];
		}
	return(u);
	}
