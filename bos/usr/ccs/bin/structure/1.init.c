static char sccsid[] = "@(#)62	1.3  src/bos/usr/ccs/bin/structure/1.init.c, cmdprog, bos411, 9428A410j 6/15/90 22:54:29";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: graf_init, line_init, prog_init, routini
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
#include "1.defs.h"
#include  "1.incl.h"
#include "def.h"


prog_init()
	{
	endline = endcom = 0;	endchar = -1;
	comchar = -1;
	graph = (int **) challoc(sizeof(*graph) * maxnode);
	}

routinit()
	{
	graf_init();
	progtype = !sub;
	routbeg = endline + 1;
	rtnbeg = endchar + 1;
	nameline = 0;
	stflag = UNDEFINED;
	}
line_init()
	{
	struct lablist *makelab();
	freelabs();
	newlab = linelabs = makelab(0L);
	flag = counter = nlabs = lswnum = swptr = p1 = 0;
	p3 = 5;
	endcom = endline;
	comchar = endchar;
	begline = endline + 1;	begchar = endchar + 1;
	reflab = endlab = errlab = 0;
	r1 = r2 = 0;
	}
graf_init()
	{
	int arctype[3];  long arclab[3];
	nodenum = 0;
	doptr = UNDEFINED;
	retvert = stopvert = UNDEFINED;
	ENTLST = FMTLST = 0;

	
	arctype[0] = -2;  arclab[0] = implicit;
	START = makenode(DUMVX,FALSE,FALSE,implicit,1,arctype,arclab);
	}

