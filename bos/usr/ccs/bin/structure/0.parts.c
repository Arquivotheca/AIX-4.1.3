static char sccsid[] = "@(#)54	1.3  src/bos/usr/ccs/bin/structure/0.parts.c, cmdprog, bos411, 9428A410j 6/15/90 22:53:56";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: arc, create, expres, lchild, level, negpart, predic, stlfmt,
	      vxpart
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

char *typename[TYPENUM]	= {"STLNVX",	"IFVX",		"DOVX",		"IOVX",	"FMTVX",
			"COMPVX",	"ASVX",		"ASGOVX",	"LOOPVX",	"WHIVX",
			"UNTVX",	"ITERVX",	"THENVX",	"STOPVX",	"RETVX",
			"DUMVX",	"GOVX",		"BRKVX",	"NXTVX",	"SWCHVX",
			"ACASVX",	"ICASVX"
	};
int hascom[TYPENUM]	= {2,		2,		2,		2,		2,
			2,		2,		2,		0,		0,
			0,		0,		2,		0,		0,
			0,		0,		0,		0,		2,
			2,		0
			};

int nonarcs[TYPENUM]  	= {FIXED+3,   	FIXED+4,	FIXED+2,	FIXED+3, 	FIXED+2,
			FIXED+2,	FIXED+2,	FIXED+2,	FIXED+1,	FIXED+1,
			FIXED+1,	FIXED+4,	FIXED+3,	FIXED,		FIXED,
			FIXED+2,	FIXED+1,	FIXED + 1,	FIXED + 1,	FIXED+3,
			FIXED+4,	FIXED+2
			};

int childper[TYPENUM]	= {0,	2,	1,	0,	0,
			0,	0,	0,	1,	1,
			1,	1,	1,	0,	0,
			1,	0,	0,	0,	1,
			2,	1
			};

int arcsper[TYPENUM]	= {1,		2,		2,	3,	0,
			-(FIXED+1),	1,	-(FIXED+1),	1,	1,
			1,		1,		2,	0,	0,
			-FIXED,		1,	1,		1,	-(FIXED+1),
			2,		1
			};

VERT *arc(v,i)
VERT v;
int i;
	{
	ASSERT(DEFINED(v),arc);
	ASSERT(0 <= i && i < ARCNUM(v), arc);
	return(&graph[v][nonarcs[NTYPE(v)] + i ]);
	}

VERT *lchild(v,i)
VERT v; int i;
	{
	ASSERT(DEFINED(v),lchild);
	ASSERT(0 <= i && i < childper[NTYPE(v)],lchild);
	return(&graph[v][nonarcs[NTYPE(v)]-i-1]);
	}

int *vxpart(v,type,j)
VERT v;
int type,j;
	{
	ASSERT((NTYPE(v) == type) && (0 <= j) && (j < nonarcs[type] - FIXED), vxpart);
	return(&graph[v][FIXED+j]);
	}

int *expres(v)
VERT v;
	{
	int ty;
	ty = NTYPE(v);
	ASSERT(ty == COMPVX || ty == ASGOVX || ty == ASVX || ty == SWCHVX || ty == ICASVX,expres);
	return(&graph[v][FIXED]);
	}

int *negpart(v)
VERT v;
	{
	ASSERT(NTYPE(v) == IFVX || NTYPE(v) == ACASVX,negpart);
	return(&graph[v][FIXED+1]);
	}

int *predic(v)
VERT v;
	{
	ASSERT(NTYPE(v) == IFVX || NTYPE(v) == ACASVX, predic);
	return(&graph[v][FIXED]);
	}

int *level(v)
VERT v;
	{
	ASSERT(NTYPE(v) == GOVX || NTYPE(v) == BRKVX || NTYPE(v) == NXTVX, level);
	return(&graph[v][FIXED]);
	}
int *stlfmt(v,n)
VERT v;
int n;
	{
	ASSERT(NTYPE(v) == STLNVX || NTYPE(v) == FMTVX,stlfmt);
	return(&graph[v][FIXED + n]);
	}

create(type,arcnum)
int type, arcnum;
	{
	int i, *temp, wds;
	if (nodenum >= maxnode)
		{
		maxnode += 100;
		temp=(int *) realloc(graph,maxnode*sizeof(*graph));
		free(graph);
		graph=(int **)temp;
		}
	wds = nonarcs[type] + arcnum;
	graph[nodenum] = (int *) galloc(sizeof(*graph) * wds);
	for (i = 0; i < wds; i++)  graph[nodenum][i] = 0;
	NTYPE(nodenum) = type;
	if (arcsper[type] < 0)
		ARCNUM(nodenum) = arcnum;
	
	return(nodenum++);
	}

