static char sccsid[] = "@(#)65	1.2  src/bos/usr/ccs/bin/structure/1.node.c, cmdprog, bos411, 9428A410j 6/15/90 22:54:40";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: fiximp, makenode
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

makenode(type,addimp,addcom, labe,arcnum,arctype,arclab)
LOGICAL addimp,addcom;
int type, arctype[], arcnum;
long arclab[], labe;
	{
	int i;
	VERT num;
	
	ASSERT(arcsper[type] < 0 || arcnum == arcsper[type], makenode);
	num = create(type,arcnum);
	
	if (addimp)  fiximp(num,labe);
	
	for (i = 0; i < arcnum; ++i)
		{
		if (arctype[i] == -2)
			addref(arclab[i],&ARC(num,i));
		else
			ARC(num,i) = arctype[i];
		}
	
	
	if (hascom[type] )
		{
		if (!addcom || endcom < begline)
			BEGCOM(num) = UNDEFINED;
		else
			BEGCOM(num) = begchar - rtnbeg;
		}
	return(num);
	}





fiximp(num,labe)		/* fix implicit links, check nesting */
VERT num;
long labe;
	{
	fixvalue(implicit, num);		/* set implicit links to this node */
	clear(implicit);
	if(labe != implicit) fixvalue(labe, num);
	}
