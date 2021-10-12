static char sccsid[] = "@(#)76	1.2  src/bos/usr/ccs/bin/structure/3.branch.c, cmdprog, bos411, 9428A410j 6/15/90 22:55:19";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: addlab, chkbranch, getbranch, nxtlab
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
#include "3.def.h"


getbranch(head)
VERT *head;
	{
	VERT v;
	for (v = 0; v < nodenum; ++v)
		LABEL(v) = FALSE;
	for (v = START; DEFINED(v); v = RSIB(v))
		chkbranch(v,head);
	addlab(START);
	}



chkbranch(v,head)
VERT v,*head;
	{
	VERT  w;
	int i;
	switch(NTYPE(v))
		{
		case GOVX:
				for (i = 1, w = head[v]; DEFINED(w); w = head[w], ++i)
					{
					if (i > 1 && !levnxt && !levbrk) break;
					if (ARC(v,0) == BRK(w) && (levbrk || i == 1))
						{
						NTYPE(v) = BRKVX;
						LEVEL(v) = i;
						break;
						}
					else if (ARC(v,0) == NXT(w) && (levnxt || i == 1))
						{
						NTYPE(v) = NXTVX;
						LEVEL(v) = i;
						break;
						}
					}
			if (NTYPE(v) == GOVX)
				{
				if (ARC(v,0) == stopvert)
					NTYPE(v) = STOPVX;
				else if (ARC(v,0) == retvert)
					NTYPE(v) = RETVX;
				else LABEL(ARC(v,0)) = TRUE;
				}
			break;
		case COMPVX:
		case ASGOVX:
			for (i = 0; i < ARCNUM(v); ++i)
				LABEL(ARC(v,i)) = TRUE;
			break;
		case IOVX:
				if (DEFINED(ARC(v,ENDEQ)))
					LABEL(ARC(v,ENDEQ)) = TRUE;
				if (DEFINED(ARC(v,ERREQ)))
					LABEL(ARC(v,ERREQ)) = TRUE;
				if (DEFINED(FMTREF(v)))
					LABEL(FMTREF(v)) = TRUE;
				break;
		}
	for (i = 0; i < CHILDNUM(v); ++i)
		for (w = LCHILD(v,i); DEFINED(w); w = RSIB(w))
			chkbranch(w,head);
	}


addlab(v)		/* add labels */
VERT v;
	{
	int recvar;
	if (NTYPE(v) != ITERVX && LABEL(v) )
		LABEL(v) = nxtlab();
	RECURSE(addlab,v,recvar);
	if (NTYPE(v) == ITERVX && LABEL(NXT(v)))
		LABEL(NXT(v)) = nxtlab();
	}


nxtlab()
	{
	static count;
	return(labinit + (count++) * labinc);
	}
