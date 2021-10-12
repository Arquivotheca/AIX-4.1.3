static char sccsid[] = "@(#)57	1.2  src/bos/usr/ccs/bin/structure/1.finish.c, cmdprog, bos411, 9428A410j 6/15/90 22:54:09";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: addum, fingraph
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
#include "def.h"
#include "1.incl.h"

fingraph()
	{
	/* if any entry statements, add a DUMVX with arcs to all entry statements */
	if (ENTLST)
		{
		ARC(START,0) = addum(ARC(START,0),ENTLST);
		freelst(ENTLST);
		}
	/* if any FMTVX, add a DUMVX with arcs to all FMTVX's */
	if (FMTLST)
		{
		ARC(START,0) = addum(ARC(START,0),FMTLST);
		freelst(FMTLST);
		}
	}

addum(v,lst)
VERT v;
struct list *lst;
	{
	VERT new;
	int count,i;
	struct list *ls;
	count = lslen(lst);		/* length of lst */
	new = create(DUMVX,1+count);
	ARC(new,0) = v;
	for (i = count, ls = lst; i >= 1; --i, ls = ls->nxtlist)
		{
		ASSERT(ls,addum);
		ARC(new,i) = ls->elt;
		}
	ASSERT(!ls, addum);
	return(new);
	}
