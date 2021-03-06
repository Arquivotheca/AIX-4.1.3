static char sccsid[] = "@(#)84	1.2  src/bos/usr/ccs/bin/structure/4.brace.c, cmdprog, bos411, 9428A410j 6/15/90 22:55:46";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: compound, ndbrace
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
#include "4.def.h"
#include "3.def.h"

ndbrace(v)			/* determine whether braces needed around subparts of v */
				/* return TRUE if v ends with IF THEN not in braces */
VERT v;
	{
	VERT w;
	int i;
	LOGICAL endif;
	endif = FALSE;
	for (i = 0; i < CHILDNUM(v); ++i)
		{
		endif = FALSE;
		for (w = LCHILD(v,i); DEFINED(w); w = RSIB(w))
			endif = ndbrace(w);
		if (NTYPE(v) != DUMVX && NTYPE(v) != ITERVX &&
			(!DEFINED(LCHILD(v,i)) || compound(v,i) ||
			(endif && NTYPE(v) == IFVX && !IFTHEN(v) && i == THEN )))
				/* DUMVX doesn't nest, ITERVX doen't nest since
					nesting is done at LOOPNODE, etc., must
					check for IFTHEN followed by unrelated ELSE */
			{
			YESBRACE(v,i);
			endif = FALSE;
			}
		}
	return(endif || IFTHEN(v) );
	}


compound(v,ch)		/* return TRUE iff subpart ch of v has multiple statements */
VERT v;
int ch;
	{
	VERT w;
	w = LCHILD(v,ch);
	if (!DEFINED(w))
		return(FALSE);
	if (NTYPE(w) == ITERVX)
		{
		ASSERT(DEFINED(NXT(w)),compound);
		if (LABEL(NXT(w)))
			return(TRUE);		/* loop ends with labeled CONTINUE statement */
		else
			return(compound(w,0));
		}
	else if (DEFINED(RSIB(w)))
		return(TRUE);
	else if (NTYPE(w) == STLNVX && CODELINES(w) > 1)
		return(TRUE);
	else
		return(FALSE);
	}
